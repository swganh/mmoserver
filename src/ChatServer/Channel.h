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
};

//======================================================================================================================

class Channel
{
	friend class ChatManager;

	public:

		Channel();
		~Channel();

		uint32				getId(){ return mId; }
		void				setId(uint32 id){ mId = id; }

		string				getName(){ return mName; }
		void				setName(const string name){ mName = name; }

		string				getFullPath();

		string				getGalaxy() { return mGalaxy; }
		void				setGalaxy(const string galaxy){ mGalaxy = galaxy; }

		string				getTitle(){ mTitle.convert(BSTRType_Unicode16); return mTitle; }
		void				setTitle(const string title){ mTitle = title; }

		ChatAvatarId*		getOwner(){ return mOwner; }
		void				setOwner(ChatAvatarId* owner){ mOwner = owner; }

		ChatAvatarId*		getCreator(){ return mCreator; }
		void				setCreator(ChatAvatarId* creator){ mCreator = creator; }

		uint8				isPrivate(){ return mPrivate; }
		void				setPrivate(uint8 priv){ mPrivate = priv; }

		uint8				isModerated(){ return mModerated; }
		void				setModerated(uint8 moderated){ mModerated = moderated; }

		void				addUser(ChatAvatarId* avatar);
		void				removeUser(Player* player);
		ChatAvatarId*		removeUser(string name);
		ChatAvatarId*		findUser(string name);
		ChatAvatarId*		findUser(Player* player);

		void				addInvitedUser(string* name);
		
		string*				removeInvitedUser(string name);
		bool				isInvited(string name);

		void				addModerator(string* name);
		string*				removeModerator(string name);
		bool				isModerator(string name);
		bool				isOwner(string name);

		void				banUser(string* name);
		string*				unBanUser(string name);
		bool				isBanned(string name);

		void				clearChannel();

		ChatAvatarIdList*	getUserList(){ return &mUsers; }
		NameByCrcMap*		getModeratorList() { return &mModerators; }
		NameByCrcMap*		getBanned() { return &mBanned; }
		NameByCrcMap*		getInvited() { return &mInvited; }

	private:

		uint32				mId;
		string				mName;
		ChatAvatarId*		mOwner;
		ChatAvatarId*		mCreator;
		string				mTitle;
		string				mGalaxy;
		uint8				mPrivate;
		uint8				mModerated;

		ChatAvatarIdList	mUsers;
		AvatarNameMap		mUserMap;
		NameByCrcMap		mModerators;
		NameByCrcMap		mBanned;
		NameByCrcMap		mInvited;
};

#endif

