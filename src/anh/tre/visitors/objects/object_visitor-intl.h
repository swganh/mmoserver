// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once
#include<stdexcept>
namespace swganh
{
namespace tre
{
	template <class T> T ObjectVisitor::attribute(const std::string& key)
	{
		std::map<std::string, std::shared_ptr<boost::any>>::const_iterator it = attributes_.find(key);
		if(it != attributes_.cend())
		{
			return boost::any_cast<T>(*it->second);
		}
		throw std::runtime_error("Invalid type requested for attribute");
	}
}
}
