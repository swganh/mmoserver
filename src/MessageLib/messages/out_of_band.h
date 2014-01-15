// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <memory>
#include <string>


namespace swganh
{
class ByteBuffer;
}
/*! \brief Common is a catch-all library containing primarily base classes and
 * classes used for maintaining application lifetimes.
 */
namespace swganh
{
namespace messages
{

/**
 * @brief The ProsePackage is part of the OutOfBand attachment and is used to send custom STF strings.
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
enum ProseType
{
    TU = 0,
    TT,
    TO,
    DI,
    DF
};
struct ProsePackage
{
    /**
     * Default constructor.
     *
     * This default constructor creates an empty ProsePackage.
     */
    ProsePackage();

    /**
     * Custom constructor taking the message STF.
     *
     * This overloaded constructor takes the message STF as its
     * arguments. Custom data can then be set as needed.
     *
     * @param stf_file The file containing the STF message.
     * @param stf_label The label of the STF message.
     */
    ProsePackage(std::string stf_file, std::string stf_label);


    /// Helper functions for AddProsePackage

    ProsePackage(std::string stf_file, std::string stf_label, ProseType prose_type, int32_t di_integer);

    ProsePackage(std::string stf_file, std::string stf_label, ProseType prose_type, uint64_t t_object_id, const std::string& t_stf_file = "", const std::string& t_stf_label ="", const std::wstring& t_custom_string = L"");

    ProsePackage(std::string stf_file, std::string stf_label, ProseType prose_type, float df_val);

    /// Default deconstructor.
    ~ProsePackage();

    std::string  base_stf_file;  ///< File containing the STF message.
    std::string  base_stf_label; ///< Label of the STF message.

    uint64_t     tu_object_id;     ///< Text You value: object id
    std::string  tu_stf_file;      ///< Text You value: STF file
    std::string  tu_stf_label;     ///< Text You value: STF label
    std::wstring tu_custom_string; ///< Text You value: custom string

    uint64_t     tt_object_id;     ///< Text Target value: object id
    std::string  tt_stf_file;      ///< Text Target value: STF file
    std::string  tt_stf_label;     ///< Text Target value: STF label
    std::wstring tt_custom_string; ///< Text Target value: custom string

    uint64_t     to_object_id;     ///< Text Object value: object id
    std::string  to_stf_file;      ///< Text Object value: STF file
    std::string  to_stf_label;     ///< Text Object value: STF label
    std::wstring to_custom_string; ///< Text Object value: custom string

    int32_t di_integer; ///< Decimal value: integer
    float df_float;     ///< Decimal value: float

    /// @TODO: this value needs further testing.
    bool display_flag; ///< True display's the message and false leaves it blank
};

/// @TODO fully document the WaypointPackage
struct WaypointPackage
{
    float position_x;
    float position_y;
    float position_z;
    uint32_t planet_crc;
    std::wstring waypoint_name;
    uint64_t waypoint_id;
    uint8_t color;
    bool activated;
};

/**
 * @brief The OutOfBand attachment is used in multiple places to send out
 * customized data to the client.
 *
 * The OutOfBand attachment is actually made up of several sub-types,
 * for further information on each sub-type supported see it's related struct.
 *
 * @see ProsePackage
 * @see WaypointPackage
 */
class OutOfBand
{
public:
    /// Default constructor, generates an empty OutOfBand package.
    OutOfBand();

    /**
     * Constructor overload for OutOfBand taking a ProsePackage.
     *
     * @see ProsePackage for further information on parameters.
     */
    explicit OutOfBand(const ProsePackage& prose);

    /**
     * Constructor overload for common Prose Packages
     *
     *
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              ProseType prose_type, uint64_t object_id, bool display_flag = 0);

    /**
    * Constructor overload for common Prose Packages
    *
    *
    */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              ProseType prose_type, const std::wstring& message, bool display_flag = 0);
    /**
     * Constructor overload for OutOfBand taking a ProsePackage.
     *
     * @see ProsePackage for further information on parameters.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              uint64_t tu_object_id = 0, const std::string& tu_stf_file = "", const std::string& tu_stf_string ="", const std::wstring& tu_custom_string = L"",
              uint64_t tt_object_id = 0, const std::string& tt_stf_file = "", const std::string& tt_stf_string ="", const std::wstring& tt_custom_string = L"",
              uint64_t to_object_id = 0, const std::string& to_stf_file = "", const std::string& to_stf_string ="", const std::wstring& to_custom_string = L"",
              int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses object ids for value replacement.
     *
     * @see ProsePackage for further information on parameters.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
              int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);


    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses stfs for value replacement.
     *
     * @see ProsePackage for further information on parameters.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              const std::string& tu_stf_file, const std::string& tu_stf_string,
              const std::string& tt_stf_file, const std::string& tt_stf_string,
              const std::string& to_stf_file, const std::string& to_stf_string,
              int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /**
     * Constructor overload for OutOfBand taking a ProsePackage that uses custom strings for value replacement.
     *
     * @see ProsePackage for further information on parameters.
     */
    OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
              const std::wstring& tu_custom_string, const std::wstring& tt_custom_string, const std::wstring& to_custom_string,
              int32_t di_integer = 0, float df_float = 0.0f, bool display_flag = 0);

    /// Default deconstructor.
    ~OutOfBand();

    /**
     * Returns a count of the packages contained in this OutOfBand attachment.
     *
     * @return Count of packages in the OutOfBand attachment.
     */
    uint16_t Count() const;

    /**
     * Returns the length of the OutOfBand attachment.
     *
     * The OutOfBand attachment is treated by a wide character string by the packets
     * under the hood. This returns the length of the OutOfBand attachment as if it
     * where a wide character string.
     *
     * @return Length of the OutOfBand attachment.
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
     * @return A const pointer to a ByteBuffer instance containing the OutOfBand data.
     */
    const swganh::ByteBuffer* Pack() const;

protected:
    void Initialize_();
    void SetCount_(swganh::ByteBuffer& buffer, uint16_t count);
    void SetLength_(swganh::ByteBuffer& buffer);

    uint16_t count_;

    std::shared_ptr<swganh::ByteBuffer> data_;
};
}
} // swganh::messages