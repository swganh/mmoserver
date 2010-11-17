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

#ifndef ANH_ZONESERVER_UIDROPDOWNBOX_H
#define ANH_ZONESERVER_UIDROPDOWNBOX_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "UIElement.h"

//================================================================================

class UIDropdownBox : public UIElement
{
public:

    UIDropdownBox() {}
    UIDropdownBox(uint32 id,BString name,bool enabled,const BStringVector elements,uint16 maxLength);
    virtual ~UIDropdownBox();

    BString			getName() {
        return mName;
    }
    void			setName(BString name) {
        mName = name;
    }

    bool			getEnabled() {
        return mEnabled;
    }
    void			setEnabled(bool e) {
        mEnabled = e;
    }

    uint16			getMaxLength() {
        return mMaxLength;
    }
    void			setMaxLength(uint16 maxLength) {
        mMaxLength = maxLength;
    }

    BStringVector*	getElements() {
        return &mElements;
    }

    virtual uint32	getPropertyCount();

    virtual void	addMessageData();

private:

    BString			mName;
    bool			mEnabled;
    BStringVector	mElements;
    uint16			mMaxLength;
};

#endif


