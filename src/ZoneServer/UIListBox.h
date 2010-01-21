/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UILISTBOX_H
#define ANH_ZONESERVER_UILISTBOX_H

#include "Utils/typedefs.h"
#include "UIWindow.h"


//================================================================================

class UIListBox : public UIWindow
{
	public:

		UIListBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,string caption,string prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType = SUI_LB_OK, float distance = 0, uint64 object = 0, void* container = NULL);
		virtual ~UIListBox();

		BStringVector*	getDataItems(){ return &mDataItems; }
		void			addDataItem(string item){ mDataItems.push_back(item); }

		virtual void	handleEvent(Message* message);
		void			sendCreate();

	protected:

		void		_initChildren();

		string			mCaption;

		string			mPrompt;
		uint8			mLbType;
		BStringVector	mDataItems;
		float			mDistance;
		uint64			mObjectID;
};

//================================================================================

#endif




