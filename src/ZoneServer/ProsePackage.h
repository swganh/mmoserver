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

#ifndef SRC_ZONESERVER_OUTOFBANDPACKAGE_H_
#define SRC_ZONESERVER_OUTOFBANDPACKAGE_H_

#include <cstdint>
#include <string>

class MessageFactory;

/**
 * The ProsePackage is a utility class for assisting in the use of custom messages.
 *
 * Several types of text responses such as spatial chat and system messages use 
 * this package for displaying custom text or text from an STF file. Once initialized
 * with data the ProsePackage can then be appended to these types of messages.
 *
 * The ProsePackage works off the base STF message passed in. This message can contain
 * place holders which are replaced by the associated variables. It is believed the 
 * priority within the different types is: object id -> stf -> custom string
 *
 * Placeholders:
 *      TU - Text You
 *      TT - Text Target
 *      TO - Text Object
 *      DI - Decimal Integer
 *      DF - Decimal Float
 *
 * Credit to Xenozephyr for information.
 */
class ProsePackage {
public:
    ProsePackage();
    ProsePackage(std::string base_stf_file, std::string base_stf_string, 
        uint64_t tu_object_id = 0, std::string tu_stf_file = "", std::string tu_stf_string ="", std::wstring tu_custom_string = L"",
        uint64_t tt_object_id = 0, std::string tt_stf_file = "", std::string tt_stf_string ="", std::wstring tt_custom_string = L"",
        uint64_t to_object_id = 0, std::string to_stf_file = "", std::string to_stf_string ="", std::wstring to_custom_string = L"",
        int32_t di_integer = 0, float df_float = 0.0f, uint8_t display_flag = 0);

    ~ProsePackage();

    /**
     * Write's the ProsePackage to a message that is currently being constructed.
     *
     * @note: Ideally this class should be streaming out it's contents rather than
     * requiring knowledge of MessageFactory's internal workings. Look to change
     * this in the future.
     *
     * @param message_factory An instance of MessageFactory that is being used to create a response.
     */
    void WriteToMessageFactory(MessageFactory* message_factory) const;

private:

    // Return's the size of the prose package as a wide-character string.
    uint32_t real_size_() const;

    std::string base_stf_file_;
    uint32_t base_stf_int_;
    std::string base_stf_string_;

    uint64_t tu_object_id_;
    std::string tu_stf_file_;
    uint32_t tu_stf_int_;
    std::string tu_stf_string_;
    std::wstring tu_custom_string_;

    uint64_t tt_object_id_;
    std::string tt_stf_file_;
    uint32_t tt_stf_int_;
    std::string tt_stf_string_;
    std::wstring tt_custom_string_;

    uint64_t to_object_id_;
    std::string to_stf_file_;
    uint32_t to_stf_int_;
    std::string to_stf_string_;
    std::wstring to_custom_string_;

    int32_t di_integer_;
    float df_float_;

    uint8_t display_flag_;
};

#endif  // SRC_ZONESERVER_OUTOFBANDPACKAGE_H_
