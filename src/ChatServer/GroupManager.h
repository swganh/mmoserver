/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006, 2007 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_GroupManager_H
#define ANH_ZONESERVER_GroupManager_H


#define	gGroupManager GroupManager::getSingletonPtr()

#include <map>
#include "Common/MessageDispatchCallback.h"

//======================================================================================================================

class Database;
class GroupObject;
class Message;
class MessageDispatch;

//======================================================================================================================

typedef	std::map<uint64,GroupObject*> GroupMap;

//======================================================================================================================

class GroupManager : public MessageDispatchCallback
{
public:
	static GroupManager*	getSingletonPtr() { return mSingleton; }
	static GroupManager*	Init(MessageDispatch* dispatch);

	GroupManager(MessageDispatch* dispatch);
	~GroupManager();

	void				Shutdown();
	virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);

	GroupObject*		getGroupById(uint64 groupId);
	void				removeGroup(uint64 groupId);

	uint64				getNextGroupId(){ ++mNextGroupId; return mNextGroupId; }

	void				_processGroupInviteRequest(Message* message, DispatchClient* client);
	void				_processGroupInviteResponse(Message* message, DispatchClient* client);
	void				_processGroupUnInvite(Message* message, DispatchClient* client);
	void				_processGroupDisband(Message* message, DispatchClient* client);	
	void				_processGroupLeave(Message* message, DispatchClient* client);
	void				_processGroupDismissGroupMember(Message* message, DispatchClient* client);
	void				_processGroupMakeLeader(Message* message, DispatchClient* client);
	void				_processGroupBaselineRequest(Message* message, DispatchClient* client);
	void				_processGroupPositionNotification(Message* message, DispatchClient* client);
	void				_processGroupLootModeRequest(Message* message, DispatchClient* client);
	void				_processGroupLootModeResponse(Message* message, DispatchClient* client);
	void				_processGroupLootMasterRequest(Message* message, DispatchClient* client);
	void				_processGroupLootMasterResponse(Message* message, DispatchClient* client);
	void				_processIsmInviteInRangeResponse(Message* message, DispatchClient* client);

private:

	static GroupManager*		mSingleton;
	static bool					mInsFlag;
	DispatchClient*				mClient;

	MessageDispatch*			mMessageDispatch;

	GroupMap 					mGroups;
	uint64						mNextGroupId;

	BStringVector				mLootModeMessages;
	BStringVector				mLootModeNotAllowedMessages;

};

#endif 

