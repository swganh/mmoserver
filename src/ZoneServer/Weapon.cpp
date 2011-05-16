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

#include "Weapon.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//=============================================================================

Weapon::Weapon() : Item(),
    mGroup(0)
{
}

//=============================================================================

Weapon::~Weapon()
{
}

//=============================================================================

void Weapon::sendAttributes(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    gMessageFactory->addUint32(1 + mAttributeMap.size());

    wchar_t tempValue[64];
    BString	value;

    swprintf(tempValue, 20, L"%u/%u",mMaxCondition - mDamage,mMaxCondition);

    gMessageFactory->addString(BString("condition"));
    gMessageFactory->addString(tempValue);

    AttributeMap::iterator			mapIt;
    AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

    while(orderIt != mAttributeOrderList.end())
    {
        mapIt = mAttributeMap.find(*orderIt);

        gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));

        value = (*mapIt).second.c_str();
        value.convert(BSTRType_Unicode16);

        gMessageFactory->addString(value);

        ++orderIt;
    }

    newMessage = gMessageFactory->EndMessage();

    (playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================


