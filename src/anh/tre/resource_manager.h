// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <map>
#include <boost/thread/lock_guard.hpp>
#include "anh/tre/tre_archive.h"
#include "visitors/visitor_interface.h"

namespace swganh {
namespace tre {
	
	typedef std::map<std::string, std::shared_ptr<swganh::tre::VisitorInterface>> ResourceCache;

	class ResourceManager
	{
	public:
		ResourceManager(std::shared_ptr<swganh::tre::TreArchive> archive);
		~ResourceManager() {}
		
		void LoadResourceByName(const std::string& name, std::shared_ptr<VisitorInterface> visitor, bool is_cached=true);

		template<class ValueType>
		std::shared_ptr<ValueType> GetResourceByName(const std::string& name, bool is_cached=true)
		{
			{
				boost::lock_guard<boost::mutex> lock(resource_mutex_);
				auto itr = loadedResources_.find(name);
				if(itr != loadedResources_.end())
				{
					return std::static_pointer_cast<ValueType>(itr->second);
				}
			}

			if(name.size() != 0)
			{
				std::shared_ptr<ValueType> visitor = std::make_shared<ValueType>();
				LoadResourceByName(name, visitor, is_cached);
				return visitor;
			}
			else
			{
				return nullptr;
			}
		}

	private:
		boost::mutex resource_mutex_;
		ResourceCache loadedResources_;
		std::shared_ptr<swganh::tre::TreArchive> archive_;
	};

}
}
