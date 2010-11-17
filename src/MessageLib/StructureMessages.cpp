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

#include "ZoneServer/Deed.h"
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
// Building Baselines Type 3
// contain: name,
//


bool MessageLib::sendBaselinesBUIO_3(TangibleObject* structure,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(structure->getId());
    mMessageFactory->addUint32(opBUIO);
    mMessageFactory->addUint8(3);

    uint32 byteCount = 49 + structure->getNameFile().getLength() + structure->getName().getLength();
    mMessageFactory->addUint32(byteCount);
    mMessageFactory->addUint16(11);
    mMessageFactory->addFloat(1.0);
    mMessageFactory->addString(structure->getNameFile());
    mMessageFactory->addUint32(0);
    mMessageFactory->addString(structure->getName());
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0xFF);
    mMessageFactory->addUint16(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(256);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(1000);
    mMessageFactory->addUint8(1);

    newMessage = mMessageFactory->EndMessage();

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Building Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesBUIO_6(TangibleObject* structure,PlayerObject* player)
{
    if(!(player->isConnected()))
        return(false);

    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(structure->getId());
    mMessageFactory->addUint32(opBUIO);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(14);
    mMessageFactory->addUint16(2);	// unknown
    mMessageFactory->addUint32(66); // unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);

    newMessage = mMessageFactory->EndMessage();

    (player->getClient())->SendChannelA(newMessage, player->getAccountId(), CR_Client, 5);

    return(true);
}
