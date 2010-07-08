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

#include <gtest/gtest.h>

#include "Common/ByteBuffer.h"

using ::common::ByteBuffer;

namespace {

TEST(ByteBufferTests, ByteBufferIsEmptyWhenCreated) {
    ByteBuffer buffer;
    EXPECT_EQ(0, buffer.Size());
}

TEST(ByteBufferTests, WritingIntReportsCorrectSizeAndCapacity) {
    ByteBuffer buffer;
    buffer.Write<int>(10);
    
    EXPECT_EQ(4, buffer.Size());
}

TEST(ByteBufferTests, WritingTwoIntsReportsCorrectSizeAndCapacity) {
    ByteBuffer buffer;
    
    buffer.Write<int>(10);
    EXPECT_EQ(4, buffer.Size());
    
    buffer.Write<int>(20);
    EXPECT_EQ(8, buffer.Size());
}

TEST(ByteBufferTests, CanReadIntWrittenToTheBuffer) {
    ByteBuffer buffer;
    
    buffer.Write<int>(10);
    EXPECT_EQ(10, buffer.Read<int>());
}

TEST(ByteBufferTests, CanReadTwoIntsWrittenToTheBuffer) {
    ByteBuffer buffer;
    
    buffer.Write<int>(10);
    buffer.Write<int>(20);
    
    EXPECT_EQ(10, buffer.Read<int>());
    EXPECT_EQ(20, buffer.Read<int>());
}

TEST(ByteBufferTests, ReadingPastBufferEndThrowsException) {
    ByteBuffer buffer;
    EXPECT_EQ(0, buffer.Size());
    
    EXPECT_THROW(buffer.Read<int>(), std::out_of_range);
}

TEST(ByteBufferTests, WritingStringReportsCorrectSizeAndCapacity) {
    ByteBuffer buffer;
    EXPECT_EQ(0, buffer.Size());
    
    buffer.Write<std::string>(std::string("test string"));
    
    EXPECT_EQ(13, buffer.Size());
}

TEST(ByteBufferTests, CanReadStringWrittenToTheBuffer) {
    ByteBuffer buffer;
    std::string test_string("test string data");
    
    buffer.Write<std::string>(test_string);
    
    EXPECT_EQ(test_string, buffer.Read<std::string>());
}

TEST(ByteBufferTests, CanReadTwoStringsWrittenToTheBuffer) {
    ByteBuffer buffer;
    std::string test_string1("first test string");
    std::string test_string2("second test string");
    
    buffer.Write<std::string>(test_string1);
    buffer.Write<std::string>(test_string2);
    
    EXPECT_EQ(test_string1, buffer.Read<std::string>());
    EXPECT_EQ(test_string2, buffer.Read<std::string>());
}

TEST(ByteBufferTests, WritingUnicodeStringReportsCorrectSizeAndCapacity) {
    ByteBuffer buffer;
    EXPECT_EQ(0, buffer.Size());
    
    buffer.Write<std::wstring>(std::wstring(L"test string"));
    
    // Size should be size of int + size of string * size of wchar_t.
    EXPECT_EQ(sizeof(uint32_t) + (11 * 2), buffer.Size());
}

TEST(ByteBufferTests, CanReadUnicodeStringWrittenToTheBuffer) {
    ByteBuffer buffer;
    std::wstring test_string(L"test string data");
    
    buffer.Write<std::wstring>(test_string);
    EXPECT_EQ(test_string, buffer.Read<std::wstring>());
}

TEST(ByteBufferTests, UnicodeStringStoredAs16Bit) {
    ByteBuffer buffer;
    std::wstring test_string(L"testdata");
    
    buffer.Write<std::wstring>(test_string);
    
    EXPECT_EQ(8, buffer.Read<uint32_t>());
    
    EXPECT_EQ('t', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('e', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('s', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('t', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('d', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('a', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('t', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
    EXPECT_EQ('a', buffer.Read<uint8_t>());
    EXPECT_EQ(0, buffer.Read<uint8_t>());
}

TEST(ByteBufferTests, CanClearBufferData) {
    ByteBuffer buffer;
    EXPECT_EQ(0, buffer.Size());
    
    buffer.Write<int>(3);
    EXPECT_EQ(4, buffer.Size());
    
    buffer.Clear();
    EXPECT_EQ(0, buffer.Size());
}

TEST(ByteBufferTests, CanStreamData) {
    ByteBuffer buffer;
    int testInt = 3;
    
    buffer << testInt;
    EXPECT_EQ(3, buffer.Read<int>());
}

TEST(ByteBufferTests, PeekingDataDoesNotMoveReadPosition) {
    ByteBuffer buffer;
    buffer.Write<int>(3);
    buffer.Write<int>(10);
    
    // Peek the data twice, this should return the same integer value (3) both times.
    EXPECT_EQ(3, buffer.Peek<int>());
    EXPECT_EQ(3, buffer.Peek<int>());
}

TEST(ByteBufferTests, CanReadAndWriteUnsignedData) {
    ByteBuffer buffer;
    buffer.Write<uint16_t>(3);
    
    EXPECT_EQ(2, buffer.Size()); // First check that the right datasize was written.
    EXPECT_EQ(3, buffer.Read<uint16_t>()); // Check that the value is correct.
}

TEST(ByteBufferTests, CanPeekAtOffset) {
    ByteBuffer buffer;
    buffer.Write<int>(3);
    buffer.Write<int>(32);
    buffer.Write<int>(979);
    buffer.Write<int>(5467);
    
    EXPECT_EQ(979, buffer.PeekAt<int>(8));
    EXPECT_EQ(32, buffer.PeekAt<int>(4));
}

TEST(ByteBufferTests, CanWriteAtOffset) {
    ByteBuffer buffer;
    buffer.Write<int>(3);
    buffer.Write<int>(32);
    buffer.Write<int>(979);
    buffer.Write<int>(5467);
    
    buffer.WriteAt<int>(8, 52);
    buffer.WriteAt<int>(4, 3532);
    
    EXPECT_EQ(52, buffer.PeekAt<int>(8));
    EXPECT_EQ(3532, buffer.PeekAt<int>(4));
}

TEST(ByteBufferTests, CanAppendBuffers) {
    ByteBuffer buffer1;
    buffer1.Write<int>(0);
    buffer1.Write<int>(1);
    buffer1.Write<int>(2);
    
    ByteBuffer buffer2;
    buffer2.Write<int>(3);
    buffer2.Write<int>(4);
    buffer2.Write<int>(5);
    
    buffer1.Append(buffer2);
    
    EXPECT_EQ(6 * sizeof(int), buffer1.Size());
    EXPECT_EQ(5, buffer1.PeekAt<int>(5 * sizeof(int)));
}

}