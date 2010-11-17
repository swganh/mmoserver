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

#include "Resource.h"
#include "PlayerObject.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"


//=============================================================================

Resource::Resource()
{
}

//=============================================================================

Resource::~Resource()
{
}

//=============================================================================


void Resource::sendAttributes(PlayerObject* playerObject)
{
    if(playerObject->getConnectionState() != PlayerConnState_Connected)
        return;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    uint32 attrCount = 0;

    for(uint8 i = 0; i < 11; i++)
    {
        if(mAttributes[i])
            attrCount++;
    }

    gMessageFactory->addUint32(attrCount);

    char	tmpStr[128];
    BString	tmpValueStr;

    for(uint8 i = 0; i < 11; i++)
    {
        uint16	attrValue = 0;
        BString	attrName;

        if((attrValue = mAttributes[i]) != 0)
        {
            switch(i)
            {
            case ResAttr_OQ:
                attrName = "res_quality";
                break;
            case ResAttr_CR:
                attrName = "res_cold_resist";
                break;
            case ResAttr_CD:
                attrName = "res_conductivity";
                break;
            case ResAttr_DR:
                attrName = "res_decay_resist";
                break;
            case ResAttr_HR:
                attrName = "res_heat_resist";
                break;
            case ResAttr_MA:
                attrName = "res_malleability";
                break;
            case ResAttr_SR:
                attrName = "res_shock_resistance";
                break;
            case ResAttr_UT:
                attrName = "res_toughness";
                break;
            case ResAttr_ER:
                attrName = "entangle_resistance";
                break;
            case ResAttr_PE:
                attrName = "res_potential_energy";
                break;
            case ResAttr_FL:
                attrName = "res_flavor";
                break;
            }

            sprintf(tmpStr,"%u",attrValue);
            tmpValueStr = tmpStr;
            tmpValueStr.convert(BSTRType_Unicode16);

            gMessageFactory->addString(attrName);
            gMessageFactory->addString(tmpValueStr);
        }
    }

    gMessageFactory->addUint32(0xffffffff);

    newMessage = gMessageFactory->EndMessage();

    //(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 9,false);
    (playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================

