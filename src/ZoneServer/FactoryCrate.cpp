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

#include "FactoryCrate.h"
#include "Inventory.h"
#include "Item_Enums.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "StructureManager.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//
#include "DatabaseManager/Database.h"
#include "Utils/colors.h"
#include <cassert>

//=============================================================================

FactoryCrate::FactoryCrate() : Item()
{
    mTempAmount = 0;
    mSlotCount = 0;
}

//=============================================================================

FactoryCrate::~FactoryCrate()
{
}

//=============================================================================
//handles the radial selection

void FactoryCrate::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
    {
        switch(messageType)
        {
        case radId_itemUse:
        {

        }

        }

    }
}

//=============================================================================

void FactoryCrate::sendAttributes(PlayerObject* playerObject)
{

    if(!(playerObject->isConnected()))
        return;

    AttributeMap*				iAttributeMap		= this->getLinkedObject()->getAttributeMap();
    AttributeOrderList*			iAttributeOrderList	= this->getLinkedObject()->getAttributeOrder();

    Message* newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opAttributeListMessage);
    gMessageFactory->addUint64(mId);

    gMessageFactory->addUint32(2 + mAttributeMap.size()+iAttributeMap->size());

    BString	tmpValueStr = BString(BSTRType_Unicode16,64);
    BString	value,aStr;

    tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),50,L"%u/%u",mMaxCondition - mDamage,mMaxCondition));

    gMessageFactory->addString(BString("condition"));
    gMessageFactory->addString(tmpValueStr);

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

    gMessageFactory->addString(BString("factory_attribs"));
    aStr = "\\#"SOE_RED" --------------";
    aStr.convert(BSTRType_Unicode16);
    gMessageFactory->addString(aStr);

    orderIt = iAttributeOrderList->begin();

    while(orderIt != iAttributeOrderList->end())
    {
        mapIt = iAttributeMap->find(*orderIt);

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

void FactoryCrate::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

    RadialMenu* radial	= new RadialMenu();

    radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
    radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
    radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
    RadialMenuPtr radialPtr(radial);
    mRadialMenu = radialPtr;
}


TangibleObject*	FactoryCrate::getLinkedObject()
{
    ObjectIDList*			ol = this->getObjects();
    ObjectIDList::iterator	it = ol->begin();

    //just get the first linked object - crates only have the one

    TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
    if(!tO)
    {
        assert(false && "FactoryCrate::getLinkedObject WorldManager did not return a valid TangibleObject");
        return NULL;
    }

    return tO;

}

int32 FactoryCrate::decreaseContent(uint32 amount)
{
    uint32 crateAmount = 0;
    if(this->hasAttribute("factory_count"))
    {
        crateAmount = this->getAttribute<int>("factory_count");
    }
    else
    {
        this->setAttributeIncDB("factory_count","1");
        crateAmount = 1;
    }

    int32 newAmount = crateAmount - amount;

    if(newAmount < 0)
    {
        //??? If we came in here and there was no attrib for this crate
        //....no matter what 'amount' is (unless it's 0) we're going to fail hardcore.
        //Lets try to recover from this DB error by returning 0.
        //assert(false);
        newAmount = 0;
    }

    this->setAttribute("factory_count",boost::lexical_cast<std::string>(newAmount));
    gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%"PRIu64" AND attribute_id=%u",newAmount,this->getId(),AttrType_factory_count);
    

    return newAmount;
}

//========================================================================================
//used by the factoryfactory to update hoppercontent when looking at a hopper
//
void FactoryCrate::upDateFactoryVolume(BString amount)
{
    if(!this->hasAttribute("factory_count"))
    {
        return;
    }

    std::string v = this->getAttribute<std::string>("factory_count");
    BString value = v.c_str();

    if(value.getCrc() == amount.getCrc())
    {
        return;
    }
    this->setAttribute("factory_count",amount.getAnsi());

    PlayerObjectSet*			knownPlayers	= this->getKnownPlayers();
    PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

    while(playerIt != knownPlayers->end())
    {
        PlayerObject* player = (*playerIt);
        if(player)
            gMessageLib->sendUpdateCrateContent(this,player);

        playerIt++;
    }

}


