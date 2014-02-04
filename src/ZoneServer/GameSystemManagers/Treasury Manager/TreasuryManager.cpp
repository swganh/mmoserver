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

#include "TreasuryManager.h"
#include "Zoneserver/Objects/Bank.h"
#include "Zoneserver/Objects/BankTerminal.h"
#include "Zoneserver/Objects/Inventory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"


//======================================================================================================================

bool TreasuryManager::mInsFlag = false;
TreasuryManager* TreasuryManager::mSingleton = NULL;

//======================================================================================================================

TreasuryManager::TreasuryManager(swganh::database::Database* database) :
    mDatabase(database)
{

}

//======================================================================================================================

TreasuryManager* TreasuryManager::Init(swganh::database::Database* database)
{
    if(mInsFlag == false)
    {
        mSingleton = new TreasuryManager(database);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;
}

//======================================================================================================================

TreasuryManager::~TreasuryManager()
{
    mInsFlag = false;
    delete(mSingleton);
}

//======================================================================================================================

std::shared_ptr<RadialMenu> TreasuryManager::bankBuildTerminalRadialMenu(CreatureObject* creatureObject)
{
    std::shared_ptr<RadialMenu> radial(new RadialMenu());

    Bank*		bank	= dynamic_cast<Bank*>(creatureObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

    radial->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    radial->addItem(2,1,radId_bankTransfer,radAction_ObjCallback,"@sui:bank_credits");

    // case its our bank
    if(static_cast<uint32>(bank->getPlanet()) == gWorldManager->getZoneId())
    {
        radial->addItem(3,1,radId_bankItems,radAction_ObjCallback,"@sui:bank_items");
        radial->addItem(4,1,radId_bankQuit,radAction_ObjCallback,"@sui:bank_quit");
        radial->addItem(5,1,radId_bankWithdrawAll,radAction_ObjCallback,"@sui:bank_withdrawall");
        radial->addItem(6,1,radId_bankDepositAll,radAction_ObjCallback,"@sui:bank_depositall");
    }

    // case we have no binded bank
    // Do not allow to join bank in tutorial, player will never be able to quit that bank when he/she has left tutorial.
    else if ((bank->getPlanet() < 0) && (!gWorldConfig->isTutorial()))
    {
        radial->addItem(3,1,radId_bankJoin,radAction_ObjCallback,"@sui:bank_join");
        radial->addItem(4,1,radId_bankWithdrawAll,radAction_ObjCallback,"@sui:bank_withdrawall");
        radial->addItem(5,1,radId_bankDepositAll,radAction_ObjCallback,"@sui:bank_depositall");
    }

    // case its not our bank
    else
    {
        radial->addItem(3,1,radId_bankWithdrawAll,radAction_ObjCallback,"@sui:bank_withdrawall");
        radial->addItem(4,1,radId_bankDepositAll,radAction_ObjCallback,"@sui:bank_depositall");
    }

    return radial;
}

//======================================================================================================================

void TreasuryManager::bankDepositAll(PlayerObject* playerObject)
{
    if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
    {
        if(Bank* bank	= dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
        {
            int32 credits = inventory->getCredits();
            if(credits > 0)
            {
                // bank credits = bank + inventory.
                // inventory = 0
                bank->updateCredits(credits);
                inventory->setCredits(0);
                
                gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_deposit_success", 0, 0, 0, credits), playerObject);
            }
            else
            {
                //There has been an error during an attempt to deposit funds to your bank account. Verify you have sufficient funds for the desired transaction.
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "bank_deposit"), playerObject);
            }
        }
    }
}

//======================================================================================================================

void TreasuryManager::bankWithdrawAll(PlayerObject* playerObject)
{
	Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    if(! bank)	{
		LOG (error) << "TreasuryManager::bankWithdrawAll No Bank Object for " << playerObject->getId();
		return;
	}
    
	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    if(!inventory)	{
		LOG (error) << "TreasuryManager::bankWithdrawAll No Inventory Object for " << playerObject->getId();
		return;
	}
            
	if(!bank->getCredits())            {
		//There has been an error during an attempt to withdraw funds from your bank account. Verify you have sufficient funds for the desired transaction.
        gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "bank_withdraw"), playerObject);
		return;
	}

    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_withdraw_success", 0, 0, 0, bank->getCredits()), playerObject);

    inventory->updateCredits(bank->getCredits());
    bank->setCredits(0);
}

//======================================================================================================================

void TreasuryManager::bankTransfer(int32 inventoryMoneyDelta, int32 bankMoneyDelta, PlayerObject* playerObject)
{
	Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    if(! bank)	{
		LOG (error) << "TreasuryManager::bankWithdrawAll No Bank Object for " << playerObject->getId();
		return;
	}
    
	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    if(!inventory)	{
		LOG (error) << "TreasuryManager::bankWithdrawAll No Inventory Object for " << playerObject->getId();
		return;
	}
    

    // do we deposit or withdraw?
    if(bankMoneyDelta > 0)
    {
        // we deposit

        // update the deposit amount
        // in case our inventory cash
        // changed since the window popped
        bankMoneyDelta -= (bankMoneyDelta + inventoryMoneyDelta);

        // do we still have enough money?
        if(bankMoneyDelta < 1)
        {
            // transaction error
            // - we dont have enough money
            // - deposit equals zero
            return;
        }

        // ok, update the treasury
        inventory->setCredits(inventory->getCredits() - bankMoneyDelta);
        bank->updateCredits(bankMoneyDelta);
        
        // system message
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_deposit_success", 0, 0, 0, bankMoneyDelta), playerObject);

    }
    else
    {
        // we withdraw

        // update the withdraw amount
        // in case our bank cash
        // changed since the window popped
        inventoryMoneyDelta -= (bankMoneyDelta + inventoryMoneyDelta);

        // do we still have enough money?
        if(inventoryMoneyDelta < 1)
        {
            // transaction error
            // - we dont have enough money
            // - withdraw equals zero
            return;
        }

        // ok, update the treasury
        
        bank->updateCredits(-inventoryMoneyDelta);
        inventory->updateCredits(inventoryMoneyDelta);

        // system message
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_withdraw_success", 0, 0, 0, inventoryMoneyDelta), playerObject);
    }

}

//======================================================================================================================

void TreasuryManager::bankOpenSafetyDepositContainer(PlayerObject* playerObject)
{
}

//======================================================================================================================

void TreasuryManager::bankQuit(PlayerObject* playerObject)
{
    if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
    {
        // check if the player is really binded to this bank
        if(static_cast<uint32>(bank->getPlanet()) != gWorldManager->getZoneId())
        {
            gMessageLib->SendSystemMessage(L"You are not a member of this bank.", playerObject);
            return;
        }

        // check if the bank item box is empty

        // update the playerObject
        bank->setPlanet(-1);

        //This message has a period added to the end as it was missing from client.
        gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "succesfully_quit_bank"), playerObject);
    }
}

//======================================================================================================================

void TreasuryManager::bankJoin(PlayerObject* playerObject)
{
	Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    if(!bank)    {
		LOG (error) << "TreasuryManager::bankJoin No bank for " << playerObject->getId();
		return;
	}

	// check if we're not already binded here
    if(static_cast<uint32>(bank->getPlanet()) == gWorldManager->getZoneId())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "already_member_of_bank"), playerObject);
        return;
    }

    // check if we are not binded to any other bank
    if(bank->getPlanet() >= 0)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "member_of_different_bank"), playerObject);
        return;
    }

    bank->setPlanet((int8)gWorldManager->getZoneId());

    //This message period added at the end as its missing from client.
    gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "succesfully_joined_bank"), playerObject);

}

//======================================================================================================================

void TreasuryManager::bankTipOffline(uint32 amount,PlayerObject* playerObject,BString targetName)
{

    //============================================
    //check whether we have sufficient funds
    //dont forget the surcharge
    Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    int32 credits = bank->getCredits();

    int32 surcharge = (int32)((amount/100)*5);

    if((amount + surcharge) > credits)
    {
        BString uniName = targetName;
        uniName.convert(BSTRType_Unicode16);

        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_nsf_bank", L"", L"", uniName.getUnicode16(), amount), playerObject);
        return;
    }
    //now get the player
    int8 name[50];
    mDatabase->escapeString(name,targetName.getAnsi(),targetName.getLength());

    int8 sql[256];
    sprintf(sql,"SELECT id FROM %s.characters WHERE firstname like '%s'",mDatabase->galaxy(),name);

    TreasuryManagerAsyncContainer* asyncContainer;
    asyncContainer = new TreasuryManagerAsyncContainer(TREMQuery_BankTipgetId,playerObject->getClient());
    asyncContainer->amount = amount;
    asyncContainer->surcharge = surcharge;
    asyncContainer->targetName = targetName;
    asyncContainer->player = playerObject;

    mDatabase->executeSqlAsync(this,asyncContainer,sql);
}

//======================================================================================================================

void TreasuryManager::bankTipOnline(uint32 amount, PlayerObject* playerObject, PlayerObject* targetObject )
{
    //check if we have enough money
    uint32 surcharge = (uint32)((amount/100)*5);

    if((amount+surcharge) > dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits())
    {
        std::string s;
        s = targetObject->getFirstName();
		std::wstring s_w(s.begin(), s.end());
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_nsf_cash", L"", s_w, L"", amount), playerObject);
        return;
    }

    Bank* playerBank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    Bank* targetBank = dynamic_cast<Bank*>(targetObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

    playerBank->updateCredits(-(amount+surcharge));
    targetBank->updateCredits(amount);

    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_self", 0, targetObject->getId(), 0, amount), playerObject);
    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_target", 0, playerObject->getId(), 0, amount), targetObject);

    gMessageLib->sendBanktipMail(playerObject,targetObject,amount);
}

//======================================================================================================================

void TreasuryManager::inventoryTipOnline(uint32 amount, PlayerObject* playerObject,PlayerObject* targetObject )
{
    if(!targetObject)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "tip_error"), playerObject);
        return;

    }

    //check if we have enough money
    if(amount > dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits())
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_nsf_cash", 0, targetObject->getId(), 0, amount), playerObject);
        return;
    }

    if( glm::distance(playerObject->mPosition, targetObject->mPosition) > 10.0)
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_range", 0, targetObject->getId(), 0, amount), playerObject);
        return;
    }

    Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    Inventory* targetInventory = dynamic_cast<Inventory*>(targetObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    playerInventory->setCredits(playerInventory->getCredits() - amount);
    targetInventory->setCredits(targetInventory->getCredits() + amount);

    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_self", 0, targetObject->getId(), 0, amount), playerObject);
    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_target", 0, playerObject->getId(), 0, amount), targetObject);
}

//======================================================================================================================
//we confirmed that we are content with the surcharge and go on
//reuse the asynccontainer
void TreasuryManager::handleBankTipSurchargeConfirmed(std::shared_ptr<TreasuryManagerAsyncContainer> container )
{
	TreasuryManagerAsyncContainer* cnt = new TreasuryManagerAsyncContainer(container->mQueryType, container->mClient);
	cnt->amount		= container->amount;
	cnt->player		= container->player;
	cnt->PlayerId	= container->PlayerId;
	cnt->surcharge	= container->surcharge;
	cnt->target		= container->target;
	cnt->targetId	= container->targetId;
	cnt->targetName	= container->targetName;
	
	
    swganh::database::Transaction* mTransaction = mDatabase->startTransaction(this,container.get());
    
	std::stringstream sql;

	sql << "UPDATE " << mDatabase->galaxy() 
		<< ".banks SET credits=credits-" << (container->amount + container->surcharge)
		<< " WHERE id= " << container->player->getId() + BANK_OFFSET;
    mTransaction->addQuery(sql.str().c_str());

	sql.str() = std::string("");
	sql << "UPDATE " << mDatabase->galaxy() 
		<< ".banks SET credits=credits+" << (container->amount)
		<< " WHERE id= " << container->targetId + BANK_OFFSET;

	mTransaction->addQuery(sql.str().c_str());
    mTransaction->execute();

}

//======================================================================================================================

void TreasuryManager::handleUIEvent(uint32 action,int32 element,std::u16string inputStr,UIWindow* window)
{
    // gLogger->logMsgF("CloningTerminal::handleUIEvent You are here!",MSG_NORMAL);

    if(window == NULL)
    {
        return;
    }

    PlayerObject* playerObject = window->getOwner(); // window owner

    if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->states.checkState(CreatureState_Combat))
    {
        return;
    }

    switch(window->getWindowType())
    {
    case SUI_Window_Trade_BankTip_ConfirmSurcharge:
    {
        if (action == 1)
        {
            // That's the Cancel.
            //what
        }
        else
        {
            // This is the OK.  (action == 0)
			std::shared_ptr<TreasuryManagerAsyncContainer> container = std::static_pointer_cast<TreasuryManagerAsyncContainer>(window->getAsyncContainer());
           
            handleBankTipSurchargeConfirmed(container);
        }
    }
    break;
    }
}


//======================================================================================================================

void TreasuryManager::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{
    TreasuryManagerAsyncContainer* asynContainer = (TreasuryManagerAsyncContainer*)ref;


    switch(asynContainer->mQueryType)
    {

    case TREMQuery_BankTipgetId:
    {
        if(!result->getRowCount())
        {
            gMessageLib->SendSystemMessage(L"You may only /tip or /tip bank to other players.", asynContainer->player);
            return;
        }

        uint64 id;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint64,0,8);
        result->getNextRow(binding,&id);

        //ok we just established that our target exists
        //we now need to update the bank on the db side
		//uint32 enum_ = TREMQuery_BankTipTransaction;
		std::shared_ptr<TreasuryManagerAsyncContainer> container = std::make_shared<TreasuryManagerAsyncContainer>(TREMQuery_BankTipTransaction, asynContainer->player->getClient());
        
        container->amount		= asynContainer->amount;
        container->surcharge	= asynContainer->surcharge;
        container->player		= asynContainer->player;
        container->targetId		= id;
        container->targetName	= asynContainer->targetName;

        //=======================================================
        //now remind the other player of the surcharge
        gUIManager->createNewMessageBox(this,"","@base_player:tip_wire_title","@base_player:tip_wire_prompt",asynContainer->player, SUI_Window_Trade_BankTip_ConfirmSurcharge, SUI_MB_OKCANCEL,container);

        break;


    }
    break;

    case TREMQuery_BankTipTransaction:
    {
        uint32 error;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);
        result->getNextRow(binding,&error);

        if (error == 0)
        {
            Bank* bank = dynamic_cast<Bank*>(asynContainer->player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

            //update our own bankaccount
            bank->updateCredits((0-asynContainer->amount));

            gMessageLib->sendBankTipDustOff(asynContainer->player,asynContainer->targetId,asynContainer->amount,asynContainer->targetName);
            //notify the chatserver for the EMails and the off zone accounts

            int8 sql[1024];

            //CAVE galaxy id is hardcoded!!!!!!1
            int8 galaxyId = 2;

            sprintf(sql, "CALL %s.sp_GalaxyAccountDepositCredits(%u, %u, %"PRIu64",%u", mDatabase->galaxy(), galaxyId, Account_TipSurcharge, asynContainer->player->getId(), asynContainer->surcharge);
            TreasuryManagerAsyncContainer* asyncContainer = new TreasuryManagerAsyncContainer(TREMQuery_BankTipUpdateGalaxyAccount,0);

            mDatabase->executeProcedureAsync(this,asyncContainer,sql);

        }
        else
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "bank_error"), asynContainer->player);
        }
    }
    break;

    case TREMQuery_BankTipUpdateGalaxyAccount:
    {
        uint32 error;
        swganh::database::DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(swganh::database::DFT_uint32,0,4);
        result->getNextRow(binding,&error);

        if (error > 0)
        {
            DLOG(info) << "TreasuryManager::Account_TipSurcharge: error " << error;
        }
    }
    break;
    case TREMQuery_NULL:
    {

    }
    break;

    default:
        break;
    }

    delete(asynContainer);
}

//======================================================================================================================


