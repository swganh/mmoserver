// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/byte_buffer.h"
#include "resource_manager.h"

#include "iff/iff.h"

#include "anh/logger.h"

using namespace swganh::tre;

ResourceManager::ResourceManager(std::shared_ptr<TreArchive> archive)
    : archive_(archive)
{
}

void ResourceManager::LoadResourceByName(const std::string& name, std::shared_ptr<VisitorInterface> type, bool is_cached)
{
    auto resource = archive_->GetResource(name);
    iff_file::loadIFF(resource, type);
    if(is_cached)
    {
        boost::lock_guard<boost::mutex> lock(resource_mutex_);
        loadedResources_.insert(ResourceCache::value_type(name, type));
    }
}
