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
#include "WorldConfig.h"
#include "UIManager.h"
#include "Common/atMacroString.h"
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
#include "Utils/rand.h"

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

	if(window == NULL)
	{
		gLogger->log(LogManager::DEBUG,"UIManager::_processEventNotification: could not find window %u",windowId);
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

void UIManager::createNewListBox(UICallback* callback,const int8* eventStr,string caption,BString prompt,const BStringVector dataItems,PlayerObject* playerObject,ui_window_types windowType,uint8 lbType, uint64 object, float distance, void* container)
{
	if((!distance)&&object)
	{
		distance = gWorldConfig->getConfiguration<float>("Player_UI_Closure",(float)30.0);
	}

	uint32 lbId = _getFreeId();

	UIListBox* listBox =  new UIListBox(callback,lbId,windowType,eventStr,caption,prompt,dataItems,playerObject,lbType,distance, object, container);

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

void UIManager::createNewPlayerSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems, std::list<PlayerObject*> playerList,  PlayerObject* playerObject,uint8 lbType)
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
		gLogger->log(LogManager::DEBUG,"UIManager::destroyWindow: couldn't find window %u",id);
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
