/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Channel.h"
#include "ChatManager.h"
#include "ChatAvatarId.h"
#include "Player.h"

#include "LogManager/LogManager.h"

#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include <cstring>

//======================================================================================================================

Channel::Channel() :
mPrivate(0),
mModerated(0)
{
}

//======================================================================================================================

Channel::~Channel()
{
}

//======================================================================================================================

void Channel::addUser(ChatAvatarId *avatar)
{
	mUsers.push_back(avatar);
	string key = avatar->getLoweredName();
	// chat avatar id should be lowercase already avatar->getLoweredName().toLower();
	//strcpy(key, avatar->getLoweredName().getAnsi());
	mUserMap.insert(std::make_pair(key.getCrc(), avatar));
}

//======================================================================================================================

ChatAvatarId* Channel::findUser(string name)
{
	// string strname = name.getAnsi();		// Don't compare lowercase with mix of upper-lower.
	// strname.toLower();
	name.toLower();

	AvatarNameMap::iterator iter = mUserMap.find(name.getCrc());

	if (iter != mUserMap.end())
		return (*iter).second;
	else
		return NULL;
}
//======================================================================================================================

ChatAvatarId* Channel::findUser(Player* player)
{

	AvatarNameMap::iterator iter = mUserMap.find(player->getKey());

	if (iter != mUserMap.end())
		return (*iter).second;
	else
		return NULL;
}

//======================================================================================================================

void Channel::removeUser(Player* player)
{
	ChatAvatarIdList::iterator iter = mUsers.begin();
	while (iter != mUsers.end())
	{
		if ((*iter)->getPlayer() == player)
		{
			mUsers.erase(iter);
			break;
		}
		++iter;
	}

	AvatarNameMap::iterator mapiter = mUserMap.find(player->getKey());
	if(mapiter != mUserMap.end())
	{
		mUserMap.erase(mapiter);
		gLogger->logMsgF("Channel::remove user : removing player from channel user map : %s",MSG_NORMAL, player->getName().getAnsi());
	}
	else
		gLogger->logMsgF("Channel::remove user : Can't find player on channel user map : %s",MSG_NORMAL,  player->getName().getAnsi());
}

//======================================================================================================================

ChatAvatarId* Channel::removeUser(string name)
{
	ChatAvatarIdList::iterator iter = mUsers.begin();
	while (iter != mUsers.end())
	{
		if (strcmp((*iter)->getLoweredName().getRawData(), name.getRawData()) == 0)
		{
			mUsers.erase(iter);
			break;
		}
		++iter;
	}

	name.toLower();
	AvatarNameMap::iterator mapiter = mUserMap.find(name.getCrc());

	ChatAvatarId* ret = NULL;
	if (mapiter != mUserMap.end())
	{
		ret = (*mapiter).second;
		mUserMap.erase(mapiter);
		return ret;
	}

	return NULL;
}


//======================================================================================================================

void Channel::addModerator(string* name)
{
	string* modName = new string (name->getAnsi());
	modName->toLower();
	mModerators.insert(CrcStringPair(modName->getCrc(), modName));
}

//======================================================================================================================

string* Channel::removeModerator(string name)
{
	NameByCrcMap::iterator iter = mModerators.find(name.getCrc());
	string* ret = NULL;
	if (iter != mModerators.end())
	{
		ret = (*iter).second;
		mModerators.erase(iter);
	}
	
	return ret;
}

//======================================================================================================================

bool Channel::isModerator(string name)
{
	// name.toLower(); // Always handle search-able names (data) as lowercase. 
	NameByCrcMap::iterator iter = mModerators.find(name.getCrc());
	return iter != mModerators.end();
}

//======================================================================================================================

bool Channel::isOwner(string name)
{
	return (strcmp(name.getAnsi(), mOwner->getLoweredName().getAnsi()) == 0);
}

//======================================================================================================================

string* Channel::removeInvitedUser(string name)
{
	NameByCrcMap::iterator iter = mInvited.find(name.getCrc());
	string* ret = NULL;
	if (iter != mInvited.end())
	{
		ret = (*iter).second;
		mInvited.erase(iter);
	}

	return ret;
}

//======================================================================================================================

bool Channel::isInvited(string name)
{
	// name.toLower(); // Always handle search-able names (data) as lowercase. 
	NameByCrcMap::iterator iter = mInvited.find(name.getCrc());
	return iter != mInvited.end();
}

//======================================================================================================================

string* Channel::unBanUser(string name)
{
	NameByCrcMap::iterator iter = mBanned.find(name.getCrc());
	string* ret = NULL;
	if (iter != mBanned.end())
	{
		ret = (*iter).second;
		mBanned.erase(iter);
	}

	return ret;
}

//======================================================================================================================

void Channel::banUser(string* name)
{
	string* banName = new string (name->getAnsi());
	banName->toLower();
	mBanned.insert(CrcStringPair(banName->getCrc(), banName));
}

//======================================================================================================================

bool Channel::isBanned(string name)
{
	NameByCrcMap::iterator iter = mBanned.find(name.getCrc());
	return iter != mBanned.end();
}

//======================================================================================================================

void Channel::addInvitedUser(string* name)
{
	string* invName = new string (name->getAnsi());
	invName->toLower();
	mInvited.insert(CrcStringPair(invName->getCrc(), invName));
}

//======================================================================================================================

string Channel::getFullPath()
{
	string path = "SWG.";
	path << mGalaxy.getAnsi() << ".";
	path << mName.getAnsi();
	return path;
}

//======================================================================================================================

void Channel::clearChannel()
{
	ChatAvatarIdList::iterator iter = mUsers.begin();
	while (!mUsers.empty())
	{
		delete (*iter);
		mUsers.erase(iter);
		iter = mUsers.begin();
	}

	NameByCrcMap::iterator seconditer = mModerators.begin();
	while (!mModerators.empty())
	{
		delete (*seconditer).second;
		mModerators.erase(seconditer);
		seconditer = mModerators.begin();
	}

	seconditer = mInvited.begin();
	while (!mInvited.empty())
	{
		delete (*seconditer).second;
		mInvited.erase(seconditer);
		seconditer = mInvited.begin();
	}

	seconditer = mBanned.begin();
	while (!mBanned.empty())
	{
		delete (*seconditer).second;
		mBanned.erase(seconditer);
		seconditer = mBanned.begin();
	}
}