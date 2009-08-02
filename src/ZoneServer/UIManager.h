/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIMANAGER_H
#define ANH_ZONESERVER_UIMANAGER_H

#include "Common/MessageDispatchCallback.h"
#include "UIOpcodes.h"
#include "UIEnums.h"
#include "UIWindow.h"
#include <boost/ptr_container/ptr_map.hpp>


//======================================================================================================================

#define 	gUIManager	UIManager::getSingletonPtr()

typedef boost::ptr_map<uint32,UIWindow>	UIWindowMap;
typedef	std::vector<uint64>				ResourceIdList;

//======================================================================================================================

struct LBContainer
{
	string port;
};

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class DispatchClient;
class UICallback;
class UISkillSelectBox;
class UIOfferTeachBox;
class UIResourceSelectListBox;
class BuildingObject;
class UICloneSelectListBox;

//======================================================================================================================

class UIManager : public MessageDispatchCallback
{
	public:

		~UIManager();
		static UIManager*	Init(Database* database,MessageDispatch* dispatch);
		static UIManager*	getSingletonPtr() { return mSingleton; }

		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);

		UIWindow*			getUIWindow(uint32 id);
		void				destroyUIWindow(uint32 id,bool sendForceClose = false);

		void				sendForceCloseWindow(UIWindow* window);

		// generic
		void				createNewMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,ui_window_types windowType = SUI_Window_MessageBox,uint8 mbType = SUI_MB_OK);
		void				createNewListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,ui_window_types windowType = SUI_Window_ListBox,uint8 lbType = SUI_LB_OK);
		void				createNewInputBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const BStringVector dropdownElements,PlayerObject* playerObject,uint8 ibType,ui_window_types windowType = SUI_Window_InputBox,uint16 maxInputLength = 127);
		void				createNewTransferBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,const int8* leftTitle,const int8* rightTitle,uint32 leftValue, uint32 rightValue,PlayerObject* playerObject);

		// custom list boxes
		void				createNewSkillSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType,PlayerObject* pupil);
		void				createNewPlayerSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerList playerList, PlayerObject* playerObject,uint8 lbType = SUI_LB_OK);
		void				createNewTicketSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,string port,Shuttle* shuttle,uint8 lbType = SUI_LB_OK);
		void				createNewResourceSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,ResourceIdList resourceIdList,PlayerObject* playerObject,uint8 windowType,uint8 lbType = SUI_LB_OK);
		void				createNewCloneSelectListBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,std::vector<BuildingObject*> buildingList,PlayerObject* playerObject,uint8 lbType = SUI_LB_OK);
		void				createNewDiagnoseListBox(UICallback* callback,PlayerObject* Medic,PlayerObject* Patient);
		// custom message boxes
		void				createNewSkillTeachMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType,PlayerObject* pupil,Skill* skill);

	private:

		UIManager(Database* database,MessageDispatch* dispatch);

		void				_processEventNotification(Message* message,DispatchClient* client);
		void				_registerCallbacks();
		void				_unregisterCallbacks();
		uint32				_getFreeId();

		static bool			mInsFlag;
		static UIManager*	mSingleton;

		Database*			mDatabase;
		MessageDispatch*    mMessageDispatch;

		UIWindowMap			mUIWindows;
};

//======================================================================================================================

#endif



