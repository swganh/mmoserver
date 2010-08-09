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

#include "Common/CommonDeclspec.h"

namespace common {

class ByteBuffer;
  
/**
 * Allows streaming of values to a ByteBuffer instance.
 *
 * @param buffer Instance of a ByteBuffer.
 * @param value The data value to write to the ByteBuffer.
 */
template<typename T> 
COMMON_API ByteBuffer& operator<<(ByteBuffer& buffer, const T& value);

/**
 * Writes a ByteBuffer to an output stream in a human readable format.
 *
 * @param message The output stream to write the data to.
 * @param buffer The ByteBuffer to stream data from.
 */
COMMON_API std::ostream& operator<<(std::ostream& message, const ByteBuffer& buffer);

/**
 * This utility class is used to package various data types into a byte stream.
 *
 * Byte streams are commonly used for packets and binary files, this utility class
 * eases the task of reading and writing data to these resources.
 */
class COMMON_API ByteBuffer
{		
public:
	enum { SWAP_ENDIAN = 1 };

public:
    /// Default constructor, creates an empty ByteBuffer.
	ByteBuffer();

    /**
     * Constructor overload used to set an explicit size for the ByteBuffer.
     */
	explicit ByteBuffer(size_t length);

    /**
     * Constructor overload that creates a ByteBuffer from a data array.
     *
     * @param data The data to initialize the ByteBuffer with.
     */
	explicit ByteBuffer(std::vector<unsigned char>& data);

    /**
     * Constructor overload that creates a ByteBuffer from a C array.
     *
     * @param data A C array containing the data.
     * @param length Length of the C array.
     */
	ByteBuffer(const unsigned char* data, size_t length);

    /// Default deconstructor.
	~ByteBuffer();

    /**
     * Copy constructor, used to copy one ByteBuffer to another.
     * 
     * @param from The ByteBuffer to use as the source in the copy.
     */
	ByteBuffer(const ByteBuffer& from);

    /**
     * Assignment operator, assigns one ByteBuffer's contents to another.
     *
     * @param from The ByteBuffer to use as the source in the assignment.
     */
	ByteBuffer& operator=(const ByteBuffer& from);

    /**
     * A no-throw swap used for swapping the contents of two ByteBuffers.
     *
     * @param from The source ByteBuffer to swap contents with.
     */
	void Swap(ByteBuffer& from);

    /**
     * Appends one ByteBuffer to another.
     *
     * @param from The ByteBuffer that should be appended.
     */
    void Append(const ByteBuffer& from);

    /**
     * Write's data of type T to the ByteBuffer.
     *
     * This templated function allows writing different basic types to the ByteBuffer.
     * Below is an example usage:
     *
     * @code
     * ByteBuffer buffer;
     * buffer.Write<int>(0);
     * buffer.Write<std::string>("This is a string");
     * @endcode
     *
     * @param data The data to write to the ByteBuffer.
     */
	template<typename T> ByteBuffer& Write(T data);

    /**
     * Write's data of type T to the ByteBuffer at an offset.
     *
     * This templated function allows writing different basic types to the ByteBuffer
     * at a specific offset. This is very useful for replacing data at known locations
     * in an existing ByteBuffer (ie., changing object id's or counters in packets):
     *
     * @code
     * ByteBuffer buffer;
     * buffer.WriteAt<int>(42, 0); // Writes an int of 0 at position 42.
     * buffer.WriteAt<std::string>(22, "This is a string"); // Writes a string starting at position 22.
     * @endcode
     *
     * @param offset The offset to start writing the data at.
     * @param data The data to write to the ByteBuffer.
     */
	template<typename T> ByteBuffer& WriteAt(size_t offset, T data);

    /**
     * Reads a value from the ByteBuffer at the current read position without it.
     *
     * @param do_swap_endian Swaps the endian of the value from the ByteBuffer before it is returned.
     * @returns The value at the current position.
     */
	template<typename T> const T Peek(bool do_swap_endian = false) const;

    /**
     * Reads a value from the ByteBuffer at an offset without moving the read position.
     *
     * @param offset The offset to read the data from.
     * @param do_swap_endian Swaps the endian of the value from the ByteBuffer before it is returned.
     * @returns The value at the current position.
     */
	template<typename T> const T PeekAt(size_t offset, bool do_swap_endian = false) const;

    /**
     * Reads a value from the ByteBuffer at the current read position.
     *
     * @param do_swap_endian Swaps the endian of the value from the ByteBuffer before it is returned.
     * @returns The value at the current position.
     */
	template<typename T> const T Read(bool do_swap_endian = false);

    /**
     * Write's data to the ByteBuffer.
     *
     * @param data The data to write to the ByteBuffer.
     * @param size Size of the data to write to the ByteBuffer.
     */
	void Write(const unsigned char* data, size_t size);
    
    /**
     * Write's data to the ByteBuffer at an offset.
     *
     * @param offset The offset to start writing the data at.
     * @param data The data to write to the ByteBuffer.
     * @param size Size of the data to write to the ByteBuffer.
     */
	void Write(size_t offset, const unsigned char* data, size_t size);

    /// Clear's the ByteBuffer (useful for reusing a buffer to save memory allocations).
	void Clear();

    /**
     * Gets the current read position
     *
     * @returns The current read position.
     */
    size_t ReadPosition() const;

    /** 
     * Sets the current read position.
     *
     * @param position The new read position.
     */
    void ReadPosition(size_t position);

    /**
     * Gets the current write position.
     *
     * @returns The current write position.
     */
    size_t WritePosition() const;

    /**
     * Sets the current write position.
     *
     * @param position The new write position.
     */
    void WritePosition(size_t position);

    /**
     * Reserves a specific amount of space for the ByteBuffer.
     *
     * @param size The size of space to reserve for the ByteBuffer.
     */
    void Reserve(size_t size);

    /**
     * Gets the current size of the ByteBuffer.
     *
     * @returns The current size of the ByteBuffer.
     */
	size_t Size() const;

    /**
     * Returns the ByteBuffer contents, useful for working with C functions.
     *
     * @returns The raw ByteBuffer contents.
     */
	const unsigned char* Data() const;

    /** 
     * Returns the ByteBuffer contents in a modifyable format. This should rarely
     * be used, opt for safer methods of access unless you actually need to modify
     * the ByteBuffer internals and the normal accessors cannot do the job.
     *
     * @returns A modifyable form of the ByteBuffer's internal data.
     */
    std::vector<uint8_t>& Raw();

private:
	template<typename T> void SwapEndian_(T& data) const;

	std::vector<uint8_t> data_;
	size_t read_position_;
	size_t write_position_;

}; // ByteBuffer

}  // namespace common

// Move inline implementations to a separate file to clean up the declaration header.
#include "ByteBuffer-Inl.h"

#endif  // SRC_COMMON_BYTEBUFFER_H_
