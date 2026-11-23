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

#include "TreasuryManager.h"
#include "Bank.h"
#include "BankTerminal.h"
#include "ContainerManager.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "TangibleFactory.h"
#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include <sstream>

#include <cppconn/resultset.h>


//======================================================================================================================

bool TreasuryManager::mInsFlag = false;
TreasuryManager* TreasuryManager::mSingleton = NULL;

//======================================================================================================================

TreasuryManager::TreasuryManager(Database* database) :
    mDatabase(database)
{

}

//======================================================================================================================

TreasuryManager* TreasuryManager::Init(Database* database)
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
    if(static_cast<uint32>(bank->planet()) == gWorldManager->getZoneId())
    {
        radial->addItem(3,1,radId_bankItems,radAction_ObjCallback,"@sui:bank_items");
        radial->addItem(4,1,radId_bankQuit,radAction_ObjCallback,"@sui:bank_quit");
        radial->addItem(5,1,radId_bankWithdrawAll,radAction_ObjCallback,"@sui:bank_withdrawall");
        radial->addItem(6,1,radId_bankDepositAll,radAction_ObjCallback,"@sui:bank_depositall");
    }

    // case we have no binded bank
    // Do not allow to join bank in tutorial, player will never be able to quit that bank when he/she has left tutorial.
    else if ((bank->planet() < 0) && (!gWorldConfig->isTutorial()))
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
                bank->credits(bank->credits() + credits);
                inventory->setCredits(0);
                // save to the db
                mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.banks SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),bank->credits(),bank->getId()));

                mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.inventories SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),inventory->getCredits(),inventory->getId()));


                //send the appropriate deltas.
                gMessageLib->sendInventoryCreditsUpdate(playerObject);
                gMessageLib->sendBankCreditsUpdate(playerObject);

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
    if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
    {
        if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
        {
            if(bank->credits() > 0)
            {
                gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_withdraw_success", 0, 0, 0, bank->credits()), playerObject);

                // inventory credits = bank + inventory.
                // bank = 0
                inventory->setCredits(inventory->getCredits() + bank->credits());
                bank->credits(0);

                // save to the db
                mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.banks SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),bank->credits(),bank->getId()));

                mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.inventories SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),inventory->getCredits(),inventory->getId()));


                //send the appropriate deltas.
                gMessageLib->sendInventoryCreditsUpdate(playerObject);
                gMessageLib->sendBankCreditsUpdate(playerObject);
            }
            else
            {
                //There has been an error during an attempt to withdraw funds from your bank account. Verify you have sufficient funds for the desired transaction.
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "bank_withdraw"), playerObject);
            }
        }
    }
}

//======================================================================================================================

void TreasuryManager::bankTransfer(int32 inventoryMoneyDelta, int32 bankMoneyDelta, PlayerObject* playerObject)
{
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
        if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
        {
            if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
            {
                inventory->setCredits(inventory->getCredits() - bankMoneyDelta);
                bank->credits(bank->credits() + bankMoneyDelta);
            }
        }

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
        if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
        {
            if(Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)))
            {
                bank->credits(bank->credits() - inventoryMoneyDelta);
                inventory->setCredits(inventory->getCredits() + inventoryMoneyDelta);

            }
        }

        // system message
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_withdraw_success", 0, 0, 0, inventoryMoneyDelta), playerObject);
    }

    // save to the db
    saveAndUpdateInventoryCredits(playerObject);
    saveAndUpdateBankCredits(playerObject);
}

//======================================================================================================================

// The bank's safe deposit box (KNOWN_ISSUES #27). The bank is a container
// the client already has (it is created with the other equipped objects at
// login, and its owner is registered to it), so opening it is: load what is
// in it the first time, then tell the client it is open. Items moved in or
// out go through the ordinary transfer, which saves the new parent.
void TreasuryManager::bankOpenSafetyDepositContainer(PlayerObject* playerObject)
{
    Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    if(!bank)
    {
        return;
    }

    // The box is kept at the bank you are a member of.
    if(static_cast<uint32>(bank->planet()) != gWorldManager->getZoneId())
    {
        gMessageLib->SendSystemMessage(L"You are not a member of this bank.", playerObject);
        return;
    }

    if(bank->getLoadState() == LoadState_Loaded)
    {
        _openLoadedSafetyDeposit(bank);
        return;
    }

    // Already loading: it opens when the last item arrives.
    if(mSafetyDepositLoads.find(bank->getId()) != mSafetyDepositLoads.end())
    {
        return;
    }
    mSafetyDepositLoads[bank->getId()] = 0;

    const uint64 bankId = bank->getId();
    std::stringstream sql;
    sql << "(SELECT 'containers', containers.id FROM " << mDatabase->galaxy() << ".containers INNER JOIN " << mDatabase->galaxy()
        << ".container_types ON (containers.container_type = container_types.id) WHERE container_types.name NOT LIKE 'unknown' AND containers.parent_id = " << bankId << ")"
        << " UNION (SELECT 'items', id FROM " << mDatabase->galaxy() << ".items WHERE parent_id = " << bankId << ")"
        << " UNION (SELECT 'resource_containers', id FROM " << mDatabase->galaxy() << ".resource_containers WHERE parent_id = " << bankId << ")";

    mDatabase->executeAsyncSql(sql, [this, bankId] (DatabaseResult* result) {
        std::vector<std::pair<uint16, uint64>> contents;
        if(result && result->getResultSet())
        {
            std::unique_ptr<sql::ResultSet>& rows = result->getResultSet();
            while(rows->next())
            {
                const std::string table = rows->getString(1);
                const uint16 group = (table == "items") ? TanGroup_Item
                                   : (table == "containers") ? TanGroup_Container : TanGroup_ResourceContainer;
                contents.push_back(std::make_pair(group, rows->getUInt64(2)));
            }
        }

        Bank* bank = dynamic_cast<Bank*>(gWorldManager->getObjectById(bankId));
        if(!bank)
        {
            // The owner has gone.
            mSafetyDepositLoads.erase(bankId);
            return;
        }

        if(contents.empty())
        {
            mSafetyDepositLoads.erase(bankId);
            bank->setLoadState(LoadState_Loaded);
            _openLoadedSafetyDeposit(bank);
            return;
        }

        PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(bank->getParentId()));
        DispatchClient* client = owner ? owner->getClient() : NULL;

        mSafetyDepositLoads[bankId] = static_cast<uint32>(contents.size());
        for(auto& entry : contents)
        {
            gTangibleFactory->requestObject(this, entry.second, entry.first, 0, client);
        }
    });
}

//======================================================================================================================

void TreasuryManager::handleObjectReady(Object* object,DispatchClient* client)
{
    const uint64 bankId = object->getParentId();
    auto load = mSafetyDepositLoads.find(bankId);
    if(load == mSafetyDepositLoads.end())
    {
        LOG(WARNING) << "TreasuryManager: item " << object->getId() << " loaded for no safe deposit box (" << bankId << ")";
        delete object;
        return;
    }

    Bank* bank = dynamic_cast<Bank*>(gWorldManager->getObjectById(bankId));
    if(!bank)
    {
        // The owner logged out while the box loaded.
        delete object;
        if(--load->second == 0)
        {
            mSafetyDepositLoads.erase(load);
        }
        return;
    }

    if(!gWorldManager->existObject(object))
    {
        bank->addObject(object);
        gWorldManager->addObject(object, true);
        // The owner is registered to the bank: this creates the item for them.
        gContainerManager->createObjectToRegisteredPlayers(bank, object);
    }
    else
    {
        delete object;
    }

    if(--load->second == 0)
    {
        mSafetyDepositLoads.erase(load);
        bank->setLoadState(LoadState_Loaded);
        _openLoadedSafetyDeposit(bank);
    }
}

//======================================================================================================================

void TreasuryManager::_openLoadedSafetyDeposit(Bank* bank)
{
    PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(bank->getParentId()));
    if(!owner || !owner->isConnected())
    {
        return;
    }

    // Normally done at login; a no-op when it is.
    gContainerManager->registerPlayerToContainer(bank, owner);
    gMessageLib->sendOpenedContainer(bank->getId(), owner);
}

//======================================================================================================================

void TreasuryManager::bankQuit(PlayerObject* playerObject)
{
    if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
    {
        // check if the player is really binded to this bank
        if(static_cast<uint32>(bank->planet()) != gWorldManager->getZoneId())
        {
            gMessageLib->SendSystemMessage(L"You are not a member of this bank.", playerObject);
            return;
        }

        // The safe deposit box must be empty (KNOWN_ISSUES #27): its items
        // are kept at this bank.
        const uint64 bankId = bank->getId();
        std::stringstream sql;
        sql << "SELECT (SELECT COUNT(*) FROM " << mDatabase->galaxy() << ".items WHERE parent_id = " << bankId << ")"
            << " + (SELECT COUNT(*) FROM " << mDatabase->galaxy() << ".resource_containers WHERE parent_id = " << bankId << ")"
            << " + (SELECT COUNT(*) FROM " << mDatabase->galaxy() << ".containers WHERE parent_id = " << bankId << ")";

        mDatabase->executeAsyncSql(sql, [this, bankId] (DatabaseResult* result) {
            Bank* bank = dynamic_cast<Bank*>(gWorldManager->getObjectById(bankId));
            if(!bank)
            {
                return;
            }
            PlayerObject* owner = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(bank->getParentId()));
            if(!owner || !owner->isConnected())
            {
                return;
            }

            uint64 stored = 0;
            if(result && result->getResultSet() && result->getResultSet()->next())
            {
                stored = result->getResultSet()->getUInt64(1);
            }
            // Items moved in this session are counted too, should their
            // save not have reached the database yet.
            if(stored > 0 || (bank->getLoadState() == LoadState_Loaded && !bank->getObjects()->empty()))
            {
                gMessageLib->SendSystemMessage(L"You must empty your safe deposit box before you can quit this bank.", owner);
                return;
            }

            // update the playerObject
            bank->planet(-1);

            // save to db
            mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.banks SET planet_id = -1 WHERE id=%" PRIu64 "",mDatabase->galaxy(),bank->getId());

            //This message has a period added to the end as it was missing from client.
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "succesfully_quit_bank"), owner);
        });
    }
}

//======================================================================================================================

void TreasuryManager::bankJoin(PlayerObject* playerObject)
{
    if(Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank)))
    {
        // check if we're not already binded here
        if(static_cast<uint32>(bank->planet()) == gWorldManager->getZoneId())
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "already_member_of_bank"), playerObject);
            return;
        }

        // check if we are not binded to any other bank
        if(!(bank->planet() < 0))
        {
            gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "member_of_different_bank"), playerObject);
            return;
        }

        bank->planet((int8)gWorldManager->getZoneId());

        // save to db
        mDatabase->executeSqlAsync(NULL,NULL,"UPDATE %s.banks SET planet_id=%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),bank->planet(),bank->getId());

        //This message period added at the end as its missing from client.
        gMessageLib->SendSystemMessage(::common::OutOfBand("system_msg", "succesfully_joined_bank"), playerObject);
    }
}

//======================================================================================================================

void TreasuryManager::saveAndUpdateInventoryCredits(PlayerObject* playerObject)
{
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.inventories SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits(),playerObject->getId() + 1));

    gMessageLib->sendInventoryCreditsUpdate(playerObject);
}

//======================================================================================================================

void TreasuryManager::saveAndUpdateBankCredits(PlayerObject* playerObject)
{
    mDatabase->destroyResult(mDatabase->executeSynchSql("UPDATE %s.banks SET credits=%u WHERE id=%" PRIu64 "",mDatabase->galaxy(),dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->credits(), playerObject->getId() + 4));

    gMessageLib->sendBankCreditsUpdate(playerObject);
}

//======================================================================================================================

void TreasuryManager::bankTipOffline(int32 amount,PlayerObject* playerObject,BString targetName)
{

    //============================================
    //check whether we have sufficient funds
    //dont forget the surcharge
    Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    int32 credits = bank->credits();

    int32 surcharge = (int32)((amount/100)*5);

    if((amount + surcharge) > credits)
    {
        BString uniName = targetName;
        uniName.convert(BSTRType_Unicode16);

        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_nsf_bank", L"", L"", uniName.getUnicode16(), amount), playerObject);
        return;
    }
    //now get the player
    std::string sql = std::string("SELECT id FROM ") + mDatabase->galaxy() + ".characters WHERE firstname like ?";

    TreasuryManagerAsyncContainer* asyncContainer;
    asyncContainer = new TreasuryManagerAsyncContainer(TREMQuery_BankTipgetId,playerObject->getClient());
    asyncContainer->amount = amount;
    asyncContainer->surcharge = surcharge;
    asyncContainer->targetName = targetName;
    asyncContainer->player = playerObject;

    mDatabase->executePreparedAsync(this, asyncContainer, sql, {targetName.getAnsi()});
}

//======================================================================================================================

void TreasuryManager::bankTipOnline(int32 amount, PlayerObject* playerObject, PlayerObject* targetObject )
{
    //check if we have enough money
    int32 surcharge = (int32)((amount/100)*5);

    if((amount+surcharge) > dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits())
    {
        BString s;
        s = targetObject->getFirstName();
        s.convert(BSTRType_Unicode16);
        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_nsf_cash", L"", s.getUnicode16(), L"", amount), playerObject);
        return;
    }

    Bank* playerBank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));
    Bank* targetBank = dynamic_cast<Bank*>(targetObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

    playerBank->credits(playerBank->credits() - (amount+surcharge));
    targetBank->credits(targetBank->credits() + amount);

    saveAndUpdateBankCredits(playerObject);
    saveAndUpdateBankCredits(targetObject);

    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_self", 0, targetObject->getId(), 0, amount), playerObject);
    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_target", 0, playerObject->getId(), 0, amount), targetObject);

    gMessageLib->sendBanktipMail(playerObject,targetObject,amount);
}

//======================================================================================================================

void TreasuryManager::inventoryTipOnline(int32 amount, PlayerObject* playerObject,PlayerObject* targetObject )
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

    saveAndUpdateInventoryCredits(playerObject);
    saveAndUpdateInventoryCredits(targetObject);

    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_self", 0, targetObject->getId(), 0, amount), playerObject);
    gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "prose_tip_pass_target", 0, playerObject->getId(), 0, amount), targetObject);
}

//======================================================================================================================
//we confirmed that we are content with the surcharge and go on
//reuse the asynccontainer
void TreasuryManager::handleBankTipSurchargeConfirmed(TreasuryManagerAsyncContainer* asyncContainer)
{
    Transaction* mTransaction = mDatabase->startTransaction(this,asyncContainer);
    int8 sql[256];
    sprintf(sql,"UPDATE %s.banks SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),(asyncContainer->amount + asyncContainer->surcharge), asyncContainer->player->getId() + 4);
    mTransaction->addQuery(sql);
    sprintf(sql,"UPDATE %s.banks SET credits=credits+%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),asyncContainer->amount, asyncContainer->targetId + BANK_OFFSET);
    mTransaction->addQuery(sql);
    mTransaction->execute();


}

//======================================================================================================================

void TreasuryManager::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
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

            TreasuryManagerAsyncContainer* asynContainer = (TreasuryManagerAsyncContainer*) window->getAsyncContainer();
            handleBankTipSurchargeConfirmed(asynContainer);
        }
    }
    break;
    }
}


//======================================================================================================================

void TreasuryManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    TreasuryManagerAsyncContainer* asynContainer = (TreasuryManagerAsyncContainer*)ref;

    // A prepared statement that failed (logged) has no result.
    if (!result)
    {
        delete(asynContainer);
        return;
    }

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
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,0,8);
        result->getNextRow(binding,&id);

        //ok we just established that our target exists
        //we now need to update the bank on the db side
        TreasuryManagerAsyncContainer* asyncContainer = new TreasuryManagerAsyncContainer(TREMQuery_BankTipTransaction,asynContainer->player->getClient());

        asyncContainer->amount		= asynContainer->amount;
        asyncContainer->surcharge	= asynContainer->surcharge;
        asyncContainer->player		= asynContainer->player;
        asyncContainer->targetId	= id;
        asyncContainer->targetName = asynContainer->targetName;

        //=======================================================
        //now remind the other player of the surcharge
        gUIManager->createNewMessageBox(this,"","@base_player:tip_wire_title","@base_player:tip_wire_prompt",asynContainer->player, SUI_Window_Trade_BankTip_ConfirmSurcharge, SUI_MB_OKCANCEL,asyncContainer);

        break;


    }
    break;

    case TREMQuery_BankTipTransaction:
    {
        uint32 error;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&error);

        if (error == 0)
        {
            Bank* bank = dynamic_cast<Bank*>(asynContainer->player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

            //update our own bankaccount
            bank->credits(bank->credits() - asynContainer->amount);

            gMessageLib->sendBankCreditsUpdate(asynContainer->player);
            gMessageLib->sendBankTipDustOff(asynContainer->player,asynContainer->targetId,asynContainer->amount,asynContainer->targetName);
            //notify the chatserver for the EMails and the off zone accounts

            int8 sql[1024];

            //CAVE galaxy id is hardcoded!!!!!!1
            int8 galaxyId = 2;

            sprintf(sql, "CALL %s.sp_GalaxyAccountDepositCredits(%u, %u, %" PRIu64 ",%u", mDatabase->galaxy(), galaxyId, Account_TipSurcharge, asynContainer->player->getId(), asynContainer->surcharge);
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
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);
        result->getNextRow(binding,&error);

        if (error > 0)
        {
            DLOG(INFO) << "TreasuryManager::Account_TipSurcharge: error " << error;
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


