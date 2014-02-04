/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2014 The SWG:ANH Team

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

#ifndef ANH_MEMORY_H_
#define ANH_MEMORY_H_

#include <memory>

#include "tbb/scalable_allocator.h"

void* operator new (size_t size);

void* operator new[] (size_t size);

void* operator new (size_t size, const std::nothrow_t&) throw ();

void* operator new[ ] (size_t size, const std::nothrow_t&) throw ();

void operator delete (void* ptr) throw ();

void operator delete[] (void* ptr) throw ();

void operator delete (void* ptr, const std::nothrow_t&) throw ();

void operator delete[] (void* ptr, const std::nothrow_t&) throw ();

#endif  // ANH_MEMORY_H_
