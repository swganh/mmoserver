/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_OBJCONTROLLER_COMMAND_MESSAGE_H
#define ANH_OBJCONTROLLER_COMMAND_MESSAGE_H

#include "Utils/typedefs.h"
#include "ObjectControllerCommandMap.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"


//======================================================================================================================

class ObjControllerCommandMessage
{
	public:

		ObjControllerCommandMessage(uint32 opcode,const uint64 executionTime,uint64 targetId)
									: mOpcode(opcode),mData(NULL),mExecutionTime(executionTime),mTargetId(targetId),
									  mSequence(0),mProperties(NULL){}

		virtual ~ObjControllerCommandMessage()
		{
			gMessageFactory->DestroyMessage(mData);
		}

		virtual inline uint32	getOpcode() const { return mOpcode; }
		virtual inline void		setOpcode(uint32 opcode){ mOpcode = opcode; }

		virtual inline uint64	getTargetId() const { return mTargetId; }
		virtual inline void		setTargetId(uint32 targetId){ mTargetId = targetId; }

		virtual inline uint32	getSequence() const { return mSequence; }
		virtual inline void		setSequence(uint32 sequence){ mSequence = sequence; }

		virtual inline Message*	getData(){ return mData; }
		virtual inline void		setData(Message* message){ mData = message; }

		virtual inline uint64	getExecutionTime() const { return mExecutionTime; }
		virtual inline void		setExecutionTime(uint64 time){ mExecutionTime = time; }

		virtual inline uint64	getQueuesTime() const { return mQueuedTime; }
		virtual inline void		setQueuedTime(uint64 time){ mQueuedTime = time; }

		virtual inline ObjectControllerCmdProperties*	getCmdProperties(){ return mProperties; }
		virtual inline void								setCmdProperties(ObjectControllerCmdProperties*	properties){ mProperties = properties; }

	protected:

		uint32		mOpcode;
		uint64		mTargetId;
		uint32		mSequence;
		Message*	mData;
		uint64		mExecutionTime;
		uint64		mQueuedTime;

		ObjectControllerCmdProperties*	mProperties;
};

//===============================================================================================================

class CompareCommandMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getExecutionTime() > right->getExecutionTime());
		} 
};

//===============================================================================================================

class CompareCombatMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getSequence() > right->getSequence());
		} 
};


#endif

