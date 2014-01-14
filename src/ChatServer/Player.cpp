/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Player.h"

//======================================================================================================================

void Player::removeFriend(uint32 nameCrc)
{
	ContactMap::iterator it = mFriendsList.find(nameCrc);

	if(it != mFriendsList.end())
	{
		mFriendsList.erase(it);
		return;
	}
}

//======================================================================================================================

bool Player::checkFriend(uint32 nameCrc)
{
	ContactMap::iterator it = mFriendsList.find(nameCrc);

	if(it != mFriendsList.end())
	{
		return(true);
	}

	return(false);
}

//======================================================================================================================

void Player::removeIgnore(uint32 nameCrc)
{
	ContactMap::iterator it = mIgnoreList.find(nameCrc);

	if(it != mIgnoreList.end())
	{
		mIgnoreList.erase(it);
		return;
	}
}

//======================================================================================================================

bool Player::checkIgnore(uint32 nameCrc)
{
	ContactMap::iterator it = mIgnoreList.find(nameCrc);

	if(it != mIgnoreList.end())
	{
		return(true);
	}

	return(false);
}

//======================================================================================================================



