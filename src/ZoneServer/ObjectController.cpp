/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "CombatManager.h"
#include "EVQueueSize.h"
#include "EVCmdProperty.h"
#include "EVAbility.h"
#include "EVState.h"
#include "EVSurveySample.h"
#include "EVWeapon.h"
#include "EVPosture.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "objcontrollercommandmessage.h"
#include "PlayerObject.h"
#include "PVHam.h"
#include "PVPosture.h"
#include "PVState.h"
#include "StateManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "SpatialIndexManager.h"

#include "MessageLib/MessageLib.h"

// Fix for issues with glog redefining this constant
#ifdef ERROR
#undef ERROR
#endif

#include <glog/logging.h>

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/EventDispatcher.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/Message.h"
#include "Utils/clock.h"

#include "SwgProtocol/ObjectControllerEvents.h"

#include <cassert>

using ::common::IEventPtr;
using ::swg_protocol::object_controller::PostCommandEvent;
using ::swg_protocol::object_controller::PreCommandEvent;

//=============================================================================
//
// Constructor
//
ObjectController::ObjectController()
    : mCmdMsgPool(sizeof(ObjControllerCommandMessage))
    , mDBAsyncContainerPool(sizeof(ObjControllerAsyncContainer))
    , mEventPool(sizeof(ObjControllerEvent))
    , mDatabase(gWorldManager->getDatabase())
    , mObject(NULL)
    , mCommandQueueProcessTimeLimit(5)
    , mEventQueueProcessTimeLimit(2)
    , mNextCommandExecution(0)
    , mTaskId(0)
    , mUnderrunTime(0)
    , mMovementInactivityTrigger(5)
    , mFullUpdateTrigger(0)
    , mDestroyOutOfRangeObjects(false)
    , mInUseCommandQueue(false)
    , mRemoveCommandQueue(false)
    , mUpdatingObjects(false)
{}

//=============================================================================
//
// Constructor
//

ObjectController::ObjectController(Object* object)
    : mCmdMsgPool(sizeof(ObjControllerCommandMessage))
    , mDBAsyncContainerPool(sizeof(ObjControllerAsyncContainer))
    , mEventPool(sizeof(ObjControllerEvent))
    , mDatabase(gWorldManager->getDatabase())
    , mObject(object)
    , mCommandQueueProcessTimeLimit(5)
    , mEventQueueProcessTimeLimit(2)
    , mNextCommandExecution(0)
    , mTaskId(0)
    , mUnderrunTime(0)
    , mMovementInactivityTrigger(5)
    , mFullUpdateTrigger(0)
    , mDestroyOutOfRangeObjects(false)
    , mInUseCommandQueue(false)
    , mRemoveCommandQueue(false)
    , mUpdatingObjects(false)
{}

//=============================================================================
//
// Deconstructor
//

ObjectController::~ObjectController()
{
    // Have to kill whats in there...
    mInUseCommandQueue = false;
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
}

//=============================================================================
//
// clear queues
//

/*
void ObjectController::emptyCommandQueue()
{
    // command queue

    CommandQueue::iterator cmdIt = mCommandQueue.begin();

    while(cmdIt != mCommandQueue.end())
    {
        ObjControllerCommandMessage* cmdMsg = (*cmdIt);

            else if(PlayerObject* player = dynamic_cast<PlayerObject*>(mObject))
    {
        gMessageLib->sendCommandQueueRemove(sequence,0.0f,reply1,reply2,player);
    }

        cmdMsg->~ObjControllerCommandMessage();
        mCmdMsgPool.free(cmdMsg);

        cmdIt = mCommandQueue.erase(cmdIt);
    }
}
*/

//=============================================================================
//
// clear queues
//

void ObjectController::clearQueues()
{
    // command queue

    if (!mInUseCommandQueue)
    {
        CommandQueue::iterator cmdIt = mCommandQueue.begin();

        while(cmdIt != mCommandQueue.end())
        {
            ObjControllerCommandMessage* cmdMsg = (*cmdIt);

            cmdMsg->~ObjControllerCommandMessage();
            mCmdMsgPool.free(cmdMsg);

            cmdIt = mCommandQueue.erase(cmdIt);
        }
        mRemoveCommandQueue = false;
    }
    else
    {
        mRemoveCommandQueue = true;
    }

    // mCommandQueue.clear(); // Will not free the boost shit used (mCmdMsgPool).

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

    while (cmdIt != mCommandQueue.end())
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

    // uint64 currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();

    PlayerObject* player  = dynamic_cast<PlayerObject*>(mObject);
    if (!player)
    {
        assert(false && "ObjectController::_processCommandQueue mObject is not a PlayerObject");
        return false;
    }

    // If queue empty and we are in combat, insert player initiated auto-attack when the previous command has run out it's cooldown.
    if (mCommandQueue.empty() &&  player->autoAttackEnabled() && (mNextCommandExecution <= currentTime))
    {
        // Auto attack current target.
        uint64 autoTargetId = player->getCombatTargetId();
        if (autoTargetId != 0)
        {
            this->enqueueAutoAttack(autoTargetId);
        }
        else
        {
            player->disableAutoAttack();
        }
    }

    // For debug
    int16 loopCounter = 0;

    // loop until empty or our time is up
    while (!mCommandQueue.empty() && (processTime < mCommandQueueProcessTimeLimit))
    {
        mInUseCommandQueue = true;

        if (++loopCounter > 1)
        {
        }

        // see if we got something to execute yet
        ObjControllerCommandMessage* cmdMsg = mCommandQueue.front();

        // we might need to execute the command directly if it is part of the combat queue
        // and our queue execution timer permits it
        // or it is not part of the combat queue and can be executed directly in any case
        if ((mNextCommandExecution <= currentTime) || !cmdMsg->getCmdProperties()->mAddToCombatQueue)
        {
            if (cmdMsg->getCmdProperties()->mAddToCombatQueue)
            {
                // Compensate combat command for any lag, i.e. command arriving late.
                mUnderrunTime += (currentTime - mNextCommandExecution);
            }

            // get the commands data
            Message*	message		= cmdMsg->getData();	// Be aware, internally created messages are NULL (auto-attack)
            uint32		command		= cmdMsg->getOpcode();
            uint64		targetId	= cmdMsg->getTargetId();
            uint32		reply1		= 0;
            uint32		reply2		= 0;

            ObjectControllerCmdProperties*	cmdProperties = cmdMsg->getCmdProperties();

            // validate if we are still able to execute
            if (cmdProperties && _validateProcessCommand(reply1,reply2,targetId,command,cmdProperties))
            {

                // Do not mess with cooldowns for non combat commands, those timer values are for preventing spam of the same message,
                // and spam preventing is not implemented yet.
                uint64 timeToNextCommand = 0;
                if (cmdMsg->getCmdProperties()->mAddToCombatQueue)
                {
                    // Set up the cooldown time.
                    if (mUnderrunTime < cmdProperties->mDefaultTime)
                    {
                        timeToNextCommand = (cmdProperties->mDefaultTime - mUnderrunTime);
                        mUnderrunTime = 0;
                    }
                    else
                    {
                        // Compensate as much as we can.
                        timeToNextCommand = cmdProperties->mDefaultTime / 2;
                        mUnderrunTime -= timeToNextCommand;
                    }
                }

                // keep a pointer to the start
                uint16	paramsIndex = 0;
                if (message)
                {
                    paramsIndex = message->getIndex();
                }

                bool cmdExecutedOk = true;

                // call the proper handler
                switch(cmdProperties->mCmdGroup)
                {
                case ObjControllerCmdGroup_Common:
                {
                    // Check the new style of handlers first.
                    CommandMap::const_iterator it = gObjectControllerCommands->getCommandMap().find(command);

                    // Find the target object (if one is given) and pass it in.
                    Object* target = NULL;

                    if (targetId) {
                        target = gWorldManager->getObjectById(targetId);
                    }

                    // If a new style handler is found process it.
                    if (message && it != gObjectControllerCommands->getCommandMap().end()) {
                        // Create a pre-command processing event.
                        auto pre_event = std::make_shared<PreCommandEvent>(mObject->getId());
                        pre_event->target_id(targetId);
                        pre_event->command_crc(cmdProperties->mCmdCrc);

                        // Trigger a pre-command processing event and get the result. This allows
                        // any listeners to veto the processing of the command (such as validators).
                        // Only process the command if it passed validation.
                        if (gEventDispatcher.Deliver(pre_event).get()) {
                            ((*it).second)(mObject, target, message, cmdProperties);

                            auto post_event = std::make_shared<PostCommandEvent>(mObject->getId());
                            gEventDispatcher.Deliver(post_event);
                        }
                    } else {
                        // Otherwise, process the old style handler.
                        OriginalCommandMap::iterator it = gObjControllerCmdMap.find(command);

                        if (message && it != gObjControllerCmdMap.end()) {
                            ((*it).second)(this, targetId, message, cmdProperties);
                            //(this->*((*it).second))(targetId,message,cmdProperties);
                        } else {
                            DLOG(WARNING) << "ObjectController::processCommandQueue: ObjControllerCmdGroup_Common Unhandled Cmd 0"<<command<<" for "<<mObject->getId();
                            //gLogger->hexDump(message->getData(),message->getSize());
                        }
                    }
                }
                break;
                case ObjControllerCmdGroup_Attack:
                {
                    // If player activated combat or me returning fire, the peace is ended, and auto-attack allowed.
                    gStateManager.setCurrentActionState(player, CreatureState_Combat);
                    // TODO: add auto attack to enter combat state.
                    player->enableAutoAttack();

                    // CreatureObject* creature = NULL;
                    if (targetId != 0)
                    {
                        cmdExecutedOk = gCombatManager->handleAttack(player, targetId, cmdProperties);
                        if (!cmdExecutedOk)
                        {
                            // We have lost our target.
                            player->setCombatTargetId(0);
                            player->disableAutoAttack();
                        }
                        else
                        {
                            // All is well in la-la-land
                            // Keep track of the target we are attacking, it's not always your "look-at" target.
                            if (targetId != player->getCombatTargetId() && (targetId != 0))
                            {
                                // We have a new target
                                // new target still valid?
                                if (player->setAsActiveDefenderAndUpdateList(targetId))
                                {
                                    player->setCombatTargetId(targetId);
                                }
                                else
                                {
                                    player->disableAutoAttack();
                                    player->setCombatTargetId(0);
                                }
                            }
                            else
                            {
                                player->setCombatTargetId(targetId);
                            }
                        }
                    }
                    else
                    {
                        cmdExecutedOk = false;
                        player->setCombatTargetId(0);
                    }
                }
                break;

                default:
                {
					DLOG(WARNING) << "ObjectController::processCommandQueue: ObjControllerCmdGroup_Common Unhandled Cmd 0"<<cmdProperties->mCmdGroup <<" for "<<mObject->getId();
                }
                break;
                }

                // Do not mess with cooldowns for non combat commands...
                if (cmdMsg->getCmdProperties()->mAddToCombatQueue)
                {
                    if (cmdExecutedOk)
                    {
                        mNextCommandExecution = currentTime + timeToNextCommand;
                    }
                    else
                    {
                        // we will not spam the command queue if auto-attack is set to an invalid target.
                        mNextCommandExecution = currentTime;
                    }
                }

                // execute any attached scripts
                if (message)	// Auto-attack commands have no message body.
                {
                    BString params;
                    message->setIndex(paramsIndex);
                    message->getStringUnicode16(params);
                    params.convert(BSTRType_ANSI);

                    gObjectControllerCommands->mCmdScriptListener.handleScriptEvent(cmdProperties->mCommandStr.getAnsi(),params);
                }
            }

            //its processed, so ack and delete it
            if (message && cmdMsg->getSequence())
            {
                // if (PlayerObject* player = dynamic_cast<PlayerObject*>(mObject))
                gMessageLib->sendCommandQueueRemove(cmdMsg->getSequence(),(static_cast<float>(cmdMsg->getExecutionTime())/1000),reply1,reply2,player);
            }

            // Be aware, internally created messages are NULL (auto-attack)
            if (message)
            {
                message->setPendingDelete(true);
            }
            // Remove the command from queue. Note: pop() invokes object destructor.
            mCommandQueue.pop_front();

            // cmdMsg->~ObjControllerCommandMessage();
            mCmdMsgPool.free(cmdMsg);
        }
        else
        {
            // Make it simple, the time was not up yet, just leave, let other instances have the cpu, and handle it the next cycle.
            break;
        }

        // update timers
        currentTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
        processTime = currentTime - startTime;
    }

    // if we didn't manage to process all or theres an event still waiting, don't remove us from the scheduler
    // We need to keep the queue as long as we are in combat.

    mInUseCommandQueue = false;

    if (mRemoveCommandQueue)
    {
        this->clearQueues();
    }
    return ((!mCommandQueue.empty() ||  player->autoAttackEnabled())? true : false);
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
// Add the command in the queue. No fancy stuff like predicting execution time etc... just add the fracking command.
//
void ObjectController::enqueueCommandMessage(Message* message)
{
	// this is required with how we are grabbing the packets
	// basically think of this as just grabbing the first uint32
	// value and throwing it away to get to the get stuff...
	message->getUint32();
    uint32	sequence		= message->getUint32();
    uint32	opcode			= message->getUint32();
    uint64	targetId		= message->getUint64();
    uint32	reply1			= 0;
    uint32	reply2			= 0;

    ObjectControllerCmdProperties* cmdProperties = NULL;

    if (_validateEnqueueCommand(reply1,reply2,targetId,opcode,cmdProperties))
    {
        // schedule it for immidiate execution initially
        // uint64 execTime	= Anh_Utils::Clock::getSingleton()->getLocalTime();

        // make a copy of the message, since we may need to keep it for a while
        gMessageFactory->StartMessage();
        gMessageFactory->addData(message->getData(),message->getSize());

        Message* newMessage = gMessageFactory->EndMessage();
        newMessage->setIndex(message->getIndex());

        // create the queued message, need setters since boost pool constructor templates take 3 params max

        // The cmdProperties->mDefaultTime is NOT a delay for NEXT command, it's the cooldown for THIS command.
        ObjControllerCommandMessage* cmdMsg = new(mCmdMsgPool.malloc()) ObjControllerCommandMessage(opcode,cmdProperties->mDefaultTime,targetId);
        cmdMsg->setSequence(sequence);
        cmdMsg->setData(newMessage);
        cmdMsg->setCmdProperties(cmdProperties);

        // Do we have any commands in the queue?
        if (mCommandQueue.empty())
        {
            // No, this will now become the very top command in the queue.

            // Are there any cooldown left from previous command?
            uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
            if (mNextCommandExecution <= now)
            {
                uint64 underrun = now - mNextCommandExecution;

                if (underrun <= mUnderrunTime)
                {
                    mUnderrunTime -= underrun;
                }
                else
                {
                    mUnderrunTime = 0;
                }

                // This command are to be handled as fast as possible.
                mNextCommandExecution = now;
            }
        }
        // add it
        if ((sequence && cmdProperties->mAddToCombatQueue) || (mCommandQueue.empty()))
        {
            mCommandQueue.push_back(cmdMsg);
        }
        else
        {
            // We may have prio command already scheduled at top, and we should not reverse the order of them.
            // Plus, loop-backed admin messsages would be picked and destroyed in the wrong order.

            // Note .insert(..) may invalidate all iterators if not inserted at top or back.
            CommandQueue::iterator cmdIt = mCommandQueue.begin();
            bool inserted = false;
            while(cmdIt != mCommandQueue.end())
            {
                if ((*cmdIt)->getSequence() && (*cmdIt)->getCmdProperties()->mAddToCombatQueue)
                {
                    mCommandQueue.insert(cmdIt, cmdMsg);
                    inserted = true;
                    break;
                }
                else
                {
                    cmdIt++;
                }
            }
            if (!inserted)
            {
                mCommandQueue.push_back(cmdMsg);
            }
        }

        // add us to the scheduler, if we aren't queued already
        if(!mTaskId)
        {
            mTaskId = gWorldManager->addObjControllerToProcess(this);
        }
    }
    // not qualified for this command, so remove it
    // Internally generated commands, like auto-attack and admin commands does not have a sequence number, and should not be acked with the client.
    else
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
        if (sequence && player)
        {
            gMessageLib->sendCommandQueueRemove(sequence,0.0f,reply1,reply2,player);
        }
    }
}

//=============================================================================
//
// enqueue an internal created message (autoattack)
//
//

void ObjectController::enqueueAutoAttack(uint64 targetId)
{
    if (mCommandQueue.empty())
    {
        uint32 sequence = 0;
        uint32 opcode = opOCattack;

        CreatureObject* creature = dynamic_cast<CreatureObject*>(mObject);
        if (!creature)
        {
            assert(false && "ObjectController::enqueueAutoAttack mObject is not a CreatureObject");
        }

        uint32	reply1 = 0;
        uint32	reply2 = 0;

        ObjectControllerCmdProperties* cmdProperties = NULL;


        if (_validateEnqueueCommand(reply1,reply2,targetId,opcode,cmdProperties))
        {
            ObjControllerCommandMessage* cmdMsg = new(mCmdMsgPool.malloc()) ObjControllerCommandMessage(opcode, cmdProperties->mDefaultTime, targetId);
            cmdMsg->setSequence(sequence);
            cmdMsg->setData(NULL);
            cmdMsg->setCmdProperties(cmdProperties);

            // Are there any cooldown left from previous command?
            uint64 now = Anh_Utils::Clock::getSingleton()->getLocalTime();
            if (mNextCommandExecution <= now)
            {
                uint64 underrun = now - mNextCommandExecution;

                if (underrun <= mUnderrunTime)
                {
                    mUnderrunTime -= underrun;
                }
                else
                {
                    mUnderrunTime = 0;
                }

                // This command are to be handled as fast as possible.
                mNextCommandExecution = now;
            }

            // add it
            mCommandQueue.push_front(cmdMsg);

            // add us to the scheduler, if we aren't queued already
            if(!mTaskId)
            {
                mTaskId = gWorldManager->addObjControllerToProcess(this);
            }
        }
        // not qualified for this command
        else
        {
            PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
            if (player)
            {
                player->disableAutoAttack();
                DLOG(INFO) << "ObjectController::enqueueAutoAttack() Error adding command.";
            }
        }
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
    // please note that sometimes messages did not seem to be deleted
    // this *might* have been due to the command having had no sequence ???
    message->getUint32();
    message->setPendingDelete(true);

    // pass sequence
    removeMsgFromCommandQueueBySequence(message->getUint32());
}

//=============================================================================
//
// remove a cmd queue message by its sequence given
//
// Note by Eru:
// if a command already have been executed, you can not remove the cooldown for that command. And you should not expect to find the already executed command still in the queue.
// if a command has not ben exected yet (waiting in queue), there is no delay set for THAT command, thus no cooldown time to remove.
// Just remove the command!


void ObjectController::removeMsgFromCommandQueueBySequence(uint32 sequence)
{
    // sanity check
    if (!sequence)
    {
        DLOG(INFO) << "ObjectController::removeMsgFromCommandQueueBySequence No sequence!!!!";
        return;
    }

    CommandQueue::iterator cmdIt = mCommandQueue.begin();

    while(cmdIt != mCommandQueue.end())
    {
        if((*cmdIt)->getSequence() == sequence)
        {
            ObjControllerCommandMessage* msg = (*cmdIt);

            // delete it
            msg->~ObjControllerCommandMessage();
            mCmdMsgPool.free(msg);

            mCommandQueue.erase(cmdIt);
            break;
        }
        ++cmdIt;
    }
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
        PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
        if(this->getCommandQueue()->size() >= COMMAND_QUEUE_MAX_SIZE)
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("client", "too_many_commands_queued_generic"),player);
            return(false);
        }
        if(!((*it)->validate(reply1,reply2,targetId,opcode,cmdProperties)))
        {
            if(opcode == opOCRequestCraftingSession)
            {
                gMessageLib->sendCraftAcknowledge(opCraftCancelResponse,0,0,player);
            }
            // we still failed the check but we're not sending anything back
            // send this generic message
            if((reply1 == 0 && reply2 == 0))
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
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
            PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
            if(opcode == opOCRequestCraftingSession)
            {
                gMessageLib->sendCraftAcknowledge(opCraftCancelResponse,0,0,player);
            }
            // we still failed the check but we're not sending anything back
            // send this generic message
            if(! (reply1 && reply2) )
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "wrong_state"), player);
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
// http://wiki.swganh.org/index.php/CommandQueueRemove_(00000117)

void ObjectController::initEnqueueValidators()
{
    //mEnqueueValidators.push_back(new EVQueueSize(this));
    mEnqueueValidators.push_back(new EVCmdProperty(this));

    switch(mObject->getType())
    {
    case ObjType_Player:
    {
        //mEnqueueValidators.push_back(new EVSurveySample(this));
    }
    case ObjType_NPC:
    case ObjType_Creature:
    {
        mEnqueueValidators.push_back(new EVPosture(this));
        mEnqueueValidators.push_back(new EVAbility(this));
        mEnqueueValidators.push_back(new EVState(this));
        mEnqueueValidators.push_back(new EVWeapon(this));
        //mEnqueueValidators.push_back(new EVTarget(this));
    }
    break;

    default:
        break;
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

    default:
        break;
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
//=============================================================================
uint32	ObjectController::getLowestCommonBit(uint64 playerMask, uint64 cmdPropertiesMask)
{
    // checks each bit and returns the value
    bool bFound = false;
    uint32 i = 0;
    for (; i < 64 && !bFound; ++i) {
        bFound = (playerMask & (cmdPropertiesMask << i)) != 0;
    }
    if (bFound) {
        return i;
    }
    return 0;
}
//=============================================================================
uint32 ObjectController::getLocoValidator(uint64 locomotion)
{
    // this is needed because of how SOE does their locomotion validation message
    uint32 locoValidator = 0;
    switch(locomotion)
    {
        case CreatureLocomotion_Standing: locoValidator = kLocoValidStanding; break;
        case CreatureLocomotion_Sneaking: locoValidator = kLocoValidSneaking; break;
        case CreatureLocomotion_Walking: locoValidator = kLocoValidWalking; break;
        case CreatureLocomotion_Running: locoValidator = kLocoValidRunning; break;
        case CreatureLocomotion_Kneeling: locoValidator = kLocoValidKneeling; break;
        case CreatureLocomotion_CrouchSneaking: locoValidator = kLocoValidCrouchWalking; break;
        case CreatureLocomotion_CrouchWalking: locoValidator = kLocoValidProne; break;
        case CreatureLocomotion_Prone: locoValidator = kLocoValidProne; break;
        case CreatureLocomotion_Crawling: locoValidator = kLocoValidCrawling; break;
        case CreatureLocomotion_ClimbingStationary: locoValidator = kLocoValidClimbingStationary; break;
        case CreatureLocomotion_Climbing: locoValidator = kLocoValidClimbing; break;
        case CreatureLocomotion_Hovering: locoValidator = kLocoValidHovering; break;
        case CreatureLocomotion_Flying: locoValidator = kLocoValidFlying; break;
        case CreatureLocomotion_LyingDown: locoValidator = kLocoValidLyingDown; break;
        case CreatureLocomotion_Sitting: locoValidator = kLocoValidSitting; break;
        case CreatureLocomotion_SkillAnimating: locoValidator = kLocoValidSkillAnimating; break;
        case CreatureLocomotion_DrivingVehicle: locoValidator = kLocoValidDrivingVehicle; break;
        case CreatureLocomotion_RidingCreature: locoValidator = kLocoValidRidingCreature; break;
        case CreatureLocomotion_KnockedDown: locoValidator = kLocoValidKnockedDown; break;
        case CreatureLocomotion_Incapacitated: locoValidator = kLocoValidIncapacitated; break;
        case CreatureLocomotion_Dead: locoValidator = kLocoValidDead; break;
        case CreatureLocomotion_Blocking: locoValidator = kLocoValidBlocking; break;
    }

    return locoValidator;
}

//======================================================================

uint32 ObjectController::getPostureValidator(uint64 posture)
{
	uint32 postureValidator = 0;
	switch(posture)
	{
		case CreaturePosture_Sitting: postureValidator = kLocoValidSitting; break;
		case CreaturePosture_Prone: postureValidator = kLocoValidProne; break;
		case CreaturePosture_Crouched: postureValidator = kLocoValidKneeling; break;
		case CreaturePosture_Upright: postureValidator = kLocoValidStanding; break;
	}
	return postureValidator;
}
