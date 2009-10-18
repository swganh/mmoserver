/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_PLAYER_H
#define ANH_CHATSERVER_PLAYER_H

#include "Utils/typedefs.h"
#include <map>

class DispatchClient;

//======================================================================================================================

typedef std::map<uint32,string> ContactMap;

//======================================================================================================================
class Bazaar
{
public:
	Bazaar () {}      
	~Bazaar () {}   
	
		uint64			ownerid;
		uint64			id;
		uint32			regionid;
		uint32			planetid;
		int8			string[128];
		int8			planet[32];
		int8			region[48];

};

struct PlayerData
{
		string name;
		string last_name;
};

class Player
{
	public:

		Player(uint64 charId,DispatchClient* client,uint32 planetId) 
			: mCharId(charId)
			, mClient(client)
			, mPlanetId(planetId)
			, mAddPending(true)
			, mKey(NULL)
			, mGroupMemberIndex(0)
			, mGroupId(0)
			, mX(0)
			, mZ(0)
			,mBazaar(NULL) 
		{}

		~Player()
		{}

		uint64			getCharId(){ return mCharId; }
		void			setCharId(uint64 charId){ mCharId = charId; }

		string			getName(){ return mPlayerData.name; }
		void			setName(const string name){ mPlayerData.name = name; }

		PlayerData* getPlayerData() { return &mPlayerData; }

		string			getLastName(){ return mPlayerData.last_name; }
		void			setLastName(const string lastName){ mPlayerData.last_name = lastName; }

		DispatchClient*	getClient(){ return mClient; }
		void			setClient(DispatchClient* client){ mClient = client; }

		uint32			getPlanetId(){ return mPlanetId; }
		void			setPlanetId(uint32 planetId){ mPlanetId = planetId; }

		bool			getAddPending(){ return mAddPending; }
		void			setAddPending(bool b){ mAddPending = b; }

		uint32			getKey(){ return mKey; }
		void			setKey() { string name = getName(); name.toLower(); mKey = name.getCrc(); }

		ContactMap*		getFriendsList(){ return &mFriendsList; }
		void			addFriend(string name){ mFriendsList.insert(std::make_pair(name.getCrc(),name.getAnsi())); }
		void			removeFriend(uint32 nameCrc);
		bool			checkFriend(uint32 nameCrc);
		
		ContactMap*		getIgnoreList(){ return &mIgnoreList; }
		void			addIgnore(string name){ mIgnoreList.insert(std::make_pair(name.getCrc(),name.getAnsi())); }
		void			removeIgnore(uint32 nameCrc);
		bool			checkIgnore(uint32 nameCrc);

		uint16			getGroupMemberIndex(){ return mGroupMemberIndex; }
		void			setGroupMemberIndex(uint16 index){ mGroupMemberIndex = index; }

		uint64			getGroupId(){ return mGroupId; }
		void			setGroupId(uint64 groupId){ mGroupId = groupId; }

		void			setPositionX(float position) {mX = position; }
		float			getPositionX() {return mX;}

		void			setBazaar(Bazaar* bazaar) {mBazaar = bazaar; }
		Bazaar*			getBazaar() {return mBazaar;}

		void			setPositionZ(float position) {mZ = position; }
		float			getPositionZ() {return mZ;}

	private:
		PlayerData  mPlayerData;
		uint32			mPlanetId;
		uint64			mCharId;
		DispatchClient*	mClient;
		bool			mAddPending;
		uint32			mKey;
		ContactMap		mFriendsList,mIgnoreList;
		uint16			mGroupMemberIndex;
		uint64			mGroupId;
		Bazaar*		    mBazaar;

		float			mX;
		float			mZ;
};

#endif

