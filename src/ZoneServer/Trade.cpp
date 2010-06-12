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

#include "Trade.h"

#include "Bank.h"
#include "Inventory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "TangibleEnums.h"
#include "Wearable.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "Utils/Scheduler.h"

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
		if ((*it)->getPlayerObjId() == player->getPlayerObjId()){
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
		else
		if ((*it)->getPlayerObjId()== player->getPlayerObjId()){
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

	return false;//the invitation is removed no need to execute this again
}

//=============================================================================

void Trade::cancelTradeSession()
{
	gMessageLib->sendAbortTradeMessage(getPlayerObject());
	TradeListDelete();
	getPlayerObject()->setTradePartner(NULL);
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

void Trade::updateCash(uint32 amount)
{
	Inventory* inventory = dynamic_cast<Inventory*>(getPlayerObject()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	inventory->setCredits(inventory->getCredits() + amount);

	gMessageLib->sendInventoryCreditsUpdate(getPlayerObject());
}

//=============================================================================

void Trade::updateBank(uint32 amount)
{
	Bank* bank = dynamic_cast<Bank*>(getPlayerObject()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

	bank->setCredits(bank->getCredits() + amount);

	gMessageLib->sendBankCreditsUpdate(getPlayerObject());
}

//=============================================================================

void Trade::endTradeSession()
{
	gMessageLib->sendTradeCompleteMessage(getPlayerObject());
	TradeListDelete();
	getPlayerObject()->setTradePartner(NULL);
	getPlayerObject()->setTradeStatus(false);
	mMoney = 0;
	mTradingFin = false;
	mAcceptTrade = false;
}

//=============================================================================

bool Trade::checkTradeListtoInventory()
{
	ItemTradeList::iterator it			= mItemTradeList.begin();
	Inventory*				inventory	= dynamic_cast<Inventory*>(getPlayerObject()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	if(!inventory)
	{
		gLogger->log(LogManager::DEBUG,"Trade::checkTradeListtoInventory():: No inventory for %I64u",getPlayerObject()->getId());
		return(false);
	}

	if(!inventory->checkSlots(mItemTradeList.size()))
	{
		gMessageLib->sendSystemMessage(getPlayerObject(),L"","error_message","inv_full");
		return false;
	}

	return(true);
}

//=============================================================================

void  Trade::processTradeListPreTransaction(Transaction* mTransaction)
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

					default:break;
				}
			}
			break;

			case ObjType_Waypoint:
			{
				sprintf(sql,"UPDATE waypoints SET parent_id = %"PRIu64" WHERE id = %"PRIu64"",(*it)->getNewOwner()->getId(),(*it)->getObject()->getId());
			}
			break;

			default:break;
		}

		mTransaction->addQuery(sql);

		++it;
	}
}

//=============================================================================

void  Trade::processTradeListPostTransaction()
{
	//only process our list this will be called by both trade partners
	//The transaction has now been approved so we can do all the other stuff

	ItemTradeList::iterator it					= mItemTradeList.begin();
	Inventory*				inventory			= dynamic_cast<Inventory*>(getPlayerObject()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	//Tradepartners Inventory
	PlayerObject*			TradePartner		= dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(getPlayerObject()->getTradePartner()));
	Inventory*				partnerInventory	= dynamic_cast<Inventory*>(TradePartner->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	while(it != mItemTradeList.end())
	{
		uint64 itemId = (*it)->getObject()->getId();
		TangibleGroup tanGroup = (*it)->getObject()->getTangibleGroup();

		//delete out of our inventory / backpack
		gMessageLib->sendDestroyObject(itemId,getPlayerObject());		

		//assign the Bazaar as the new owner to the item
		gObjectFactory->GiveNewOwnerInDB((*it)->getObject(),partnerInventory->getId());

		//the item could be in a backpack or in a different container - get it out
		TangibleObject* container = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)->getObject()->getParentId()));
		
		if(!container)
		{
			//please note, that the worldmanagers ObjectList does not contain inventories.
			//equipped items should be unequipped by this time
			container = inventory;
		}
		
		container->deleteObject((*it)->getObject());

		//create in our tradepartners Inventory
		if((*it)->getNewOwner() && (*it)->getNewOwner()->isConnected())
		{
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
			gLogger->log(LogManager::DEBUG,"PlayerObject: Item already on the tradeList");
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

