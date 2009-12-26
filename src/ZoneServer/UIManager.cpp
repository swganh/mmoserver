/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "UIManager.h"
#include "HarvesterObject.h"
#include "Inventory.h"
#include "Bank.h"
#include "PlayerObject.h"
#include "UICloneSelectListBox.h"
#include "UIInputBox.h"
#include "UIListBox.h"
#include "UIMessageBox.h"
#include "UIOfferTeachBox.h"
#include "UIOpcodes.h"
#include "UIPlayerSelectBox.h"
#include "UIResourceSelectListBox.h"
#include "UITicketSelectListBox.h"
#include "UITransferBox.h"
#include "UISkillSelectBox.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"

//======================================================================================================================

bool		UIManager::mInsFlag		= false;
UIManager*	UIManager::mSingleton	= NULL;

//======================================================================================================================

UIManager::UIManager(Database* database,MessageDispatch* dispatch) :
mDatabase(database),
mMessageDispatch(dispatch)
{
	_registerCallbacks();
}

//======================================================================================================================

UIManager* UIManager::Init(Database* database,MessageDispatch* dispatch)
{
	if(mInsFlag == false)
	{
		mSingleton = new UIManager(database,dispatch);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

UIManager::~UIManager()
{
	mUIWindows.clear();

	_unregisterCallbacks();

	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================

void UIManager::_registerCallbacks()
{
	mMessageDispatch->RegisterMessageCallback(opSuiEventNotification,this);
}

//======================================================================================================================

void UIManager::_unregisterCallbacks()
{
	mMessageDispatch->UnregisterMessageCallback(opSuiEventNotification);
}

//======================================================================================================================

UIWindow* UIManager::getUIWindow(uint32 id)
{
	UIWindowMap::iterator it = mUIWindows.find(id);

	if(it != mUIWindows.end())
		return((*it).second);

	return(NULL);
}

//======================================================================================================================

void UIManager::handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client)
{
	switch(opcode)
	{
		case opSuiEventNotification:
			_processEventNotification(message,client);
		break;

		default: break;
	} 
}

//======================================================================================================================

void UIManager::_processEventNotification(Message* message,DispatchClient* client)
{
	uint32		windowId	= message->getUint32();
	UIWindow*	window		= getUIWindow(windowId);

	gLogger->logMsgF("UI Event %u",MSG_LOW,windowId);

	if(window == NULL)
	{
		gLogger->logMsgF("UIManager::_processEventNotification: could not find window %u",MSG_NORMAL,windowId);
		return;
	}

	window->handleEvent(message);
}

//======================================================================================================================
//
// create a message box
//

void UIManager::createNewMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,ui_window_types windowType,uint8 mbType, void* container)
{
	uint32 mbId = _getFreeId();

	UIMessageBox* messageBox =  new UIMessageBox(callback,mbId,windowType,eventStr,caption,text,playerObject,mbType,container);

	mUIWindows.insert(mbId,messageBox);
	playerObject->addUIWindow(mbId);

	messageBox->sendCreate();
}

//======================================================================================================================
//
// create a listbox
//

void UIManager::createNewListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,ui_window_types windowType,uint8 lbType, void* container)
{
	uint32 lbId = _getFreeId();

	UIListBox* listBox =  new UIListBox(callback,lbId,windowType,eventStr,caption,prompt,dataItems,playerObject,lbType, container);

	mUIWindows.insert(lbId,listBox);
	playerObject->addUIWindow(lbId);

	listBox->sendCreate();
}

//======================================================================================================================
//
// create an input box
//

void UIManager::createNewInputBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const BStringVector dropdownElements,PlayerObject* playerObject,uint8 ibType,ui_window_types windowType,uint16 maxInputLength)
{
	uint32 ibId = _getFreeId();

	UIInputBox* inputBox =  new UIInputBox(callback,ibId,windowType,eventStr,caption,text,dropdownElements,playerObject,ibType,maxInputLength);

	mUIWindows.insert(ibId,inputBox);
	playerObject->addUIWindow(ibId);

	inputBox->sendCreate();
}

//======================================================================================================================
//
// create a skill select box(teaching)
//

void UIManager::createNewSkillSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType,PlayerObject* pupil)
{
	uint32 lbId = _getFreeId();

	UISkillSelectBox* listBox =  new UISkillSelectBox(callback,lbId,eventStr,caption,prompt,dataItems,playerObject,lbType,pupil);

	mUIWindows.insert(lbId,listBox);
	playerObject->addUIWindow(lbId);

	listBox->sendCreate();
}
//======================================================================================================================
//
// create a ticket select list box(travel - by command)
//

void UIManager::createNewTicketSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,string port,Shuttle* shuttle,uint8 lbType)
{
	uint32 lbId = _getFreeId();

	UITicketSelectListBox* ticketSelectBox =  new UITicketSelectListBox(callback,lbId,eventStr,caption,prompt,dataItems,playerObject,port,shuttle,lbType);

	mUIWindows.insert(lbId,ticketSelectBox);
	playerObject->addUIWindow(lbId);

	ticketSelectBox->sendCreate();
}

//======================================================================================================================
//
// create a transfer box(trade)
//

void UIManager::createNewTransferBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const int8* leftTitle,const int8* rightTitle,uint32 leftValue, uint32 rightValue,PlayerObject* playerObject, uint8 windowType)
{
	uint32 ibId = _getFreeId();

	UITransferBox* transferBox =  new UITransferBox(callback,ibId,eventStr,caption,text,leftTitle,rightTitle,leftValue,rightValue,playerObject,windowType);

	mUIWindows.insert(ibId,transferBox);
	playerObject->addUIWindow(ibId);

	transferBox->sendCreate();
}

//======================================================================================================================
//
// create a skill teach request message box(teaching)
//

void UIManager::createNewSkillTeachMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType,PlayerObject* pupil,Skill* skill)
{
	uint32 mbId = _getFreeId();

	UIOfferTeachBox* messageBox =  new UIOfferTeachBox(callback,mbId,eventStr,caption,text,playerObject,mbType,pupil,skill);

	mUIWindows.insert(mbId,messageBox);
	playerObject->addUIWindow(mbId);

	messageBox->sendCreate();
}

//======================================================================================================================
//
// create a player select list box(group loot master)
//

void UIManager::createNewPlayerSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems, std::vector<PlayerObject*> playerList,  PlayerObject* playerObject,uint8 lbType)
{
	uint32 lbId = _getFreeId();

	UIPlayerSelectBox* listBox =  new UIPlayerSelectBox(callback,lbId,eventStr,caption,prompt,dataItems,playerList,playerObject,lbType);

	mUIWindows.insert(lbId,listBox);
	playerObject->addUIWindow(lbId);

	listBox->sendCreate();
}

//======================================================================================================================
//
// create a resource picker list box, used for all types(category,resourceType,resource)
//

void UIManager::createNewResourceSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,ResourceIdList resourceIdList,PlayerObject* playerObject,uint8 windowType,uint8 lbType)
{
	uint32 lbId = _getFreeId();

	UIResourceSelectListBox* listBox =  new UIResourceSelectListBox(callback,lbId,eventStr,caption,prompt,dataItems,resourceIdList,playerObject,windowType,lbType);

	mUIWindows.insert(lbId,listBox);
	playerObject->addUIWindow(lbId);

	listBox->sendCreate();
}

//======================================================================================================================
//
// create a clone location select list box
//

void UIManager::createNewCloneSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,std::vector<BuildingObject*> buildingList,PlayerObject* playerObject,uint8 lbType)
{
	uint32 lbId = _getFreeId();

	UICloneSelectListBox* listBox =  new UICloneSelectListBox(callback,lbId,eventStr,caption,prompt,dataItems,buildingList,playerObject,lbType);

	mUIWindows.insert(lbId,listBox);
	playerObject->addUIWindow(lbId);

	listBox->sendCreate();
}

//======================================================================================================================
//
// get a random window id
//

uint32 UIManager::_getFreeId()
{
	uint32 id;

	do
	{
#if defined(_MSC_VER)
		id = gRandom->getRand()%4294967294 + 1;
#else
		id = gRandom->getRand()%4294967294LLU + 1;
#endif
	}
	while(getUIWindow(id) != NULL);

	return(id);
}

//======================================================================================================================

void UIManager::destroyUIWindow(uint32 id,bool sendForceClose)
{
	UIWindowMap::iterator it = mUIWindows.find(id);

	if(it != mUIWindows.end())
	{
		if(sendForceClose)
		{
			sendForceCloseWindow((*it).second);
		}

		mUIWindows.erase(it);	
	}
	else
		gLogger->logMsgF("UIManager::destroyWindow: couldn't find window %u",MSG_NORMAL,id);
}

//======================================================================================================================

void UIManager::sendForceCloseWindow(UIWindow* window)
{
	PlayerObject*	player = window->getOwner();
	if(!player ||( player->getConnectionState() != PlayerConnState_Connected))
		return;

	Message*		newMessage;
	

	gMessageFactory->StartMessage();             
	gMessageFactory->addUint32(opSuiForceClosePage);  
	gMessageFactory->addUint32(window->getId());

	newMessage = gMessageFactory->EndMessage();

	
	(player->getClient())->SendChannelA(newMessage,player->getAccountId(),CR_Client,2);
}

//======================================================================================================================
void UIManager::createNewDiagnoseListBox(UICallback* callback,PlayerObject* Medic, PlayerObject* Patient)
{
	BStringVector attributesMenu;

	BString FirstName = Patient->getFirstName(); FirstName.toUpper();
	BString LastName = Patient->getLastName(); LastName.toUpper();

	int8 title[64];
	sprintf(title,"PATIENT %s %s'S WOUNDS",FirstName.getAnsi(), LastName.getAnsi());

	int8 desc[512];
	sprintf(desc, "Below is a listing of the Wound and Battle Fatigue levels of %s %s. Wounds are healed through /tendWound or use of Wound Medpacks. High levels of Battle Fatigue can inhibit the healing process, and Battle Fatigue can only be healed by the patient choosing to watch performing entertainers",Patient->getFirstName().getAnsi(), Patient->getLastName().getAnsi());

	int8 Health[32];
	sprintf(Health,"Health -- %i",Patient->getHam()->mHealth.getWounds());
	attributesMenu.push_back(Health);

	int8 Strength[32];
	sprintf(Strength,"Strength -- %i",Patient->getHam()->mStrength.getWounds());
	attributesMenu.push_back(Strength);
	
	int8 Constitution[32];
	sprintf(Constitution,"Constitution -- %i",Patient->getHam()->mConstitution.getWounds());
	attributesMenu.push_back(Constitution);
	
	int8 Action[32];
	sprintf(Action,"Action -- %i",Patient->getHam()->mAction.getWounds());
	attributesMenu.push_back(Action);

	int8 Quickness[32];
	sprintf(Quickness,"Quickness -- %i",Patient->getHam()->mQuickness.getWounds());
	attributesMenu.push_back(Quickness);

	int8 Stamina[32];
	sprintf(Stamina,"Stamina -- %i",Patient->getHam()->mStamina.getWounds());
	attributesMenu.push_back(Stamina);

	int8 Mind[32];
	sprintf(Mind,"Mind -- %i",Patient->getHam()->mMind.getWounds());
	attributesMenu.push_back(Mind);

	int8 Focus[32];
	sprintf(Focus,"Focus -- %i",Patient->getHam()->mFocus.getWounds());
	attributesMenu.push_back(Focus);

	int8 Willpower[32];
	sprintf(Willpower,"Willpower -- %i",Patient->getHam()->mWillpower.getWounds());
	attributesMenu.push_back(Willpower);

	int8 BattleFatigue[32];
	sprintf(BattleFatigue,"Battle Fatigue -- %i",Patient->getHam()->getBattleFatigue());
	attributesMenu.push_back(BattleFatigue);

	createNewListBox(callback,"handleDiagnoseMenu",title, desc, attributesMenu, Medic, SUI_Window_ListBox);
}

void UIManager::createNewStructureDestroyBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure, bool redeed)
{
	BStringVector attributesMenu;

	string text = "You have elected to destroy a structure. Pertinent structure data can be found in the list below. Please complete the following steps to confirm structure deletion.\xa\xa";
			text <<"If you wish to redeed your structure, all structure data must be GREEN. \xa To continue with structure deletion, click YES. Otherwise, please click NO.\xa";
			
	if(structure->canRedeed())
	{
		text <<"WILL REDEED: \\#006400 YES \\#FFFFFF";			

		int8 redeedText[64];
		sprintf(redeedText,"CAN REDEED: \\#006400 YES\\#FFFFFF");
		attributesMenu.push_back(redeedText);
	}
	else
	{
		text <<"WILL REDEED: \\#FF0000 NO \\#FFFFFF";			

		int8 redeedText[64];
		sprintf(redeedText,"CAN REDEED: \\#FF0000 NO\\#FFFFFF");
		attributesMenu.push_back(redeedText);
	}
	
	uint32 maxCond = structure->getMaxCondition();
	uint32 cond = structure->getCondition();
		
	if( cond < maxCond)
	{
		int8 condition[64];
		sprintf(condition,"-CONDITION:\\#FF0000%u/%u\\#FFFFFF",cond,maxCond);
		attributesMenu.push_back(condition);
		
	}
	else
	{
		int8 condition[64];
		sprintf(condition,"-CONDITION:\\#006400%u/%u\\#FFFFFF",cond,maxCond);
		attributesMenu.push_back(condition);
	}


	uint32 maintIs = structure->getCurrentMaintenance();
	uint32 maintNeed = structure->getMaintenanceRate()*45;

	if(maintIs >= maintNeed)
	{
		int8 maintenance[128];
		sprintf(maintenance,"-MAINTENANCE:\\#006400%u/%u\\#FFFFFF",maintIs,maintNeed);
		attributesMenu.push_back(maintenance);
	}
	else
	{
		int8 maintenance[128];
		sprintf(maintenance,"-MAINTENANCE:\\#FF0000%u/%u\\#FFFFFF",maintIs,maintNeed);
		attributesMenu.push_back(maintenance);
	}

	int8 sName[128];

	string name = structure->getCustomName();			
	name.convert(BSTRType_ANSI);
	sprintf(sName,"%s",name.getAnsi());
	if(!name.getLength())
	{
		sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());
		
	}

	//answer = x/(total/100);
	//answer = x*(total/100);
	// total = 100%
	
	createNewListBox(callback,"handle Structure Destroy",sName, text.getAnsi(), attributesMenu, player, SUI_Window_Structure_Delete,SUI_LB_OKCANCEL);
}

void UIManager::createNewStructureDeleteConfirmBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure)
{

	string text = "Your structure";
	if(structure->getRedeed())
	{
		text <<"\\#006400WILL\\#FFFFFF ";
	}
	else
		text <<"\\#FF0000WILL NOT \\#FFFFFF";

	text <<"be redeeded. If you wish to continue with destroying your structure, please enter the following code into the input box.\xa\xa";
	
	int8 code [32];
	structure->setCode();
	sprintf(code,"code: %s",structure->getCode().getAnsi());
	text << code;

	int8 caption[32];
	sprintf(caption,"CONFIRM STRUCTURE DESTRUCTION");

	BStringVector vector;

	createNewInputBox(callback,"",caption,text.getAnsi(),vector,player,SUI_IB_NODROPDOWN_OKCANCEL,SUI_Window_Structure_Delete_Confirm,6);
	
}

//============================================================================================
//	 transfers maintenance between player and structure
//

void UIManager::createPayMaintenanceTransferBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure)
{
	int32 structureFunds = structure->getCurrentMaintenance();

	int8 text[255];
	sprintf(text,"Select the total amount you would like to pay to the existing maintenance pool.\xa\xa Current maintenance pool: %u cr.",structureFunds);
	
	int8 caption[32];
	sprintf(caption,"SELECT AMOUNT");
	int8 sName[128];

	string name = structure->getCustomName();			
	name.convert(BSTRType_ANSI);
	sprintf(sName,"%s",name.getAnsi());
	if(!name.getLength())
	{
		sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());
		
	}


	uint32 funds = dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits();
	funds += dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits();

	createNewTransferBox(callback,sName,caption,text,"Total Funds","To Pay",funds,structureFunds,player,SUI_Window_Pay_Maintenance);
	
}


//============================================================================================
//Transfers power between inventory and harvester
//
void UIManager::createPowerTransferBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure)
{

	int8 text[255];
	
	uint32 structurePower = structure->getCurrentPower();
	uint32 playerPower = gStructureManager->getCurrentPower(player);

	sprintf(text,"Select the amount of power you would like to deposit.\xa\xa Current Power Value = %u ",structurePower);
	
	int8 caption[32];
	sprintf(caption,"SELECT AMOUNT");
	int8 sName[128];

	string name = structure->getCustomName();			
	name.convert(BSTRType_ANSI);
	sprintf(sName,"%s",name.getAnsi());
	if(!name.getLength())
	{
		sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());
		
	}

	createNewTransferBox(callback,sName,caption,text,"Total Energy","To Deposit",playerPower,0,player,SUI_Window_Deposit_Power);
	
}




//============================================================================================
// Renames a structure
//
void UIManager::createRenameStructureBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure)
{

	string text = "Please enter the new name you would like for this object.";
	
	int8 caption[32];
	sprintf(caption,"NAME THE OBJECT");

	BStringVector vector;


	int8 sName[128];

	string name = structure->getCustomName();			
	name.convert(BSTRType_ANSI);
	sprintf(sName,"%s",name.getAnsi());
	if(!name.getLength())
	{
		sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());
		
	}

	vector.push_back(sName);

	createNewInputBox(callback,sName,caption,text.getAnsi(),vector,player,SUI_IB_NODROPDOWN_OKCANCEL,SUI_Window_Structure_Rename,68);
	
}





//============================================================================================
// Creates the status box for a structure
//
void UIManager::createNewStructureStatusBox(UICallback* callback,PlayerObject* player, PlayerStructure* structure)
{

	int8 sName[128];

	string name = structure->getCustomName();			
	name.convert(BSTRType_ANSI);
	sprintf(sName,"%s",name.getAnsi());
	if(!name.getLength())
	{
		sprintf(sName,"@%s:%s",structure->getNameFile().getAnsi(),structure->getName().getAnsi());
		
	}

	int8 wText[128];
	sprintf(wText,"Structure Name: %s",sName);

	BStringVector attributesMenu;

	//Owner
	int8 text[128];
	sprintf(text,"Owner:%s",structure->getOwnersName());
	attributesMenu.push_back(text);


	//private vs public
	if(structure->getPrivate())
	{
		sprintf(text,"This structure is private");
	}
	else
	{
		sprintf(text,"This structure is public");
	}

	attributesMenu.push_back(text);

	// condition
	uint32 currentCondition = structure->getMaxCondition() - structure->getDamage();

	sprintf(text,"Condition: %u%s",(uint32)(currentCondition/(structure->getMaxCondition() /100)),"%");

	attributesMenu.push_back(text);

	
	//Maintenance Pool
	float maint = (float)structure->getCurrentMaintenance();
	float rate  = (float)structure->getMaintenanceRate();
	uint32 hours , days, minutes;
	

	days = (uint32)(maint / (rate *24));
	maint -= days *(rate*24);

	hours = (uint32)(maint / rate);
	maint -= (uint32)(hours *rate);

	minutes = (uint32)(maint/(rate/60));
	
	sprintf(text,"Maintenance Pool: %f(%u days, %u hours, %u minutes)",(float)structure->getCurrentMaintenance(),days,hours,minutes);
	attributesMenu.push_back(text);

	//Maintenance rate
	sprintf(text,"Maintenance Rate: %f/hr",rate);
	attributesMenu.push_back(text);

	//Power Pool
	uint32 power = structure->getCurrentPower();
	rate = (float)structure->getPowerConsumption();
	
	days = (uint32)(power / (rate *24));
	power -=(uint32)( days *(rate*24));
	
	hours = (uint32)(power / rate);
	power -= (uint32)(hours *rate);
	
	minutes = (uint32)(power/ (rate/60));
	
	sprintf(text,"Power Reserves: %u(%u days, %u hours, %u minutes)",structure->getCurrentPower(),days,hours,minutes);
	attributesMenu.push_back(text);

	//Power Consumption
	sprintf(text,"Power Consumption: %u units/hr",structure->getPowerConsumption());
	attributesMenu.push_back(text);

	

	//answer = x/(total/100);
	//answer = x*(total/100);
	// total = 100%
	
	createNewListBox(callback,"handle Structure Destroy","STRUCTURE STATUS", wText, attributesMenu, player, SUI_Window_Structure_Status,SUI_LB_CANCELREFRESH);
}
