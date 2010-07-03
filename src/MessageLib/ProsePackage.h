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
    /**
     * Default constructor
     */
    ProsePackage();

    /**
     * Constructor overload for ProsePackage that provides maximum flexibility 
     * in specifying string replacements.
     */
    ProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id = 0, const std::string& tu_stf_file = "", const std::string& tu_stf_string ="", const std::wstring& tu_custom_string = L"",
        uint64_t tt_object_id = 0, const std::string& tt_stf_file = "", const std::string& tt_stf_string ="", const std::wstring& tt_custom_string = L"",
        uint64_t to_object_id = 0, const std::string& to_stf_file = "", const std::string& to_stf_string ="", const std::wstring& to_custom_string = L"",
        int32_t di_integer = 0, float df_float = 0.0f, uint8_t display_flag = 0);
    
    /**
     * Constructor overload for ProsePackage that uses object ids for value replacement.
     */
    ProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
        int32_t di_integer = 0, float df_float = 0.0f, uint8_t display_flag = 0);

    
    /**
     * Constructor overload for ProsePackage that uses stfs for value replacement.
     */
    ProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::string& tu_stf_file, const std::string& tu_stf_string,
        const std::string& tt_stf_file, const std::string& tt_stf_string,
        const std::string& to_stf_file, const std::string& to_stf_string,
        int32_t di_integer = 0, float df_float = 0.0f, uint8_t display_flag = 0);
    
    /**
     * Constructor overload for ProsePackage that uses custom strings for value replacement.
     */
    ProsePackage(const std::string& base_stf_file, const std::string& base_stf_string, 
        const std::wstring& tu_custom_string, const std::wstring& tt_custom_string, const std::wstring& to_custom_string,
        int32_t di_integer = 0, float df_float = 0.0f, uint8_t display_flag = 0);
     
    /**
     * Default deconstructor
     */
    ~ProsePackage();

    /* Uses the default generated copy constructor and assignment operator. */

    /**
     * Returns the current base stf filename.
     */
    const std::string& base_stf_file() const;

    /**
     * Sets the base stf filename.
     *
     * @param file The new filename.
     */
    void base_stf_file(const std::string& file);
    
    /**
     * Returns the current base stf string
     */
    const std::string& base_stf_string() const;

    /**
     * Sets the base stf string.
     *
     * @param string The new string.
     */
    void base_stf_string(const std::string& string);

    /**
     * Returns the Text You object id.
     */
    uint64_t tu_object_id() const;

    /**
     * Sets the Text You object id.
     *
     * Setting this value overrides anything set via the Text YOU stf or custom string.
     *
     * @param object_id The new Text You object id.
     */
    void tu_object_id(uint64_t object_id);

    /**
     * Returns the current Text You stf filename.
     */
    const std::string& tu_stf_file() const;

    /**
     * Sets the Text You stf filename.
     *
     * If the Text You object id is not set then setting the Text You stf will override
     * anything set by the Text You custom string.
     *
     * @param file The new filename.
     */
    void tu_stf_file(const std::string& file);
    
    /**
     * Returns the current Text You stf string.
     */
    const std::string& tu_stf_string() const;

    /**
     * Sets the Text You stf string.
     *
     * If the Text You object id is not set then setting the Text You stf will override
     * anything set by the Text You custom string.
     *
     * @param string The new string.
     */
    void tu_stf_string(const std::string& string);
    
    /**
     * Returns the current Text You custom string.
     */
    const std::wstring& tu_custom_string() const;

    /**
     * Sets the Text You custom string.
     *
     * This value is only accepted if a Text You object id and stf are not set.
     *
     * @param string The new string.
     */
    void tu_custom_string(const std::wstring& string);

    /**
     * Returns the Text Target object id.
     */
    uint64_t tt_object_id() const;

    /**
     * Sets the Text Target object id.
     *
     * Setting this value overrides anything set via the Text Target stf or custom string.
     *
     * @param object_id The new Text Target object id.
     */
    void tt_object_id(uint64_t object_id);

    /**
     * Returns the current Text Target stf filename.
     */
    const std::string& tt_stf_file() const;

    /**
     * Sets the Text Target stf filename.
     *
     * If the Text Target object id is not set then setting the Text Target stf will override
     * anything set by the Text Target custom string.
     *
     * @param file The new filename.
     */
    void tt_stf_file(const std::string& file);
    
    /**
     * Returns the current Text Target stf string.
     */
    const std::string& tt_stf_string() const;

    /**
     * Sets the Text Target stf string.
     *
     * If the Text Target object id is not set then setting the Text Target stf will override
     * anything set by the Text Target custom string.
     *
     * @param string The new string.
     */
    void tt_stf_string(const std::string& string);
    
    /**
     * Returns the current Text Target custom string.
     */
    const std::wstring& tt_custom_string() const;

    /**
     * Sets the Text Target custom string.
     *
     * This value is only accepted if a Text Target object id and stf are not set.
     *
     * @param string The new string.
     */
    void tt_custom_string(const std::wstring& string);    

    /**
     * Returns the Text Object object id.
     */
    uint64_t to_object_id() const;

    /**
     * Sets the Text Object object id.
     *
     * Setting this value overrides anything set via the Text Object stf or custom string.
     *
     * @param object_id The new Text Object object id.
     */
    void to_object_id(uint64_t object_id);

    /**
     * Returns the current Text Object stf filename.
     */
    const std::string& to_stf_file() const;

    /**
     * Sets the Text Object stf filename.
     *
     * If the Text Object object id is not set then setting the Text Object stf will override
     * anything set by the Text Object custom string.
     *
     * @param file The new filename.
     */
    void to_stf_file(const std::string& file);

    /**
     * Returns the current Text Object stf string.
     */
    const std::string& to_stf_string() const;

    /**
     * Sets the Text Object stf string.
     *
     * If the Text Object object id is not set then setting the Text Object stf will override
     * anything set by the Text Object custom string.
     *
     * @param string The new string.
     */
    void to_stf_string(const std::string& string);
    
    /**
     * Returns the current Text Object custom string.
     */
    const std::wstring& to_custom_string() const;

    /**
     * Sets the Text Object custom string.
     *
     * This value is only accepted if a Text Object object id and stf are not set.
     *
     * @param string The new string.
     */
    void to_custom_string(const std::wstring& string);
    
    /**
     * Returns the current Decimal Integer value.
     */
    uint32_t di_integer() const;

    /**
     * Sets the Decimal Integer value.
     *
     * @param value The new Decimal Integer value.
     */
    void di_integer(uint32_t value);
    
    /**
     * Returns the current Decimal Float value.
     */
    float df_float() const;

    /**
     * Sets the Decimal Float value.
     *
     * @param value The new Decimal Float value.
     */
    void df_float(float value);
    
    /**
     * Returns the current display flag value.
     */
    uint8_t display_flag() const;

    /**
     * Sets the current display flag value.
     *
     * @param flag The new display flag value.
     */
    void display_flag(uint8_t flag);

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
