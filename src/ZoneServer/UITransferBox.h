/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UITRANSFERBOX_H
#define ANH_ZONESERVER_UITRANSFERBOX_H

#include "Utils/typedefs.h"
#include "UIWindow.h"

//================================================================================

class UITransferBox : public UIWindow
{
	public:

		UITransferBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt
			 ,const int8* leftTitle,const int8* rightTitle,uint32 leftValue, uint32 rightValue,PlayerObject* playerObject, uint8 windowType = SUI_Window_Transfer_Box);

		virtual ~UITransferBox();

		
		void		handleEvent(Message* message);
		void		sendCreate();

	private:

		void		_initChildren();

		string		mCaption;
		string		mPrompt;
		string		mLeftTitle;
		string		mRightTitle;
		string		mLeftValue;
		string		mRightValue;
		uint8		mLbType;
};

//================================================================================

#endif
