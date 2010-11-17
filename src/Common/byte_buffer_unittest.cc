/*
 * SWG:ANH Combat Upgrade Sandbox
 *
 * @copyright   Copyright (c) 2010 The SWG:ANH Team
 * @author      Eric Barr <apathy@swganh.org>
**/

#include <gtest/gtest.h>

#include "Common/byte_buffer.h"

using common::ByteBuffer;

namespace {

TEST(ByteBufferTests, ByteBufferIsEmptyWhenCreated)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.size());
}

TEST(ByteBufferTests, ByteBufferDefaultCapacityIsZero)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.capacity());
}

TEST(ByteBufferTests, WritingIntReportsCorrectSizeAndCapacity)
{
    ByteBuffer buffer;
    buffer.write<int>(10);

    EXPECT_EQ(uint32_t(4), buffer.size());
    EXPECT_EQ(uint32_t(4), buffer.capacity());
}

TEST(ByteBufferTests, WritingTwoIntsReportsCorrectSizeAndCapacity)
{
    ByteBuffer buffer;

    buffer.write<int>(10);
    EXPECT_EQ(uint32_t(4), buffer.size());
    EXPECT_EQ(uint32_t(4), buffer.capacity());

    buffer.write<int>(20);
    EXPECT_EQ(uint32_t(8), buffer.size());
    EXPECT_EQ(uint32_t(8), buffer.capacity());
}

TEST(ByteBufferTests, CanReadIntWrittenToTheBuffer)
{
    ByteBuffer buffer;

    buffer.write<int>(10);
    EXPECT_EQ(10, buffer.read<int>());
}

TEST(ByteBufferTests, CanReadTwoIntsWrittenToTheBuffer)
{
    ByteBuffer buffer;

    buffer.write<int>(10);
    buffer.write<int>(20);

    EXPECT_EQ(10, buffer.read<int>());
    EXPECT_EQ(20, buffer.read<int>());
}

TEST(ByteBufferTests, ReadingPastBufferEndThrowsException)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.size());

    EXPECT_THROW(buffer.read<int>(), std::out_of_range);
}

TEST(ByteBufferTests, WritingStringReportsCorrectSizeAndCapacity)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.size());
    EXPECT_EQ(uint32_t(0), buffer.capacity());

    buffer.write<std::string>(std::string("test string"));

    EXPECT_EQ(uint32_t(13), buffer.size());
    EXPECT_EQ(uint32_t(13), buffer.capacity());
}

TEST(ByteBufferTests, CanReadStringWrittenToTheBuffer)
{
    ByteBuffer buffer;
    std::string test_string("test string data");

    buffer.write<std::string>(test_string);

    EXPECT_EQ(test_string, buffer.read<std::string>());
}

TEST(ByteBufferTests, CanReadTwoStringsWrittenToTheBuffer)
{
    ByteBuffer buffer;
    std::string test_string1("first test string");
    std::string test_string2("second test string");

    buffer.write<std::string>(test_string1);
    buffer.write<std::string>(test_string2);

    EXPECT_EQ(test_string1, buffer.read<std::string>());
    EXPECT_EQ(test_string2, buffer.read<std::string>());
}

TEST(ByteBufferTests, WritingUnicodeStringReportsCorrectSizeAndCapacity)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.size());
    EXPECT_EQ(uint32_t(0), buffer.capacity());

    buffer.write<std::wstring>(std::wstring(L"test string"));

    // Length and Capacity should be size of int + size of string * size of wchar_t.
    EXPECT_EQ(sizeof(uint32_t) + (11 * 2), buffer.size());
    EXPECT_EQ(sizeof(uint32_t) + (11 * 2), buffer.capacity());
}

TEST(ByteBufferTests, CanReadUnicodeStringWrittenToTheBuffer)
{
    ByteBuffer buffer;
    std::wstring test_string(L"test string data");

    buffer.write<std::wstring>(test_string);
    EXPECT_STREQ(test_string.c_str(), buffer.read<std::wstring>().c_str());
}

TEST(ByteBufferTests, UnicodeStringStoredAs16Bit)
{
    ByteBuffer buffer;
    std::wstring test_string(L"testdata");

    EXPECT_EQ(uint32_t(8), test_string.length());
    
    buffer.write<std::wstring>(test_string);

    EXPECT_EQ(uint32_t(8), buffer.read<uint32_t>());

    EXPECT_EQ('t', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('e', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('s', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('t', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('d', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('a', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('t', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
    EXPECT_EQ('a', buffer.read<uint8_t>());
    EXPECT_EQ(0, buffer.read<uint8_t>());
}

TEST(ByteBufferTests, CanClearBufferData)
{
    ByteBuffer buffer;
    EXPECT_EQ(uint32_t(0), buffer.size());

    buffer.write<int>(3);
    EXPECT_EQ(uint32_t(4), buffer.size());

    buffer.clear();
    EXPECT_EQ(uint32_t(0), buffer.size());
    EXPECT_EQ(uint32_t(0), buffer.read_position());
    EXPECT_EQ(uint32_t(0), buffer.write_position());
}

TEST(ByteBufferTests, CanStreamData)
{
    ByteBuffer buffer;
    int testInt = 3;

    buffer << testInt;
    EXPECT_EQ(3, buffer.read<int>());
}

TEST(ByteBufferTests, PeekingDataDoesNotMoveReadPosition)
{
    ByteBuffer buffer;
    buffer.write<int>(3);
    buffer.write<int>(10);

    // Peek the data twice, this should return the same integer value (3) both times.
    EXPECT_EQ(3, buffer.peek<int>());
    EXPECT_EQ(3, buffer.peek<int>());
}

TEST(ByteBufferTests, CanReadAndWriteUnsignedData)
{
    ByteBuffer buffer;
    buffer.write<uint16_t>(3);

    EXPECT_EQ(uint32_t(2), buffer.size()); // First check that the right datasize was written.
    EXPECT_EQ(3, buffer.read<uint16_t>()); // Check that the value is correct.
}

TEST(ByteBufferTests, CanPeekAtOffset)
{
    ByteBuffer buffer;
    buffer.write<int>(3);
    buffer.write<int>(32);
    buffer.write<int>(979);
    buffer.write<int>(5467);

    EXPECT_EQ(979, buffer.peekAt<int>(8));
    EXPECT_EQ(32, buffer.peekAt<int>(4));
}

TEST(ByteBufferTests, CanWriteAtOffset)
{
    ByteBuffer buffer;
    buffer.write<int>(3);
    buffer.write<int>(32);
    buffer.write<int>(979);
    buffer.write<int>(5467);

    buffer.writeAt<int>(8, 52);
    buffer.writeAt<int>(4, 3532);

    EXPECT_EQ(52, buffer.peekAt<int>(8));
    EXPECT_EQ(3532, buffer.peekAt<int>(4));
}

TEST(ByteBufferTests, CanAppendBuffers)
{
    ByteBuffer buffer1;
    buffer1.write<int>(0);
    buffer1.write<int>(1);
    buffer1.write<int>(2);

    ByteBuffer buffer2;
    buffer2.write<int>(3);
    buffer2.write<int>(4);
    buffer2.write<int>(5);

    buffer1.append(buffer2);

    EXPECT_EQ(6 * sizeof(int), buffer1.size());
    EXPECT_EQ(5, buffer1.peekAt<int>(5 * sizeof(int)));
}

TEST(ByteBufferTests, CanSwapEndian)
{
    ByteBuffer buffer;
    
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(2);
    
    EXPECT_EQ(uint32_t(2), buffer.peek<uint32_t>(true));

    // Start a new check with a 64bit value
    buffer.clear();
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(0);
    buffer.write<char>(2);

    EXPECT_EQ(uint64_t(2), buffer.peek<uint64_t>(true));
}

}
