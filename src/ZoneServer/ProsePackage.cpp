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

#include <ZoneServer/ProsePackage.h>
#include <Common/MessageFactory.h>

ProsePackage::ProsePackage()
: base_stf_file_("")
, base_stf_int_(0)
, base_stf_string_("")
, tu_object_id_(0)
, tu_stf_file_("")
, tu_stf_int_(0)
, tu_stf_string_("")
, tu_custom_string_(L"")

, tt_object_id_(0)
, tt_stf_file_("")
, tt_stf_int_(0)
, tt_stf_string_("")
, tt_custom_string_(L"")

, to_object_id_(0)
, to_stf_file_("")
, to_stf_int_(0)
, to_stf_string_("")
, to_custom_string_(L"")

, di_integer_(0)
, df_float_(0.0f)
, display_flag_(0) {}


ProsePackage::ProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
    uint64_t tu_object_id, const std::string& tu_stf_file, const std::string& tu_stf_string, const std::wstring& tu_custom_string,
    uint64_t tt_object_id, const std::string& tt_stf_file, const std::string& tt_stf_string, const std::wstring& tt_custom_string,
    uint64_t to_object_id, const std::string& to_stf_file, const std::string& to_stf_string, const std::wstring& to_custom_string,
    int32_t di_integer, float df_float, uint8_t display_flag)
: base_stf_file_(base_stf_file)
, base_stf_int_(0)
, base_stf_string_(base_stf_string)
, tu_object_id_(tu_object_id)
, tu_stf_file_(tu_stf_file)
, tu_stf_int_(0)
, tu_stf_string_(tu_stf_string)
, tu_custom_string_(tu_custom_string)

, tt_object_id_(tt_object_id)
, tt_stf_file_(tt_stf_file)
, tt_stf_int_(0)
, tt_stf_string_(tt_stf_string)
, tt_custom_string_(tt_custom_string)

, to_object_id_(to_object_id)
, to_stf_file_(to_stf_file)
, to_stf_int_(0)
, to_stf_string_(to_stf_string)
, to_custom_string_(to_custom_string)

, di_integer_(di_integer)
, df_float_(df_float)
, display_flag_(display_flag) {}


ProsePackage::~ProsePackage() {}


void ProsePackage::WriteToMessageFactory(MessageFactory* message_factory) const {
    // Find the length of the ProsePackage.
    uint32_t std_string_lengths = base_stf_file_.length() + base_stf_string_.length() +
        tu_stf_file_.length() + tu_stf_string_.length() +
        tt_stf_file_.length() + tt_stf_string_.length() +
        to_stf_file_.length() + to_stf_string_.length();
    
    // The constant 42 here is the sum of the size's of all the int/float values in
    // the ProsePackage, including the string lengths that prefix each string, divided by 2.
    uint32_t real_size = 42 + static_cast<uint32_t>(ceil(static_cast<float>(std_string_lengths + tu_custom_string_.length() + tt_custom_string_.length() + to_custom_string_.length()) / 2.0f));
    
    message_factory->addUint32(real_size);

    // Technically you can send more than one element in an OutOfBand packge but
    // we never do, so for the time being this is hard-coded.
    //
    // @todo: Support multiple packages in OutOfBand messages.
    message_factory->addInt16(1);
    
    // Signifies this is a ProsePackage
    message_factory->addUint8(1);

    // This is believed to be a type id as in the WaypointPackage this must
    // be -3.
    message_factory->addUint32(0xFFFFFFFF);

    // Add the ProsePackage data to the message.
    message_factory->addString(base_stf_file_);
    message_factory->addUint32(base_stf_int_);
    message_factory->addString(base_stf_string_);

    message_factory->addUint64(tu_object_id_);
    message_factory->addString(tu_stf_file_);
    message_factory->addUint32(tu_stf_int_);
    message_factory->addString(tu_stf_string_);
    message_factory->addString(tu_custom_string_);
    
    message_factory->addUint64(tt_object_id_);
    message_factory->addString(tt_stf_file_);
    message_factory->addUint32(tt_stf_int_);
    message_factory->addString(tt_stf_string_);
    message_factory->addString(tt_custom_string_);
    
    message_factory->addUint64(to_object_id_);
    message_factory->addString(to_stf_file_);
    message_factory->addUint32(to_stf_int_);
    message_factory->addString(to_stf_string_);
    message_factory->addString(to_custom_string_);

    message_factory->addInt32(di_integer_);
    message_factory->addFloat(df_float_);
    message_factory->addUint8(display_flag_);
    
    // This whole ProsePackage is treated as a wide-character string, so if 
    // the std::string length totals are odd then we need to add some padding.
    if (std_string_lengths % 2) {
        message_factory->addUint8(0);
    }
}
