// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE


// K-3D
// Copyright (c) 1995-2008, Timothy M. Shead
//
// Contact: tshead@k-3d.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

/** \file
	\author Timothy M. Shead (tshead@k-3d.com)
*/
// modified by Kyle Craviotto for ANHStudios
#pragma once

#include <boost/python/import.hpp>
#include <boost/python/list.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/object.hpp>

#include <stdexcept>

namespace utility
{

/// Provides a boilerplate implementation of __len__ for objects with constant length 
template<typename self_t, int size>
static int constant_len_len(const self_t& Self)
{
	return size;
}

/// Provides a boilerplate implementation of __getitem__ for objects with constant length and operator[] 
template<typename self_t, int size, typename value_t>
static value_t constant_len_get_item(const self_t& Self, int Item)
{
	if(Item < 0 || Item >= size)
		throw std::out_of_range("index out-of-range");

	return Self[Item];
}

/// Provides a boilerplate implementation of __setitem__ for objects with constant length and operator[] 
template<typename self_t, int size, typename value_t>
static void constant_len_set_item(self_t& Self, int Item, const value_t& Value)
{
	if(Item < 0 || Item >= size)
		throw std::out_of_range("index out-of-range");

	Self[Item] = Value;
}

/// Provides a boilerplate implementation of __len__ for objects that are wrapped by-reference
template<typename self_t>
static int wrapped_len(const self_t& Self)
{
	return Self.wrapped().size();
}

/// Provides a boilerplate implementation of __getitem__ for objects with at() that are wrapped by-reference 
template<typename self_t, typename value_t>
static value_t wrapped_get_item(self_t& Self, int Item)
{
	if(Item < 0 || Item >= Self.wrapped().size())
		throw std::out_of_range("index out-of-range");

	return Self.wrapped().at(Item);
}

/// Provides a boilerplate implementation of __getitem__ for objects with find(std::string_t) that are wrapped by-reference 
template<typename self_t>
static boost::python::object wrapped_get_wrapped_item_by_key(self_t& Self, const std::string& Key)
{
	typename self_t::wrapped_type::const_iterator iterator = Self.wrapped().find(Key);
	if(iterator == Self.wrapped().end())
		throw std::runtime_error("unknown key: " + Key);

	return wrap(iterator->second);
}

/// Copies a Python list to an STL vector
template<typename target_t>
void copy(const boost::python::list& Source, target_t& Target)
{
	const uint16_t count = boost::python::len(Source);
	Target.resize(count);
	for(uint16_t i = 0; i != count; ++i)
		Target[i] = boost::python::extract<typename target_t::value_type>(Source[i]);
}

/// Creates a function with documentation
template<typename FunctionT>
inline boost::python::object make_function(FunctionT Function, const char* const Docstring = "")
{
	boost::python::object result = boost::python::make_function(Function);
	setattr(result, "__doc__", Docstring);
	return result;
}

/// Adds a method to an existing Python class instance
inline void add_method(const boost::python::object& Function, const std::string& Name, boost::python::object& Result)
{
	setattr(Result, Name, boost::python::import("types").attr("MethodType")(Function, Result));
}

} // namespace utility
