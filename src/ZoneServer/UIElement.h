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

#ifndef ANH_ZONESERVER_UIELEMENT_H
#define ANH_ZONESERVER_UIELEMENT_H

#include "Utils/typedefs.h"
#include "UIEnums.h"

//================================================================================

class UIElement
{
public:

    UIElement() {}
    UIElement(uint32 id,uint8 elementType);
    virtual ~UIElement();

    uint32			getId() {
        return mId;
    }
    void			setId(uint32 id) {
        mId = id;
    }

    uint8			getElementType() {
        return mElementType;
    }
    void			setElementType(uint8 type) {
        mElementType = type;
    }

    virtual void	addMessageData() {}
    virtual uint32	getPropertyCount() {
        return(0);
    }

protected:

    uint32	mId;
    uint8	mElementType;
};

#endif


