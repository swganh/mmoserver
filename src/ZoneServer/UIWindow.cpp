/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "UIWindow.h"
#include "UICallback.h"
#include "NetworkManager/Message.h"


//================================================================================

UIWindow::UIWindow(UICallback* callback,uint32 id,uint8 windowType,const BString windowTypeStr,const int8* eventStr, void* container)
    : UIElement(id,SUI_Element_Window)
    , mWindowTypeStr(windowTypeStr)
    , mUICallback(callback)
    , mWindowType(windowType)
{
    mEventStr = eventStr;
    mChildElements.reserve(5);
    mContainer = container;
}

//================================================================================

UIWindow::~UIWindow()
{
    Children::iterator it = mChildElements.begin();

    while(it != mChildElements.end())
    {
        delete(*it);
        mChildElements.erase(it);
        it = mChildElements.begin();
    }
}

//================================================================================

UIElement* UIWindow::getChildById(uint32 id)
{
    Children::iterator it = mChildElements.begin();

    while(it != mChildElements.end())
    {
        if((*it)->getId() == id)
            return(*it);

        ++it;
    }

    return nullptr;
}

//================================================================================

bool UIWindow::removeChild(UIElement* element)
{
    Children::iterator it = mChildElements.begin();

    while(it != mChildElements.end())
    {
        if((*it) == element)
        {
            mChildElements.erase(it);
            return(true);
        }

        ++it;
    }

    return(false);
}

//================================================================================

bool UIWindow::removeChild(uint32 id)
{
    Children::iterator it = mChildElements.begin();

    while(it != mChildElements.end())
    {
        if((*it)->getId() == id)
        {
            mChildElements.erase(it);
            return(true);
        }

        ++it;
    }

    return(false);
}

//================================================================================

uint32 UIWindow::getChildrenPropertyCount()
{
    uint32 count = 0;
    Children::iterator childrenIt = mChildElements.begin();

    while(childrenIt != mChildElements.end())
    {
        count += (*childrenIt)->getPropertyCount();
        ++childrenIt;
    }

    return(count);
}

//================================================================================


