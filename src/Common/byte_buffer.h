/*
 * SWG:ANH Combat Upgrade Sandbox
 *
 * @copyright   Copyright (c) 2010 The SWG:ANH Team
 * @author      Eric Barr <apathy@swganh.org>
**/

#ifndef SRC_SHARED_BYTE_BUFFER_H_
#define SRC_SHARED_BYTE_BUFFER_H_

#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

namespace common {

class ByteBuffer {
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

  void swap(ByteBuffer& from); // NOLINT

  void append(const ByteBuffer& from);

  template<typename T> ByteBuffer& write(T data);
  template<typename T> ByteBuffer& writeAt(size_t offset, T data);
  template<typename T> const T peek(bool doSwapEndian = false) const;
  template<typename T> const T peekAt(size_t offset, bool doSwapEndian = false) const;
  template<typename T> const T read(bool doSwapEndian = false);

  void write(const unsigned char* data, size_t size);
  void write(size_t offset, const unsigned char* data, size_t size);
  void clear();

  size_t readPosition() const;
  void readPosition(size_t position);

  size_t writePosition() const;
  void writePosition(size_t position);

  void reserve(size_t length);
  size_t size() const;
  size_t capacity() const;
  const unsigned char* data() const;

  std::vector<unsigned char>& raw();

 private:
  template<typename T> void swapEndian(T& data) const;
  template<typename T> void swapEndian16(T& data) const;
  template<typename T> void swapEndian32(T& data) const;
  template<typename T> void swapEndian64(T& data) const;

  std::vector<unsigned char> data_;
  size_t read_position_;
  size_t write_position_;
};

}  // namespace common

// Move inline implementations to a separate file to
// clean up the declaration header.
#include "Common/byte_buffer-inl.h"

#endif  // SRC_SHARED_BYTE_BUFFER_H_
