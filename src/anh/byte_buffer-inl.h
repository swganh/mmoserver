/**
 * SWG:ANH Sandbox
 *
 * @copyright   Copyright (c) 2010 The SWG:ANH Team
 * @author      Eric Barr <apathy@swganh.org>
 */

#ifndef ANH_BYTE_BUFFER_INL_H_
#define ANH_BYTE_BUFFER_INL_H_

#include <string>
#include <stdexcept>

namespace anh {

template<typename T>
void ByteBuffer::swapEndian(T& data) const {
  //data; /* Only template specializations of swapEndian should be used */
}

template<typename T>
ByteBuffer& ByteBuffer::write(T data) {
  write(reinterpret_cast<unsigned char*>(&data), sizeof(T));
  return *this;
}

template<typename T>
ByteBuffer& ByteBuffer::writeAt(size_t offset, T data) {
  write(offset, reinterpret_cast<unsigned char*>(&data), sizeof(T));
  return *this;
}

template<typename T>
const T ByteBuffer::read(bool doSwapEndian) {
  T data = peek<T>(doSwapEndian);
  read_position_ += sizeof(T);
  return data;
}

template<typename T>
const T ByteBuffer::peek(bool doSwapEndian) const {
  return peekAt<T>(read_position_, doSwapEndian);
}

template<typename T>
const T ByteBuffer::peekAt(size_t offset, bool doSwapEndian) const {
  if (data_.size() < offset + sizeof(T)) {
    throw std::out_of_range("Read past end of buffer");
  }

  T data = *reinterpret_cast<const T*>(&data_[offset]);

  if (doSwapEndian)
    swapEndian<T>(data);

  return data;
}

template<typename T>
void ByteBuffer::swapEndian16(T& data) const {
  data = (data >> 8) | (data << 8);
}

template<typename T>
void ByteBuffer::swapEndian32(T& data) const {
  data =  (data >> 24) |
         ((data & 0x00FF0000) >> 8) |
         ((data & 0x0000FF00) << 8) |
          (data << 24);
}

template<typename T>
void ByteBuffer::swapEndian64(T& data) const {
  data = (data  >> 56) |

#ifdef _WIN32
    ((data & 0x00FF000000000000) >> 40) |
    ((data & 0x0000FF0000000000) >> 24) |
    ((data & 0x000000FF00000000) >> 8)  |
    ((data & 0x00000000FF000000) << 8)  |
    ((data & 0x0000000000FF0000) << 24) |
    ((data & 0x000000000000FF00) << 40) |
#else
    ((data & 0x00FF000000000000LLU) >> 40) |
    ((data & 0x0000FF0000000000LLU) >> 24) |
    ((data & 0x000000FF00000000LLU) >> 8)  |
    ((data & 0x00000000FF000000LLU) << 8)  |
    ((data & 0x0000000000FF0000LLU) << 24) |
    ((data & 0x000000000000FF00LLU) << 40) |
#endif

    (data  << 56);
}

template<> void ByteBuffer::swapEndian(uint16_t& data) const;
template<> void ByteBuffer::swapEndian(uint32_t& data) const;
template<> void ByteBuffer::swapEndian(uint64_t& data) const;
template<> void ByteBuffer::swapEndian(int16_t& data) const;
template<> void ByteBuffer::swapEndian(int32_t& data) const;
template<> void ByteBuffer::swapEndian(int64_t& data) const;

template<> ByteBuffer& ByteBuffer::write<std::string>(std::string data);
template<> const std::string ByteBuffer::read<std::string>(bool doSwapEndian);
template<> ByteBuffer& ByteBuffer::write<std::wstring>(std::wstring data);
template<> const std::wstring ByteBuffer::read<std::wstring>(bool doSwapEndian);

}  // namespace anh

template<typename T>
anh::ByteBuffer& operator<<(anh::ByteBuffer& buffer, const T& value) {
  buffer.write<T>(value);
  return buffer;
}

#endif  // ANH_BYTE_BUFFER_INL_H_
