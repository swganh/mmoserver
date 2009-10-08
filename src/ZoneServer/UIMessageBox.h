/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIMESSAGEBOX_H
#define ANH_ZONESERVER_UIMESSAGEBOX_H

#include "Utils/typedefs.h"
#include "UIWindow.h"


//================================================================================


class UIMessageBox : public UIWindow
{
	public:

		UIMessageBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType = SUI_MB_OK);
		virtual ~UIMessageBox();

		virtual void	handleEvent(Message* message);
		void			sendCreate();

	protected:

		void	_initChildren();

		string		mCaption;
		string		mText;
		uint8		mMBType;

};

//================================================================================

#endif




