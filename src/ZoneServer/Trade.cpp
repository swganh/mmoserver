 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib/MessageLib.h"
#include "Trade.h"
#include "Wearable.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "TangibleEnums.h"
#include "Bank.h"
#include "ObjectFactory.h"

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
	//when we start trading - or the invitation exspires
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
		gLogger->logMsgF("Trade::checkTradeListtoInventory():: No inventory for %I64u",MSG_NORMAL,getPlayerObject()->getId());
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
		uint64 itemId = (*it)->getObject()->getId();
		TangibleGroup tanGroup = (*it)->getObject()->getTangibleGroup();
		
		//make sure the item is not equipped
		if (checkEquipped((Item*)(*it)->getObject()))
			((Wearable*)(*it)->getObject())->setInternalAttribute("equipped","false");


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
						sprintf(sql,"UPDATE items SET parent_id = %I64u WHERE id = %I64u",(*it)->getNewOwner()->getId()+1,(*it)->getObject()->getId());
					}
					break;

					case TanGroup_ResourceContainer:
					{
						sprintf(sql,"UPDATE resource_containers SET parent_id = %I64u WHERE id = %I64u",(*it)->getNewOwner()->getId()+1,(*it)->getObject()->getId());
					}
					break;

					default:break;
				}
			}
			break;

			case ObjType_Waypoint:
			{
				sprintf(sql,"UPDATE waypoints SET parent_id = %I64u WHERE id = %I64u",(*it)->getNewOwner()->getId(),(*it)->getObject()->getId());
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

	ItemTradeList::iterator it			= mItemTradeList.begin();
	Inventory*				inventory	= dynamic_cast<Inventory*>(getPlayerObject()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

	while(it != mItemTradeList.end())
	{
		uint64 itemId = (*it)->getObject()->getId();
		TangibleGroup tanGroup = (*it)->getObject()->getTangibleGroup();

		//delete out of our inventory
		gMessageLib->sendDestroyObject(itemId,getPlayerObject());
		inventory->deleteObject((*it)->getObject());		

		//need to access new Owner over id to prevent problems with outlogging players
		//or we need to put the logtime higher so that might be unnecessary after all

		//create in our tradepartners Inventory
		if((*it)->getNewOwner() && (*it)->getNewOwner()->isConnected())
		{
			gObjectFactory->createIteminInventory(dynamic_cast<Inventory*>((*it)->getNewOwner()->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),itemId,tanGroup);
		}
			
		it = mItemTradeList.erase(it);
	}
}

//=======================================================================================================================

bool Trade::checkEquipped(Item* addedItem)
{
	if(addedItem->getItemFamily() == ItemFamily_Wearable)
	{
		Wearable* weareableObject = (Wearable*)addedItem;

		return(weareableObject->getInternalAttribute<bool>("equipped"));
	}
	else
	{
		return false;
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
			gLogger->logMsgF("PlayerObject: Item already on the tradeList",MSG_HIGH);
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

