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

    BString		mCaption;
    BString		mPrompt;
    BString		mLeftTitle;
    BString		mRightTitle;
    BString		mLeftValue;
    BString		mRightValue;
    uint8		mLbType;
};

//================================================================================

#endif
