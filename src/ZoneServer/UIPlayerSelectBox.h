/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIPLAYERSELECTBOX_H
#define ANH_ZONESERVER_UIPLAYERSELECTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"
#include "PlayerObject.h"

//================================================================================
typedef std::vector<PlayerObject*>				PlayerList;
//================================================================================


class UIPlayerSelectBox : public UIListBox
{
	public:

		UIPlayerSelectBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems, PlayerList playerList, PlayerObject* playerObject,uint8 lbType)
			: UIListBox(callback,id,SUI_Window_SelectGroupLootMaster_Listbox,eventStr,caption,prompt,dataItems,playerObject,lbType), mPlayers(playerList) {}

		virtual			~UIPlayerSelectBox(){}

		PlayerList		getPlayers(){ return mPlayers; }

	private:

		PlayerList		mPlayers;
};


//================================================================================

#endif
