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

#include "Scout.h"
#include "Camp.h"
#include "Inventory.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "ScoutManager.h"
#include "StructureManager.h"
#include "WorldManager.h"
#include "ContainerManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"


#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//=============================================================================

Scout::Scout() : Item()
{
}

//=============================================================================

Scout::~Scout()
{
}

//=============================================================================
//handles the radial selection

void Scout::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
    {
        switch(messageType)
        {
        case radId_itemUse:
        {
            //make sure it is a camp
            if(this->getItemType() >= ItemType_Camp_basic && this->getItemType() <= ItemType_Camp_quality)
            {
                //place camp TODO check whether camp can be placed ie whether we are in an urban area
                //ie create in the world for all known players
                //Camps use BUIO 3 and 6

                if(gScoutManager->createCamp(this->getItemType(),0,player->mPosition,"",player))
                {
                    TangibleObject* parentContainer = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(this->getParentId()));
                    gContainerManager->deleteObject(this, parentContainer);
                }
            }

        }
        }
    }
}

//=============================================================================

void Scout::sendAttributes(PlayerObject* playerObject)
{
    if(!(playerObject->isConnected()))
        return;

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    gMessageFactory->addUint32(1 + mAttributeMap.size());

    BString	value;
	wchar_t temp[20];
    swprintf(temp,20,L"%u/%u",mMaxCondition - mDamage,mMaxCondition);

    gMessageFactory->addString(BString("condition"));
    gMessageFactory->addString(temp);

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

void Scout::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

    RadialMenu* radial	= new RadialMenu();

    radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
    radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
    radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
    RadialMenuPtr radialPtr(radial);
    mRadialMenu = radialPtr;
}

