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


#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/FactoryCrate.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"



#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"
#include "NetworkManager/MessageOpcodes.h"

//======================================================================================================================
//
// Tangible Baselines Type 3
// contain: general information, name, customization, type, condition
//

bool MessageLib::sendBaselinesTYCF_3(FactoryCrate* crate,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    BString customName;
    BString NameFile;
    BString Name;

    TangibleObject* tO = crate->getLinkedObject();
    if(!tO)
    {
        customName = crate->getCustomName();
        NameFile = crate->getNameFile();
        Name = crate->getName();
    }
    else
    {
        customName = tO->getCustomName();
        NameFile = tO->getNameFile();
        Name = tO->getName();
    }


    customName.convert(BSTRType_Unicode16);

    mMessageFactory->StartMessage();


    mMessageFactory->addUint16(11);	//op count
    mMessageFactory->addFloat(1.0);//tangibleObject->getComplexity());
    mMessageFactory->addString(NameFile.getAnsi());
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addString(Name.getAnsi());
    mMessageFactory->addString(customName.getUnicode16());
    uint32 uses = 0;

    mMessageFactory->addUint32(1);//volume gives the volume taken up in the inventory!!!!!!!!
    //mMessageFactory->addString(crate->getCustomizationStr());
    mMessageFactory->addUint16(0);//crate customization
    mMessageFactory->addUint64(0);	// unknown list might be defender list
    mMessageFactory->addUint32(0);//crate->getTypeOptions());bitmask - insured etc

    if(crate->hasAttribute("factory_count"))
    {
        uses = crate->getAttribute<int>("factory_count");
    }

    mMessageFactory->addUint32(uses);//
    mMessageFactory->addUint32(0);
    mMessageFactory->addUint32(0);

    //1 when not moveable
    mMessageFactory->addUint8(0);	//
    mMessageFactory->addUint64(0);	//


    Message* data = mMessageFactory->EndMessage();


    mMessageFactory->StartMessage();

    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(crate->getId());
    mMessageFactory->addUint32(opFCYT);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(data->getSize());
    mMessageFactory->addData(data->getData(), data->getSize());

    data->setPendingDelete(true);

    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Crate Baselines Type 6
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_6( FactoryCrate*  crate,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* data;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(3);	// unknown
    mMessageFactory->addUint32(0);	// unknown
    //mMessageFactory->addString();//crate->getDetailFile());
    mMessageFactory->addUint32(0);	// unknown
    //mMessageFactory->addString(0);//crate->getNameFile());

    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown

    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown


    mMessageFactory->addUint8(0);	// unknown

    data = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(crate->getId());
    mMessageFactory->addUint32(opFCYT);
    mMessageFactory->addUint8(6);

    mMessageFactory->addUint32(data->getSize());
    mMessageFactory->addData(data->getData(), data->getSize());

    data->setPendingDelete(true);


    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}

//======================================================================================================================
//
// Tangible Baselines Type 8
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_8(FactoryCrate* crate,PlayerObject* targetObject)
{
    if(!(targetObject->isConnected()))
        return(false);

    Message* data;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(3);	// unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown

    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown

    data = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(crate->getId());
    mMessageFactory->addUint32(opFCYT);
    mMessageFactory->addUint8(8);

    mMessageFactory->addUint32(data->getSize());
    mMessageFactory->addData(data->getData(), data->getSize());

    data->setPendingDelete(true);


    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}



//======================================================================================================================
//
// Tangible Baselines Type 9
// contain: unknown
//

bool MessageLib::sendBaselinesTYCF_9(FactoryCrate* crate,PlayerObject* targetObject)

{
    if(!(targetObject->isConnected()))
        return(false);

    Message* data;

    mMessageFactory->StartMessage();

    mMessageFactory->addUint16(3);	// unknown
    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown

    mMessageFactory->addUint32(0);	// unknown
    mMessageFactory->addUint32(0);	// unknown

    data = mMessageFactory->EndMessage();

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opBaselinesMessage);
    mMessageFactory->addUint64(crate->getId());
    mMessageFactory->addUint32(opFCYT);
    mMessageFactory->addUint8(9);

    mMessageFactory->addUint32(data->getSize());
    mMessageFactory->addData(data->getData(), data->getSize());

    data->setPendingDelete(true);


    (targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

    return(true);
}


bool MessageLib::sendUpdateCrateContent(FactoryCrate* crate,PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return(false);


    uint32 amount = 0;
    if(crate->hasAttribute("factory_count"))
    {
        amount = crate->getAttribute<int>("factory_count");
    }


    Message* newMessage;

    mMessageFactory->StartMessage();
    mMessageFactory->addUint32(opDeltasMessage);
    mMessageFactory->addUint64(crate->getId());
    mMessageFactory->addUint32(opFCYT);
    mMessageFactory->addUint8(3);

    mMessageFactory->addUint32(8);
    mMessageFactory->addUint16(1);

    mMessageFactory->addUint16(7);
    mMessageFactory->addUint32(amount);

    newMessage = mMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelA(newMessage,playerObject->getAccountId(),CR_Client,5);

    return(true);
}
