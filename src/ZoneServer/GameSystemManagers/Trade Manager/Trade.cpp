/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer/GameSystemManagers/Trade Manager/Trade.h"

#include "Zoneserver/Objects/Bank.h"
#include "Zoneserver/Objects/Inventory.h"
#include "ZoneServer/Objects/Object/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/Objects/Tangible Object/TangibleEnums.h"
#include "Zoneserver/Objects/Wearable.h"
#include "ZoneServer/WorldManager.h"

#include "ZoneServer\Services\equipment\equipment_service.h"

#include "ZoneServer/GameSystemManagers/Container Manager/ContainerManager.h"

#include "MessageLib/MessageLib.h"
#include "anh/utils/Scheduler.h"

SkillTeachContainer::SkillTeachContainer()
{}


SkillTeachContainer::~SkillTeachContainer()
{
    mTradeSkills.clear();
}


void SkillTeachContainer::addSkill(uint32 nr, uint32 id)
{
    mTradeSkills.insert(
        std::make_pair(nr, gSkillManager->getSkillById(id))
    );
}


mySkillList* SkillTeachContainer::getList()
{
    return &mTradeSkills;
}

Skill* SkillTeachContainer::getEntry(uint32 nr)
{
    mySkillList::iterator it = mTradeSkills.find(nr);

    if(it != mTradeSkills.end())
    {
        return (*it).second;
    }

    return NULL;
}

Trade::Trade(PlayerObject* playerobject)
{
    mPlayerObject=playerobject;
    mTradingFin=false;
    mAcceptTrade=false;
    mMoney=0;
    mteacher=NULL;
}
Trade::~Trade()
{
    TradeListDelete();
}

bool Trade::verifyInvitation(PlayerObject* player)
{
    PlayerObjectList::iterator it = mPlayerObjectList.begin();
    while(it != mPlayerObjectList.end())
    {
        if ((*it)->getId() == player->getId()) {
            return(true);
        }
        ++it;
    }
    return(false);
}
//=============================================================================
void Trade::tradeInvitationAdded(PlayerObject* inviter)
{
    //gets called by the inviter -> We (this) are the invited
    //check if already in
    if (verifyInvitation(inviter) == false)
    {
        mPlayerObjectList.push_back(inviter);
        TimerAsyncContainer* asyncContainer = new TimerAsyncContainer(Timer_CancelTradeInvitation,this);
        asyncContainer->mToBeRemoved = inviter;
        asyncContainer->mPlayer = this;
        //30 second timer
        (gWorldManager->getPlayerScheduler())->addTask(fastdelegate::MakeDelegate(this,&Trade::_handleCancelTradeInvitation),7,30000,asyncContainer);
    }

}
//=============================================================================
void Trade::deleteTradeInvitation(PlayerObject* player)
{
    //when we start trading - or the invitation expires
    //we must delete the now obsolete invitation from the list
    PlayerObjectList::iterator it = mPlayerObjectList.begin();
    while(it != mPlayerObjectList.end())
    {
        if(!(*it))
        {
            mPlayerObjectList.erase(it);
            it = mPlayerObjectList.begin();
        }
        else if ((*it)->getId()== player->getId()) {
            mPlayerObjectList.erase(it);
            break;
        }
        it++;
    }
}
//=============================================================================
bool Trade::_handleCancelTradeInvitation(uint64 callTime, void* ref)
{
    TimerAsyncContainer* asynContainer = (TimerAsyncContainer*)ref;

    //get the player who will be deleted from our invitation list
    PlayerObject* inviter = (PlayerObject*)asynContainer->mToBeRemoved;
    deleteTradeInvitation(inviter);

	SAFE_DELETE(asynContainer);
    return false;//the invitation is removed no need to execute this again
}

//=============================================================================

void Trade::cancelTradeSession()
{
    gMessageLib->sendAbortTradeMessage(getPlayerObject());
    TradeListDelete();
    getPlayerObject()->setTradePartner(0);
    getPlayerObject()->setTradeStatus(false);
    ItemTradeList::iterator it = mItemTradeList.begin();
    while(it != mItemTradeList.end())
    {
        mItemTradeList.erase(it);
        it = mItemTradeList.begin();
    }
    mMoney = 0;
    mTradingFin = false;
    mAcceptTrade = false;
}

//=============================================================================

void Trade::updateCash(int32 amount)
{
	getPlayerObject()->updateInventoryCredits(amount);
  
}

//=============================================================================

void Trade::updateBank(int32 amount)
{
    getPlayerObject()->updateBankCredits(amount);
 
}

//=============================================================================

void Trade::endTradeSession()
{
    gMessageLib->sendTradeCompleteMessage(getPlayerObject());
    TradeListDelete();
    getPlayerObject()->setTradePartner(0);
    getPlayerObject()->setTradeStatus(false);
    mMoney = 0;
    mTradingFin = false;
    mAcceptTrade = false;
}

//=============================================================================

bool Trade::checkTradeListtoInventory()
{
    auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto inventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(getPlayerObject(), "inventory"));

    if(!inventory)
    {
        DLOG(info) << "Trade::checkTradeListtoInventory():: No inventory for " << getPlayerObject()->getId();
        return(false);
    }

    if(!inventory->checkSlots(mItemTradeList.size()))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "inv_full"), getPlayerObject());
        return false;
    }

    return(true);
}

//=============================================================================

void  Trade::processTradeListPreTransaction(swganh::database::Transaction* mTransaction)
{
    //only process our list this will be called by both trade partners
    ItemTradeList::iterator it = mItemTradeList.begin();
    int8 sql[256];

    while(it != mItemTradeList.end())
    {

        //change Owner in the db via transaction

        switch((*it)->getObject()->getType())
        {
        case ObjType_Tangible:
        {
            TangibleObject* tangibleObject = ((*it)->getObject());

            switch(tangibleObject->getTangibleGroup())
            {
            case TanGroup_Item:
            {
                sprintf(sql,"UPDATE items SET parent_id = %"PRIu64" WHERE id = %"PRIu64"",(*it)->getNewOwner()->getId()+1,(*it)->getObject()->getId());
            }
            break;

            case TanGroup_ResourceContainer:
            {
                sprintf(sql,"UPDATE resource_containers SET parent_id = %"PRIu64" WHERE id = %"PRIu64"",(*it)->getNewOwner()->getId()+1,(*it)->getObject()->getId());
            }
            break;

            default:
                break;
            }
        }
        break;

        case ObjType_Waypoint:
        {
            sprintf(sql,"UPDATE waypoints SET parent_id = %"PRIu64" WHERE id = %"PRIu64"",(*it)->getNewOwner()->getId(),(*it)->getObject()->getId());
        }
        break;

        default:
            break;
        }

        mTransaction->addQuery(sql);


        ++it;
    }
}

//=============================================================================
//the trade has been approved and we iterate now through the items to be traded.
//they get removed out of their old owners inventory
//and will be created newly (via db) in the receivers inventory
//
void  Trade::processTradeListPostTransaction()
{
    //only process our list this will be called by both trade partners
    //The transaction has now been approved so we can do all the other stuff

    ItemTradeList::iterator it					= mItemTradeList.begin();

    //Tradepartners Inventory
    PlayerObject*			TradePartner		= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(getPlayerObject()->getTradePartner()));
    
	auto equip_service = gWorldManager->getKernel()->GetServiceManager()->GetService<swganh::equipment::EquipmentService>("EquipmentService");
	auto partnerInventory = dynamic_cast<Inventory*>(equip_service->GetEquippedObject(TradePartner, "inventory"));

    while(it != mItemTradeList.end())
    {
        uint64 itemId = (*it)->getObject()->getId();
        TangibleGroup tanGroup = (*it)->getObject()->getTangibleGroup();

        //assign the new owner to the item
        gObjectFactory->GiveNewOwnerInDB((*it)->getObject(),partnerInventory->getId());

        //get it out of its container
        TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)->getObject()->getParentId()));
        gContainerManager->removeObject((*it)->getObject(), container);
		gWorldManager->destroyObject((*it)->getObject());

        //create in our tradepartners Inventory
        if((*it)->getNewOwner() && (*it)->getNewOwner()->isConnected())
        {
            //this currently creates the item newly from db
            gObjectFactory->createIteminInventory(partnerInventory,itemId,tanGroup);
        }

        it = mItemTradeList.erase(it);
    }
}

//=======================================================================================================================

bool Trade::checkEquipped(Item* addedItem)
{
    //only players can trade
    PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(addedItem->getParentId()));
    if(player)
    {
        return false;
    }
    else
    {
        return true;
    }
}

//=============================================================================

bool Trade::ItemTradeCheck(uint64 ItemId)
{
    //returns false if the item which id is provided is not on the trade items list
    //otherwise it will return true
    ItemTradeList::iterator it = mItemTradeList.begin();

    while(it != mItemTradeList.end())
    {
        if ((*it)->getObject()->getId() == ItemId)
        {
            DLOG(info) << "PlayerObject: Item already on the tradeList";
            return(true);
        }

        ++it;
    }

    return(false);
}


//=============================================================================

void Trade::TradeListDelete()
{
    //returns false if the item which id is provided is not on the trade items list
    //otherwise it will return true
    ItemTradeList::iterator it = mItemTradeList.begin();

    while(it != mItemTradeList.end())
    {
        mItemTradeList.erase(it);
        it = mItemTradeList.begin();
    }
}

//=============================================================================

void Trade::ItemTradeAdd( TangibleObject* addedItem,PlayerObject* newOwner,PlayerObject* oldOwner)
{
    //make sure that the item is not already in our list before calling me
    TradeContainer* container = new TradeContainer();
    container->setObject(addedItem);
    container->setNewOwner(newOwner);
    container->setOldOwner(oldOwner);
    mItemTradeList.push_back(container);
}

//=============================================================================

