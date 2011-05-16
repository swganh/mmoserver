/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_ACTIVE_CONVERSATION_H
#define ANH_ZONESERVER_ACTIVE_CONVERSATION_H

#include <vector>

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "Conversation.h"

//=============================================================================

class NPCObject;
class PlayerObject;

//=============================================================================

class ActiveConversation
{
public:

    ActiveConversation(Conversation* baseConv,PlayerObject* player,NPCObject* npc);
    ~ActiveConversation();

    ConversationPage*		getCurrentPage() {
        return mCurrentPage;
    }
    void					setCurrentPage(uint32 pageLink);
    void					prepareFilteredOptions();

    void					updateCurrentPage(uint32 selectId);
    bool					preProcessConversation();
    void					postProcessCurrentPage();


    ConversationOptions*	getFilteredOptions() {
        return &mSelectOptionMap;
    }

    NPCObject*				getNpc() {
        return mNpc;
    }

    int32					getDI() {
        return mDI;
    }
    BString					getTTStfFile() {
        return mTTStfFile;
    }
    BString					getTTStfVariable() {
        return mTTStfVariable;
    }
    BString					getTTCustom() {
        return mTTCustom;
    }
    uint64					getTTId() {
        return mTTId;
    }
    BString					getTOStfFile() {
        return mTOStfFile;
    }
    BString					getTOStfVariable() {
        return mTOStfVariable;
    }
    BString					getTOCustom() {
        return mTOCustom;
    }
    uint64					getTOId() {
        return mTOId;
    }

private:

    // void				_prepareFilteredOptions();

    Conversation*		mBaseConversation;
    PlayerObject*		mPlayer;
    NPCObject*			mNpc;

    ConversationPage*	mCurrentPage;

    ConversationOptions	mSelectOptionMap;

    int32				mDI;
    BString				mTTStfFile;
    BString				mTTStfVariable;
    BString				mTTCustom;
    uint64				mTTId;
    BString				mTOStfFile;
    BString				mTOStfVariable;
    BString				mTOCustom;
    uint64				mTOId;
};

//=============================================================================

#endif

