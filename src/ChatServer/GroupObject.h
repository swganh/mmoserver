/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006, 2007 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_CHATSERVER_GROUPOBJECT_H
#define ANH_CHATSERVER_GROUPOBJECT_H

#include <vector>
#include "Player.h"
#include "Channel.h"
#include "Utils/typedefs.h"

//======================================================================================================================

typedef std::vector<Player*>		PlayerList;

//======================================================================================================================

class GroupObject
{

public :

	GroupObject(Player* leader, uint64 groupId);
	~GroupObject();

	Player*				getLeader(){ return mMembers[0]; }
	PlayerList*			getMembers() { return &mMembers; }
	uint64				getId(){ return mId; }
	uint16				getMemberCount(){ return mMemberCount;}
	uint32				getLootMode(){ return mLootMode; }
	uint64				getMasterLooter() { return mMasterLooter; }

	uint16				getSize() { return mMembers.size(); }
	Channel*			getChannel(){ return mChannel; }

	void				addTempMember(Player* player);
	void				removeTempMember(Player* player);
	void				addMember(Player* player);
	void				removeMember(Player* player);
	void				changeLeader(Player* player);
	void				changeLootMode(uint32 newLootMode);
	void				changeMasterLooter(Player* newMasterLooter);

	void				disband();

	void				resetIndexes();
	void				sendCreate(Player* player);
	void				createChannel();
	void				sendUpdate(Player* player);			// Used when entering a new location (new scene)

	void				broadcastMessage(Message* message, bool ignoreLeader = false);
	void				broadcastPlayerPositionNotification(Player* player);
	void				broadcastPlayerZoneNotification(Player* player);
	void				sendGroupLocationNotification(Player* player);

	void				sendBaseline(Player* player);
	void				broadcastDeltaResetAll();
	void				broadcastDeltaAdd(Player* player);
	void				broadcastDeltaRemove(Player* player);
	void				broadcastUpdateLootMode();
	void				broadcastUpdateMasterLooter();
	



private :

	uint64				mId;
	PlayerList			mMembers;
	uint16				mMemberCount;
	uint32				mMembersUpdateCount;
	uint64				mMasterLooter;
	uint32				mLootMode;
	Channel*			mChannel;

};
#endif
//======================================================================================================================
