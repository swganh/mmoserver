/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2010 The SWG:ANH Team

 MMOServer is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 MMOServer is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with MMOServer.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ANH_BYTE_BUFFER_H_
#define ANH_BYTE_BUFFER_H_

#include <cstdint>
#include <vector>
#include <string>

namespace anh {

/*! \brief The ByteBuffer is a handy utility class for packing data into a
* binary format fit for storage or passing over the network or persistent
* storage.
*/
class ByteBuffer {
public:
    enum { SWAP_ENDIAN = 1 };

public:
    /// Default constructor.
    ByteBuffer();
    
    /// Explicit constructor reserves memory for the specified length.
    explicit ByteBuffer(size_t length);
    
    /// Explicit constructor builds ByteBuffer instance from the passed data.
    explicit ByteBuffer(std::vector<unsigned char>& data);
    
    /// Explicit constructor builds ByteBuffer instance from the passed data.
    ByteBuffer(const unsigned char* data, size_t length);
    
    /// Default destructor.
    ~ByteBuffer();

    /// Copy constructor.
    ByteBuffer(const ByteBuffer& other);

    /// Move constructor.
    ByteBuffer(ByteBuffer&& other);

    /// Universal assignment operator.
    ByteBuffer& operator=(ByteBuffer other);

    /*! Provides an exception safe swap mechanisim for swapping the internals
    * of two ByteBuffer instances.
    *
    * \param other ByteBuffer to swap internals with.
    */
    void swap(ByteBuffer& other);

    /*! Append one ByteBuffer to another.
    *
    * \param other ByteBuffer to append to the current instance.
    */
    void append(const ByteBuffer& other);

    /*! Writes the data value to the ByteBuffer.
    *
    * \param data Value to write to the buffer.
    *
    * \return Reference to the ByteBuffer instance (used for chaining streams
    * together):
    *
    * \code anh::ByteBuffer buf;
    * int value1 = 5;
    * int value2 = 6;
    *
    * buf.write(value1).write(value2); \endcode
    */
    template<typename T> ByteBuffer& write(T data);

    /*! Writes data to the ByteBuffer without moving the write position.
    *
    * Writes data to the ByteBuffer without moving the write position.
    * This is useful when the buffer is already filled and there is a need
    * to replace values at a specific position.
    *
    * \param position Starting position to write the data at.
    * \param data Data to write at the specified position.
    *
    * \return Reference to the ByteBuffer instance (used for chaining streams
    * together):
    *
    * \code anh::ByteBuffer buf;
    * int value1 = 5;
    * int value2 = 6;
    *
    * buf.writeAt(0, value1).write_at(4, value2); \endcode
    */
    template<typename T> ByteBuffer& writeAt(size_t offset, T data);

    /*! Reads the next value in the ByteBuffer without moving the read
    * position.
    *
    * \param do_swap_endian Swap the endian type of the read value.
    *
    * \return The next value in the ByteBuffer.
    */
    template<typename T> const T peek(bool do_swap_endian = false) const;

    /*! Reads the value at the specified position.
    *
    * \param position Position to start reading from.
    * \param do_swap_endian Swap the endian type of the read value.
    *
    * \return The value in the specied position.
    */
    template<typename T> const T peekAt(size_t offset, bool do_swap_endian = false) const;

    /*! Reads the next value in the ByteBuffer.
    *
    * \param do_swap_endian Swap the endian type of the read value.
    *
    * \return The next value in the ByteBuffer.
    */
    template<typename T> const T read(bool do_swap_endian = false);

    /*! Writes raw data to the ByteBuffer.
    *
    * \param data Data to write to the buffer.
    * \param size Size of the data to write to the buffer.
    */
    void write(const unsigned char* data, size_t size);

    /*! Writes raw data to the ByteBuffer at a specified offset.
    *
    * \param offset Offset to write the data at.
    * \param data Data to write to the buffer.
    * \param size Size of the data to write to the buffer.
    */
    void write(size_t offset, const unsigned char* data, size_t size);

    /*! Clears the ByteBuffer */
    void clear();

    /*! \return Returns the read position of the ByteBuffer */
    size_t read_position() const;

    /*! Sets the read position of the ByteBuffer
    *
    * \param position The read position of the buffer.
    */
    void read_position(size_t position);

    /*! \return Returns the write position of the ByteBuffer */
    size_t write_position() const;

    /*! Sets the write position of the ByteBuffer
    *
    * \param position The write position of the buffer.
    */
    void write_position(size_t position);

    /*! Reserves memory for the ByteBuffer to use.
    *
    * \param length The length to reserve for the ByteBuffer.
    */
    void reserve(size_t length);

    /*! \return Returns the size of the ByteBuffer (actual memory used)
    */
    size_t size() const;

    /*! \return Returns the capacity of the ByteBuffer (total memory reserved) */
    size_t capacity() const;

    /*! \return Returns the raw ByteBuffer data */
    const unsigned char* data() const;

    /*! \return Returns the raw ByteBuffer data */
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

}  // namespace anh

/*! This helper function is used to provide support for streaming values to a
* ByteBuffer instance (making it the lvalue):
*
* \code anh::ByteBuffer lvalue;
* int rvalue = 5;
*
* lvalue << rvalue; \endcode
*
* \param buffer Reference to the ByteBuffer to stream data to.
* \param value Value to add to the ByteBuffer instance.
*
* \return Reference to the ByteBuffer instance (used for chaining streams
* together):
*
* \code anh::ByteBuffer buf;
* int value1 = 5;
* int value2 = 6;
*
* buf << value1 << value2; \endcode
*/
template<typename T>
anh::ByteBuffer& operator<<(anh::ByteBuffer& buffer, const T& value);

/*! This helper function provides support for streaming ByteBuffer instances
* to out output stream (most generally for debugging).
*
* \param stream Reference to the output stream to write the ByteBuffer to.
* \param buffer Reference to a ByteBuffer instance.
*
* \return Reference to the output stream.
*/
std::ostream& operator<<(std::ostream& message, const anh::ByteBuffer& buffer);

// Move inline implementations to a separate file to
// clean up the declaration header.
#include "anh/byte_buffer-inl.h"

#endif  // ANH_BYTE_BUFFER_H_
