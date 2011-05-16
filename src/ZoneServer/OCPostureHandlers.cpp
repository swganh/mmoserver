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

#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "StateManager.h"
#include "CellObject.h"
#include "WorldManager.h"
#include "NetworkManager/Message.h"

//=============================================================================
//
// sit
//

void ObjectController::_handleSitServer(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{    
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

    if(playerObject)
    {
        BString			data;
        glm::vec3       chair_position;
        uint64			chairCell		= 0;
        uint32			elementCount	= 0;

        if(playerObject->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
        {
            playerObject->togglePlayerFlagOff(PlayerCustomFlag_LogOut);	
            gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), playerObject);
        }

        // see if we need to get out of sampling mode
        if(playerObject->getSamplingState())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), playerObject);
            playerObject->setSamplingState(false);
        }
        
        message->getStringUnicode16(data); //Should be okay even if data is null! (I hope)  

        // sitting on chair
        if(data.getLength())
        {
            elementCount = swscanf(data.getUnicode16(), L"%f,%f,%f,%"WidePRIu64, &chair_position.x, &chair_position.y, &chair_position.z, &chairCell);

            if(elementCount == 4)
            {
                // outside
                playerObject->updatePosition(chairCell,chair_position);
			
			//this->mDirection = Anh_Math::Quaternion();

			if(chairCell)
			{
				gMessageLib->sendDataTransformWithParent053(playerObject);
			}
			else
			{
				gMessageLib->sendDataTransform053(playerObject);
			}

			//gMessageLib->sendUpdateMovementProperties(playerObject);
			//gMessageLib->sendPostureAndStateUpdate(playerObject);

			gMessageLib->sendSitOnObject(playerObject);
            }
        }
        // sitting on ground
        else
        {
            //gMessageLib->sendPostureUpdate(playerObject);
            //gMessageLib->sendSelfPostureUpdate(playerObject);
        }
        gStateManager.setCurrentPostureState(playerObject, CreaturePosture_Sitting);
    }
}

//=============================================================================
//
// stand
//

void ObjectController::_handleStand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

    if(playerObject)
        gStateManager.setCurrentPostureState(playerObject, CreaturePosture_Upright);
}

//=============================================================================
//
// prone
//

void ObjectController::_handleProne(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);

    if(playerObject)
        gStateManager.setCurrentPostureState(playerObject, CreaturePosture_Prone);
}

//=============================================================================
//
// kneel
//

void ObjectController::_handleKneel(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*	playerObject = dynamic_cast<PlayerObject*>(mObject);

    if(playerObject)
		
		gStateManager.setCurrentPostureState(playerObject, CreaturePosture_Crouched);
}

//=============================================================================



