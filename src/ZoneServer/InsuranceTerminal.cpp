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
#include "InsuranceTerminal.h"
#include "Bank.h"
#include "Inventory.h"
#include "WorldConfig.h"
#include "PlayerObject.h"
#include "TangibleObject.h"
#include "TreasuryManager.h"
#include "Tutorial.h"
#include "UIManager.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"



//=============================================================================

InsuranceTerminal::InsuranceTerminal() : Terminal()
{
	mInsuranceFee		= gWorldConfig->getConfiguration<int>("Player_ItemInsuranceFee",(int)100);
}

//=============================================================================

InsuranceTerminal::~InsuranceTerminal()
{
}

void InsuranceTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	if (gWorldConfig->isTutorial())
	{
		// Simplified options for the tutorial.
		mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback, "@sui:mnu_insure_all");
		mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
	}
	else
	{
		// any object with callbacks needs to handle those (received with menuselect messages) !
		mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
		mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
		mRadialMenu->addItem(3,0,radId_Unknown,radAction_ObjCallback, "@sui:mnu_insure_all");
	}
}

//=============================================================================

void InsuranceTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	PlayerObject* playerObject = (PlayerObject*)srcObject;

	if (playerObject && playerObject->isConnected())
	{
		// Fetch all items that can be insured.
		BStringVector insuranceList;
		this->getUninsuredItems(playerObject, &insuranceList);

		if (gWorldConfig->isTutorial())
		{
			// We do a simpleified version of insurance when using the terminal in the Tutorial.
			if (this->getParentId() && gWorldManager->getObjectById(this->getParentId())->getParentId())
			{
				// We are located inside a building.

				// Insure all insurable items.
				if (mSortedInsuranceList.size() ==  0)
				{
					// You do not have any items that can be insured. BUT.. we ignore this for now when running the tutorial.
					// gMessageLib->sendSystemMessage(playerObject, L"", "terminal_ui", "no_insurable_items");
				}
				else
				{
					// Insure all the items.

					// Let's warn about the fatal error conditions first (object destroyed or invalid type),
					SortedInventoryItemList::iterator it = mSortedInsuranceList.begin();
					while (it != mSortedInsuranceList.end())
					{
						string selectedItemm = (*it).first;
						selectedItemm.convert(BSTRType_Unicode16);

						Object* object = gWorldManager->getObjectById((*it).second);
						if (!object)
						{
							// Invalid object, we send a warning about this error.
							gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
						}
						else 
						{
							TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
							if (!tangibleObject)
							{
								// Not a tangible object, we send a warning about this error.
									gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
							}
							else if (!tangibleObject->hasInternalAttribute("insured"))
							{
								// This is not a fatal error, but should never happen.

								// [Insurance] Item uninsurable: %TT.
                gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_uninsurable", "","", L"", 0, "", "", selectedItemm.getUnicode16());
							}
							else if (tangibleObject->getInternalAttribute<bool>("insured"))
							{
								// This is not a fatal error, but should never happen.

								// [Insurance] Item already insured: %TT. 
                gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_already_insured", "","", L"", 0, "", "", selectedItemm.getUnicode16());
							}
							else
							{
								// Update attribute.
								tangibleObject->setInternalAttribute("insured","1");
								gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=1 WHERE item_id=%"PRIu64" AND attribute_id=%u",tangibleObject->getId(), 1270);

								tangibleObject->setTypeOptions(tangibleObject->getTypeOptions() | 4);

								// Update insurance status.
								(void)gMessageLib->sendUpdateTypeOption(tangibleObject, playerObject);
							}
						}
						it++;
					}
				}
				// Always display success when doing the tutorial.

				// Insurance transaction successfully completed. 
				gMessageLib->sendSystemMessage(playerObject,L"","base_player","insure_success");

				// Inform Tutorial about the insurance.
				playerObject->getTutorial()->tutorialResponse("insureItemsDone");
			}
		}
		else
		{
			switch(messageType)
			{
				case radId_itemUse:
				{
					if (playerObject->getNewPlayerExemptions())	
					{
						// player have free deatchs left.
						gUIManager->createNewMessageBox(this,"","@base_player:noob_confirm_insure_title","@base_player:noob_confirm_insure_prompt",playerObject, SUI_Window_Insurance_Newbie_MessageBox, SUI_MB_YESNO);
					}
					else
					{
						// We should display all uninsured items that can be insured, and that are wearing or carrying in our inventory.
						// Items in backpackage or in other containers within our inventory shall also be handled.
						gUIManager->createNewListBox(this,"insure","@sui:mnu_insure","Select an item to insure.",insuranceList,playerObject,SUI_Window_Insurance_ListBox, SUI_MB_OKCANCEL);
					}
				}
				break;

				case radId_Unknown:	// Insure All
				{
					if (insuranceList.size() == 0)
					{
						// You do not have any items that can be insured. 
						gMessageLib->sendSystemMessage(playerObject, L"", "terminal_ui", "no_insurable_items");
					}
					else
					{
						if (playerObject->getNewPlayerExemptions())	
						{
							// player have free deatchs left.
							gUIManager->createNewMessageBox(this,"","@base_player:noob_confirm_insure_title","@base_player:noob_confirm_insure_prompt",playerObject, SUI_Window_InsureAll_Newbie_MessageBox, SUI_MB_YESNO);
						}
						else
						{
							uint32 insuranceFee = insuranceList.size() * 100;
							int8 sql[256];
							sprintf(sql,"@terminal_ui:insure_all_d_prefix %u @terminal_ui:insure_all_d_suffix \n\n @terminal_ui:insure_all_confirm", insuranceFee);
							gUIManager->createNewMessageBox(this,"","@terminal_ui:insure_all_t",sql,playerObject, SUI_Window_InsuranceAll_MessageBox, SUI_MB_YESNO);
						}
					}
				}
				break;

				default:
				{
					gLogger->log(LogManager::NOTICE,"InsuranceTerminal::handleObjectMenuSelect Unhandled MenuSelect: %u",messageType);
				}
				break;
			}
		}
	}
}


//=============================================================================

void InsuranceTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{

	if(window == NULL)
	{
		return;
	}

	PlayerObject* playerObject = window->getOwner(); // window owner

	if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead()|| playerObject->checkState(CreatureState_Combat))
	{
		return;
	}

	switch(window->getWindowType())
	{
		case SUI_Window_Insurance_Newbie_MessageBox:		// Tried to insure item when still having free rounds left.
		{
			switch(action)
			{
				case 0: // Yes
				{
					// Player selected to continue with insurance of item even if no need for.

					// Build the items list and optional use error-messages if needed.
					BStringVector insuranceList;
					this->getUninsuredItems(playerObject, &insuranceList);
					
					// We should display all uninsured items that can be insured, and that are wearing or carrying in our inventory.
					// Items in backpackage or in other containers within our inventory shall also be handled.
					gUIManager->createNewListBox(this,"insure","@sui:mnu_insure","Select an item to insure.",insuranceList,playerObject,SUI_Window_Insurance_ListBox, SUI_MB_OKCANCEL);
				}
				break;

				case 1: // No
				{
					// Player selected to abort, since all items are still treated as insured.
				}
				break;

				default:
				{
					gLogger->log(LogManager::DEBUG,"SUI_Window_Insurance_Newbie_MessageBox Invalid selection!");
				}
				break;
			}
		}
		break;

		case SUI_Window_Insurance_ListBox:
		{
			switch (action)
			{
				case 0: // OK
				{
					// Insure one item.
					
					Inventory* inventoryObject = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)); 
					Bank* bankObject = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

					if(!inventoryObject || !bankObject)
						return;

					int32 creditsInInventory = inventoryObject->getCredits();
					int32 creditsAtBank = bankObject->getCredits();

					if (mSortedInsuranceList.size() ==  0)
					{
						// You have no insurable items.
						gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "no_insurables");
					}
					else if (element > (int32)mSortedInsuranceList.size() - 1 || element < 0)
					{
						// Unable to process insure item request.
						gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "bad_insure_request");
					}
					else
					{
						string selectedItemm((mSortedInsuranceList.at(element).first).getAnsi());
						selectedItemm.convert(BSTRType_Unicode16);

						Object* object = gWorldManager->getObjectById(mSortedInsuranceList.at(element).second);
						if (!object)
						{
							// Invalid object.
							// Insure attempt failed.
							gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
							break;
						}
						TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
						if (!tangibleObject)
						{
							// Not a tangible object.
							// Insure attempt failed.
							gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
						}
						else if (!tangibleObject->hasInternalAttribute("insured"))
						{
							// [Insurance] Item uninsurable: %TT.
              gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_uninsurable", "","", L"", 0, "", "", selectedItemm.getUnicode16());
						}
						else if (tangibleObject->getInternalAttribute<bool>("insured"))
						{
							// [Insurance] Item already insured: %TT. 
              gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_already_insured", "","", L"", 0, "", "", selectedItemm.getUnicode16());
						}
						else if ((creditsAtBank+creditsInInventory) < mInsuranceFee)
						{
							// You have insufficient funds to insure your %TT. 
              gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_nsf_insure", "","", L"", 0, "", "", selectedItemm.getUnicode16());
						}
						else
						{
							int32 delta = creditsInInventory - mInsuranceFee;

							if(delta >= 0)
							{
								inventoryObject->updateCredits(-mInsuranceFee);
							}
							else if(delta < 0 && creditsAtBank >= (-delta))
							{
								inventoryObject->updateCredits(mInsuranceFee + delta);
								bankObject->updateCredits(delta);
							}

							// The credits is drawn from the player inventory and/or bank.
							// System message: You successfully make a payment of %DI credits to %TO.
							gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "prose_pay_acct_success", "terminal_name", "terminal_insurance", L"", mInsuranceFee);

							// Update attribute.
							// string str("insured");
							tangibleObject->setInternalAttribute("insured","1");
							gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=1 WHERE item_id=%"PRIu64" AND attribute_id=%u",tangibleObject->getId(), 1270);
							
							tangibleObject->setTypeOptions(tangibleObject->getTypeOptions() | 4);

							// Update insurance status.
							(void)gMessageLib->sendUpdateTypeOption(tangibleObject, playerObject);

							// You successfully insure your %TT. 
              gMessageLib->sendSystemMessage(playerObject,L"","base_player","prose_insure_success", "","", L"", 0, "", "", selectedItemm.getUnicode16());
						}
						/*else
						{
							// An attempt to insure your %TT has failed. Most likely, this is due to lack of funds. 
							gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_insure_fail", "","", L"", 0, "", "", selectedItemm);
						}*/
					}
				}
				break;

				case 1: // Cancel
				{
				}
				break;

				default:
				{
				}
				break;
			}
		}
		break;

		case SUI_Window_InsureAll_Newbie_MessageBox:
		{
			switch(action)
			{
				case 0: // Yes
				{
					// Player selected to continue with insurance of item even if no need for.

					// Fetch all items that can be insured.
					BStringVector insuranceList;
					this->getUninsuredItems(playerObject, &insuranceList);

					uint32 insuranceFee = insuranceList.size() * mInsuranceFee;
					int8 sql[256];
					sprintf(sql,"@terminal_ui:insure_all_d_prefix %u @terminal_ui:insure_all_d_suffix \n\n @terminal_ui:insure_all_confirm", insuranceFee);
					gUIManager->createNewMessageBox(this,"","@terminal_ui:insure_all_t",sql,playerObject, SUI_Window_InsuranceAll_MessageBox, SUI_MB_YESNO);
				}
				break;

				case 1: // No
				{
					// Player selected to abort, since all items are still treated as insured.
				}
				break;

				default:
				{
					gLogger->log(LogManager::DEBUG,"SUI_Window_InsureAll_Newbie_MessageBox Invalid selection!");
				}
				break;
			}
		}
		break;

		case SUI_Window_InsuranceAll_MessageBox:
		{
			switch(action)
			{
				case 0: // Yes
				{
					// Insure all insurable items.
					int32 creditsAtBank = (dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits());
					string selectedItemm;
					int32 fee = mSortedInsuranceList.size() * mInsuranceFee;

					if (mSortedInsuranceList.size() ==  0)
					{
						// You do not have any items that can be insured. 
						gMessageLib->sendSystemMessage(playerObject, L"", "terminal_ui", "no_insurable_items");
					}
					else if (creditsAtBank < fee)
					{
						// You have insufficient funds to insure your %TT. 
						gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_nsf_insure", "","", L"", 0, "", "", L"items");
					}
					else
					{
						// Insure all the items.
						bool abortInsurance = false;

						// Let's clear the fatal error conditions first (object destroyed or invalid type),
						SortedInventoryItemList::iterator it = mSortedInsuranceList.begin();
						while (it != mSortedInsuranceList.end())
						{
							Object* object = gWorldManager->getObjectById((*it).second);
							if (!object)
							{
								// Invalid object, we abort this transaction.

								// Insure attempt failed.
								gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
								abortInsurance = true;
								break;
							}
		
							TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);
							if (!tangibleObject)
							{
								// Not a tangible object, we abort this transaction.
								// Insure attempt failed.
								gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "insure_fail");
								abortInsurance = true;
								break;
							}
							it++;
						}

						if (abortInsurance)
						{
							break;
						}

						if ((dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->updateCredits(-fee)))
						{
							// The credits is drawn from the player bank.
							// System message: You successfully make a payment of %DI credits to %TO.
							gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "prose_pay_acct_success", "terminal_name", "terminal_insurance", L"", fee);

							it = mSortedInsuranceList.begin();
							while (it != mSortedInsuranceList.end())
							{
								selectedItemm = (*it).first;
								selectedItemm.convert(BSTRType_Unicode16);

								Object* object = gWorldManager->getObjectById((*it).second);
								TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object);

								if (!tangibleObject->hasInternalAttribute("insured"))
								{
									// This is not a fatal error, but should never happen.

									// [Insurance] Item uninsurable: %TT.
                  gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_uninsurable", "","", L"", 0, "", "", selectedItemm.getUnicode16());
									// fee -= insuranceFee;
									it++;
									continue;
								}

								if (tangibleObject->getInternalAttribute<bool>("insured"))
								{
									// This is not a fatal error, but should never happen.

									// [Insurance] Item already insured: %TT. 
                  gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_item_already_insured", "","", L"", 0, "", "", selectedItemm.getUnicode16());
									// fee -= insuranceFee;
									it++;
									continue;
								}

								// Insure the item.
								// Update attribute.
								tangibleObject->setInternalAttribute("insured","1");
								gWorldManager->getDatabase()->ExecuteSqlAsync(NULL,NULL,"UPDATE item_attributes SET value=1 WHERE item_id=%"PRIu64" AND attribute_id=%u",tangibleObject->getId(), 1270);

								tangibleObject->setTypeOptions(tangibleObject->getTypeOptions() | 4);

								// Update insurance status.
								(void)gMessageLib->sendUpdateTypeOption(tangibleObject, playerObject);

								it++;
							}

							// Insurance transaction successfully completed. 
							gMessageLib->sendSystemMessage(playerObject,L"","base_player","insure_success");

						}
						else
						{
							// An attempt to insure your %TT has failed. Most likely, this is due to lack of funds. 
							gMessageLib->sendSystemMessage(playerObject,L"","error_message","prose_insure_fail", "","", L"", 0, "", "", L"items");
						}
					}

				}
				break;

				case 1: // No
				{
				}
				break;

				default:
				{
					gLogger->log(LogManager::DEBUG,"SUI_Window_InsuranceAll_MessageBox Invalid selection!");
				}
				break;
			}
		}
		break;

		default:
		{
		}
		break;
	}
}
//=============================================================================
//
//	Fetch all items, in inventory or equipped, that are un-insured, but can be insured.
//

void InsuranceTerminal::getUninsuredItems(PlayerObject* playerObject, BStringVector* insuranceList)
{
	// Fetch all items that can be insured.
	mSortedInsuranceList.clear();

	// Build the items list and optional use error-messages if needed.
	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	if (inventory)
	{

		// Fetch the items.
		inventory->getUninsuredItems(&mSortedInsuranceList);

		// Sort the items by stf-names.
		SortedInventoryItemList::iterator it;

		it = mSortedInsuranceList.begin();
		while (it != mSortedInsuranceList.end())
		{
			insuranceList->push_back((*it).first);
			it++;
		}
	}
}

//=============================================================================
//
//	Fetch all items, in inventory or equipped, that are insured.
//

void InsuranceTerminal::getInsuredItems(PlayerObject* playerObject, BStringVector* insuranceList)
{
	// Fetch all items that can be insured.
	mSortedInsuranceList.clear();

	// Build the items list and optional use error-messages if needed.
	Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	if (inventory)
	{

		// Fetch the items.
		inventory->getInsuredItems(&mSortedInsuranceList);

		// Sort the items by stf-names.
		SortedInventoryItemList::iterator it;

		it = mSortedInsuranceList.begin();
		while (it != mSortedInsuranceList.end())
		{
			insuranceList->push_back((*it).first);
			it++;
		}
	}
}
