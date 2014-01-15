// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "tre_reader.h"
#include "anh/byte_buffer.h"
#include "anh/logger.h"

#include <array>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

#include <zlib.h>

#include "tre_data.h"

using namespace swganh::tre;

using std::find_if;
using std::for_each;
using std::ifstream;
using std::ios_base;
using std::runtime_error;
using std::string;
using std::stringstream;
using std::vector;

TreReader::TreReader(const string& filename, ResourceLookup& lookup_, uint32_t index)
    : filename_(filename)
{
    input_stream_.exceptions(ifstream::failbit | ifstream::badbit);

    try
    {
        input_stream_.open(filename_.c_str(), ios_base::binary);
        ReadHeader();
        ReadIndex(lookup_, index);
    }
    catch(std::ifstream::failure e)
    {
        LOG(fatal) << "Failure opening/reading file:" << filename;
        throw e;
    }
}

TreReader::~TreReader()
{
    {
        boost::lock_guard<boost::mutex> lg(mutex_);
        input_stream_.close();
    }
}

uint32_t TreReader::GetResourceCount() const
{
    return header_.resource_count;
}

const string& TreReader::GetFilename() const
{
    return filename_;
}

vector<string> TreReader::GetResourceNames() const
{
    vector<string> resource_names;

    for_each(
        begin(resource_lookup_),
        end(resource_lookup_),
        [this, &resource_names] (const std::pair<const char*, TreResourceInfo>& info)
    {
        resource_names.push_back(info.first);
    });

    return resource_names;
}

swganh::ByteBuffer TreReader::GetResource(const std::string& resource_name)
{
    swganh::ByteBuffer data;

    GetResource(resource_name, data);

    return data;
}

void TreReader::GetResource(const std::string& resource_name, swganh::ByteBuffer& buffer)
{
    auto file_info = GetResourceInfo(resource_name);

    if (file_info.data_size > buffer.size())
    {
        buffer.resize(file_info.data_size);
    }

    if (file_info.data_size != 0)
    {
        ReadDataBlock(
            file_info.data_offset,
            file_info.data_compression,
            file_info.data_compressed_size,
            file_info.data_size,
            (char*)(&buffer.raw()[0]));
    }

}

bool TreReader::ContainsResource(const string& resource_name) const
{
    auto find_iter = resource_lookup_.find(resource_name.c_str());
    return find_iter != end(resource_lookup_);
}

uint32_t TreReader::GetResourceSize(const string& resource_name) const
{
    auto find_iter = resource_lookup_.find(resource_name.c_str());

    if (find_iter == resource_lookup_.end())
    {
        throw std::runtime_error("File name invalid");
    }

    return find_iter->second.data_size;
}

const TreResourceInfo& TreReader::GetResourceInfo(const string& resource_name) const
{
    auto find_iter = resource_lookup_.find(resource_name.c_str());
    if (find_iter == end(resource_lookup_))
    {
        throw std::runtime_error("Requested info for invalid file: " + resource_name);
    }

    return find_iter->second;
}

void TreReader::ReadHeader()
{
    {
        boost::lock_guard<boost::mutex> lg(mutex_);
        input_stream_.read(reinterpret_cast<char*>(&header_), sizeof(header_));
    }

    ValidateFileType(string(header_.file_type, 4));
    ValidateFileVersion(string(header_.file_version, 4));
}

void TreReader::ReadIndex(ResourceLookup& lookup_, uint32_t index)
{
    std::vector<TreResourceInfo> resource_block_ = ReadResourceBlock();
    name_block_ = ReadNameBlock();

	std::vector<TreResourceInfo>::iterator it;

    for(it = resource_block_.begin(); it != resource_block_.end(); it++)
    {
        //Insert into the global index pointing to this reader
        char* name = &name_block_[it->name_offset];
        auto lb = lookup_.lower_bound(name);
        if(lb != lookup_.end() && !(lookup_.key_comp()(name, lb->first)))
        {
            lb->second = index;
        }
        else
        {
            lookup_.insert(lb, std::make_pair(name, index));
        }

        //Insert into the local index pointing to the info.
        resource_lookup_.insert(std::make_pair(name, (*it)));
    }
}

vector<TreResourceInfo> TreReader::ReadResourceBlock()
{
    uint32_t uncompressed_size = header_.resource_count * sizeof(TreResourceInfo);

    vector<TreResourceInfo> files(header_.resource_count);

    ReadDataBlock(header_.info_offset,
                  header_.info_compression,
                  header_.info_compressed_size,
                  uncompressed_size,
                  reinterpret_cast<char*>(&files[0]));

    return files;
}

vector<char> TreReader::ReadNameBlock()
{
    vector<char> data(header_.name_uncompressed_size);

    uint32_t name_offset = header_.info_offset + header_.info_compressed_size;

    ReadDataBlock(
        name_offset,
        header_.name_compression,
        header_.name_compressed_size,
        header_.name_uncompressed_size,
        &data[0]);

    return data;
}

void TreReader::ValidateFileType(string file_type) const
{
    if (file_type.compare("EERT") != 0)
    {
        throw runtime_error("Invalid tre file format");
    }
}

void TreReader::ValidateFileVersion(string file_version) const
{
    if (file_version.compare("5000") != 0)
    {
        throw runtime_error("Invalid tre file version");
    }
}

void TreReader::ReadDataBlock(
    uint32_t offset,
    uint32_t compression,
    uint32_t compressed_size,
    uint32_t uncompressed_size,
    char* buffer)
{
    if (compression == 0)
    {
        {
            boost::lock_guard<boost::mutex> lg(mutex_);
            input_stream_.seekg(offset, ios_base::beg);
            input_stream_.read(buffer, uncompressed_size);
        }
    }
    else if (compression == 2)
    {
        vector<char> compressed_data(compressed_size);

        {
            boost::lock_guard<boost::mutex> lg(mutex_);
            input_stream_.seekg(offset, ios_base::beg);
            input_stream_.read(&compressed_data[0], compressed_size);
        }

        int result;
        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = Z_NULL;
        stream.next_in = Z_NULL;
        result = inflateInit(&stream);

        if (result != Z_OK)
        {
            throw std::runtime_error("Zlib error: " );//+ std::to_string(result));
        }

        stream.next_in = reinterpret_cast<Bytef*>(&compressed_data[0]);
        stream.avail_in = compressed_size;
        stream.next_out = reinterpret_cast<Bytef*>(buffer);
        stream.avail_out = uncompressed_size;

        inflate(&stream, Z_FINISH);

        if(stream.total_out > 0)
        {
            inflateEnd(&stream);
        }
    }
    else
    {
        throw std::runtime_error("Unknown format");
    }
}
