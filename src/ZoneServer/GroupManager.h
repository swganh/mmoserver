/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_GroupManager_H
#define ANH_ZONESERVER_GroupManager_H

#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include "ObjectFactory.h"
#include "ZoneOpcodes.h"
#include "MissionObject.h"
#include "GroupObject.h"

#include <vector>

//======================================================================================================================

#define	gGroupManager	GroupManager::getSingletonPtr()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;

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
		MissionObject*		getZoneGroupMission(Uint64List* members);

		GroupObject*		getGroupObject(uint64 id);
		void				addGroupObject(GroupObject* group){mGroupList.push_back(group);}
		void				deleteGroupObject(uint64 id);
		GroupList*			getGroupList(){return &mGroupList;}

	private:

		void				_processIsmInviteRequest(Message* message);
		void				_processIsmGroupCREO6deltaGroupId(Message* message);
		void				_processIsmGroupLootModeResponse(Message* message);
		void				_processIsmGroupLootMasterResponse(Message* message);

		static GroupManager*	mSingleton;
		static bool				mInsFlag;

		Database*				mDatabase;
		MessageDispatch*		mMessageDispatch;
		GroupList				mGroupList;

		
};

#endif 

