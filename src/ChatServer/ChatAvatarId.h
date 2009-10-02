/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATAVATARID_CHANNEL_H
#define ANH_CHATAVATARID_CHANNEL_H

#include "Utils/typedefs.h"

class ChatSystemAvatar;
class Player;

#define gSystemAvatar ChatSystemAvatar::GetSingleton()

//======================================================================================================================

class ChatAvatarId
{
public:

	ChatAvatarId(){};
	~ChatAvatarId(){};

	string			getGalaxy() { return mGalaxy; }
	void			setGalaxy(const string name) { mGalaxy = name; }

	Player*			getPlayer() { return mPlayer; }
	void			setPlayer(Player* player);
	void			setPlayer(const string player);

	virtual string	getLoweredName() { return mName; }

	string	getPath();

protected:
	string		mGalaxy;
	string		mName;
	Player*		mPlayer;
};

//======================================================================================================================

class ChatSystemAvatar : public ChatAvatarId
{
public: 

	~ChatSystemAvatar(){}
	static ChatSystemAvatar* GetSingleton();

	string getLoweredName();

private:

	ChatSystemAvatar() : ChatAvatarId() {};

	static bool					mInsFlag;
	static ChatSystemAvatar*	mSingleton;
};

#endif