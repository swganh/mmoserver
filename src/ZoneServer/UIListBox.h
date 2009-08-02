/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

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

		UIListBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType = SUI_LB_OK);
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
};

//================================================================================

#endif




