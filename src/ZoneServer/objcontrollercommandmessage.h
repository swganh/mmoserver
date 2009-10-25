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
#include "Common/MessageFactory.h"

class Message;
class ObjectControllerCmdProperties;

//======================================================================================================================

class ObjControllerCommandMessage
{
	public:

		ObjControllerCommandMessage(uint32 opcode,const uint64 executionTime,uint64 targetId);

		virtual ~ObjControllerCommandMessage();
		virtual uint32 getOpcode() const;
		virtual void setOpcode(uint32 opcode);
		virtual uint64 getTargetId() const;
		virtual void setTargetId(uint32 targetId);

		virtual uint32	getSequence() const;
		virtual void		setSequence(uint32 sequence);

		virtual Message*	getData();
		virtual void		setData(Message* message);

		virtual uint64	getExecutionTime() const;
		virtual void		setExecutionTime(uint64 time);

		// virtual inline uint64	getQueuesTime() const { return mQueuedTime; }
		// virtual inline void		setQueuedTime(uint64 time){ mQueuedTime = time; }

		virtual ObjectControllerCmdProperties*	getCmdProperties();
		virtual void	setCmdProperties(ObjectControllerCmdProperties*	properties);

	protected:
		Message*	mData;
		ObjectControllerCmdProperties*	mProperties;
		uint64		mExecutionTime;
		uint64		mQueuedTime;
		uint64		mTargetId;
		uint32		mOpcode;
		uint32		mSequence;
};

//===============================================================================================================
/*
// Not used
class CompareCommandMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getExecutionTime() > right->getExecutionTime());
		}
};
*/
//===============================================================================================================

/*
// Not used
class CompareCombatMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getSequence() > right->getSequence());
		}
};
*/

#endif

