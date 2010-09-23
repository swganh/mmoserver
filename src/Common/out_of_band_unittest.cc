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

#include "Common/OutOfBand.h"

#include <gtest/gtest.h>

#include "Common/byte_buffer.h"

using common::ByteBuffer;
using common::OutOfBand;
using common::ProsePackage;

// An default created (empty) OutOfBand attachment should have a zero package count.
TEST(OutOfBandTests, DefaultPackageCountIsZero) {
    OutOfBand attachment;
        
	EXPECT_EQ(0, attachment.Count());
}

// Adding a ProsePackage increases the count.
TEST(OutOfBandTests, AddingProsePackageIncreasesCount) {
    OutOfBand attachment;
    
    ProsePackage prose;
    prose.base_stf_file = "test_file";
    prose.base_stf_label = "test_label";

    attachment.AddProsePackage(prose);

    EXPECT_EQ(1, attachment.Count());
}

// Can add a prose by using a ProsePackage (will make preloading
// text much easier).
TEST(OutOfBandTests, AddingProseSetsAppropriateLength) {
    OutOfBand attachment;

    ProsePackage prose;
    prose.base_stf_file = "test_file";
    prose.base_stf_label = "test_label";

    attachment.AddProsePackage(prose);

    EXPECT_EQ(uint32_t(52), attachment.Length());
}

TEST(OutOfBandTests, PackingAttachmentReturnsCorrectOutput) {
    OutOfBand attachment;

    attachment.AddProsePackage("test_file", "test_label");

    const ByteBuffer* output = attachment.Pack();

    // Make sure it's the correct length.
    EXPECT_EQ(uint32_t(108), output->size());

    // Make sure the test from our adding a prose package worked.
    // Check the 'test' from 'test_file' is in the right location.
    EXPECT_EQ('t', output->peekAt<uint8_t>(13));    
    EXPECT_EQ('e', output->peekAt<uint8_t>(14));    
    EXPECT_EQ('s', output->peekAt<uint8_t>(15));    
    EXPECT_EQ('t', output->peekAt<uint8_t>(16));
    
    // Check the 'test' from 'test_label' is in the right location.
    EXPECT_EQ('t', output->peekAt<uint8_t>(28));    
    EXPECT_EQ('e', output->peekAt<uint8_t>(29));    
    EXPECT_EQ('s', output->peekAt<uint8_t>(30));    
    EXPECT_EQ('t', output->peekAt<uint8_t>(31));
}

TEST(OutOfBandTests, CanCreateOutOfBandFromProse) {
    // First test simple stf string prose creation.
    OutOfBand attachment("test_file", "test_label");
    
    EXPECT_EQ(1, attachment.Count());
    
    // Next test a more complex created prose (for example, prose loaded at startup).
    ProsePackage prose;
    prose.base_stf_file = "test_file";
    prose.base_stf_label = "test_label";
    
    OutOfBand attachment2(prose);

    EXPECT_EQ(1, attachment2.Count());
}

TEST(OutOfBandTests, CanAddMultipleProsePackages) {
    OutOfBand attachment;

    ProsePackage prose;
    prose.base_stf_file = "test_file";
    prose.base_stf_label = "test_label";

    attachment.AddProsePackage(prose);
    attachment.AddProsePackage(prose);
    attachment.AddProsePackage(prose);
    
    EXPECT_EQ(3, attachment.Count());
    
    // Make sure it's the correct size.
    EXPECT_EQ(uint32_t(154), attachment.Length());
}
