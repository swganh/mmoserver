/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UITICKETSELECT_LISTBOX_H
#define ANH_ZONESERVER_UITICKETSELECT_LISTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"

class PlayerObject;
class Shuttle;


//================================================================================

class UITicketSelectListBox : public UIListBox
{
	public:

		UITicketSelectListBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,string port,Shuttle* shuttle,uint8 lbType)
			: UIListBox(callback,id,SUI_Window_TicketSelect_ListBox,eventStr,caption,prompt,dataItems,playerObject,lbType),mShuttle(shuttle)
		{
			mPort = port.getAnsi();
		}

		virtual		~UITicketSelectListBox(){}

		string		getPort(){ return mPort; }
		Shuttle*	getShuttle(){ return mShuttle; }

	private:

		Shuttle*	mShuttle;
		string		mPort;
};

//================================================================================

#endif


