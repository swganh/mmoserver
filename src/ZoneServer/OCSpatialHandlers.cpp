/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

#include <glog/logging.h>

#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "PlayerObject.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include <boost/lexical_cast.hpp>

#ifdef WIN32
#undef ERROR
#endif

//=============================================================================
//
// chat
//
void ObjectController::_handleSpatialChatInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

    // Get the unicode data and convert it to ansii, then get the raw data.
    std::u16string chat_data = message->getStringUnicode16();

    std::vector<std::u16string> tmp;
    std::vector<uint64_t> chat_elements;
    int elements_size = 0;

    // The spatial chat data is all in a ustring. This consists of 5 chat elements
    // and the text of the spatial chat. The 5 chat elements are integers that are
    // sent as strings so here we use an istream_iterator which splits the strings
    // at spaces and then we use a transform to converts them to uint64_t's.
    std::basic_istringstream<char16_t> iss(chat_data);
    std::copy_n(std::istream_iterator<std::u16string, char16_t, std::char_traits<char16_t>>(iss), 5,
              std::back_inserter<std::vector<std::u16string>>(tmp));

    try {
        std::transform(tmp.begin(), tmp.end(), std::back_inserter<std::vector<uint64_t>>(chat_elements),
            [&elements_size] (std::u16string s) -> uint64_t {
                // Convert the element to a uint64_t
                uint64_t output = boost::lexical_cast<uint64_t>(std::string(s.begin(), s.end())); 

                // After successful conversion update we need to store how long
                // the string was (plus 1 for the space delimiter that came after it).
                elements_size += s.size() + 1; 

                return output;
            });
    } catch(const boost::bad_lexical_cast& e) {
        LOG(ERROR) << e.what();
        return; // We suffered an unrecoverable error, bail out now.
    }
    
    uint64_t chat_target_id     = chat_elements[0];
    SocialChatType chat_type_id = static_cast<SocialChatType>(chat_elements[1]);
    MoodType mood_id            = static_cast<MoodType>(chat_elements[2]);

    // After pulling out the chat elements store the rest of the data as the spatial text body.
    std::wstring spatial_text(chat_data.begin()+elements_size, chat_data.end());
        
    if (!gWorldConfig->isInstance()) {
        gMessageLib->SendSpatialChat(playerObject, spatial_text, NULL, chat_target_id, 0x32, chat_type_id, mood_id);
    } else {
        gMessageLib->SendSpatialChat(playerObject, spatial_text, playerObject, chat_target_id, 0x32, chat_type_id, mood_id);
    }
}

//=============================================================================
//
// emote
//

void ObjectController::_handleSocialInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
    BString			emoteData;
    BStringVector	emoteElement;

    message->getStringUnicode16(emoteData);

    // Have to convert BEFORE using split, since the conversion done there is removed It will assert().. evil grin...
    // Either do the conversion HERE, or better fix the split so it handles unicoe also.
    emoteData.convert(BSTRType_ANSI);
    emoteData.split(emoteElement,' ');

    uint64_t emoteTarget = boost::lexical_cast<uint64>(emoteElement[0].getAnsi());
    uint32_t emoteId     = atoi(emoteElement[1].getAnsi());

    // Social emotes can have one of 3 types:
    // 1 - Performs an animation
    // 2 - Sends a text message
    // 3 - Both
    uint8_t sendType = 1;

    // if set, send text along with animation
    if(atoi(emoteElement[3].getAnsi()) == 1) {
        // if the player is mounted (or perhaps other states to, such as sitting) only the
        // text should be shown. Otherwise display both text and the animation.
        if (playerObject->checkIfMounted()) {
            sendType = 2;
        } else {
            sendType = 3;
        }
    }

    gMessageLib->SendSpatialEmote(playerObject, emoteId, emoteTarget, sendType);
}

//=============================================================================
//
// mood
//

void ObjectController::_handleSetMoodInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
    BString			moodStr;
    int8			sql[256];

    message->getStringUnicode16(moodStr);
    moodStr.convert(BSTRType_ANSI);
    uint32 mood = boost::lexical_cast<uint32>(moodStr.getAnsi());

    playerObject->setMoodId(static_cast<uint8>(mood));

    gMessageLib->sendMoodUpdate(playerObject);

    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_Nope);
    sprintf(sql,"UPDATE swganh.character_attributes SET moodId = %u where character_id = %"PRIu64"",mood,playerObject->getId());

    mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}

//=============================================================================


