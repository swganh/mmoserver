/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_UISKILLSELECT_LISTBOX_H
#define ANH_ZONESERVER_UISKILLSELECT_LISTBOX_H

#include "Utils/typedefs.h"
#include "UIListBox.h"

class PlayerObject;


//================================================================================

class UISkillSelectBox : public UIListBox
{
public:

    UISkillSelectBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType,PlayerObject* pupil)
        : UIListBox(callback,id,SUI_Window_Teach_SelectSkill_ListBox,eventStr,caption,prompt,dataItems,playerObject,lbType),mPupil(pupil) {}

    virtual		~UISkillSelectBox() {}

    PlayerObject*		getPupil() {
        return mPupil;
    }
    void				setPupil(PlayerObject* pupil) {
        mPupil = pupil;
    }

private:

    PlayerObject*		mPupil;
};

//================================================================================

#endif


