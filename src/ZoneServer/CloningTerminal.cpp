/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "CloningTerminal.h"
#include "Bank.h"
#include "PlayerObject.h"
#include "TreasuryManager.h"
#include "ZoneServer/Tutorial.h"
#include "UIManager.h"
#include "ZoneServer/WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "MathLib/Quaternion.h"

// TODO: Make this a Var when we start use the 20% discount from player city with "Clone Lab" specializations.
const int32 cloningCost = 1000;

//=============================================================================

CloningTerminal::CloningTerminal() : Terminal()
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

CloningTerminal::~CloningTerminal()
{
}

//=============================================================================

void CloningTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = (PlayerObject*)srcObject;

	switch(messageType)
	{
		case radId_itemUse:
		{
			if (playerObject)
			{
				if (playerObject->isConnected())
				{
					if (gWorldConfig->isTutorial())
					{
						if (this->getParentId() && gWorldManager->getObjectById(this->getParentId())->getParentId())
						{
							// We are located inside a building.

							// Update player with pre-des cloning facility. It's a terminal in a cell in a building...
							playerObject->setPreDesignatedCloningFacilityId(gWorldManager->getObjectById(this->getParentId())->getParentId());

							// store the location where we are bind
							playerObject->setBindPlanet((uint8)gWorldManager->getZoneId());

							Anh_Math::Vector3 bindPosition = gWorldManager->getObjectById(playerObject->getPreDesignatedCloningFacilityId())->mPosition;
							playerObject->setBindCoords(bindPosition);

							// TODO: We need to save the current data before creating the clone data.

							int8 sql[128];
							sprintf(sql,"call swganh.sp_CharacterCreateClone(%"PRIu64",%"PRIu64")", playerObject->getId(),playerObject->getPreDesignatedCloningFacilityId());
							(gWorldManager->getDatabase())->ExecuteSqlAsync(NULL,NULL,sql);

							// Clone location successfully updated
							gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "clone_success");

							// Inform Tutorial about the cloning.
							playerObject->getTutorial()->tutorialResponse("cloneDataSaved");
						}
					}
					else
					{
						gUIManager->createNewMessageBox(this,"","@base_player:clone_confirm_title","@base_player:clone_confirm_prompt",playerObject, SUI_Window_MessageBox, SUI_MB_OKCANCEL);
					}
				}
			}
		}
		break;

		default:
			gLogger->logMsgF("CloningTerminal::handleObjectMenuSelect Unhandled MenuSelect: %u",MSG_HIGH,messageType);

		break;
	}
}


//=============================================================================


// void CloningTerminal::handleUIEvent(string strInventoryCash, string strBankCash, UIWindow* window)
void CloningTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
	// gLogger->logMsgF("CloningTerminal::handleUIEvent You are here!",MSG_NORMAL);

	if(window == NULL)
	{
		return;
	}

	PlayerObject* playerObject = window->getOwner(); // window owner

	if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	if (this->getParentId() && gWorldManager->getObjectById(this->getParentId())->getParentId())
	{
		// We are located inside a building.

		if (action == 1)
		{
			// That's the Cancel.
			// gLogger->logMsgF("CloningTerminal::handleUIEvent That's the Cancel?",MSG_NORMAL);
		}
		else
		{
			// This is the OK.  (action == 0)
			// TODO: If the player have the "coupon", they should get a special message.
			// For now, we skip the "coupon", because of the risk of player deleting them, how do they advance in the Tutorial then?

			int32 creditsAtBank = (dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits());

			// TODO: Some cities have 20% reduction of cloning fee, depending of city status
			if (creditsAtBank < cloningCost)
			{
				if (creditsAtBank == cloningCost - 1)
				{
					// nsf_clone1       You lack the 1 additional credit required to cover the cost of cloning.
					gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "nsf_clone1", "", "", L"");
				}
				else
				{
					// You lack the %DI additional credits required to cover the cost of cloning.
					gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "nsf_clone", "", "", L"", cloningCost - creditsAtBank);
				}
			}
			else if ((dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->updateCredits(-cloningCost)))
			{
				// The credits is drawn from the player bank.
				// System message: You successfully make a payment of %DI credits to %TO.
				gMessageLib->sendSystemMessage(playerObject,
												L"",
												"base_player",
												"prose_pay_acct_success",
												"terminal_name",
												"terminal_cloning",		// Cloning Terminal
												// "money/acct_n",
												// "cloningsystem",		// Cloning System
												L"",
												cloningCost);

				// Update player with pre-des cloning facility. It's a terminal in a cell in a building...
				playerObject->setPreDesignatedCloningFacilityId(gWorldManager->getObjectById(this->getParentId())->getParentId());

				// store the location where we are bind
				playerObject->setBindPlanet((uint8)gWorldManager->getZoneId());

				Anh_Math::Vector3 bindPosition = gWorldManager->getObjectById(playerObject->getPreDesignatedCloningFacilityId())->mPosition;
				playerObject->setBindCoords(bindPosition);

				// gLogger->logMsgF("CloningTerminal::handleUIEvent Saving clone facility id %"PRId64"",MSG_HIGH,playerObject->getPreDesignatedCloningFacilityId());

				int8 sql[128];
				sprintf(sql,"call swganh.sp_CharacterCreateClone(%"PRIu64",%"PRIu64")", playerObject->getId(),playerObject->getPreDesignatedCloningFacilityId());
				(gWorldManager->getDatabase())->ExecuteSqlAsync(NULL,NULL,sql);

				// Clone location successfully updated
				gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "clone_success");

				// Re-enable when/if we starts to use the "coupon" for a free cloning in the Tutorial.
				// if (playerObject->isConnected() && gWorldConfig->isTutorial())
				// {
				// 	playerObject->getTutorial()->tutorialResponse("cloneDataSaved");
				// }
			}
			else
			{
				// This is a system error.
				// The player had no credits in his/her bank.

				// You lack the bank funds to complete this transaction request
				gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insufficient_funds_bank", "", "", L"");
				gLogger->logMsg("CloningTerminal::handleUIEvent: ERROR: Error verifying credits\n");
			}
		}
	}
	// gLogger->logMsgF("CloningTerminal::handleUIEvent You sure handled this UI-event!, Action = %d",MSG_NORMAL, action);
}
//=============================================================================
