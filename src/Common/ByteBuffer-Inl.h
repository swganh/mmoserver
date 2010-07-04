/**
 * SWG:ANH Sandbox
 *
 * @copyright   Copyright (c) 2010 The SWG:ANH Team
 * @author      Eric Barr <apathy@swganh.org>
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

template<> void ByteBuffer::SwapEndian_(uint16_t& data) const;
template<> void ByteBuffer::SwapEndian_(uint32_t& data) const;
template<> void ByteBuffer::SwapEndian_(uint64_t& data) const;

template<> ByteBuffer& ByteBuffer::Write<std::string>(std::string data);
template<> const std::string ByteBuffer::Read<std::string>(bool do_swap_endian);
template<> ByteBuffer& ByteBuffer::Write<std::wstring>(std::wstring data);
template<> const std::wstring ByteBuffer::Read<std::wstring>(bool do_swap_endian);

}  // namespace common

#endif  // SRC_COMMON_BYTEBUFFERINL_H_
