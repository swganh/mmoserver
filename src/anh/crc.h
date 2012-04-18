/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2010 The SWG:ANH Team

 MMOServer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 MMOServer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with MMOServer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LIBANH_CRC_H_
#define LIBANH_CRC_H_

#include <cstdint>
#include <string>

namespace anh {

/*! Calculates a 32-bit checksum of a c-style string.
 *
 * Variation on the memcrc function which is part of the cksum utility.
 *
 * \see http://www.opengroup.org/onlinepubs/009695399/utilities/cksum.html
 *
 * \param source_string The string to use as the basis for generating the checksum.
 * \param length The length of the source_string.
 * \returns A 32-bit checksum of the string.
 */
uint32_t memcrc(const char* source_string, uint32_t length);

/*! Calculates a 32-bit checksum of a std::string.
 *
 * Variation on the memcrc function which is part of the cksum utility.
 *
 * \see http://www.opengroup.org/onlinepubs/009695399/utilities/cksum.html
 *
 * \param source_string The string to use as the basis for generating the checksum.
 * \returns A 32-bit checksum of the string.
 */
uint32_t memcrc(const std::string& source_string);

}  // namespace anh

#endif  // LIBANH_CRC_H_
