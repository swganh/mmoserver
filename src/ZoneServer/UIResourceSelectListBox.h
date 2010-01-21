/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIRESOURCESELECTLISTBOX_H
#define ANH_ZONESERVER_UIRESOURCESELECTLISTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"


//================================================================================

typedef std::vector<uint64>	ResourceIdList;

//================================================================================

class UIResourceSelectListBox : public UIListBox
{
	public:

		UIResourceSelectListBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,ResourceIdList resourceIdList,PlayerObject* playerObject,uint8 windowType,uint8 lbType)
			: UIListBox(callback,id,windowType,eventStr,caption,prompt,dataItems,playerObject,lbType), mResourceIdList(resourceIdList) {}

		virtual			~UIResourceSelectListBox(){}

		ResourceIdList	getResourceIdList(){ return mResourceIdList; }

	private:

		ResourceIdList	mResourceIdList;
};


//================================================================================

#endif
