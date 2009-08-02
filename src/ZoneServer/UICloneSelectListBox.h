/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UICLONESELECTLISTBOX_H
#define ANH_ZONESERVER_UICLONESELECTLISTBOX_H

#include "Utils/typedefs.h"
#include "BuildingObject.h"
#include "UIListBox.h"


//================================================================================

typedef std::vector<BuildingObject*> BuildingList;

//================================================================================

class UICloneSelectListBox : public UIListBox
{
	public:

		UICloneSelectListBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,BuildingList buildingList,PlayerObject* playerObject,uint8 lbType)
			: UIListBox(callback,id,SUI_Window_CloneSelect_ListBox,eventStr,caption,prompt,dataItems,playerObject,lbType), mBuildingList(buildingList) {}

		virtual			~UICloneSelectListBox(){}

		BuildingList*	getBuildingList(){ return &mBuildingList; }

	private:

		BuildingList	mBuildingList;
};


//================================================================================

#endif
