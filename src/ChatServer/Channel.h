/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_CHANNEL_H
#define ANH_CHATSERVER_CHANNEL_H

#include <map>
#include <vector>

#include "Utils/typedefs.h"

class BString;
class ChatAvatarId;
class Player;

//======================================================================================================================

typedef std::map<uint32,ChatAvatarId*>  AvatarNameMap;
typedef std::vector<ChatAvatarId*>      ChatAvatarIdList;
typedef std::pair<uint32, BString*>     CrcStringPair;
typedef std::map<uint32, BString*>      NameByCrcMap;

//======================================================================================================================

struct ChannelData
{
	ChannelData()
		: is_private(0)
		, is_moderated(0)
	{}

	string	name;
	string	title;
	uint32	id;
	uint8		is_private;
	uint8		is_moderated;
};

//======================================================================================================================

class Channel
{
public:
	Channel();
	~Channel();

	ChannelData* getChannelData();

	uint32 getId() const;
	void setId(uint32 id);

	string getName() const;
	void setName(const string name);

	string getFullPath() const;

	string getGalaxy() const;
	void setGalaxy(const string galaxy);

	string getTitle() const;
	void setTitle(const string title);

	ChatAvatarId* getOwner() const;
	void setOwner(ChatAvatarId* owner);

	ChatAvatarId* getCreator() const;
	void setCreator(ChatAvatarId* creator);

	uint8 isPrivate() const;
	void setPrivate(uint8 priv);

	uint8 isModerated() const;
	void setModerated(uint8 moderated);

	void addUser(ChatAvatarId* avatar);
	void removeUser(Player* player);

	ChatAvatarId*	removeUser(string name);
	ChatAvatarId*	findUser(string name);
	ChatAvatarId*	findUser(Player* player);

	void addInvitedUser(string* name);

	string* removeInvitedUser(string name);
	bool isInvited(string name) const;

	void addModerator(string* name);
	string* removeModerator(string name);
	bool isModerator(string name) const;
	bool isOwner(string name) const;

	void banUser(string* name);
	string* unBanUser(string name);
	bool isBanned(string name) const;

	void clearChannel();

	ChatAvatarIdList* getUserList();
	NameByCrcMap* getModeratorList();
	NameByCrcMap* getBanned();
	NameByCrcMap* getInvited();

private:
	ChannelData				mChannelData;
	ChatAvatarId*			mOwner;
	ChatAvatarId*			mCreator;
	string						mGalaxy;
	ChatAvatarIdList	mUsers;
	AvatarNameMap			mUserMap;
	NameByCrcMap			mModerators;
	NameByCrcMap			mBanned;
	NameByCrcMap			mInvited;
};

#endif

