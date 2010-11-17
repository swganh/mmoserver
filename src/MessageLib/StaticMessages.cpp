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

#include "MessageLib.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/StaticObject.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/DispatchClient.h"

//======================================================================================================================
//
// Static Baselines type 3
// contain:
//

bool MessageLib::sendBaselinesSTAO_3(TangibleObject* staticObject, PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;
    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(staticObject->getId());
    mMessageFactory->addUint32(opSTAO);
    mMessageFactory->addUint8(3);
    mMessageFactory->addUint32(41);
    mMessageFactory->addUint32(4);
    mMessageFactory->addUint16(0);
    mMessageFactory->addString("obj_n");
    mMessageFactory->addUint32(0);
    mMessageFactory->addString("unknown_object");
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint16(0xFF);
    message = mMessageFactory->EndMessage();
    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Static Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesSTAO_6(TangibleObject* staticObject, PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(staticObject->getId());
    mMessageFactory->addUint32(opSTAO);
    mMessageFactory->addUint8(6);
    mMessageFactory->addUint32(29);
    mMessageFactory->addUint16(2);
    mMessageFactory->addUint32(0x00000072);
    mMessageFactory->addString("string_id_table");
    mMessageFactory->addUint32(0x00);
    mMessageFactory->addUint16(0x00);
    message = mMessageFactory->EndMessage();
    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
