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

#ifndef ANH_UTILITIES_H_
#define ANH_UTILITIES_H_

#include <cstdint>
#include <type_traits>

namespace anh {

    namespace detail {
        /// @TODO Remove comment when visual studio begins supporting constexpr.
        /*constexpr*/ bool inline isBigEndian() {
            uint16_t x = 1;
            return !(*reinterpret_cast<char*>(&x));
        }
        
        template<typename T>
        T swapEndian_(T value, std::integral_constant<size_t, 1>) {
            return value;
        }

        template<typename T>
        T swapEndian_(T value, std::integral_constant<size_t, 2>) {
            //std::make_unsigned<T>::type& tmp = reinterpret_cast<std::make_unsigned<T>::type&>(value);
			typedef typename std::make_unsigned<T>::type tmpt;
			tmpt tmp = tmpt(value);
            tmp = (tmp >> 8) | (tmp << 8);

			return tmp;// value;
        }
        
        template<typename T>
        T swapEndian_(T value, std::integral_constant<size_t, 4>) {
            //std::make_unsigned<T>::type& tmp = reinterpret_cast<std::make_unsigned<T>::type&>(value);
			typedef typename std::make_unsigned<T>::type tmpt;
			tmpt tmp = tmpt(value);

            tmp = (tmp >> 24) |
                   ((tmp & 0x00FF0000) >> 8) | ((tmp & 0x0000FF00) << 8) |
                   (tmp << 24);

            return tmp;//value
        }
        
        template<typename T>
        T swapEndian_(T value, std::integral_constant<size_t, 8>) {
            //std::make_unsigned<T>::type& tmp = reinterpret_cast<std::make_unsigned<T>::type&>(value);
			typedef typename std::make_unsigned<T>::type tmpt;
			tmpt tmp = tmpt(value);

            tmp = (tmp >> 56) |
                ((tmp & 0x00FF000000000000ULL) >> 40) |
                ((tmp & 0x0000FF0000000000ULL) >> 24) |
                ((tmp & 0x000000FF00000000ULL) >> 8)  |
                ((tmp & 0x00000000FF000000ULL) << 8)  |
                ((tmp & 0x0000000000FF0000ULL) << 24) |
                ((tmp & 0x000000000000FF00ULL) << 40) |
                (tmp  << 56);

            return tmp;//value
        }
    }

    /*! Swaps the endianness of integral values and returns the results.
    *
    * \param value An integral value for which to swap the endianness.
    * \return A copy of the input parameter with its endianness swapped.
    */
    template<typename T>
    T swapEndian(T value) {
        static_assert(std::is_integral<int>::value, "swap_endian<T> requires T to be an integral type.");
        return detail::swapEndian_<T>(value, std::integral_constant<size_t, sizeof(T)>());
    }

    /*! Converts an integral value from host-byte order to little endian.
    *
    * \param value An integral value to convert to little endian.
    * \return The value converted to little endian order.
    */
    template<typename T>
    T hostToLittle(T value) {
        static_assert(std::is_integral<int>::value, "host_to_little<T> requires T to be an integral type.");
        return detail::isBigEndian() ? swapEndian(value) : value;
    }
    
    /*! Converts an integral value from host-byte order to big endian.
    *
    * \param value The value to convert to big endian
    * \return The value converted to big endian order.
    */
    template<typename T>
    T hostToBig(T value) {
        static_assert(std::is_integral<int>::value, "host_to_big<T> requires T to be an integral type.");
        return detail::isBigEndian() ? value : swapEndian(value);
    }
    
    /*! Converts an integral value from big endian to host-byte order.
    *
    * \param value The value to convert to host-byte order.
    * \return The value converted to host-byte order.
    */
    template<typename T>
    T bigToHost(T value) {
        static_assert(std::is_integral<int>::value, "big_to_host<T> requires T to be an integral type.");
        return detail::isBigEndian() ? value : swapEndian(value);
    }
    
    /*! Converts an integral value from little endian to host-byte order.
    *
    * \param value The value to convert to host-byte order.
    * \return The value converted to host-byte order.
    */
    template<typename T>
    T littleToHost(T value) {
        static_assert(std::is_integral<int>::value, "little_to_host<T> requires T to be an integral type.");
        return detail::isBigEndian() ? swapEndian(value) : value;
    }
        
}  // namespace anh

#endif  // ANH_UTILITIES_H_
