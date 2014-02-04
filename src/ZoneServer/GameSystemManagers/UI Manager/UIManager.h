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

#ifndef ANH_ZONESERVER_UIMANAGER_H
#define ANH_ZONESERVER_UIMANAGER_H

#include "Zoneserver/GameSystemManagers/UI Manager/UIEnums.h"
#include "UIWindow.h"
#include <boost/ptr_container/ptr_map.hpp>
#include <list>


//======================================================================================================================

#define 	gUIManager	UIManager::getSingletonPtr()

typedef boost::ptr_map<uint32,UIWindow>	UIWindowMap;
typedef	std::vector<uint64>				ResourceIdList;
typedef std::vector<std::string>		StringVector;

//======================================================================================================================

struct LBContainer
{
    BString port;
};

//======================================================================================================================

class Message;
class Shuttle;
//class Database;
class MessageDispatch;
class DispatchClient;
class Skill;
class UICallback;
class UISkillSelectBox;
class UIOfferTeachBox;
class UIResourceSelectListBox;
class BuildingObject;
class UICloneSelectListBox;
class PlayerStructure;
class FactoryObject;

//======================================================================================================================

class UIManager
{
public:

    ~UIManager();
    static UIManager*	Init(MessageDispatch* dispatch);
    static UIManager*	getSingletonPtr() {
        return mSingleton;
    }

    UIWindow*			getUIWindow(uint32 id);
    void				destroyUIWindow(uint32 id,bool sendForceClose = false);

    void				sendForceCloseWindow(UIWindow* window);

    // generic
    void				createNewMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,ui_window_types windowType = SUI_Window_MessageBox,uint8 mbType = SUI_MB_OK, std::shared_ptr<WindowAsyncContainerCommand> container = nullptr);
	void				createNewListBox(UICallback* callback,const int8* eventStr,BString caption,BString prompt,const StringVector dataItems,PlayerObject* playerObject,ui_window_types windowType = SUI_Window_ListBox,uint8 lbType = SUI_LB_OK,uint64 object = 0, float distance = 0, std::shared_ptr<WindowAsyncContainerCommand> container = nullptr);
    void				createNewInputBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const StringVector dropdownElements,PlayerObject* playerObject,uint8 ibType,ui_window_types windowType = SUI_Window_InputBox,uint16 maxInputLength = 127, std::shared_ptr<WindowAsyncContainerCommand> container = nullptr);
    
	/*	@brief creates a transfer box like for example the dialog to transfer maintenance to / from a harvester
	*
	*/
	void				createNewTransferBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const int8* leftTitle,const int8* rightTitle,uint32 leftValue, uint32 rightValue,PlayerObject* playerObject, uint8 windowType = SUI_Window_Transfer_Box, std::shared_ptr<WindowAsyncContainerCommand> container = nullptr);

    // custom list boxes
    void				createNewSkillSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const StringVector dataItems,PlayerObject* playerObject,uint8 lbType,PlayerObject* pupil);
    void				createNewPlayerSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const StringVector dataItems, std::list<PlayerObject*> playerList, PlayerObject* playerObject,uint8 lbType = SUI_LB_OK);
    void				createNewTicketSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const StringVector dataItems,PlayerObject* playerObject,BString port,Shuttle* shuttle,uint8 lbType = SUI_LB_OK);
    void				createNewResourceSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const StringVector dataItems,ResourceIdList resourceIdList,PlayerObject* playerObject,uint8 windowType,uint8 lbType = SUI_LB_OK);
    void				createNewCloneSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const StringVector dataItems,std::vector<BuildingObject*> buildingList,PlayerObject* playerObject,uint8 lbType = SUI_LB_OK);
    void				createNewDiagnoseListBox(UICallback* callback,PlayerObject* Medic,PlayerObject* Patient, StringVector vector_string);



    // custom message boxes
    void				createNewSkillTeachMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType,PlayerObject* pupil,Skill* skill);

private:

    UIManager(MessageDispatch* dispatch);

    void				_processEventNotification(Message* message,DispatchClient* client);
    void				_registerCallbacks();
    void				_unregisterCallbacks();
    uint32				_getFreeId();

    static bool			mInsFlag;
    static UIManager*	mSingleton;

    //swganh::database::Database*			mDatabase;
    MessageDispatch*    mMessageDispatch;

    UIWindowMap			mUIWindows;
};

//======================================================================================================================

#endif



