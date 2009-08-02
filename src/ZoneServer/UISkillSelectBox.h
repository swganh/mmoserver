/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UISKILLSELECT_LISTBOX_H
#define ANH_ZONESERVER_UISKILLSELECT_LISTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"
#include "PlayerObject.h"


//================================================================================

class UISkillSelectBox : public UIListBox
{
	public:

		UISkillSelectBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType,PlayerObject* pupil)
			: UIListBox(callback,id,SUI_Window_Teach_SelectSkill_ListBox,eventStr,caption,prompt,dataItems,playerObject,lbType),mPupil(pupil) {}

		virtual		~UISkillSelectBox(){}

		PlayerObject*		getPupil(){ return mPupil; }
		void				setPupil(PlayerObject* pupil){ mPupil = pupil; }

	private:

		PlayerObject*		mPupil;
};

//================================================================================

#endif


