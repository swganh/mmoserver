// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "tre_archive.h"

#include "config_reader.h"
#include "anh/byte_buffer.h"

using namespace swganh::tre;

using std::for_each;
using std::move;
using std::runtime_error;
using std::string;
using std::vector;

TreArchive::TreArchive(vector<std::unique_ptr<TreReader>>&& readers)
    : readers_(move(readers))
{}

TreArchive::TreArchive(vector<string>&& resource_files)
{
    CreateReaders(resource_files);
}

TreArchive::~TreArchive()
{
    readers_.clear();
}

TreArchive::TreArchive(string config_file)
{
    ConfigReader config_reader(config_file);

    CreateReaders(config_reader.GetTreFilenames());
}

bool TreArchive::Open()
{
    return true;
}

uint32_t TreArchive::GetResourceSize(const string& resource_name) const
{
	swganh::tre::TreArchive::ReaderList::const_iterator it;
	it = readers_.begin();

	for (it = readers_.begin(); it != readers_.end(); it++)
    {
		if (it->get()->ContainsResource(resource_name))
        {
            return it->get()->GetResourceSize(resource_name);
        }
    }

    throw runtime_error("Requested unknown resource " + resource_name);
}

swganh::ByteBuffer TreArchive::GetResource(const string& resource_name)
{
    auto find_itr = lookup_.find(resource_name.c_str());
    if(find_itr != lookup_.end())
    {
        return readers_[find_itr->second]->GetResource(resource_name);
    }

    throw runtime_error("Requested unknown resource " + resource_name);
}

void TreArchive::GetResource(const std::string& resource_name, swganh::ByteBuffer& buffer)
{
    auto find_itr = lookup_.find(resource_name.c_str());
    if(find_itr != lookup_.end())
    {
        return readers_[find_itr->second]->GetResource(resource_name, buffer);
    }

    throw runtime_error("Requested unknown resource " + resource_name);
}

vector<string> TreArchive::GetTreFilenames() const
{
    vector<string> filenames;
	
	swganh::tre::TreArchive::ReaderList::const_iterator it;
	it = readers_.begin();

	for (it = readers_.begin(); it != readers_.end(); it++)
    {
		filenames.push_back(it->get()->GetFilename());
    }

    return filenames;
}

vector<string> TreArchive::GetAvailableResources() const
{
    return GetAvailableResources(std::function<void (int total, int completed)>());
}

std::vector<std::string> TreArchive::GetAvailableResources(std::function<void (int total, int completed)> progress_callback) const
{
    vector<string> resource_list;

    int total = readers_.size();
    int completed = 0;

	swganh::tre::TreArchive::ReaderList::const_iterator it;
	it = readers_.begin();

	for (it = readers_.begin(); it != readers_.end(); it++)
    {
        auto resources = it->get()->GetResourceNames();
        resource_list.insert(begin(resource_list), begin(resources), end(resources));

        ++completed;
        progress_callback(total, completed);
    }

    // sort and remove duplicates
    std::sort(std::begin(resource_list), std::end(resource_list));
    auto erase_iter = std::unique(resource_list.begin(), resource_list.end());
    resource_list.erase(erase_iter, resource_list.end());

    return resource_list;
}


void TreArchive::CreateReaders(const vector<string>& resource_files)
{
    uint32_t index = 0;
    std::for_each(resource_files.rbegin(), resource_files.rend(), [&, this] (std::string name)
    {
        readers_.push_back(std::unique_ptr<TreReader>(new TreReader(name, lookup_, index++)));
    });
}
