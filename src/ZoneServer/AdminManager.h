/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ADMIN_MANAGER_H
#define ANH_ZONESERVER_ADMIN_MANAGER_H

#include "Common/MessageDispatchCallback.h"
#include <map>
//=============================================================================

class AdminRequestObject;
class MessageDispatch;

typedef std::map<uint64, AdminRequestObject*> AdminRequests;

enum AdminRequestt
{
	AdminScheduledShutdown		= 0,
	AdminEmergenyShutdown		= 1,
};

//=============================================================================

class AdminManager : public MessageDispatchCallback
{
	public:

		static AdminManager* AdminManager::Instance(void);
		static AdminManager* Init(MessageDispatch* messageDispatch);

		static inline void deleteManager(void)    
		{ 
			if (mInstance)
			{
				delete mInstance;
				mInstance = NULL;
			}
		}
		virtual void handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
		void registerCallbacks(void);
		void unregisterCallbacks(void);
		void _processScheduleShutdown(Message* message, DispatchClient* client);
		void _processCancelScheduledShutdown(Message* message, DispatchClient* client);

		uint64 handleAdminRequest(uint64 requestId, uint64 timeOverdue);
		void addAdminRequest(uint64 type, string message, int32 ttl);
		void cancelAdminRequest(uint64 type, string message);

		bool shutdownPending(void) { return mPendingShutdown;}
		bool shutdownZone(void) { return mTerminateServer;}

	protected:
		// AdminManager(AdminRequestObject* adminObject);
		// AdminManager();
		AdminManager(MessageDispatch* messageDispatch);
		~AdminManager();

	private:
		// This constructor prevents the default constructor to be used, since it is private.
		AdminManager();
	
		static AdminManager* mInstance;
		bool	mPendingShutdown;
		bool	mTerminateServer;

		AdminRequests mAdminRequests;
		MessageDispatch* mMessageDispatch;
};

//=============================================================================



#endif

