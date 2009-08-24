/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ObjectControllerDispatch.h"
#include "ObjectFactory.h"
#include "WorldManager.h"
#include "ObjectControllerOpcodes.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "DatabaseManager/Database.h"

//======================================================================================================================

ObjectControllerDispatch::ObjectControllerDispatch()
{
}

//======================================================================================================================

ObjectControllerDispatch::~ObjectControllerDispatch()
{
}

//======================================================================================================================

void ObjectControllerDispatch::Startup(Database* database, MessageDispatch* dispatch)
{
	mDatabase = database;
	mMessageDispatch = dispatch;
	mMessageDispatch->RegisterMessageCallback(opObjControllerMessage,this); 
	mMessageDispatch->RegisterMessageCallback(opObjectMenuSelection,this);
}


//======================================================================================================================

void ObjectControllerDispatch::Shutdown()
{
	mMessageDispatch->UnregisterMessageCallback(opObjControllerMessage);  
	mMessageDispatch->UnregisterMessageCallback(opObjectMenuSelection);
}


//======================================================================================================================

void ObjectControllerDispatch::Process(void)
{

}


//======================================================================================================================

void ObjectControllerDispatch::handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client)
{
	if(opcode == opObjControllerMessage)
		_dispatchMessage(message,client);
	else if(opcode == opObjectMenuSelection)
		_dispatchObjectMenuSelect(message,client);

	else
		gLogger->logMsgF("ObjectControllerDispatch: Unhandled opcode %u\n",MSG_HIGH,opcode);
}

//======================================================================================================================

void ObjectControllerDispatch::_dispatchMessage(Message* message, DispatchClient* client)
{
	uint32 subOp1 = message->getUint32();
	uint32 subOp2 = message->getUint32();
	uint64 objId = message->getUint64();

	if(CreatureObject* object = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(objId)))
	{
		if(!object->getReady())
		{
			// gLogger->logMsgF("ObjectControllerDispatch: Object not ready %llu", MSG_HIGH, object->getId());
			return;
		}

		switch(subOp1)
		{
			case 0x00000021:
			{
				switch(subOp2)
				{
					case opDataTransform: 
					{
						ObjController->handleDataTransform(message,false);
					}
					break;

					case opDataTransformWithParent:
					{
						ObjController->handleDataTransformWithParent(message,false);
					}
					break;

					default:
						gLogger->logMsgF("ObjectControllerDispatch: Unhandled Cmd(0x00000021) %x\n",MSG_HIGH,subOp2);
					break;
				}
			}
			break;

			case 0x00000023:
			{
				switch(subOp2)
				{
					case opImageDesignChangeMessage:
					{
						ObjController->handleImageDesignChangeMessage(message,objId);
					}
					break;

					case opImageDesignStopMessage:
					{
						ObjController->handleImageDesignStopMessage(message,objId);
					}
					break;

					case opDataTransform:
					{
						ObjController->handleDataTransform(message,true);
					}
					break;

					case opDataTransformWithParent:
					{
						ObjController->handleDataTransformWithParent(message,true);
					}
					break;

					case opCommandQueueEnqueue: 
					{
						ObjController->enqueueCommandMessage(message);
					}
					break;

					case opCommandQueueRemove:
					{
						ObjController->removeCommandMessage(message);
					}
					break;

					case opObjectMenuRequest:
					{
						ObjController->handleObjectMenuRequest(message);
					}
					break;

					case opTeleportAck:
					{
						gMessageLib->sendHeartBeat(client);
					}
					break;

					case opSecureTrade:
					{
						ObjController->handleSecureTradeInvitation(objId,message);
					}
					break;

					default:
						gLogger->logMsgF("ObjectControllerDispatch: Unhandled Cmd(0x00000023) %x\n",MSG_HIGH,subOp2);
					break;
				}
			}
			break;

			case 0x00000083:
			{
				// skip ticks
				message->getUint32();

				switch(subOp2)
				{
					case opOCCurrentTarget:
					{
						ObjController->setTarget(message);
					}
					break;

					case opCraftFillSlot:
					{
						ObjController->handleCraftFillSlot(message);
					}
					break;

					case opCraftEmptySlot:
					{
						ObjController->handleCraftEmptySlot(message);
					}
					break;

					case opCraftExperiment:
					{
						ObjController->handleCraftExperiment(message);
					}
					break;

					case opCraftCustomization:
					{
						ObjController->handleCraftCustomization(message);
					}
					break;

                    case opMissionTerminalOpen:
					{
                        ObjController->handleMissionListRequest(message);
					}
                    break;

                    case opMissionDetailsRequest:
                        ObjController->handleMissionDetailsRequest(message);
                        break;

					case opGenericMissionRequest:
						ObjController->handleGenericMissionRequest(message);
						break;

                    case opMissionCreateRequest:
                        ObjController->handleMissionCreateRequest(message);
                        break;

					case opMissionAbort:
						ObjController->handleMissionAbort(message);
						break;

					default:
						gLogger->logMsgF("ObjectControllerDispatch: Unhandled Cmd(0x00000083) %x\n",MSG_HIGH,subOp2);
					break;
				}
			}
			break;

			default:
				gLogger->logMsgF("ObjectControllerDispatch: Unhandled Cmd(op1) %x %x\n",MSG_HIGH,subOp1,subOp2);
			break;
		}
	}
	else
		gLogger->logMsgF("ObjectControllerDispatch: Couldn't find Object %lld\n",MSG_HIGH,objId);
}

//======================================================================================================================

void ObjectControllerDispatch::_dispatchObjectMenuSelect(Message* message,DispatchClient* client)
{
	uint64	objectId = message->getUint64();
	Object* object = gWorldManager->getObjectById(objectId);
	
	if(object != NULL)
		object->handleObjectMenuSelect(message->getUint8(),gWorldManager->getPlayerByAccId(client->getAccountId()));
	else
		gLogger->logMsgF("ObjController::handleRadialSelect: Object not found %lld\n",MSG_HIGH,objectId);

}

//======================================================================================================================


