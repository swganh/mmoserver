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

#include "ZoneServer/IntangibleObject.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"



//======================================================================================================================
//
// Intangible Baselines Type 3
// contain: name,volume,custom name
//

bool MessageLib::sendBaselinesITNO_3(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;
    BString customName = intangibleObject->getCustomName().getAnsi();
    customName.convert(BSTRType_Unicode16);

    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(intangibleObject->getId());
    mMessageFactory->addUint32(opITNO);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(26 + (customName.getLength() << 1) + intangibleObject->getName().getLength() + intangibleObject->getNameFile().getLength());
    mMessageFactory->addUint16(5);	//opperand count
    mMessageFactory->addFloat(intangibleObject->getComplexity());

    mMessageFactory->addString(intangibleObject->getNameFile().getAnsi());
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addString(intangibleObject->getName().getAnsi());
    mMessageFactory->addString(customName);
    mMessageFactory->addUint32(intangibleObject->getVolume());
    mMessageFactory->addUint32(0); //GenericInt(CellID,SchematicQuantity)

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================
//
// Intangible Baselines Type 6
// contain: detail
//

bool MessageLib::sendBaselinesITNO_6(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(intangibleObject->getId());
    mMessageFactory->addUint32(opITNO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(14 + intangibleObject->getDetail().getLength() + intangibleObject->getDetailFile().getLength());
    mMessageFactory->addUint16(2);	//opperand count
    mMessageFactory->addUint32(74);	// unknown
    mMessageFactory->addString(intangibleObject->getDetailFile().getAnsi());
    mMessageFactory->addUint32(0); //spacer
    mMessageFactory->addString(intangibleObject->getDetail().getAnsi());


    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}

//======================================================================================================================
//
// Intangible Baselines Type 8
// contain: nothing!
//

bool MessageLib::sendBaselinesITNO_8(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(intangibleObject->getId());
    mMessageFactory->addUint32(opITNO);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(2); //Size
    mMessageFactory->addUint16(0);	//opperand count

    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}


//======================================================================================================================
//
// Intangible Baselines Type 9
// contain: nothing!
//

bool MessageLib::sendBaselinesITNO_9(IntangibleObject* intangibleObject,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* message;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(intangibleObject->getId());
    mMessageFactory->addUint32(opITNO);
    mMessageFactory->addUint8(9);
    mMessageFactory->addUint32(2); //Size
    mMessageFactory->addUint16(0);	//opperand count
    message = mMessageFactory->EndMessage();

    (targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

    return true;
}
