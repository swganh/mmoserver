/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ChatAvatarId.h"
#include "ChatManager.h"
#include "Utils/typedefs.h"
#include "LogManager/LogManager.h"

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
	gLogger->logMsgF("Chatavatar:: setplayer %s", MSG_NORMAL, mName.getAnsi());
}

//======================================================================================================================

void ChatAvatarId::setPlayer(const string player)
{
	gLogger->logMsgF("Chatavatar:: setplayer %s string only", MSG_NORMAL, mName.getAnsi());
	mPlayer = gChatManager->getPlayerByName(player); 
	
	mName = player;
	mName.toLower();
}

//======================================================================================================================

string ChatAvatarId::getPath()
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

string ChatSystemAvatar::getLoweredName()
{
	return BString("SYSTEM");
}