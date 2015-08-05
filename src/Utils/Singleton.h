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

#ifndef SRC_UTILS_SINGLETON_H_
#define SRC_UTILS_SINGLETON_H_

namespace utils {

/*! \brief This implementation of the Meyer's singleton is meant to provide singleton
 * capabilities non-intrusively to client classes.
 *
 * This is a limited implementation that only works with classes that provide a
 * default constructor and which are intended to be kept alive for the remainder
 * of the host application's lifetime.
 *
 * \code
 * class Foo {
 * public:
 *   void bar();
 * };
 *
 * ...
 *
 * // Use the singleton to directly access the classes member function.
 * Singleton<Foo>::Instance().bar();
 *
 * ...
 *
 * // Or get a handle to the singleton class and access it directly.
 * Foo& foo = Singleton<Foo>::Instance();
 * foo.bar(); *
 * \endcode
 */
template <class T>
class Singleton
{
public:
    static T& Instance() {
        static T _instance;
        return _instance;
    }

private:
    // Disable compiler generated code to prevent default constuction/destruction
    // and copy/assignment semantics.
    Singleton();
    ~Singleton();
    Singleton(Singleton const&);
    Singleton& operator=(Singleton const&);
};

}  // namespace utils

#endif  // SRC_UTILS_SINGLETON_H_
