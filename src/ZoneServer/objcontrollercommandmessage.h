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
		virtual inline uint32	getOpcode() const;
		virtual inline void		setOpcode(uint32 opcode);
		virtual inline uint64	getTargetId() const;
		virtual inline void		setTargetId(uint32 targetId);

		virtual inline uint32	getSequence() const;
		virtual inline void		setSequence(uint32 sequence);

		virtual inline Message*	getData();
		virtual inline void		setData(Message* message);

		virtual inline uint64	getExecutionTime() const;
		virtual inline void		setExecutionTime(uint64 time);

		// virtual inline uint64	getQueuesTime() const { return mQueuedTime; }
		// virtual inline void		setQueuedTime(uint64 time){ mQueuedTime = time; }

		virtual inline ObjectControllerCmdProperties*	getCmdProperties();
		virtual inline void	setCmdProperties(ObjectControllerCmdProperties*	properties);

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

