// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>

namespace swganh {
namespace tre {

#pragma pack(1)
    struct TreHeader
    {
        char file_type[4];
        char file_version[4];
        uint32_t resource_count;
        uint32_t info_offset;
        uint32_t info_compression;
        uint32_t info_compressed_size;
        uint32_t name_compression;
        uint32_t name_compressed_size;
        uint32_t name_uncompressed_size;
    };
    
    struct TreResourceInfo
    {
        uint32_t checksum;
        uint32_t data_size;
        uint32_t data_offset;
        uint32_t data_compression;
        uint32_t data_compressed_size;
        uint32_t name_offset;
    };
#pragma pack()

}}  // namespace swganh::tre
