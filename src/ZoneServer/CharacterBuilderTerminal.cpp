/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "MessageLib/MessageLib.h"
#include "UIManager.h"
#include "UIResourceSelectListBox.h"
#include "ResourceManager.h"
#include "TreasuryManager.h"
#include "TradeManager.h"
#include "SkillManager.h"
#include "CharacterBuilderTerminal.h"
#include "Buff.h"

//=============================================================================

CharacterBuilderTerminal::CharacterBuilderTerminal() : Terminal(), mSortedList(NULL)
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);

	mMainMenu.push_back("Manage Experience");
	mMainMenu.push_back("Manage Credits");
	mMainMenu.push_back("Manage Attributes");
	mMainMenu.push_back("Manage Items");
	mMainMenu.push_back("Manage Professions");
	mMainMenu.push_back("Manage Resources");

	mCreditsMenu.push_back("Inventory credits");
	mCreditsMenu.push_back("Bank credits");

	
	mAttributesMenu.push_back("Battle fatigue   (50)");
	mAttributesMenu.push_back("Battle fatigue  (250)");
	mAttributesMenu.push_back("Mind Wounds      (50)");
	mAttributesMenu.push_back("Mind Wounds     (250)");
	mAttributesMenu.push_back("Heal Fatigue    (123)");
	mAttributesMenu.push_back("Heal Wounds     (123)");
	mAttributesMenu.push_back("Mugly's cocktail");
	mAttributesMenu.push_back("Lloyd's Health Buffs");
	mAttributesMenu.push_back("Lloyd's Action Buffs");
	mAttributesMenu.push_back("Lloyd's Mind Buffs");
	mAttributesMenu.push_back("Jawa beer (Mask Scent)");
	mAttributesMenu.push_back("Damage Health 200");
	mAttributesMenu.push_back("Damage Action 200");

}

//=============================================================================

CharacterBuilderTerminal::~CharacterBuilderTerminal()
{
}

//=============================================================================


void CharacterBuilderTerminal::prepareRadialMenu()
{	
}

//=============================================================================

void CharacterBuilderTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

		// bring up the terminal window 
		if(playerObject && playerObject->isConnected())
		{
			if(playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
			{
				return;
			}

			gUIManager->createNewListBox(this,"handleMainMenu","Main menu","Select a category.", mMainMenu,playerObject,SUI_Window_CharacterBuilderMainMenu_ListBox);
		}
	}
	else
	{
		gLogger->logMsgF("TravelTerminal: Unhandled MenuSelect: %u",MSG_HIGH,messageType);
	}
}

//=============================================================================

void CharacterBuilderTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{

	PlayerObject* playerObject = window->getOwner();

	if(!playerObject || action || playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	switch(window->getWindowType())
	{

		//================================
		// main menu

		case SUI_Window_CharacterBuilderMainMenu_ListBox:
		{
			
			switch(element)
			{
				case 0: //manage xp
				{
					BStringVector availableXpTypes;
					XPList* xpList = playerObject->getXpList();
					XPList::iterator xpIt = xpList->begin();

					// Allocate space where we can sort this stuff.
					mSortedList = new SortedList;
					SortedList::iterator it;

					while (xpIt != xpList->end())
					{
						it = mSortedList->begin();

						for (uint32 index = 0; index < mSortedList->size(); index++)
						{
							if (_strcmpi(gSkillManager->getXPTypeExById((*xpIt).first).getAnsi(), (*it).first.getAnsi()) < 0)
							{
								break;
							}
							it++;
						}
						mSortedList->insert(it, std::make_pair(gSkillManager->getXPTypeExById((*xpIt).first),(*xpIt).first));
						
						++xpIt;
					}

					// Let's put up the sorted strings for the show.
					it = mSortedList->begin();
					while (it != mSortedList->end())
					{
						availableXpTypes.push_back((*it).first);
						it++;
					}

					if (availableXpTypes.size() == 0)
					{
						gMessageLib->sendSystemMessage(playerObject, L"You currently do not have any skills.");
						delete mSortedList;
						mSortedList = NULL;
					}
					else
					{
						gUIManager->createNewListBox(this,"handleGetXp","Select Xp Type","Select from the list below.",availableXpTypes,playerObject,SUI_Window_CharacterBuilderXpMenu_ListBox);
					}
				}
				break;

				case 1: // manage credits
				{
					if(playerObject->isConnected())
					{
						gUIManager->createNewListBox(this,"handleCreditsMenu","Credits","Select a category.",mCreditsMenu,playerObject,SUI_Window_CharacterBuilderCreditsMenu_ListBox);
					}
				}
				break;

				case 2: // manage attributes
				{
					if(playerObject->isConnected())
					{
						gUIManager->createNewListBox(this,"handleAttributesMenu","Attributes","Select an attributes.",mAttributesMenu,playerObject,SUI_Window_CharacterBuilderAttributesMenu_ListBox);
					}
				}
				break;

				case 3: // manage items
				{
					if(playerObject->isConnected())
					{
							BStringVector dropDowns;
							gUIManager->createNewInputBox(	this,
															"handleInputItemId",
															"Get Item",
															"Enter the item ID",
															dropDowns, 
															playerObject, 
															SUI_IB_NODROPDOWN_OKCANCEL, 
															SUI_Window_CharacterBuilderItemIdInputBox,
															8);
						/*
						BStringVector					nameList;
						ResourceIdList					itemIdList;

						ItemFrogTypeList typeList = gTradeManager->mItemFrogClass.mItemFrogTypeList;
						ItemFrogTypeList::iterator it = typeList.begin();

						while(it != typeList.end())
						{
							if((*it)->mItemFrogItemList.size())
							{
								itemIdList.push_back((*it)->family);
								nameList.push_back((*it)->f_name);
							}
								
							it++;	
						}

						gUIManager->createNewResourceSelectListBox(this,"","Item family","Select",nameList,itemIdList,playerObject,SUI_Window_CharacterBuilderItemType_ListBox);
						*/
					}
				}
				break;
				case 4://Manage professions
				{
					BStringVector		availableProfessions;
					SkillList*			skillList	= gSkillManager->getMasterProfessionList();
					SkillList::iterator skillIt		= skillList->begin();

					while(skillIt != skillList->end())
					{
						// skip jedi professions, if flag isn't set
						if(!playerObject->getJediState() && strstr((*skillIt)->mName.getAnsi(),"discipline"))
						{
							++skillIt;

							continue;
						}

						availableProfessions.push_back((*skillIt)->mName);

						++skillIt;
					}
					gUIManager->createNewListBox(this,"handleGetProf","Select Profession to Master","Select from the list below.",availableProfessions,playerObject,SUI_Window_CharacterBuilderProfessionMastery_ListBox);
				}
				break;

				// Manage resources
				case 5: //resi test
				{
					if(playerObject->isConnected())
					{
						ResourceTypeMap*				rtMap				= gResourceManager->getResourceTypeMap();
						ResourceCategoryMap*			rcMap				= gResourceManager->getResourceCategoryMap();
						ResourceCategoryMap::iterator	rcIt				= rcMap->begin();
						BStringVector					resourceNameList;
						ResourceIdList					resourceIdList;
						uint32							counter = 0;

						while(rcIt != rcMap->end())
						{
							ResourceTypeMap::iterator rtIt = rtMap->begin();

							while(rtIt != rtMap->end())
							{
								//we want only parent ==1 to begin with
								uint32 parentId = (*(*rcIt).second).getParentId();
								if(parentId == 1)
								{
									resourceIdList.push_back((*rcIt).first);
									resourceNameList.push_back((*(*rcIt).second).getName());
									break;
								}
								
								++rtIt;
							}

							++rcIt;
						}

						gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_ResourcesParent_ListBox);
					}
				}
				break;

				
				default:{}break;
			}
		}
		break;

		//================================
		// item retrieve by ID

		case SUI_Window_CharacterBuilderItemIdInputBox :
		{
			int32 inputId = 0;

			if(swscanf(inputStr.getUnicode16(),L"%i",&inputId) == 1)
			{
				ItemFrogItemClass* item = gTradeManager->mItemFrogClass.LookUpType(inputId);

				if(item)
				{
					Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
					gObjectFactory->requestNewDefaultItem(inventory,item->family,item->type,inventory->getId(),99,Anh_Math::Vector3(),"");	
					gMessageLib->sendSystemMessage(playerObject, L"The item has been placed in your inventory.");
				}
				else
				{
					gMessageLib->sendSystemMessage(playerObject, L"No such item.");
				}
			}

			BStringVector dropDowns;
			gUIManager->createNewInputBox(this, "handleInputItemId", "Get Item", "Enter the item ID", dropDowns, playerObject, SUI_IB_NODROPDOWN_OKCANCEL, SUI_Window_CharacterBuilderItemIdInputBox,8);
		}
		break;

		//we need to test whether our selection has childs types or child resources
		case SUI_Window_ResourcesParent_ListBox:
		{
			ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();
			ResourceTypeMap*				rtMap				= gResourceManager->getResourceTypeMap();

			if(element > (int32)resourceIdList.size()||element < 0)
			{
				break;
			}

			uint64							catId				= resourceIdList[element];
			ResourceCategory*				rParent				= gResourceManager->getResourceCategoryById((uint32)catId);

			resourceIdList.clear();
			if(rParent)
			{
				BStringVector					resourceNameList;
				ResourceIdList					resourceIdList;

				//do we have children categories or children resources?
				if(rParent->getChildren()->size())
				{
					gLogger->logMsgF("",MSG_HIGH);
					gLogger->logMsgF("has children ",MSG_HIGH);
					//iterate through the children categories and display them
					ResourceCategoryList*			rcList				= rParent->getChildren();
					ResourceCategoryList::iterator	rcIt				= rcList->begin();
					
					bool found = false;
					//iterate through the children and add them as necessary
					while(rcIt != rcList->end())
					{
						if((*(*rcIt)).getChildren()->size())
						{
							resourceIdList.push_back((*(*rcIt)).getId());
							resourceNameList.push_back((*(*rcIt)).getName());	
							rcIt++;
							continue;
						}
						ResourceTypeMap::iterator rtIt = rtMap->begin();
						while(rtIt != rtMap->end())
						{
							if(rcIt == rcList->end())
								break;

							ResourceType *rt = (*rtIt).second;
							if(!rt)
							{
								rtIt++;
								continue;
							}

							// the resources category id is equal to the category id we are about to display!
							// ie it is not empty - add it
							if(rt->getCategoryId() == (*(*rcIt)).getId())
							{
								resourceIdList.push_back((*(*rcIt)).getId());
								resourceNameList.push_back((*(*rcIt)).getName());
								rtIt++;
								//only one of each
								break;

							}
								
							if((*(*rcIt)).getParentId() == rParent->getId())
							{
								resourceIdList.push_back((*(*rcIt)).getId());
								resourceNameList.push_back((*(*rcIt)).getName());
								rtIt++;
								//only one of each
								break;
								//}
							}
							
							++rtIt;
						}
						if(rcIt != rcList->end())
							rcIt++;
					}
					gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_ResourcesParent_ListBox);
				}
				else//if(rParent->getChildren()->size())
				{				//it was a resource - create
					
					ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();

					if(element > (int32)resourceIdList.size()|| element < 0)
					{
						resourceIdList.clear();
						break;
					}

					ResourceCRCNameMap*				rCRCMap				= gResourceManager->getResourceCRCNameMap();
					ResourceCRCNameMap::iterator	rCrcMapIt;
					
					ResourceTypeMap::iterator		rtIt				= rtMap->begin();
					uint32							catId				= static_cast<uint32>(resourceIdList[element]);
					ResourceCategory*				rCategory			= gResourceManager->getResourceCategoryById(catId);
					BStringVector					resourceNameList;

					resourceIdList.clear();

					//no iterate through all the types and check for those containing resis
					while(rtIt != rtMap->end())
					{
						if((*(*rtIt).second).getCategoryId() == catId)
						{							
							// check whether we have associated resources
							
							rCrcMapIt	= rCRCMap->begin();
							while(rCrcMapIt != rCRCMap->end())
							{
								if((*(*rCrcMapIt).second).getTypeId() == (*(*rtIt).second).getId())
								{
									resourceNameList.push_back((*(*rtIt).second).getName());
									resourceIdList.push_back((*rtIt).first);

									break;
								}

								++rCrcMapIt;
							}
						}

						++rtIt;
					}
					
					// is the list filled? if not we might have resources already on our hand!
					if(resourceIdList.size())
					{
						gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox);
						return;
					}
					else
					{
						//as no types are available anymore it might be resources!

						rCrcMapIt	= rCRCMap->begin();
						while(rCrcMapIt != rCRCMap->end())
						{
							if((*(*rCrcMapIt).second).getTypeId() == catId)
							{
								resourceNameList.push_back((*(*rCrcMapIt).second).getName());
								resourceIdList.push_back((*rCrcMapIt).first);
							}

							++rCrcMapIt;
						}

						gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox);
					}
				}
			}
		}	
		break;


		//================================
		// resource type selection

		case SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox:
		{
			ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();
			if(element < 0)
			{ 
				resourceIdList.clear();
				break;
			}
			
			// now have the category we need the type
			if(playerObject->isConnected())
			{

				if(element > (int32)resourceIdList.size())
				{
					resourceIdList.clear();
					break;
				}

				uint32							typeId		= static_cast<uint8>(resourceIdList[element]);
				ResourceType*					rType		= gResourceManager->getResourceTypeById(typeId);
				ResourceTypeMap*				rtMap		= gResourceManager->getResourceTypeMap();
				ResourceCRCNameMap*				rCRCMap		= gResourceManager->getResourceCRCNameMap();
				ResourceCRCNameMap::iterator	rCrcNameIt	= rCRCMap->begin();
				BStringVector					resourceNameList;

				resourceIdList.clear();

				while(rCrcNameIt != rCRCMap->end())
				{
					if((*(*rCrcNameIt).second).getTypeId() == typeId)
					{
						resourceNameList.push_back((*(*rCrcNameIt).second).getName());
						resourceIdList.push_back((*rCrcNameIt).first);
					}

					++rCrcNameIt;
				}

				gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox);
			}
		}
		break;

		case SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox:
		{

			if(element < 0)
			{ 
				break;
			}
			
			// now have the type we need the resource
			if(playerObject->isConnected())
			{
				ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();

				if(element > (int32)resourceIdList.size())
				{
					break;
				}

				uint32		crc			= static_cast<uint32>(resourceIdList[element]);
				Resource*	resource	= gResourceManager->getResourceByNameCRC(crc);

				if(resource)
				{
					Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
					gObjectFactory->requestNewResourceContainer(inventory ,resource->getId(),inventory ->getId(),99,100000);	
				}
			}
		}
		break;
		
		//================================
		// credits type selection

		case SUI_Window_CharacterBuilderCreditsMenu_ListBox:
		{
			switch(element)
			{
				case 0: // inventory credits
				{
					BStringVector dropDowns;
					dropDowns.push_back("test");
					gUIManager->createNewInputBox(this,
						"handleInputInventoryCredits",
						"Inventory Credits",
						"Enter amount",
						dropDowns, 
						playerObject, 
						SUI_IB_NODROPDOWN_OKCANCEL, 
						SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox,
						8);
				}
				break;

				case 1: // bank credits
				{
					BStringVector dropDowns;
					dropDowns.push_back("test");
					gUIManager->createNewInputBox(this,
						"handleInputInventoryCredits",
						"Bank Credits",
						"Enter amount",
						dropDowns, 
						playerObject, 
						SUI_IB_NODROPDOWN_OKCANCEL, 
						SUI_Window_CharacterBuilderCreditsMenuBank_InputBox,
						8);
				}
				break; 

				default:{}break;
			}
		}
		break;


		//================================
		// credits manipulations (inv & bank)

		case SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox:
		case SUI_Window_CharacterBuilderCreditsMenuBank_InputBox:
		{
			
			// parse the input value
			if(swscanf(inputStr.getUnicode16(),L"%i",&mInputBoxAmount) != 1)
			{
				mInputBoxAmount = -1;
			}
			
			if(mInputBoxAmount < 0)
			{
				gMessageLib->sendSystemMessage(playerObject, L"Invalid amount.");
				return;
			}

			gLogger->logMsgF("input: %u\n", MSG_NORMAL, mInputBoxAmount);

			// bank or inv?
			if(window->getWindowType() == SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox)
			{
				dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->setCredits(mInputBoxAmount);
				gTreasuryManager->saveAndUpdateInventoryCredits(playerObject);
				
			}
			else
			{
				dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->setCredits(mInputBoxAmount);
				gTreasuryManager->saveAndUpdateBankCredits(playerObject);
			}

		}
		break;


		//================================
		// attributes menu

		case SUI_Window_CharacterBuilderAttributesMenu_ListBox:
		{
			switch(element)
			{

				case 0: // get 50 battle fatigue
				{
					playerObject->getHam()->updateBattleFatigue(50);
				}
				break;

				case 1: // get 250 battle fatigue
				{
					playerObject->getHam()->updateBattleFatigue(250);
				}
				break;

				case 2: // get 50 mind wounds
				{
					playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, 50);
				}
				break; 

				case 3: // get 250 mind wounds
				{
					playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, 250);
				}
				break; 

				case 4: //test heal bf
				{
					playerObject->getHam()->updateBattleFatigue(-123);
				}
				break; 

				case 5: //test heal wounds
				{
					playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Wounds, -123);
					playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Wounds, -123);
					playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, -123);
				}
				break; 

				case 6: //random
				{
					int randomHitPoints;
					int randomWounds;

					randomHitPoints = (gRandom->getRand() % 200)+1;
					randomWounds = (gRandom->getRand() % randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, -randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Wounds, randomWounds);

					randomHitPoints = (gRandom->getRand() % 200)+1;
										randomWounds = (gRandom->getRand() % randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints,  -randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Wounds, randomWounds);

					randomHitPoints = (gRandom->getRand() % 200)+1;
					randomWounds = (gRandom->getRand() % randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -randomHitPoints);
					playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, randomWounds);

					playerObject->getHam()->updateBattleFatigue(gRandom->getRand() % 100);
				}
				break;

				case 7: //Lloyd's Health Buffs
				{
					gMessageLib->sendSystemMessage(playerObject, L"Attempting to Add Blue Frog Doc Health Buffs");
					
					BuffAttribute* tempAttribute1 = new BuffAttribute(Health, +2400,0,-2400); 
					Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_health, gWorldManager->GetCurrentGlobalTick());
					tempBuff1->AddAttribute(tempAttribute1);	
					playerObject->AddBuff(tempBuff1);

					BuffAttribute* tempAttribute2 = new BuffAttribute(Strength, +2400,0,-2400); 
					Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_strength, gWorldManager->GetCurrentGlobalTick());
					tempBuff2->AddAttribute(tempAttribute2);	
					playerObject->AddBuff(tempBuff2);

					BuffAttribute* tempAttribute3 = new BuffAttribute(Constitution, +2400,0,-2400); 
					Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_constitution, gWorldManager->GetCurrentGlobalTick());
					tempBuff3->AddAttribute(tempAttribute3);	
					playerObject->AddBuff(tempBuff3);
				}
				break;
				case 8: //Lloyd's Action Buffs
				{
					gMessageLib->sendSystemMessage(playerObject, L"Attempting to Add Blue Frog Doc Action Buffs");
					
					BuffAttribute* tempAttribute1 = new BuffAttribute(Action, +2400,0,-2400); 
					Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_action, gWorldManager->GetCurrentGlobalTick());
					tempBuff1->AddAttribute(tempAttribute1);	
					playerObject->AddBuff(tempBuff1);

					BuffAttribute* tempAttribute2 = new BuffAttribute(Quickness, +2400,0,-2400); 
					Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_quickness, gWorldManager->GetCurrentGlobalTick());
					tempBuff2->AddAttribute(tempAttribute2);	
					playerObject->AddBuff(tempBuff2);

					BuffAttribute* tempAttribute3 = new BuffAttribute(Stamina, +2400,0,-2400); 
					Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 600000, medical_enhance_stamina, gWorldManager->GetCurrentGlobalTick());
					tempBuff3->AddAttribute(tempAttribute3);	
					playerObject->AddBuff(tempBuff3);
				}
				break;
				case 9: //Lloyd's Mind Buffs
				{
					gMessageLib->sendSystemMessage(playerObject, L"Attempting to Add Blue Frog Ent Mind Buffs");
					
					BuffAttribute* tempAttribute1 = new BuffAttribute(Mind, +600,0,-600); 
					Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 600000, performance_enhance_dance_mind, gWorldManager->GetCurrentGlobalTick());
					tempBuff1->AddAttribute(tempAttribute1);	
					playerObject->AddBuff(tempBuff1);

					BuffAttribute* tempAttribute2 = new BuffAttribute(Focus, +600,0,-600); 
					Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 600000, performance_enhance_music_focus, gWorldManager->GetCurrentGlobalTick());
					tempBuff2->AddAttribute(tempAttribute2);	
					playerObject->AddBuff(tempBuff2);

					BuffAttribute* tempAttribute3 = new BuffAttribute(Willpower, +600,0,-600); 
					Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 600000, performance_enhance_music_willpower, gWorldManager->GetCurrentGlobalTick());
					tempBuff3->AddAttribute(tempAttribute3);	
					playerObject->AddBuff(tempBuff3);
				}
				break;
				case 10: //Lloyd's Jawa Beer
				{
					gMessageLib->sendSystemMessage(playerObject, L"Attempting to Add Jawa Beer +15 Mask Scent");
					
					BuffAttribute* tempAttribute1 = new BuffAttribute(Mask_Scent, +15,0,-15); 
					Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 600000, food_drink_jawa_beer, gWorldManager->GetCurrentGlobalTick());
					tempBuff1->AddAttribute(tempAttribute1);	
					playerObject->AddBuff(tempBuff1);
				}
				break;
				case 11: //Health Damage
				{
					gMessageLib->sendSystemMessage(playerObject, L"Take That Sucker!");
					playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints, -200);
					
				}
				break;
				case 12: //Action Damage
				{
					gMessageLib->sendSystemMessage(playerObject, L"Take That Sucker!");
					playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, -200);
					
				}
				break;
				default:{}break;
			}
		}
		break;

		//================================
		// xp type selected

		case SUI_Window_CharacterBuilderXpMenu_ListBox:
		{
			if (element > (int32)playerObject->getXpList()->size() - 1 || element < 0)
			{
				gMessageLib->sendSystemMessage(playerObject, L"Error while giving Xp!");
			}
			else if (mSortedList)
			{	// Get the xp type for the selection.
				uint32 xpType = mSortedList->at(element).second;
				gSkillManager->addExperience(xpType,600000,playerObject);
			}
			delete mSortedList;
			mSortedList = NULL;
		}
		break;

		case SUI_Window_CharacterBuilderItemsMenu_ListBox:
		{
			if(element < 0 || element > (int32)mItemsTypes.size()){ break; }

			gObjectFactory->requestNewDefaultItem(dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory)),mItemsTypes[element].second,mItemsTypes[element].first,dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getId(),99,Anh_Math::Vector3(),"");
		}
		break;
		
		case SUI_Window_CharacterBuilderProfessionMastery_ListBox:
		{
			if(element < 0)
			{
				break;
			}

			SkillList* skillList = gSkillManager->getMasterProfessionList();
			SkillList::iterator skillIt = skillList->begin();
			skillIt+=element;
			gSkillManager->learnSkillLine((*skillIt)->mId, playerObject, false);		
		}
		break;

		default:{}break;
	}
}

//=============================================================================


