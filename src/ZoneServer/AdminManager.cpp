/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------

// NOTE: This file is now under re-construction, implementing the real non-persistent database interface.
// So we have to live with duplicate code for a while.

*/

#include "AdminManager.h"

#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "LogManager/LogManager.h"

#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Utils/utils.h"

#include <cassert>


//======================================================================================================================
//
// Container for admin requests
//

class AdminRequestObject
{
	public:
		AdminRequestObject(uint64 adminRequestType, string reason, int32 timeToLive) :
			mAdminRequestType(adminRequestType),
			mReason(reason),
			mTimeToLive(timeToLive){ }

		~AdminRequestObject()
		{
			// gLogger->logMsgF("AdminRequestObject::~AdminRequestObject() invoked", MSG_NORMAL);
		}

		uint64 mAdminRequestType;
		string mReason;
		int32 mTimeToLive;

	private:
		AdminRequestObject();
};


//=============================================================================

AdminManager* AdminManager::mInstance = NULL;


//======================================================================================================================

AdminManager* AdminManager::Instance()
{
	if (!mInstance)
	{
		// mInstance = new AdminManager(gZoneServer->getDispatcher());
		assert(mInstance != NULL);
	}
	return mInstance;
}
//======================================================================================================================

AdminManager* AdminManager::Init(MessageDispatch* messageDispatch)
{
	if (!mInstance)
	{
		mInstance = new AdminManager(messageDispatch);
		assert(mInstance != NULL);
	}
	return mInstance;
}

//======================================================================================================================
//
// This constructor prevents the default constructor to be used, as long as the constructor is keept private.
//

AdminManager::AdminManager()
{
}

//======================================================================================================================
//

AdminManager::AdminManager(MessageDispatch* messageDispatch) :
							mMessageDispatch(messageDispatch),
							mPendingShutdown(false),
							mTerminateServer(false)

{
	// gLogger->logMsgF("AdminManager::AdminManager() invoked", MSG_NORMAL);
	this->registerCallbacks();
}


//=============================================================================


//=============================================================================

AdminManager::~AdminManager()
{
	// gLogger->logMsgF("AdminManager::~AdminManager() invoked", MSG_NORMAL);
	this->unregisterCallbacks();

	AdminRequests::iterator adminRequestIterator = mAdminRequests.begin();

	while (adminRequestIterator != mAdminRequests.end())
	{
		delete ((*adminRequestIterator).second);
		adminRequestIterator++;
	}
	mAdminRequests.clear();
	mInstance = NULL;
}


//======================================================================================================================

void AdminManager::registerCallbacks(void)
{
	mMessageDispatch->registerSessionlessDispatchClient(AdminAccountId);
	mMessageDispatch->RegisterMessageCallback(opIsmScheduleShutdown,this);
	mMessageDispatch->RegisterMessageCallback(opIsmCancelShutdown,this);
}

//======================================================================================================================

void AdminManager::unregisterCallbacks(void)
{
	mMessageDispatch->UnregisterMessageCallback(opIsmScheduleShutdown);
	mMessageDispatch->UnregisterMessageCallback(opIsmCancelShutdown);
	mMessageDispatch->unregisterSessionlessDispatchClient(AdminAccountId);
}

//======================================================================================================================

void AdminManager::handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client)
{
	switch (opcode)
	{
		case opIsmScheduleShutdown:
		{
			this->_processScheduleShutdown(message, client);
		}
		break;

		case opIsmCancelShutdown:
		{
			this->_processCancelScheduledShutdown(message, client);
		}
		break;

		default: break;
	}
}

//======================================================================================================================



void AdminManager::addAdminRequest(uint64 requestType, string message, int32 ttl)
{
	// We will only handle one request at the time for each type.
	gWorldManager->cancelAdminRequest(static_cast<uint32>(requestType));	// Even though map's fix duplicate issues, the lower level implementation may change.

	AdminRequests::iterator adminRequestIterator = mAdminRequests.find(requestType);
	if (adminRequestIterator != mAdminRequests.end())
	{
		delete ((*adminRequestIterator).second);
		mAdminRequests.erase(adminRequestIterator);
	}

	mPendingShutdown = true;

	// Start the announcement at even minute, or at 15 sec interval when less than a minute.
	int32 minutes = ttl/60;
	int32 seconds = ttl % 60;

	int32 timeToFirstEvent;
	if (minutes > 0)
	{
		timeToFirstEvent = seconds;
	}
	else
	{
		if (seconds > 15)
		{
			timeToFirstEvent = (seconds % 15);
		}
		else
		{
			timeToFirstEvent = 0;
		}
	}
	assert(ttl >= timeToFirstEvent);
	ttl -= timeToFirstEvent;

	AdminRequestObject* requestObject = new AdminRequestObject(requestType, message, ttl);
	if (requestObject)
	{
		if (timeToFirstEvent > 0)
		{
			gLogger->logMsgF("AdminManager::addAdminRequest() You have to wait %d seconds until first announcement", MSG_NORMAL, timeToFirstEvent);
		}
		mAdminRequests.insert(std::make_pair(requestType, requestObject));
		gWorldManager->addAdminRequest(requestType, (uint64)(timeToFirstEvent * 1000));
	}
}

void AdminManager::cancelAdminRequest(uint64 requestType, string message)
{
	// We will only handle one request at the time for each type.
	gWorldManager->cancelAdminRequest(static_cast<uint8>(requestType));	// Even though map's fix duplicate issues, the lower level implementation may change.

	AdminRequests::iterator adminRequestIterator = mAdminRequests.find(requestType);
	if (adminRequestIterator != mAdminRequests.end())
	{
		delete ((*adminRequestIterator).second);
		mAdminRequests.erase(adminRequestIterator);

		message.convert(BSTRType_Unicode16);
		PlayerAccMap::const_iterator it = gWorldManager->getPlayerAccMap()->begin();

		while (it != gWorldManager->getPlayerAccMap()->end())
		{
			const PlayerObject* const player = (*it).second;
			if (player->isConnected())
			{
				gMessageLib->sendSystemMessage((PlayerObject*)player, L"Server shutdown canceled.");
				if (message.getLength())
				{
					gMessageLib->sendSystemMessage((PlayerObject*)player, message);
				}
			}
			++it;
		}
	}
	mPendingShutdown = false;
}

//=============================================================================

uint64 AdminManager::handleAdminRequest(uint64 requestType, uint64 timeOverdue)
{
	// gLogger->logMsgF("AdminManager::handleAdminRequest() Entering...", MSG_NORMAL);

	uint64 waitTime = 0;

	// Find the object.
	AdminRequests::iterator adminRequestIterator = mAdminRequests.find(requestType);
	if (adminRequestIterator != mAdminRequests.end())
	{
		if (requestType == AdminScheduledShutdown)
		{
			int8 rawData[128];
			int32 timeToNextEvent = 0;

			if (((*adminRequestIterator).second)->mTimeToLive == 0)
			{
				// We are done.
				// TODO: halt this zone.
				// gLogger->logMsgF("AdminManager::handleAdminRequest() HALTING this service.", MSG_NORMAL);
				sprintf(rawData,"Server shutting down.");
				mTerminateServer = true;
			}
			else
			{
				int32 minutes = ((*adminRequestIterator).second)->mTimeToLive/60;
				int32 seconds = ((*adminRequestIterator).second)->mTimeToLive % 60;

				string unit("minutes");
				int32 value = minutes;

				if (minutes > 1)
				{
					timeToNextEvent = 60;
				}
				else if (minutes == 1)
				{
					timeToNextEvent = 15;
					unit = "minute";
				}
				else
				{
					unit = "seconds";
					value = seconds;

					if (seconds >= 15)
					{
						timeToNextEvent = 15;
					}
					else
					{
						timeToNextEvent = seconds;
						if (seconds == 1)
						{
							unit = "second";
						}
					}
				}
				sprintf(rawData,"Server shutting down in %"PRId32" %s.", value, unit.getAnsi());
			}

			string broadcast(rawData);
			string optReason(((*adminRequestIterator).second)->mReason);

			if (optReason.getLength())
			{
				gLogger->logMsgF(optReason.getAnsi(), MSG_NORMAL);
			}
			gLogger->logMsgF(broadcast.getAnsi(), MSG_NORMAL);

			// For logging, we need ansi versions.
			string logOptReason(optReason);
			string logBroadcast(broadcast);

			// Update players in zone.
			optReason.convert(BSTRType_Unicode16);
			broadcast.convert(BSTRType_Unicode16);

			PlayerAccMap::const_iterator it = gWorldManager->getPlayerAccMap()->begin();
			while (it != gWorldManager->getPlayerAccMap()->end())
			{
				const PlayerObject* const player = (*it).second;
				if (player->isConnected())
				{
					if (optReason.getLength())
					{
						gLogger->logMsgF("%s", MSG_NORMAL, logOptReason.getAnsi());
						gMessageLib->sendSystemMessage((PlayerObject*)player, optReason);
					}
					gLogger->logMsgF("%s", MSG_NORMAL, logBroadcast.getAnsi());
					gMessageLib->sendSystemMessage((PlayerObject*)player, broadcast);
				}
				++it;
			}

			assert(((*adminRequestIterator).second)->mTimeToLive >= timeToNextEvent);
			((*adminRequestIterator).second)->mTimeToLive -= timeToNextEvent;

			if (timeToNextEvent > 0)
			{
				timeToNextEvent *= 1000;
				if (timeOverdue >= (uint64)timeToNextEvent)
				{
					waitTime = 1;
				}
				else
				{
					waitTime = (uint64)timeToNextEvent - timeOverdue;
				}
			}
			if (waitTime == 0)
			{
				// We are done, remove the object.
				delete ((*adminRequestIterator).second);
				mAdminRequests.erase(adminRequestIterator);
			}


		}
	}
	return waitTime;
}


void AdminManager::_processScheduleShutdown(Message* message, DispatchClient* client)
{
	gLogger->logMsg("AdminManager::_processScheduleShutdown");

	message->ResetIndex();

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/* uint32 opCode = */message->getUint32();
	uint32 scheduledTime = message->getUint32();
	message->getStringUnicode16(msg);

	msg.convert(BSTRType_ANSI);
	this->addAdminRequest(AdminScheduledShutdown, msg, (int32)scheduledTime);
}

void AdminManager::_processCancelScheduledShutdown(Message* message, DispatchClient* client)
{
	gLogger->logMsg("AdminManager::_processCancelScheduledShutdown");

	message->ResetIndex();

	string msg;
	msg.setType(BSTRType_Unicode16);
	msg.setLength(512);

	/* uint32 opCode = */message->getUint32();
	/* uint32 option = */message->getUint32();
	message->getStringUnicode16(msg);

	msg.convert(BSTRType_ANSI);
	this->cancelAdminRequest(AdminScheduledShutdown, msg);
}
