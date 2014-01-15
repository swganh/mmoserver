// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>

#include "galaxy.h"

namespace swganh {
namespace service {

class ServiceDescription {    
public:
    ServiceDescription();

    /*! This overloaded constructor is used when describing a service to register.
    *
    * @param type The type of the service.
    * @param version The version of the service.
    * @param address The address to communicate with the service.
    * @param tcp_port The tcp port to connect to the service on, default to 0 if not used.
    * @param udp_port The udp port to connect to the service on, default to 0 if not used.
    * @param status The current status of the service.
    * @param last_pulse The last time the service synced with the data store.
    */
    ServiceDescription(const std::string& name,
            const std::string& type,
            const std::string& version,
            const std::string& address,
            uint16_t tcp_port,
            uint16_t udp_port,
            uint16_t ping_port); 

    /*! This overloaded constructor is used when creating an instance from
    * the data store.
    *
    * @param id The id of the service in the data store.
    * @param galaxy_id The id of the galaxy this service belongs to.
    * @param type The type of the service.
    * @param version The version of the service.
    * @param address The address to communicate with the service.
    * @param tcp_port The tcp port to connect to the service on, default to 0 if not used.
    * @param udp_port The udp port to connect to the service on, default to 0 if not used.
    * @param status The current status of the service.
    * @param last_pulse The last time the service synced with the data store.
    */
    ServiceDescription(uint32_t id,
            uint32_t galaxy_id,
            const std::string& name,
            const std::string& type,
            const std::string& version,
            const std::string& address,
            uint16_t tcp_port,
            uint16_t udp_port,
            uint16_t ping_port);    

    /// Destructor
    ~ServiceDescription();    

    /// Copy constructor.
    ServiceDescription(const ServiceDescription& other);

    /// Move constructor.
    ServiceDescription(ServiceDescription&& other);
    
    /// Swap the contents of two HashStrings.
    void swap(ServiceDescription& other);

    /// Universal assignment operator.
    ServiceDescription& operator=(ServiceDescription other);

    /// Equals Operator.
    bool operator==(ServiceDescription other)
    {
        return this->id_ == other.id_;
    }
    /// Not Equals Operator.
    bool operator!=(ServiceDescription other)
    {
        return this->id_ != other.id_;
    }
        
    /*! Returns the id of the service in the data store.
    *
    * @return Returns the id of the service in the data store.
    */
    uint32_t id() const;
    void id(uint32_t id);

    /*! Returns The id of the galaxy this service belongs to.
    *
    * @return Returns The id of the galaxy this service belongs to.
    */
    uint32_t galaxy_id() const;

    /*! Returns The name of the service.
    *
    * @return Returns The name of the service.
    */
    const std::string& name() const;    

    /*! Returns The type of the service.
    *
    * @return Returns The type of the service.
    */
    const std::string& type() const;    
    
    /*! Returns the version of the service.
    *
    * @return Returns the version of the service.
    */
    const std::string& version() const;    
    
    /*! Returns the address to communicate with the service.
    *
    * @return Returns the address to communicate with the service.
    */
    const std::string& address() const;    
    
    /*! Returns the tcp port to connect to the service on, default to 0 if not used.
    *
    * @return Returns the tcp port to connect to the service on, default to 0 if not used.
    */
    uint16_t tcp_port() const;    
    
    /*! Returns the udp port to connect to the service on, default to 0 if not used.
    *
    * @return Returns the udp port to connect to the service on, default to 0 if not used.
    */
    uint16_t udp_port() const;    
    
    /*! Returns the ping port, default to 0 if not used.
    *
    * @return Returns the udp port, default to 0 if not used.
    */
    uint16_t ping_port() const;    
    
    /*! Returns the current status of the service.
    *
    * @return Returns -1 if the service is not operational otherwise it returns
    *   the number of connected clients.
    */
    int32_t status() const;    
    void status(int32_t new_status);
    
    /*! Returns the last time the service synced with the data store.
    *
    * @return Returns the last time the service synced with the data store.
    */
    const std::string& last_pulse() const;
    void last_pulse(std::string last_pulse);
    
private:
    friend class ServiceDirectory;
    
    uint32_t id_;
    uint32_t galaxy_id_;
    std::string name_;
    std::string type_;
    std::string version_;
    std::string address_;
    uint16_t tcp_port_;
    uint16_t udp_port_;
    uint16_t ping_port_;
    int32_t status_;
    std::string last_pulse_;
};

}  // namespace service
}  // namespace swganh

