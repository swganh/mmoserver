/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "WorldManager.h"
#include "CombatManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "WorldConfig.h"
#include "EVQueueSize.h"
#include "EVCmdProperty.h"
#include "EVAbility.h"
#include "EVState.h"
#include "EVSurveySample.h"
#include "EVWeapon.h"
#include "EVPosture.h"
#include "PVHam.h"
#include "PVState.h"
#include "PVPosture.h"


//=============================================================================
//
// Constructor
//

ObjectController::ObjectController() :
mObject(NULL),
mTaskId(0),
mCommandQueueProcessTimeLimit(2),
mEventQueueProcessTimeLimit(2),
mNextCombatCmdExecution(0),
mNextCommandExecution(0),
mDatabase(gWorldManager->getDatabase()),
mCmdMsgPool(sizeof(ObjControllerCommandMessage)),
mEventPool(sizeof(ObjControllerEvent)),
mDBAsyncContainerPool(sizeof(ObjControllerAsyncContainer)),
mUpdatingObjects(false),
mDestroyOutOfRangeObjects(false),
mMovementInactivityTrigger(5),
mFullUpdateTrigger(0)
{
	mSI		= gWorldManager->getSI();
	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock	= new Anh_Utils::Clock();
}

//=============================================================================
//
// Constructor
//

ObjectController::ObjectController(Object* object) :
mObject(object),
mTaskId(0),
mCommandQueueProcessTimeLimit(2),
mEventQueueProcessTimeLimit(2),
mNextCombatCmdExecution(0),
mNextCommandExecution(0),
mDatabase(gWorldManager->getDatabase()),
mCmdMsgPool(sizeof(ObjControllerCommandMessage)),
mEventPool(sizeof(ObjControllerEvent)),
mDBAsyncContainerPool(sizeof(ObjControllerAsyncContainer)),
mUpdatingObjects(false),
mDestroyOutOfRangeObjects(false),
mMovementInactivityTrigger(5),
mFullUpdateTrigger(0)
{
	mSI		= gWorldManager->getSI();
	// We do have a global clock object, don't use seperate clock and times for every process.
	// mClock	= new Anh_Utils::Clock();
}

//=============================================================================
//
// Deconstructor
//

ObjectController::~ObjectController()
{
	clearQueues();

	EnqueueValidators::iterator it = mEnqueueValidators.begin();

	while(it != mEnqueueValidators.end())
	{
		delete(*it);
		it = mEnqueueValidators.erase(it);
	}

	ProcessValidators::iterator pIt = mProcessValidators.begin();

	while(pIt != mProcessValidators.end())
	{
		delete(*pIt);
		pIt = mProcessValidators.erase(pIt);
	}

	// delete(mClock);
}

//=============================================================================
//
// clear queues
//

void ObjectController::clearQueues()
{
	// command queue
	CommandQueue::iterator cmdIt = mCommandQueue.begin();

	while(cmdIt != mCommandQueue.end())
	{
		ObjControllerCommandMessage* cmdMsg = (*cmdIt);

		cmdMsg->~ObjControllerCommandMessage();
		mCmdMsgPool.free(cmdMsg);

		cmdIt = mCommandQueue.erase(cmdIt);
	}

	// event queue
	EventQueue::iterator eventIt = mEventQueue.begin();

	while(eventIt != mEventQueue.end())
	{
		ObjControllerEvent* event = (*eventIt);

		event->~ObjControllerEvent();
		mEventPool.free(event);

		eventIt = mEventQueue.erase(eventIt);
	}
}

//=============================================================================
//
// process
//

bool ObjectController::process(uint64 callTime,void*)
{
	if(!_processCommandQueue() && !_processEventQueue())
	{
		mTaskId = 0;
		return(false);
	}

	return(true);
}

//=============================================================================
//
// remove message from queue
//

void ObjectController::removeMsgFromCommandQueue(uint32 opcode)
{
	CommandQueue::iterator cmdIt = mCommandQueue.begin();

	while(cmdIt != mCommandQueue.end())
	{
		ObjControllerCommandMessage* cmdMsg = (*cmdIt);

		if(cmdMsg->getOpcode() == opcode)
		{
			cmdMsg->~ObjControllerCommandMessage();
			mCmdMsgPool.free(cmdMsg);
			cmdIt = mCommandQueue.erase(cmdIt);
		}
		else
			++cmdIt;
	}

	mCommandQueue.assureHeap(true);
}

//=============================================================================
//
// process command queue
//
bool ObjectController::_processCommandQueue()
{
	mHandlerCompleted = false;
	// init timers
	uint64	startTime		= Anh_Utils::Clock::getSingleton()->getLocalTime();
	uint64	currentTime		= startTime;
	uint64	processTime		= 0;
	
	// loop until empty or our time is up
	while(mCommandQueue.size() && processTime < mCommandQueueProcessTimeLimit)
	{
		// see if we got something to execute yet
		ObjControllerCommandMessage* cmdMsg = mCommandQueue.top();
		
		if(cmdMsg && cmdMsg->getExecutionTime() <= currentTime)
		{
			//gLogger->logMsgF("Current Time %lld ExecTime %lld",MSG_LOW,currentTime,cmdMsg->getExecutionTime());

			mCommandQueue.pop();

			// get the commands data
			Message*	message		= cmdMsg->getData();
			uint32		command		= cmdMsg->getOpcode();
			uint64		targetId	= cmdMsg->getTargetId();
			uint32		reply1		= 0;
			uint32		reply2		= 0;
			bool		consumeHam	= true;

			ObjectControllerCmdProperties*	cmdProperties = cmdMsg->getCmdProperties();

			// validate if we are still able to execute
			if(cmdProperties && _validateProcessCommand(reply1,reply2,targetId,command,cmdProperties))
			{
				// keep a pointer to the start
				uint16	paramsIndex = message->getIndex();

				// call the proper handler
				switch(cmdProperties->mCmdGroup)
				{
					case ObjControllerCmdGroup_Common:
					{
						// get the command
						CommandMap::iterator it = gObjControllerCmdMap.find(command);

						if(it != gObjControllerCmdMap.end())
						{
							(this->*((*it).second))(targetId,message,cmdProperties);
							consumeHam = mHandlerCompleted;
						}
						else
						{
							gLogger->logMsgF("ObjectController::processCommandQueue: ObjControllerCmdGroup_Common Unhandled Cmd 0x%x for %lld\n",MSG_NORMAL,command,mObject->getId());
							//gLogger->hexDump(message->getData(),message->getSize());

							consumeHam = false;
						}
					}
					break;

					case ObjControllerCmdGroup_Attack:
					{
						// gLogger->logMsgF("ObjectController::processCommandQueue: ObjControllerCmdGroup_Attack Handled Cmd 0x%x for %lld",MSG_NORMAL,command,mObject->getId());

						if(!gCombatManager->handleAttack(dynamic_cast<CreatureObject*>(mObject),targetId,cmdProperties))
						{
							// gLogger->logMsg("ObjectController::processCommandQueue: handleAttack error");

							consumeHam = mHandlerCompleted;
						}
					}
					break;

					default:
					{
						gLogger->logMsgF("ObjectController::processCommandQueue: Default Unhandled CmdGroup %u for %lld\n",MSG_NORMAL,cmdProperties->mCmdGroup,mObject->getId());

						consumeHam = false;
					}
					break;
				}
			
				if(consumeHam && !_consumeHam(cmdProperties))
				{
					//gLogger->logMsgF("ObjectController::processCommandQueue: consume ham fail");
				}

				// execute any attached scripts
				string params;
				message->setIndex(paramsIndex);
				message->getStringUnicode16(params);
				params.convert(BSTRType_ANSI);

				gObjectControllerCommands->mCmdScriptListener.handleScriptEvent(cmdProperties->mCommandStr.getAnsi(),params);
			}
		
			//its processed, so ack and delete it
			if(PlayerObject* player	= dynamic_cast<PlayerObject*>(mObject))
			{
				gMessageLib->sendCommandQueueRemove(cmdMsg->getSequence(),0.0f,reply1,reply2,player);
			}
			
			cmdMsg->~ObjControllerCommandMessage();
			mCmdMsgPool.free(cmdMsg);
		}
		// break out, if we dont get to execute something this frame
		else if((cmdMsg->getExecutionTime() - currentTime) > (mCommandQueueProcessTimeLimit - processTime))
			break;

		// update timers
		currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
		processTime = currentTime - startTime;
	}

	// if we didn't manage to process all or theres an event still waiting, don't remove us from the scheduler
	return(mCommandQueue.size() ? true : false);
}

//=============================================================================
//
// process event queue
//

bool ObjectController::_processEventQueue()
{
	// init timers
	uint64	startTime		= Anh_Utils::Clock::getSingleton()->getLocalTime();
	uint64	currentTime		= startTime;
	uint64	processTime		= 0;

	// loop until empty or our time is up
	while(mEventQueue.size() && processTime < mEventQueueProcessTimeLimit)
	{
		// see if we got something to execute yet
		ObjControllerEvent* event = mEventQueue.top();

		if(event && event->getExecutionTime() <= currentTime)
		{
			mEventQueue.pop();

			mObject->handleEvent(event->getEvent());

			//its processed, delete it
			event->~ObjControllerEvent();
			mEventPool.free(event);
		}
		// break out, if we dont get to execute something this frame
		else if((event->getExecutionTime() - currentTime) > (mEventQueueProcessTimeLimit - processTime))
			break;

		// update timers
		currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
		processTime = currentTime - startTime;
	}

	// if we didn't manage to process all, don't remove us from the scheduler
	return(mEventQueue.size() ? true : false);
}

//=============================================================================
//
// enqueue message
//

void ObjectController::enqueueCommandMessage(Message* message)
{
	uint32	clientTicks		= message->getUint32();
	uint32	sequence		= message->getUint32();
	uint32	opcode			= message->getUint32();
	uint64	targetId		= message->getUint64();
	uint32	reply1			= 0;
	uint32	reply2			= 0;
	
	ObjectControllerCmdProperties* cmdProperties = NULL;

	//gLogger->logMsgF("ObjController enqueue tick: %u counter: 0x%4x",MSG_NORMAL,clientTicks,sequence);
	
	if(_validateEnqueueCommand(reply1,reply2,targetId,opcode,cmdProperties))
	{
		// schedule it for immidiate execution initially
		uint64 execTime	= Anh_Utils::Clock::getSingleton()->getLocalTime();

		// make a copy of the message, since we may need to keep it for a while
		gMessageFactory->StartMessage();
		gMessageFactory->addData(message->getData(),message->getSize());

		Message* newMessage = gMessageFactory->EndMessage();
		newMessage->setIndex(message->getIndex());	

		// its a scheduled action
		if(sequence && cmdProperties->mAddToCombatQueue)
		{
			// need to queue this command for execution, since there is a delay from the last one
			if(mNextCombatCmdExecution > execTime)
			{
				execTime = mNextCombatCmdExecution;
				mNextCombatCmdExecution += cmdProperties->mDefaultTime;
			}
			// no delays, execute immidiate and set delay for the next command
			else
			{
				mNextCombatCmdExecution = execTime + cmdProperties->mDefaultTime;
			}
		}

		// create the queued message, need setters since boost pool constructor templates take 3 params max
		ObjControllerCommandMessage* cmdMsg = new(mCmdMsgPool.malloc()) ObjControllerCommandMessage(opcode,execTime,targetId);
		cmdMsg->setSequence(sequence);
		cmdMsg->setData(newMessage);
		cmdMsg->setCmdProperties(cmdProperties);

		// add it
		mCommandQueue.push(cmdMsg);

		// add us to the scheduler, if we aren't queued already
		if(!mTaskId)
		{
			mTaskId = gWorldManager->addObjControllerToProcess(this);
		}
	}
	// not qualified for this command, so remove it
	else if(PlayerObject* player = dynamic_cast<PlayerObject*>(mObject))
	{
		gMessageLib->sendCommandQueueRemove(sequence,0.0f,reply1,reply2,player);
	}
}

//=============================================================================
//
// add an event
//

void ObjectController::addEvent(Anh_Utils::Event* event,uint64 timeDelta)
{
	ObjControllerEvent* ocEvent = new(mEventPool.malloc()) ObjControllerEvent(Anh_Utils::Clock::getSingleton()->getLocalTime() + timeDelta,event);

	// add it
	mEventQueue.push(ocEvent);

	// add us to the scheduler, if we aren't queued already
	if(!mTaskId)
	{
		mTaskId = gWorldManager->addObjControllerToProcess(this);
	}
}

//=============================================================================
//
// remove command message
//

void ObjectController::removeCommandMessage(Message* message)
{
	// skip tickcount
	message->getUint32();

	// pass sequence
	removeMsgFromCommandQueueBySequence(message->getUint32());
}

//=============================================================================
//
// remove a cmd queue message by its sequence given
//

void ObjectController::removeMsgFromCommandQueueBySequence(uint32 sequence)
{
	// sanity check
	if(!sequence)
		return;

	CommandQueue::iterator cmdIt = mCommandQueue.begin();

	while(cmdIt != mCommandQueue.end())
	{
		if((*cmdIt)->getSequence() == sequence)
		{
			ObjControllerCommandMessage* msg = (*cmdIt);

			// update queue timers
			if(ObjectControllerCmdProperties* properties = msg->getCmdProperties())
			{
				mNextCombatCmdExecution -= properties->mDefaultTime;
			}

			// delete it
			msg->~ObjControllerCommandMessage();
			mCmdMsgPool.free(msg);

			mCommandQueue.erase(cmdIt);

			break;
		}

		++cmdIt;
	}

	mCommandQueue.assureHeap(true);
}

//=============================================================================
//
// checks if a command is allowed to be executed
// sets the according error replies
//

bool ObjectController::_validateEnqueueCommand(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
	EnqueueValidators::iterator it = mEnqueueValidators.begin();

	while(it != mEnqueueValidators.end())
	{
		if(!((*it)->validate(reply1,reply2,targetId,opcode,cmdProperties)))
		{
			return(false);
		}

		++it;
	}

	return(true);
}

//=============================================================================
//
// checks if a command is allowed to be executed
// sets the according error replies
//

bool ObjectController::_validateProcessCommand(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
	ProcessValidators::iterator it = mProcessValidators.begin();

	while(it != mProcessValidators.end())
	{
		if(!((*it)->validate(reply1,reply2,targetId,opcode,cmdProperties)))
		{
			return(false);
		}

		++it;
	}

	return(true);
}

//=============================================================================
//
// setup enqueue cmd validators
// make sure to keep the order sane
//

void ObjectController::initEnqueueValidators()
{
	mEnqueueValidators.push_back(new EVQueueSize(this));
	mEnqueueValidators.push_back(new EVCmdProperty(this));

	switch(mObject->getType())
	{
		case ObjType_Player:
		{
			mEnqueueValidators.push_back(new EVSurveySample(this));
		}
		case ObjType_NPC:
		case ObjType_Creature:
		{
			mEnqueueValidators.push_back(new EVPosture(this));
			mEnqueueValidators.push_back(new EVAbility(this));
			//mEnqueueValidators.push_back(new EVState(this));
			mEnqueueValidators.push_back(new EVWeapon(this));
		}
		break;

		default: break;
	}
}

//=============================================================================
//
// setup process cmd validators
// make sure to keep the order sane
//

void ObjectController::initProcessValidators()
{
	switch(mObject->getType())
	{
		case ObjType_Player:
		case ObjType_NPC:
		case ObjType_Creature:
		{
			mProcessValidators.push_back(new PVPosture(this));
			mProcessValidators.push_back(new PVHam(this));
			mProcessValidators.push_back(new PVState(this));
		}
		break;

		default: break;
	}
}

//=============================================================================

bool ObjectController::_consumeHam(ObjectControllerCmdProperties* cmdProperties)
{
	if(CreatureObject* creature	= dynamic_cast<CreatureObject*>(mObject))
	{
		if(Ham* ham = creature->getHam())
		{
			if(cmdProperties->mHealthCost)
			{
				ham->updatePropertyValue(HamBar_Health,HamProperty_CurrentHitpoints,-cmdProperties->mHealthCost);
			}

			if(cmdProperties->mActionCost)
			{
				ham->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints,-cmdProperties->mActionCost);
			}

			if(cmdProperties->mMindCost)
			{
				ham->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints,-cmdProperties->mMindCost);
			}
		}
		else
		{
			return(false);
		}

		return(true);
	}

	return(false);
}
