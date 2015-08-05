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

#ifndef ANH_UTILS_EVENTHANDLER_H
#define ANH_UTILS_EVENTHANDLER_H

#include <typeinfo>
#include <boost/ptr_container/ptr_map.hpp>


//======================================================================================================================

namespace Anh_Utils
{

class TypeInfo;

//======================================================================================================================
//
// Event
//

class Event
{
public:

    virtual ~Event() {};
};

//======================================================================================================================
//
// HandlerFunctionBase
//

class HandlerFunctionBase
{
public:

    virtual ~HandlerFunctionBase() {};
    void execute(const Event* event) {
        call(event);
    }

private:

    virtual void call(const Event*) {}
};

//======================================================================================================================
//
// MemberFunctionHandler
//

template <class T,class EventT>
class MemberFunctionHandler : public HandlerFunctionBase
{
public:

    typedef void (T::*MemberFunc)(const EventT*);
    MemberFunctionHandler(T* instance,MemberFunc memFn) : mInstance(instance),mFunction(memFn) {}

    void call(const Event* event)
    {
        (mInstance->*mFunction)(static_cast<const EventT*>(event));
    }

private:

    T*			mInstance;
    MemberFunc	mFunction;
};

//======================================================================================================================
//
// EventHandler
//

class EventHandler
{
public:
    virtual ~EventHandler();

    void handleEvent(const Event*);

    template <class T,class EventT>
    void registerEventFunction(T*,void(T::*memFn)(const EventT*));

private:

    typedef boost::ptr_map<const TypeInfo, HandlerFunctionBase> Handlers;
    Handlers mHandlers;
};

//======================================================================================================================
//
// registerEventFunction
//

template <class T,class EventT>
void EventHandler::registerEventFunction(T* obj,void (T::*memFn)(const EventT*))
{
    mHandlers.insert(TypeInfo(typeid(EventT)), new MemberFunctionHandler<T,EventT>(obj,memFn));
}

//======================================================================================================================
//
// TypeInfo
//

class TypeInfo
{
public:

    explicit TypeInfo(const std::type_info& info) : mTypeInfo(info) {}

    bool operator < (const TypeInfo& rhs) const
    {
        return mTypeInfo.before(rhs.mTypeInfo) != 0;
    }

private:

    const std::type_info& mTypeInfo;
};

//======================================================================================================================

}

#endif
