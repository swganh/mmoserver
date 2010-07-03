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

#ifndef SRC_COMMON_BYTEBUFFER_H_
#define SRC_COMMON_BYTEBUFFER_H_

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

namespace common {

class ByteBuffer;
  
template<typename T> 
ByteBuffer& operator<<(ByteBuffer& buffer, const T& value);

std::ostream& operator<<(std::ostream& message, const ByteBuffer& buffer);

class ByteBuffer
{		
public:
	enum { SWAP_ENDIAN = 1 };

public:
	ByteBuffer();
	explicit ByteBuffer(size_t length);
	explicit ByteBuffer(std::vector<unsigned char>& data);
	ByteBuffer(const unsigned char* data, size_t length);
	~ByteBuffer();

	ByteBuffer(const ByteBuffer& from);
	ByteBuffer& operator=(const ByteBuffer& from);

	void Swap(ByteBuffer& from);

    void Append(const ByteBuffer& from);

	template<typename T> ByteBuffer& Write(T data);
	template<typename T> ByteBuffer& WriteAt(size_t offset, T data);
	template<typename T> const T Peek(bool do_swap_endian = false);
	template<typename T> const T PeekAt(size_t offset, bool do_swap_endian = false);
	template<typename T> const T Read(bool do_swap_endian = false);

	void Write(const unsigned char* data, size_t size);
	void Write(size_t offset, const unsigned char* data, size_t size);
	void Clear();

    size_t ReadPosition() const;
    void ReadPosition(size_t position);

    size_t WritePosition() const;
    void WritePosition(size_t position);

    void Reserve(size_t length);
	size_t Size() const;
	size_t Capacity() const;
	const unsigned char* Data() const;

    std::vector<unsigned char>& Raw();

private:
	template<typename T> void SwapEndian_(T& data);

	std::vector<unsigned char> data_;
	size_t read_position_;
	size_t write_position_;

}; // ByteBuffer

}  // namespace common

// Move inline implementations to a separate file to clean up the declaration header.
#include "ByteBuffer-Inl.h"

#endif  // SRC_COMMON_BYTEBUFFER_H_
