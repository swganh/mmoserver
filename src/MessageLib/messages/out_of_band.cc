// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include <MessageLib\messages\out_of_band.h>
#include "anh/byte_buffer.h"

using namespace std;
using namespace swganh;
using namespace swganh::messages;

ProsePackage::ProsePackage()
    : tu_object_id(0)
    , tt_object_id(0)
    , to_object_id(0)
    , di_integer(0)
    , df_float(0.0f)
{}

ProsePackage::ProsePackage(string stf_file, string stf_label)
    : base_stf_file(stf_file)
    , base_stf_label(stf_label)
    , tu_object_id(0)
    , tt_object_id(0)
    , to_object_id(0)
    , di_integer(0)
    , df_float(0.0f)
{
}

ProsePackage::~ProsePackage() {}

OutOfBand::OutOfBand()
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();
}

OutOfBand::OutOfBand(const ProsePackage& prose)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();

    AddProsePackage(prose);
}

OutOfBand::OutOfBand(const string& base_stf_file, const string& base_stf_label,
                     uint64_t tu_object_id, const string& tu_stf_file, const string& tu_stf_label, const wstring& tu_custom_string,
                     uint64_t tt_object_id, const string& tt_stf_file, const string& tt_stf_label, const wstring& tt_custom_string,
                     uint64_t to_object_id, const string& to_stf_file, const string& to_stf_label, const wstring& to_custom_string,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(new ByteBuffer())
{
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    tu_object_id, tu_stf_file, tu_stf_label, tu_custom_string,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
                     ProseType prose_type, uint64_t object_id, bool display_flag)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();
    switch (prose_type)
    {
    case TU:
        AddProsePackage(base_stf_file, base_stf_string, object_id, 0, 0, 0, 0.0f, display_flag);
        break;
    case TT:
        AddProsePackage(base_stf_file, base_stf_string, 0, object_id, 0, 0, 0.0f, display_flag);
        break;
    case TO:
        AddProsePackage(base_stf_file, base_stf_string, 0, 0, object_id, 0, 0.0f, display_flag);
        break;
    case DI:
        AddProsePackage(base_stf_file, base_stf_string, 0, 0, 0, (int32_t)object_id, 0.0f, display_flag);
        break;
    case DF:
        AddProsePackage(base_stf_file, base_stf_string, 0, 0, 0, 0, (float)object_id, display_flag);
    default:
        AddProsePackage(base_stf_file, base_stf_string, 0, object_id, 0, 0, 0.0f, display_flag);
        break;
    }

}

OutOfBand::OutOfBand(const std::string& base_stf_file, const std::string& base_stf_string,
                     ProseType prose_type, const std::wstring& message, bool display_flag)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();
    switch (prose_type)
    {
    case TU:
        AddProsePackage(base_stf_file, base_stf_string, message, L"", L"", 0, 0.0f, display_flag);
        break;
    case TT:
        AddProsePackage(base_stf_file, base_stf_string, L"", message, L"", 0, 0.0f, display_flag);
        break;
    case TO:
        AddProsePackage(base_stf_file, base_stf_string, L"", L"", message, 0, 0.0f, display_flag);
        break;
    default:
        AddProsePackage(base_stf_file, base_stf_string, 0, 0, 0, 0, 0.0f, display_flag);
        break;
    }

}

OutOfBand::OutOfBand(const string& base_stf_file, const string& base_stf_label,
                     uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_object_id, tt_object_id, to_object_id,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const string& base_stf_file, const string& base_stf_label,
                     const string& tu_stf_file, const string& tu_stf_label,
                     const string& tt_stf_file, const string& tt_stf_label,
                     const string& to_stf_file, const string& to_stf_label,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_stf_file, tu_stf_label,
                    tt_stf_file, tt_stf_label,
                    to_stf_file, to_stf_label,
                    di_integer, df_float, display_flag);
}

OutOfBand::OutOfBand(const string& base_stf_file, const string& base_stf_label,
                     const wstring& tu_custom_string,
                     const wstring& tt_custom_string,
                     const wstring& to_custom_string,
                     int32_t di_integer, float df_float, bool display_flag)
    : count_(0)
    , data_(make_shared<ByteBuffer>())
{
    Initialize_();

    AddProsePackage(base_stf_file, base_stf_label,
                    tu_custom_string, tt_custom_string, to_custom_string,
                    di_integer, df_float, display_flag);
}

OutOfBand::~OutOfBand()
{
}

uint16_t OutOfBand::Count() const
{
    return count_;
}

uint32_t OutOfBand::Length() const
{
    // Take the size of the data package minus the 4 bytes for the length value.
    return (data_->size() - 4) / 2;
}

void OutOfBand::AddProsePackage(const ProsePackage& prose)
{
    ByteBuffer tmp(*data_);

    tmp.write<uint8_t>(0x01);
    tmp.write<int32_t>(0xFFFFFFFF);
    tmp.write<string>(prose.base_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<string>(prose.base_stf_label);

    tmp.write<uint64_t>(prose.tu_object_id);
    tmp.write<string>(prose.tu_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<string>(prose.tu_stf_label);
    tmp.write<wstring>(prose.tu_custom_string);

    tmp.write<uint64_t>(prose.tt_object_id);
    tmp.write<string>(prose.tt_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<string>(prose.tt_stf_label);
    tmp.write<wstring>(prose.tt_custom_string);

    tmp.write<uint64_t>(prose.to_object_id);
    tmp.write<string>(prose.to_stf_file);
    tmp.write<uint32_t>(0);
    tmp.write<string>(prose.to_stf_label);
    tmp.write<wstring>(prose.to_custom_string);

    tmp.write<int32_t>(prose.di_integer);
    tmp.write<float>(prose.df_float);
    tmp.write<uint8_t>((prose.display_flag) ? 1 : 0);

    // Find the length of the ProsePackage.
    uint32_t std_string_lengths = prose.base_stf_file.length() + prose.base_stf_label.length() +
                                  prose.tu_stf_file.length() + prose.tu_stf_label.length() +
                                  prose.tt_stf_file.length() + prose.tt_stf_label.length() +
                                  prose.to_stf_file.length() + prose.to_stf_label.length();

    // This whole ProsePackage is treated as a wide-character string, so if
    // the string length totals are odd then we need to add some padding.
    if (std_string_lengths % 2)
    {
        tmp.write<uint8_t>(0);
    }

    // Now that all the data is written update the length and count.
    SetLength_(tmp);

    uint16_t tmp_count = count_ + 1;
    SetCount_(tmp, tmp_count);

    // Only modify our internals when all work is done, and only use
    // exception safe methods of updating.
    data_->swap(tmp);
    count_ = tmp_count;
}

void OutOfBand::AddProsePackage(const string& base_stf_file, const string& base_stf_label,
                                uint64_t tu_object_id, const string& tu_stf_file, const string& tu_stf_label, const wstring& tu_custom_string,
                                uint64_t tt_object_id, const string& tt_stf_file, const string& tt_stf_label, const wstring& tt_custom_string,
                                uint64_t to_object_id, const string& to_stf_file, const string& to_stf_label, const wstring& to_custom_string,
                                int32_t di_integer, float df_float, bool display_flag)
{
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_object_id = tu_object_id;
    prose.tu_stf_file = tu_stf_file;
    prose.tu_stf_label = tu_stf_label;
    prose.tu_custom_string = tu_custom_string;

    prose.tt_object_id = tt_object_id;
    prose.tt_stf_file = tt_stf_file;
    prose.tt_stf_label = tt_stf_label;
    prose.tt_custom_string = tt_custom_string;

    prose.to_object_id = to_object_id;
    prose.to_stf_file = to_stf_file;
    prose.to_stf_label = to_stf_label;
    prose.to_custom_string = to_custom_string;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const string& base_stf_file, const string& base_stf_label,
                                uint64_t tu_object_id, uint64_t tt_object_id, uint64_t to_object_id,
                                int32_t di_integer, float df_float, bool display_flag)
{
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_object_id = tu_object_id;
    prose.tt_object_id = tt_object_id;
    prose.to_object_id = to_object_id;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const string& base_stf_file, const string& base_stf_label,
                                const string& tu_stf_file, const string& tu_stf_label,
                                const string& tt_stf_file, const string& tt_stf_label,
                                const string& to_stf_file, const string& to_stf_label,
                                int32_t di_integer, float df_float, bool display_flag)
{
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_stf_file = tu_stf_file;
    prose.tu_stf_label = tu_stf_label;

    prose.tt_stf_file = tt_stf_file;
    prose.tt_stf_label = tt_stf_label;

    prose.to_stf_file = to_stf_file;
    prose.to_stf_label = to_stf_label;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

void OutOfBand::AddProsePackage(const string& base_stf_file, const string& base_stf_label,
                                const wstring& tu_custom_string, const wstring& tt_custom_string, const wstring& to_custom_string,
                                int32_t di_integer, float df_float, bool display_flag)
{
    ProsePackage prose;

    prose.base_stf_file = base_stf_file;
    prose.base_stf_label = base_stf_label;

    prose.tu_custom_string = tu_custom_string;
    prose.tt_custom_string = tt_custom_string;
    prose.to_custom_string = to_custom_string;

    prose.di_integer = di_integer;
    prose.df_float = df_float;
    prose.display_flag = display_flag;

    AddProsePackage(prose);
}

const ByteBuffer* OutOfBand::Pack() const
{
    return data_.get();
}


void OutOfBand::Initialize_()
{
    data_->write<uint32_t>(2);
    data_->write<uint16_t>(0);
}

void OutOfBand::SetCount_(ByteBuffer& buffer, uint16_t count)
{
    buffer.writeAt<uint16_t>(4, count);
}

void OutOfBand::SetLength_(ByteBuffer& buffer)
{
    buffer.writeAt<uint32_t>(0, (buffer.size()-4) / 2);
}