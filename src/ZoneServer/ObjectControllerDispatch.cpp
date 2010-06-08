/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "ObjectControllerDispatch.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"

//======================================================================================================================

ObjectControllerDispatch::ObjectControllerDispatch(Database* database,MessageDispatch* dispatch) :
mDatabase(database),
mMessageDispatch(dispatch)
{
	mMessageDispatch->RegisterMessageCallback(opObjControllerMessage,this); 
	mMessageDispatch->RegisterMessageCallback(opObjectMenuSelection,this);
}

//======================================================================================================================

ObjectControllerDispatch::~ObjectControllerDispatch()
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
		gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Unhandled opcode %u",opcode);

	message->setPendingDelete(true);
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
						gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Unhandled Cmd(0x00000021) %x",subOp2);
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
						gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Unhandled Cmd(0x00000023) %x",subOp2);
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
						ObjController->handleSetTarget(message);
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

					case opResourceEmptyHopper:
						ObjController->handleResourceEmptyHopper(message);
						break;

					default:
						gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Unhandled Cmd(0x00000083) %x",subOp2);
					break;
				}
			}
			break;

			default:
				gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Unhandled Cmd(op1) %x %x",subOp1,subOp2);
			break;
		}
	}
	else
		gLogger->log(LogManager::DEBUG,"ObjectControllerDispatch: Couldn't find Object %"PRIu64"",objId);

	message->setPendingDelete(true);
}

//======================================================================================================================

void ObjectControllerDispatch::_dispatchObjectMenuSelect(Message* message,DispatchClient* client)
{
	uint64	objectId = message->getUint64();
	Object* object = gWorldManager->getObjectById(objectId);
	
	if(object != NULL)
		object->handleObjectMenuSelect(message->getUint8(),gWorldManager->getPlayerByAccId(client->getAccountId()));
	else
		gLogger->log(LogManager::DEBUG,"ObjController::handleRadialSelect: Object not found %"PRIu64"",objectId);

	message->setPendingDelete(true);
}

//======================================================================================================================


