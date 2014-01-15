// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/service/service_description.h"

using namespace swganh::service;

ServiceDescription::ServiceDescription()
    : id_(0) {}

ServiceDescription::ServiceDescription(const std::string& name,
                                       const std::string& type,
                                       const std::string& version,
                                       const std::string& address,
                                       uint16_t tcp_port,
                                       uint16_t udp_port,
                                       uint16_t ping_port)
    : id_(0)
    , name_(name)
    , type_(type)
    , version_(version)
    , address_(address)
    , tcp_port_(tcp_port)
    , udp_port_(udp_port)
    , ping_port_(ping_port)
    , status_(-1)
    , last_pulse_("1970-01-01 00:00:01")
{}

ServiceDescription::ServiceDescription(uint32_t id,
                                       uint32_t galaxy_id,
                                       const std::string& name,
                                       const std::string& type,
                                       const std::string& version,
                                       const std::string& address,
                                       uint16_t tcp_port,
                                       uint16_t udp_port,
                                       uint16_t ping_port)
    : id_(id)
    , galaxy_id_(galaxy_id)
    , name_(name)
    , type_(type)
    , version_(version)
    , address_(address)
    , tcp_port_(tcp_port)
    , udp_port_(udp_port)
    , ping_port_(ping_port)
    , status_(-1)
    , last_pulse_("1970-01-01 00:00:01")
{}

ServiceDescription::~ServiceDescription() {}

ServiceDescription::ServiceDescription(const ServiceDescription& other)
{
    id_ = other.id_;
    galaxy_id_ = other.galaxy_id_;
    name_ = other.name_;
    type_ = other.type_;
    version_ = other.version_;
    address_ = other.address_;
    tcp_port_ = other.tcp_port_;
    udp_port_ = other.udp_port_;
    ping_port_ = other.ping_port_;
    status_ = other.status_;
    last_pulse_ = other.last_pulse_;
}

ServiceDescription::ServiceDescription(ServiceDescription&& other)
{
    id_ = other.id_;
    galaxy_id_ = other.galaxy_id_;
    name_ = std::move(other.name_);
    type_ = std::move(other.type_);
    version_ = std::move(other.version_);
    address_ = std::move(other.address_);
    tcp_port_ = other.tcp_port_;
    udp_port_ = other.udp_port_;
    ping_port_ = other.ping_port_;
    status_ = other.status_;
    last_pulse_ = std::move(other.last_pulse_);
}

void ServiceDescription::swap(ServiceDescription& other)
{
    std::swap(other.id_, id_);
    std::swap(other.galaxy_id_, galaxy_id_);
    std::swap(other.name_, name_);
    std::swap(other.type_, type_);
    std::swap(other.version_, version_);
    std::swap(other.address_, address_);
    std::swap(other.tcp_port_, tcp_port_);
    std::swap(other.udp_port_, udp_port_);
    std::swap(other.ping_port_, ping_port_);
    std::swap(other.status_, status_);
    std::swap(other.last_pulse_, last_pulse_);
}

ServiceDescription& ServiceDescription::operator=(ServiceDescription other)
{
    other.swap(*this);
    return *this;
}

uint32_t ServiceDescription::id() const
{
    return id_;
}

void ServiceDescription::id(uint32_t id)
{
    id_ = id;
}

uint32_t ServiceDescription::galaxy_id() const
{
    return galaxy_id_;
}

const std::string& ServiceDescription::name() const
{
    return name_;
}

const std::string& ServiceDescription::type() const
{
    return type_;
}

const std::string& ServiceDescription::version() const
{
    return version_;
}

const std::string& ServiceDescription::address() const
{
    return address_;
}

uint16_t ServiceDescription::tcp_port() const
{
    return tcp_port_;
}

uint16_t ServiceDescription::udp_port() const
{
    return udp_port_;
}

uint16_t ServiceDescription::ping_port() const
{
    return ping_port_;
}

int32_t ServiceDescription::status() const
{
    return status_;
}

void ServiceDescription::status(int32_t new_status)
{
    status_ = new_status;
}

const std::string& ServiceDescription::last_pulse() const
{
    return last_pulse_;
}

void ServiceDescription::last_pulse(std::string last_pulse)
{
    last_pulse_ = std::move(last_pulse);
}
