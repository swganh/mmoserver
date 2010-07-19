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

#ifndef SRC_COMMON_EVENT_H_
#define SRC_COMMON_EVENT_H_

#include <memory>

#include "Common/ByteBuffer.h"
#include "Common/HashString.h"

namespace common {
    
// Use a HashString as the basis for EventType's.
typedef HashString EventType;

class ByteBuffer;

class Event {
public:
    explicit Event(const EventType& event_type);
    Event(const EventType& event_type, const ByteBuffer& subject);

    /**
     * Returns the type of the event.
     *
     * \returns The type of the event.
     */
    const EventType& event_type() const;
    
    /**
     * Checks to see if a subject has been set or not.
     *
     * \returns True if a subject has been set, false if not.
     */
    bool HasSubject() const;

    /**
     * Returns a throw-away copy of the subject.
     *
     * ByteBuffer's read functionality is non-const so a throw away copy is instead 
     * created and returned. Although this copy can be modified it does not affect
     * the source subject in the event itself.
     *
     * \returns A throw-away copy of the subject.
     */
    std::unique_ptr<ByteBuffer> subject() const;

private:
    EventType event_type_;
    ByteBuffer subject_;
};

}  // namespace common

#endif  // SRC_COMMON_EVENT_H_
