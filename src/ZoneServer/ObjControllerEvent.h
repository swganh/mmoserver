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

#ifndef ANH_OBJCONTROLLER_EVENT_H
#define ANH_OBJCONTROLLER_EVENT_H

#include "Utils/typedefs.h"

namespace Anh_Utils
{
class Event;
}

//======================================================================================================================

class ObjControllerEvent
{
public:
    ObjControllerEvent(const uint64 executionTime, Anh_Utils::Event* event);

    virtual ~ObjControllerEvent();

    virtual uint64 getExecutionTime() const;
    virtual void setExecutionTime(uint64 time);

    virtual Anh_Utils::Event* getEvent() const;
    virtual void setEvent(Anh_Utils::Event* event);

protected:
    uint64				mExecutionTime;
    Anh_Utils::Event*	mEvent;
};

//===============================================================================================================

class CompareEvent
{
public:
    bool operator () (const ObjControllerEvent* left, const ObjControllerEvent* right)
    {
        return(left->getExecutionTime() > right->getExecutionTime());
    }
};

//===============================================================================================================

#endif

