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

#ifndef SRC_COMMON_APPLICATION_SERVICE_H_
#define SRC_COMMON_APPLICATION_SERVICE_H_

#include <cstdint>

#include "Utils/ActiveObject.h"

/*! \brief Common is a catch-all library containing primarily base classes and
 * classes used for maintaining application lifetimes.
 */
namespace common {

class EventDispatcher;

/*! \brief An interface class that defines the concept of the ApplicationService.
 *
 * Applications, especially server applications such as an MMO, consist of many
 * separately running entities that exist throughout the entire course of their
 * lifetimes.
 */
class IApplicationService {
public:
    virtual void tick(uint64_t new_timestamp) = 0;
};

/*! \brief The BaseApplicationService is an implementation of the IApplicationService
 * interface that provides concrete instances with the ability to run entirely
 * in their own threads of execution.
 *
 * An example of an ApplicationService would be the HamService which
 * manages the Heath, Action and Mind pools in Star Wars Galaxies. This service
 * runs on it's own thread and is entirely responsible for maintaining and
 * persisting the HAM data. It communicates with other services via an event/messaging
 * system which allows it to run independently on it's own thread without contention
 * from locks.
 */
class BaseApplicationService : public IApplicationService {
public:
    /*! \brief The only available constructor for BaseApplicationService.
     *
     * \param event_dispatcher An instance of an ApplicationService level event dispatcher.
     */
    explicit BaseApplicationService(EventDispatcher& event_dispatcher);

    /// Default deconstructor.
    virtual ~BaseApplicationService();

    /*! \brief Allows the ApplicationService an opportunity to move time dependent
    * actions forward.
    *
    * \param new_timestamp The new current time expressed in milliseconds since startup.
    */
    void tick(uint64_t new_timestamp);

    /*! \returns The current time expressed in milliseconds since startup.
     */
    uint64_t current_timestamp() const;

protected:
    EventDispatcher& event_dispatcher_;
    utils::ActiveObject active_;

    virtual void onTick() = 0;

private:
    // Disable compiler generated methods.
    BaseApplicationService();
    BaseApplicationService(const BaseApplicationService&);
    const BaseApplicationService& operator=(const BaseApplicationService&);

    uint64_t current_timestamp_;
};

} // namespace common

#endif  // SRC_COMMON_APPLICATION_SERVICE_H_
