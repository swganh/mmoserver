/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"
#include "Bank.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "TreasuryManager.h"
#include "UIManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "MathLib/Quaternion.h"


//=============================================================================

BankTerminal::BankTerminal() : Terminal()
{
}

//=============================================================================

BankTerminal::~BankTerminal()
{
}

//=============================================================================
void BankTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = (PlayerObject*)srcObject;
	
	switch(messageType)
	{
		
		case radId_bankDepositAll: // deposit all

			gTreasuryManager->bankDepositAll(playerObject);
		
		break;


		case radId_bankWithdrawAll: // withdraw all

			gTreasuryManager->bankWithdrawAll(playerObject);

		break;


		case radId_itemUse:
		case radId_bankTransfer: // deposit - withdraw

			gUIManager->createNewTransferBox(this,"handleDepositWithdraw","@base_player:bank_title"
				,"@base_player:bank_prompt", "Cash", "Bank"
				,dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits()
				,dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits()
				,playerObject);

		break;

		
		case radId_bankItems:

			gTreasuryManager->bankOpenSafetyDepositContainer(playerObject);

		break;


		case radId_bankJoin: // join
			
			gTreasuryManager->bankJoin(playerObject);
		
		break;


		case radId_bankQuit: // quit
			
			gTreasuryManager->bankQuit(playerObject);
		
		break;


		default:

			gLogger->logMsgF("BazaarTerminal: Unhandled MenuSelect: %u",MSG_HIGH,messageType);
		
		break;
	}
	
}


//=============================================================================

void BankTerminal::handleUIEvent(string strInventoryCash, string strBankCash, UIWindow* window)
{

	if(window == NULL)
	{
		return;
	}

	PlayerObject* playerObject = window->getOwner(); // window owner

	if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->checkState(CreatureState_Combat))
	{
		return;
	}

	// two money movement deltas stands for credits
	// variations into bank & inventory.
	// casting as signed cause one will be negative.
	// when inventoryDelta + bankDelta is not equal to zero,
	// that means player treasury has changed since
	// the transfer window opened.

	// we get the money deltas by parsing the string returned
	// by the SUI window

	strInventoryCash.convert(BSTRType_ANSI);
	strBankCash.convert(BSTRType_ANSI);

	int32 inventoryMoneyDelta = atoi(strInventoryCash.getAnsi()) - dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits();
	int32 bankMoneyDelta = atoi(strBankCash.getAnsi()) - dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits();

	// the amount transfered must be greater than zero
	if(bankMoneyDelta == 0 || inventoryMoneyDelta == 0)
	{
		return;
	}

	gTreasuryManager->bankTransfer(inventoryMoneyDelta, bankMoneyDelta, playerObject);
}

//=============================================================================

void BankTerminal::prepareRadialMenu()
{	
}

//=============================================================================

void BankTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu = gTreasuryManager->bankBuildTerminalRadialMenu(creatureObject);
}

//=============================================================================