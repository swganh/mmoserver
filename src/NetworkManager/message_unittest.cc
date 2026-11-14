/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
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

// Message reads are bounds checked: a malformed or hostile packet cannot make
// the server read past the end of a message.

#include <cstring>
#include <vector>

#include <gtest/gtest.h>

#include "NetworkManager/Message.h"

namespace {

// A message over a buffer with guard bytes after its end, so a read past the
// end would pick up 0xEE.
class MessageReadTest : public ::testing::Test {
protected:
    void Use(const std::vector<unsigned char>& bytes) {
        buffer_.assign(bytes.begin(), bytes.end());
        buffer_.resize(bytes.size() + 64, 0xEE);
        message_.Init(reinterpret_cast<int8*>(buffer_.data()), static_cast<uint16>(bytes.size()));
    }

    std::vector<unsigned char> buffer_;
    Message message_;
};

TEST_F(MessageReadTest, ReadsInsideTheMessage) {
    Use({0x01, 0x02, 0x03, 0x04, 0x05, 0x06});
    EXPECT_EQ(0x0201, message_.getUint16());
    EXPECT_EQ(0x06050403u, message_.getUint32());
    EXPECT_FALSE(message_.isOverrun());
    EXPECT_EQ(6, message_.getIndex());
}

TEST_F(MessageReadTest, ReadPastTheEndGivesZeroAndMarksTheMessage) {
    Use({0x01, 0x02, 0x03});
    EXPECT_EQ(0x0201, message_.getUint16());
    EXPECT_EQ(0u, message_.getUint32());   // 1 byte left, 4 wanted
    EXPECT_TRUE(message_.isOverrun());
    EXPECT_EQ(2, message_.getIndex());      // the index did not move
    EXPECT_EQ(0, message_.getUint8());      // every read after it is 0
    message_.ResetIndex();
    EXPECT_FALSE(message_.isOverrun());
    EXPECT_EQ(0x01, message_.getUint8());
}

TEST_F(MessageReadTest, PeekPastTheEndGivesZero) {
    Use({0xAA});
    EXPECT_EQ(0u, message_.peekUint64());
    EXPECT_EQ(0xAA, message_.peekUint8());
}

TEST_F(MessageReadTest, AnsiStringLongerThanTheMessageIsEmpty) {
    // Declares 200 characters, carries 3.
    Use({200, 0x00, 'a', 'b', 'c'});
    BString text;
    EXPECT_EQ(0, message_.getStringAnsi(text));
    EXPECT_EQ(0, text.getLength());
    EXPECT_TRUE(message_.isOverrun());
}

TEST_F(MessageReadTest, AnsiStringInsideTheMessage) {
    Use({3, 0x00, 'a', 'b', 'c'});
    BString text;
    EXPECT_EQ(3, message_.getStringAnsi(text));
    EXPECT_STREQ("abc", text.getAnsi());
    EXPECT_FALSE(message_.isOverrun());
}

TEST_F(MessageReadTest, UnicodeStringWithAHugeLengthIsEmpty) {
    // 0xFFFF characters would overflow BString's 16-bit allocation size.
    Use({0xFF, 0xFF, 0x00, 0x00, 'a', 0x00});
    BString text;
    EXPECT_EQ(0, message_.getStringUnicode16(text));
    EXPECT_EQ(0, text.getLength());
    EXPECT_TRUE(message_.isOverrun());
}

TEST_F(MessageReadTest, StdStringsAreBoundsChecked) {
    Use({50, 0x00, 'x'});
    EXPECT_TRUE(message_.getStringAnsi().empty());
    EXPECT_TRUE(message_.isOverrun());

    Use({2, 0x00, 0x00, 0x00, 'h', 0x00, 'i', 0x00});
    EXPECT_EQ(u"hi", message_.getStringUnicode16());
    EXPECT_FALSE(message_.isOverrun());
}

}  // namespace
