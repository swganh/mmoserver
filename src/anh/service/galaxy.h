// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>

namespace swganh {
namespace service {

class Galaxy {
public:
    enum StatusType {
        OFFLINE = 0,
        LOADING = 1,
        ONLINE = 2,
        LOCKED = 3
    };

    Galaxy();

    /*! This overloaded constructor is used when creating an instance from
    * the data store.
    *
    * @param id The id of the galaxy in the data store.
    * @param primary_id The id of the primary process for this galaxy.
    * @param name The name of the galaxy.
    * @param status The current status of the galaxy.
    * @param created_at The timestamp indicating the time the galaxy was first created.
    * @param updated_at The last time this galaxy was modified manually.
    */
    Galaxy(uint32_t id, 
            uint32_t primary_id, 
            const std::string& name, 
            const std::string& version,
            Galaxy::StatusType status, 
            const std::string& created_at, 
            const std::string& updated_at);

    /// Copy constructor.
    Galaxy(const Galaxy& other);

    /// Move constructor.
    Galaxy(Galaxy&& other);
    
    /// Swap the contents of two Galaxys.
    void swap(Galaxy& other);

    /// Universal assignment operator.
    Galaxy& operator=(Galaxy other);
        
    /*! Returns the id of the galaxy in the data store.
    *
    * @return Returns the id of the galaxy in the data store.
    */
    uint32_t id() const;
        
    /*! Returns the id of the primary process for this galaxy.
    *
    * @return Returns the id of the primary process for this galaxy.
    */
    uint32_t primary_id() const;
        
    /*! Returns the name of the galaxy.
    *
    * @return Returns the name of the galaxy.
    */
    const std::string& name() const;    
        
    /*! Returns the version of the galaxy.
    *
    * @return Returns the version of the galaxy.
    */
    const std::string& version() const;   
    
    /*! Returns the current status of the galaxy.
    *
    * @return Returns the current status of the galaxy.
    */
    Galaxy::StatusType status() const;  

    /*! sets the current status of the galaxy.
    *
    */
    void status(Galaxy::StatusType status);
    
    /*! Returns the timestamp indicating the time the galaxy was first created.
    *
    * @return Returns the timestamp indicating the time the galaxy was first created.
    */
    const std::string& created_at() const;    
    
    /*! Returns the last time this galaxy was modified manually.
    *
    * @return Returns the last time this galaxy was modified manually.
    */
    const std::string& updated_at() const;

    /*! Returns the last time this galaxy was updated in milleseconds.
    *
    * @return Returns the last time this galaxy was updated in milleseconds.
    */
    uint64_t GetGalaxyTimeInMilliseconds();

private:
    friend class ServiceDirectory;
    
    void primary_id(uint32_t primary_id);
    
    uint32_t id_;
    uint32_t primary_id_;
    std::string name_;
    std::string version_;
    Galaxy::StatusType status_;
    std::string created_at_;
    std::string updated_at_;
};

}  // namespace service
}  // namespace swganh
