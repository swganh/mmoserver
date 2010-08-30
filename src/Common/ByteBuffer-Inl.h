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

#ifndef SRC_COMMON_BYTEBUFFERINL_H_
#define SRC_COMMON_BYTEBUFFERINL_H_

namespace common {

template<typename T> void ByteBuffer::SwapEndian_(T& data) const {
    data; /* Only template specializations of swapEndian should be used */
}

template<typename T> ByteBuffer& ByteBuffer::Write(T data) {
    Write(reinterpret_cast<unsigned char*>(&data), sizeof(T));
    return *this;
}

template<typename T> ByteBuffer& ByteBuffer::WriteAt(size_t offset, T data) {
    Write(offset, reinterpret_cast<unsigned char*>(&data), sizeof(T));
    return *this;
}


template<typename T> const T ByteBuffer::Read(bool do_swap_endian) {
    T data = Peek<T>(do_swap_endian);
    read_position_ += sizeof(T);
    return data;
}

template<typename T> const T ByteBuffer::Peek(bool do_swap_endian) const {
    return PeekAt<T>(read_position_, do_swap_endian);
}

template<typename T> const T ByteBuffer::PeekAt(size_t offset, bool do_swap_endian) const {
    if (data_.size() < offset + sizeof(T)) {
        throw std::out_of_range("Read past end of buffer");
    }

    T data = *reinterpret_cast<const T*>(&data_[offset]);

    if (do_swap_endian)
        SwapEndian_<T>(data);

    return data;
}

template<typename T> ByteBuffer& operator<<(ByteBuffer& buffer, const T& value) {
    buffer.Write<T>(value);
    return buffer;
}

template<> COMMON_API void ByteBuffer::SwapEndian_(uint16_t& data) const;
template<> COMMON_API void ByteBuffer::SwapEndian_(uint32_t& data) const;
template<> COMMON_API void ByteBuffer::SwapEndian_(uint64_t& data) const;

template<> COMMON_API ByteBuffer& ByteBuffer::Write<std::string>(std::string data);
template<> COMMON_API const std::string ByteBuffer::Read<std::string>(bool do_swap_endian);
template<> COMMON_API ByteBuffer& ByteBuffer::Write<std::wstring>(std::wstring data);
template<> COMMON_API const std::wstring ByteBuffer::Read<std::wstring>(bool do_swap_endian);

}  // namespace common

#endif  // SRC_COMMON_BYTEBUFFERINL_H_
