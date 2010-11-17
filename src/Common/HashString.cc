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

#include "Common/HashString.h"

#include <cstring>

#include "Common/Crc.h"

namespace common {

std::ostream& operator<<(std::ostream& message, const HashString& string) {
    message << string.ident_string();
    return message;
}

HashString::HashString(char const * const ident_string)
    : ident_(reinterpret_cast<void*>(0))
    , ident_string_(ident_string) {
    if (ident_string) {
        ident_ = reinterpret_cast<void*>(memcrc(ident_string, strlen(ident_string)));
    }
}

HashString::~HashString() {}

bool HashString::operator<(const HashString& other) const {
    bool r = (ident() < other.ident());
    return r;
}

bool HashString::operator>(const HashString& other) const {
    bool r = (ident() > other.ident());
    return r;
}

bool HashString::operator==(const HashString& other) const {
    bool r = (ident() == other.ident());
    return r;
}

bool HashString::operator!=(const HashString& other) const {
    bool r = (ident() != other.ident());
    return r;
}

uint32_t HashString::ident() const {
    return static_cast<uint32_t>(reinterpret_cast<uint64_t>(ident_));
}

const std::string& HashString::ident_string() const {
    return ident_string_;
}

}  // namespace common
