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

Channel::Channel()
{
}

//======================================================================================================================

Channel::~Channel()
{
}

//======================================================================================================================


ChannelData* Channel::getChannelData()
{ 
	return &mChannelData; 
}

//======================================================================================================================
	
uint32 Channel::getId() const
{ 
	return mChannelData.id; 
}

//======================================================================================================================
	
void Channel::setId(uint32 id)
{ 
	mChannelData.id = id; 
}

//======================================================================================================================
	
string Channel::getName() const
{ 
	return mChannelData.name; 
}

//======================================================================================================================
	
void Channel::setName(const string name)
{ 
	mChannelData.name = name; 
}

//======================================================================================================================
	
string Channel::getGalaxy() const
{ 
	return mGalaxy;
}

//======================================================================================================================
	
void Channel::setGalaxy(const string galaxy)
{ 
	mGalaxy = galaxy; 
}

//======================================================================================================================

string Channel::getTitle() const
{ 
	string title = mChannelData.title;
	title.convert(BSTRType_Unicode16);
	return title; 
}

//======================================================================================================================
	
void Channel::setTitle(const string title)
{ 
	mChannelData.title = title; 
}

//======================================================================================================================

ChatAvatarId* Channel::getOwner() const
{ 
	return mOwner; 
}

//======================================================================================================================
	
void Channel::setOwner(ChatAvatarId* owner)
{ 
	mOwner = owner; 
}

//======================================================================================================================
	
ChatAvatarId* Channel::getCreator() const
{ 
	return mCreator; 
}

//======================================================================================================================
	
void Channel::setCreator(ChatAvatarId* creator)
{ 
	mCreator = creator; 
}

//======================================================================================================================
	
uint8 Channel::isPrivate() const
{
	return mChannelData.is_private; 
}

//======================================================================================================================
	
void Channel::setPrivate(uint8 priv)
{
	mChannelData.is_private = priv; 
}

//======================================================================================================================

uint8 Channel::isModerated() const
{ 
	return mChannelData.is_moderated;
}

//======================================================================================================================
	
void Channel::setModerated(uint8 moderated)
{
	mChannelData.is_moderated = moderated;
}

//======================================================================================================================

ChatAvatarIdList* Channel::getUserList()
{
	return &mUsers;
}

//======================================================================================================================
	
NameByCrcMap* Channel::getModeratorList()
{
	return &mModerators;
}

//======================================================================================================================
	
NameByCrcMap* Channel::getBanned()
{ 
	return &mBanned; 
}

//======================================================================================================================
	
NameByCrcMap* Channel::getInvited()
{ 
	return &mInvited;
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
		gLogger->log(LogManager::DEBUG,"Channel::remove user : removing player from channel user map : %s", player->getName().getAnsi());
	}
	else
		gLogger->log(LogManager::DEBUG,"Channel::remove user : Can't find player on channel user map : %s",  player->getName().getAnsi());
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

bool Channel::isModerator(string name) const
{
	// name.toLower(); // Always handle search-able names (data) as lowercase. 
	NameByCrcMap::const_iterator iter = mModerators.find(name.getCrc());
	return iter != mModerators.end();
}

//======================================================================================================================

bool Channel::isOwner(string name) const
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

bool Channel::isInvited(string name) const
{
	// name.toLower(); // Always handle search-able names (data) as lowercase. 
	NameByCrcMap::const_iterator iter = mInvited.find(name.getCrc());
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

bool Channel::isBanned(string name) const
{
	NameByCrcMap::const_iterator iter = mBanned.find(name.getCrc());
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

string Channel::getFullPath() const
{
	string path = "SWG.";
	path << getGalaxy().getAnsi() << ".";
	path << getName().getAnsi();
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
