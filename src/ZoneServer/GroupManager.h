/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_GroupManager_H
#define ANH_ZONESERVER_GroupManager_H

#include <list>
#include <map>

#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "GroupManagerCallback.h"
#include "Utils/typedefs.h"

//======================================================================================================================

#define	gGroupManager	GroupManager::getSingletonPtr()

//======================================================================================================================

class Database;
class DispatchClient;
class GroupObject;
class Message;
class MessageDispatch;
class MissionObject;
class PlayerObject;
class GroupManagerCallbackContainer;

typedef std::vector<std::pair<MissionObject*,uint32> >	MissionGroupRangeList;
typedef std::vector<GroupObject*>						GroupList;
//======================================================================================================================

class GroupManager : public MessageDispatchCallback, public DatabaseCallback
{
	public:

		static GroupManager*	getSingletonPtr() { return mSingleton; }
		static GroupManager*	Init(Database* database,MessageDispatch* dispatch);

		GroupManager(Database* database,MessageDispatch* dispatch);
		~GroupManager();

		static inline void deleteManager(void)    
		{ 
			if (mSingleton)
			{
				delete mSingleton;
				mSingleton = 0;
			}
		}

		void				Shutdown();

		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
		void				sendGroupMissionUpdate(GroupObject* group);
		MissionObject*		getZoneGroupMission(std::list<uint64>* members);

		GroupObject*		getGroupObject(uint64 id);
		
		void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback);
		void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, string arg);
		void				getGroupLeader(PlayerObject* requester, uint64 groupId, uint32 operation, GroupManagerCallback* callback, uint32 flourishId);

		void				addGroupObject(GroupObject* group){mGroupList.push_back(group);}
		void				deleteGroupObject(uint64 id);
		GroupList*			getGroupList(){return &mGroupList;}

	private:

		void				_processIsmInviteRequest(Message* message);
		void				_processIsmGroupCREO6deltaGroupId(Message* message);
		void				_processIsmGroupLootModeResponse(Message* message);
		void				_processIsmGroupLootMasterResponse(Message* message);
		void				_processIsmGroupInviteInRangeRequest(Message* message);
		void				_processIsmIsGroupLeaderResponse(Message* message);

		uint64				_insertLeaderRequest(GroupManagerCallbackContainer* container);

		static GroupManager*	mSingleton;
		static bool				mInsFlag;

		Database*				mDatabase;
		MessageDispatch*		mMessageDispatch;
		GroupList				mGroupList;

		std::map<uint64, GroupManagerCallbackContainer*> mLeaderRequests;
		uint64					mLeaderRequestInc;
		
};

#endif 