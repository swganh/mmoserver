/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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


#pragma once

#include "ZoneServer/Objects/Object/object_message_builder.h"

class TangibleObject;

class TangibleMessageBuilder : public BaseMessageBuilder
{
public:
    TangibleMessageBuilder(swganh::event_dispatcher::EventDispatcher* dispatcher) :
            BaseMessageBuilder(dispatcher)
    {
      //  RegisterEventHandlers();
    }

    virtual ~TangibleMessageBuilder() {}

    // deltas
	/*
    static void BuildCustomizationDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildComponentCustomizationDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildOptionsMaskDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildCounterDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildConditionDamageDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildMaxConditionDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildStaticDelta(const std::shared_ptr<TangibleObject>& tangible);
    static void BuildDefendersDelta(const std::shared_ptr<TangibleObject>& tangible);

    // baselines
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline3(const std::shared_ptr<TangibleObject>& tangible, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline6(const std::shared_ptr<TangibleObject>& tangible, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline7(const std::shared_ptr<TangibleObject>& tangible, boost::unique_lock<boost::mutex>& lock);
	*/
private:
	typedef swganh::event_dispatcher::ValueEvent<std::shared_ptr<TangibleObject>> TangibleEvent;

	swganh::event_dispatcher::EventDispatcher* event_dispatcher_;
    //void RegisterEventHandlers();
};