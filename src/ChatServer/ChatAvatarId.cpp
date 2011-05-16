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
 
#include "ChatAvatarId.h"
#include "ChatManager.h"
#include "Player.h"

// Fix for issues with glog redefining this constant
#ifdef _WIN32
#undef ERROR
#endif

#include <glog/logging.h>

#include "Utils/typedefs.h"

//======================================================================================================================

bool				ChatSystemAvatar::mInsFlag = false;
ChatSystemAvatar*	ChatSystemAvatar::mSingleton = NULL;

//======================================================================================================================

void ChatAvatarId::setPlayer(Player* player)
{
    mPlayer = player;
    //string lcName = BString(BSTRType_ANSI, player->getName().getLength());
    //memcpy(lcName.getRawData(), player->getName().getRawData(), lcName.getLength());
    //lcName.toLower();
    mName = player->getName();
    mName.toLower();
    DLOG(INFO) << "Chatavatar:: setplayer " << mName.getAnsi();
}

//======================================================================================================================

void ChatAvatarId::setPlayer(const BString player)
{
	DLOG(INFO) << "Chatavatar:: setplayer " << mName.getAnsi() << " string only";
    mPlayer = gChatManager->getPlayerByName(player);

    mName = player;
    mName.toLower();
}

//======================================================================================================================

BString ChatAvatarId::getPath()
{
    BString path = "SWG.";
    path << mGalaxy.getAnsi() << ".";
    path << mName.getAnsi();
    return path;
}

//======================================================================================================================

ChatSystemAvatar* ChatSystemAvatar::GetSingleton()
{
    if (!mInsFlag)
    {
        mSingleton = new ChatSystemAvatar();
        mInsFlag = true;
    }
    return mSingleton;
}

//======================================================================================================================

BString ChatSystemAvatar::getLoweredName()
{
    return BString("SYSTEM");
}
