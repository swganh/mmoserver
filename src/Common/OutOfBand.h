///*
//---------------------------------------------------------------------------------------
//This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)
//
//For more information, visit http://www.swganh.com
//
//Copyright (c) 2006 - 2010 The SWG:ANH Team
//---------------------------------------------------------------------------------------
//Use of this source code is governed by the GPL v3 license that can be found
//in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html
//
//This library is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 2.1 of the License, or (at your option) any later version.
//
//This library is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//---------------------------------------------------------------------------------------
//*/

#ifndef SRC_COMMON_OUTOFBAND_H_
#define SRC_COMMON_OUTOFBAND_H_

#include <cstdint>
#include <memory>
#include <string>

namespace common {

class ByteBuffer;

struct ProsePackage {
    std::string base_stf_file;
    std::string base_stf_label;

    uint64_t     tu_object_id;
    std::string  tu_stf_file;
    std::string  tu_stf_label;
    std::wstring tu_custom_string;
    
    uint64_t     tt_object_id;
    std::string  tt_stf_file;
    std::string  tt_stf_label;
    std::wstring tt_custom_string;

    uint64_t     to_object_id;
    std::string  to_stf_file;
    std::string  to_stf_label;
    std::wstring to_custom_string;

    int32_t di_integer;
    float df_float;

    bool display_flag;
};

struct WaypointPackage {
    float position_x;
    float position_y;
    float position_z;
    uint32_t planet_crc;
    std::wstring waypoint_name;
    uint64_t waypoint_id;
    uint8_t color;
    bool activated;
};

class OutOfBand {
public:
    OutOfBand();    

    /**
     * Constructor overload for OutOfBand taking a ProsePackage.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id = 0, const std::string& tu_stf_file = "", const std::string& tu_stf_string ="", const std::wstring& tu_custom_string = L"",
        uint64_t tt_object_id = 0, const std::string& tt_stf_file = "", const std::string& tt_stf_string ="", const std::wstring& tt_custom_string = L"",
        uint64_t to_object_id = 0, const std::string& to_stf_file = "", const std::string& to_stf_string ="", const std::wstring& to_custom_string = L"",
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);
    
    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses object ids for value replacement.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    
    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses stfs for value replacement.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::string& tu_stf_file, const std::string& tu_stf_string,
        const std::string& tt_stf_file, const std::string& tt_stf_string,
        const std::string& to_stf_file, const std::string& to_stf_string,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);
    
    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses custom strings for value replacement.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::wstring& tu_custom_string, const std::wstring& tt_custom_string, const std::wstring& to_custom_string,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    ~OutOfBand();

    /**
     * Returns a count of the packages contained in this OutOfBand attachment.
     *
     * @returns Count of packages in the OutOfBand attachment.
     */
    uint16_t Count() const;

    /**
     * Returns the length of the OutOfBand attachment.
     *
     * The OutOfBand attachment is treated by a wide character string by the packets
     * under the hood. This returns the length of the OutOfBand attachment as if it
     * where a wide character string.
     *
     * @returns Length of the OutOfBand attachment.
     */
    uint32_t Length() const;

    /**
     * Adds a prose package to the OutOfBand attachment.
     *
     * @param prose A reference to a prebuilt prose package..
     */
    void AddProsePackage(const ProsePackage& prose);

    /**
     * Adds a prose package to the OutOfBand attachment.
     *
     * @param stf_file The stf file containing the referenced text.
     * @param stf_label The label used to identify the referenced text in the stf file.
     */
    void AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id = 0, const std::string& tu_stf_file = "", const std::string& tu_stf_label ="", const std::wstring& tu_custom_string = L"",
        uint64_t tt_object_id = 0, const std::string& tt_stf_file = "", const std::string& tt_stf_label ="", const std::wstring& tt_custom_string = L"",
        uint64_t to_object_id = 0, const std::string& to_stf_file = "", const std::string& to_stf_label ="", const std::wstring& to_custom_string = L"",
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Adds a prose package to the OutOfBand attachment.
     *
     * @param stf_file The stf file containing the referenced text.
     * @param stf_label The label used to identify the referenced text in the stf file.
     */
    void AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Adds a prose package to the OutOfBand attachment.
     *
     * @param stf_file The stf file containing the referenced text.
     * @param stf_label The label used to identify the referenced text in the stf file.
     */
    void AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::string& tu_stf_file, const std::string& tu_stf_label,
        const std::string& tt_stf_file, const std::string& tt_stf_label,
        const std::string& to_stf_file, const std::string& to_stf_label,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Adds a prose package to the OutOfBand attachment.
     *
     * @param stf_file The stf file containing the referenced text.
     * @param stf_label The label used to identify the referenced text in the stf file.
     */
    void AddProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::wstring& tu_custom_string, const std::wstring& tt_custom_string, const std::wstring& to_custom_string,
        int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Packs the OutOfBand for packet use.
     *
     * @returns A const pointer to a ByteBuffer instance containing the OutOfBand data.
     */
    const ByteBuffer* Pack() const;

private:
    void Initialize_();
    void SetCount_(ByteBuffer& buffer, uint16_t count);
    void SetLength_(ByteBuffer& buffer);

    uint16_t count_;

    std::unique_ptr<ByteBuffer> data_;
};

}  // namespace common

#endif  // SRC_COMMON_OUTOFBAND_H_
