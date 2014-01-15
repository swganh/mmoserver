// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/service/galaxy.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace swganh::service;

Galaxy::Galaxy()
    : id_(0) {}

Galaxy::Galaxy(
    uint32_t id,
    uint32_t primary_id,
    const std::string& name,
    const std::string& version,
    Galaxy::StatusType status,
    const std::string& created_at,
    const std::string& updated_at)
    : id_(id)
    , primary_id_(primary_id)
    , name_(name)
    , status_(status)
    , created_at_(created_at)
    , updated_at_(updated_at)
{}

Galaxy::Galaxy(const Galaxy& other)
{
    id_ = other.id_;
    primary_id_ = other.primary_id_;
    name_ = other.name_;
    version_ = other.version_;
    status_ = other.status_;
    created_at_ = other.created_at_;
    updated_at_ = other.updated_at_;
}

Galaxy::Galaxy(Galaxy&& other)
{
    id_ = other.id_;
    primary_id_ = other.primary_id_;
    name_ = std::move(other.name_);
    version_ = std::move(other.version_);
    status_ = other.status_;
    created_at_ = std::move(other.created_at_);
    updated_at_ = std::move(other.updated_at_);
}

void Galaxy::swap(Galaxy& other)
{
    std::swap(other.id_, id_);
    std::swap(other.primary_id_, primary_id_);
    std::swap(other.name_, name_);
    std::swap(other.version_, version_);
    std::swap(other.status_, status_);
    std::swap(other.created_at_, created_at_);
    std::swap(other.updated_at_, updated_at_);
}

Galaxy& Galaxy::operator=(Galaxy other)
{
    other.swap(*this);
    return *this;
}

uint32_t Galaxy::id() const
{
    return id_;
}

uint32_t Galaxy::primary_id() const
{
    return primary_id_;
}

void Galaxy::primary_id(uint32_t primary_id)
{
    primary_id_ = primary_id;
}

const std::string& Galaxy::name() const
{
    return name_;
}

const std::string& Galaxy::version() const
{
    return version_;
}

Galaxy::StatusType Galaxy::status() const
{
    return status_;
}

void Galaxy::status(Galaxy::StatusType status)
{
    status_ = status;
}

const std::string& Galaxy::created_at() const
{
    return created_at_;
}

const std::string& Galaxy::updated_at() const
{
    return updated_at_;
}

uint64_t Galaxy::GetGalaxyTimeInMilliseconds()
{
    boost::posix_time::ptime start_time(boost::posix_time::time_from_string(created_at()));
    boost::posix_time::ptime current_time(boost::posix_time::second_clock::local_time());
    return (current_time - start_time).total_milliseconds();
}
