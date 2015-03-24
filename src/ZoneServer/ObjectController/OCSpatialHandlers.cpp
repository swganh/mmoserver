/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "ZoneServer/Objects/BankTerminal.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>

#include <boost/regex.hpp>
<<<<<<< HEAD:src/ZoneServer/ObjectController/OCSpatialHandlers.cpp

#include "anh/logger.h"

//#include "ZoneServer/Objects/CraftingTool.h"

#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/ObjectController/ObjectControllerOpcodes.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"

#include "ZoneServer/WorldConfig.h"
=======
#endif

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
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/OCSpatialHandlers.cpp

#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include <boost/lexical_cast.hpp>

<<<<<<< HEAD:src/ZoneServer/ObjectController/OCSpatialHandlers.cpp
using boost::regex;
using boost::smatch;
=======
#ifdef WIN32
#undef ERROR
#endif

#ifdef WIN32
#undef ERROR
#endif

#ifdef WIN32
using std::wregex;
using std::wsmatch;
using std::regex_match;
#else
using boost::wregex;
using boost::wsmatch;
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/OCSpatialHandlers.cpp
using boost::regex_match;

//=============================================================================
//
// chat
//
void ObjectController::_handleSpatialChatInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* player = creature->GetGhost();
	
    // Get the u16string and convert it to wstring for processing.
    std::u16string chat_data = message->getStringUnicode16();
    std::wstring tmp(chat_data.begin(), chat_data.end());

    // This regular expression searches for 5 numbers separated by spaces
    // followed by a string text message.
	const boost::wregex p(L"(\\d+) (\\d+) (\\d+) (\\d+) (\\d+) (.*)");
    boost::wsmatch m;

<<<<<<< HEAD:src/ZoneServer/ObjectController/OCSpatialHandlers.cpp
<<<<<<< HEAD:src/ZoneServer/ObjectController/OCSpatialHandlers.cpp
    if (! boost::regex_match(tmp, m, p)) {
        LOG(error) << "Invalid spatial chat message format";
=======
    if (! regex_match(tmp, m, p)) {
        LOG(ERROR) << "Invalid spatial chat message format";
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/OCSpatialHandlers.cpp
=======
    if (! regex_match(tmp, m, p)) {
        LOG(ERROR) << "Invalid spatial chat message format";
>>>>>>> parent of 5bd772a... got rid of google log:src/ZoneServer/OCSpatialHandlers.cpp
        return; // We suffered an unrecoverable error, bail out now.
    }
   
    gMessageLib->SendSpatialChat(player->GetCreature(), 
        m[6].str().substr(0, 256), // This is the text message
        (gWorldConfig->isInstance()) ? player : nullptr, // If it's an instance we send the player object
        std::stoull(m[1].str()), // Convert this item to a uint64_t character id
        0x32, // Always show spatial chat in the text box.
        static_cast<SocialChatType>(std::stoi(m[2].str())), 
        static_cast<MoodType>(std::stoi(m[3].str())));
}

//=============================================================================
//
// emote
//

void ObjectController::_handleSocialInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* playerObject = creature->GetGhost();
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

    gMessageLib->SendSpatialEmote(playerObject->GetCreature(), emoteId, emoteTarget, sendType);
}

//=============================================================================
//
// mood
//

void ObjectController::_handleSetMoodInternal(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    // FIXME: for now assume only players send chat
    CreatureObject* creature  = dynamic_cast<CreatureObject*>(mObject); PlayerObject* playerObject = creature->GetGhost();
    BString			moodStr;
    int8			sql[256];

    message->getStringUnicode16(moodStr);
    moodStr.convert(BSTRType_ANSI);
    uint32 mood = boost::lexical_cast<uint32>(moodStr.getAnsi());

    playerObject->GetCreature()->setMoodId(static_cast<uint8>(mood));

    gMessageLib->sendMoodUpdate(playerObject->GetCreature());

    ObjControllerAsyncContainer* asyncContainer = new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_Nope);
    sprintf(sql,"UPDATE %s.character_attributes SET moodId = %u where character_id = %"PRIu64"",mDatabase->galaxy(),mood,playerObject->getId());

    mDatabase->executeSqlAsync(this,asyncContainer,sql);

}

//=============================================================================


