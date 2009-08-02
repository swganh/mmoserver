/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIOFFERTEACH_MESSAGEBOX_H
#define ANH_ZONESERVER_UIOFFERTEACH_MESSAGEBOX_H

#include "Utils/typedefs.h"
#include "UIMessageBox.h"
#include "PlayerObject.h"
#include "Skill.h"


//================================================================================

class UIOfferTeachBox : public UIMessageBox
{
	public:

		UIOfferTeachBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType,PlayerObject* pupil,Skill* skill)
			: UIMessageBox(callback,id,SUI_Window_Teach_OfferSkill_MsgBox,eventStr,caption,text,playerObject,mbType),mPupil(pupil),mSkill(skill){}

		virtual ~UIOfferTeachBox(){}

		PlayerObject*		getPupil(){ return mPupil; }
		void				setPupil(PlayerObject* pupil){ mPupil = pupil; }
		Skill*				getSkill(){ return mSkill; }
		void				setSkill(Skill* skill){ mSkill = skill; }

	private:

		PlayerObject*		mPupil;
		Skill*				mSkill;

};

//================================================================================

#endif


