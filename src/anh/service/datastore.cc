// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "anh/service/datastore.h"

#include "DatabaseManager/DatabaseManager.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include <memory>
#include <sstream>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

#include <cppconn/exception.h>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

#include "anh/service/service_description.h"
#include "anh/app/kernel_interface.h"
#include "anh/app/swganh_kernel.h"
#include "anh/logger.h"

using namespace swganh::service;
using namespace boost::posix_time;
using namespace std;

Datastore::Datastore(swganh::app::SwganhKernel*  kernel)
    : d_kernel_(kernel)
{}

Datastore::~Datastore() {}

std::shared_ptr<Galaxy> Datastore::findGalaxyByName(const std::string& name) const
{
    shared_ptr<Galaxy> galaxy = nullptr;
	/*
    try
	{
		std::stringstream s;
		s <<       "SELECT * FROM swganh.galaxy WHERE name = \'" << d_kernel_->GetDatabase()->escapeString(name) <<  "\' LIMIT 1;";
		
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

		if (!result_set->next())		{
			d_kernel_->GetDatabase()->destroyResult(result);
			return nullptr;
		}
			
		galaxy = make_shared<Galaxy>(
			result_set->getUInt("galaxy_id"),
			result_set->getUInt("galaxy_id"),
			result_set->getString("name"),
			"", //version
			static_cast<Galaxy::StatusType>(result_set->getInt("status")),
			result_set->getString("last_update"),
			result_set->getString("last_update"));
						
		d_kernel_->GetDatabase()->destroyResult(result);
       
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
    return galaxy;
	
}

std::shared_ptr<Galaxy> Datastore::createGalaxy(
    const std::string& name,
    const std::string& version) const
{
	
    shared_ptr<Galaxy> galaxy = nullptr;

    try
    {
		/*
		std::stringstream s;
		s <<       "CALL sp_CreateGalaxy(" << name << "," << version << "," << 0 << ");";
        
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next())
        {
            d_kernel_->GetDatabase()->destroyResult(result);
            return nullptr;
        }

        galaxy = make_shared<Galaxy>(
                     result_set->getUInt("id"),
                     result_set->getUInt("primary_id"),
                     result_set->getString("name"),
                     result_set->getString("version"),
                     static_cast<Galaxy::StatusType>(result_set->getInt("status")),
                     result_set->getString("created_at"),
                     result_set->getString("updated_at"));

        d_kernel_->GetDatabase()->destroyResult(result);
		*/
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }

    return galaxy;
	
}
void Datastore::saveGalaxyStatus(int32_t galaxy_id, int32_t status) const
{
    try
    {
		LOG (info) << "Datastore::saveGalaxyStatus";
		std::stringstream s;
		s <<       "UPDATE swganh.galaxy SET status = " << status << " WHERE galaxy_id = " << galaxy_id << ";";

		//d_kernel_->GetDatabase()->executeAsyncProcedure(s.str());
		        
		
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
}

bool Datastore::createService(const Galaxy& galaxy, ServiceDescription& description) const
{	
	
    shared_ptr<ServiceDescription> service = nullptr;

    try
    {
		std::stringstream s;
		s	<< "CALL sp_CreateService(" << d_kernel_->GetAppConfig().galaxy_id << ",\'" << description.name() << "\',\'" 
			<< description.type() << "\',\'"
			<< description.version() << "\',\'"
			<< description.address() << "\',"
			<< static_cast<uint32_t>(description.tcp_port()) << ","
			<< static_cast<uint32_t>(description.udp_port()) << ","
			<< static_cast<uint32_t>(description.ping_port()) << ","
			<< static_cast<int32_t>(-1) << ");";
        
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeProcedure(s.str().c_str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next())
        {
            d_kernel_->GetDatabase()->destroyResult(result);
            return false;
        }
        

        description.id(result_set->getUInt("id"));
        description.status(result_set->getInt("status"));
        description.last_pulse(result_set->getString("last_pulse_timestamp"));

        d_kernel_->GetDatabase()->destroyResult(result);
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
        return false;
    }

    return true;

}

void Datastore::saveService(const ServiceDescription& service) const
{/*
    try
    {
		std::stringstream s;
		s	<< "CALL sp_SaveService(" << service.address() << "," << service.tcp_port() << "," 
			<< service.udp_port() << ","
			<< service.ping_port() << ","
			<< service.status() << ","
			<< prepareTimestampForStorage(service.last_pulse()) << ","
			<< service.id() << ");";
        
		d_kernel_->GetDatabase()->executeAsyncProcedure(s.str());
			
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
}

std::shared_ptr<Galaxy> Datastore::findGalaxyById(uint32_t id) const
{
    std::shared_ptr<Galaxy> galaxy = nullptr;
	/*
    try
    {
		std::stringstream s;
		s	<< "CALL sp_FindGalaxyById(" << id  << ");";
        
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next())
        {
            d_kernel_->GetDatabase()->destroyResult(result);
            return false;
        }
        
        galaxy = make_shared<Galaxy>(
                     result_set->getUInt("id"),
                     result_set->getUInt("primary_id"),
                     result_set->getString("name"),
                     result_set->getString("version"),
                     static_cast<Galaxy::StatusType>(result_set->getInt("status")),
                     result_set->getString("created_at"),
                     result_set->getString("updated_at"));

		d_kernel_->GetDatabase()->destroyResult(result);
        
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
    return galaxy;
	
}

std::shared_ptr<ServiceDescription> Datastore::findServiceById(uint32_t id) const
{
	
    shared_ptr<ServiceDescription> service = nullptr;
	/*
    try
    {
        std::stringstream s;
		s	<< "CALL sp_FindServiceById(" << id  << ");";
        
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        if (!result_set->next())
        {
            d_kernel_->GetDatabase()->destroyResult(result);
            return false;
        }

        service = make_shared<ServiceDescription>(
				result_set->getUInt("id"),
				result_set->getUInt("galaxy_id"),
				result_set->getString("name"),
				result_set->getString("type"),
				result_set->getString("version"),
				result_set->getString("address_string"),
				result_set->getUInt("tcp_port"),
				result_set->getUInt("udp_port"),
				result_set->getUInt("ping_port")
                      );

        service->status(result_set->getInt("status"));
        service->last_pulse(result_set->getString("last_pulse_timestamp"));
        
		d_kernel_->GetDatabase()->destroyResult(result);
        
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
    return service;
	
}

bool Datastore::deleteServiceById(uint32_t id) const
{
	/*
    bool deleted = false;

    try
    {
		std::stringstream s;
		s	<< "CALL sp_DeleteService(" << id  << ");";
        
		swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

		deleted = result_set->getUInt(1);

    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }

    return deleted;
	*/
	return false;
}

list<Galaxy> Datastore::getGalaxyList() const
{
	
    std::list<Galaxy> galaxy_list;
	/*
    try
    {
        std::stringstream s;
		s	<< "CALL sp_GetGalaxyList();";

        swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        // Loop through the results and create a map entry for each.
        while (result_set->next())
        {
            galaxy_list.push_back(Galaxy(
                                      result_set->getUInt("id"),
                                      result_set->getUInt("primary_id"),
                                      result_set->getString("name"),
                                      result_set->getString("version"),
                                      static_cast<Galaxy::StatusType>(result_set->getInt("status")),
                                      result_set->getString("created_at"),
                                      result_set->getString("updated_at")));
        }
        d_kernel_->GetDatabase()->destroyResult(result);
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
    return galaxy_list;
	
}

list<ServiceDescription> Datastore::getServiceList(uint32_t galaxy_id) const
{
	
    std::list<ServiceDescription> service_list;
	/*
    try
    {

		std::stringstream s;
		s	<< "CALL sp_GetServiceList(" << galaxy_id << ");";

        swganh::database::DatabaseResult* result = d_kernel_->GetDatabase()->executeSql(s.str());
		
		std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        // Loop through the results and create a map entry for each.
        while (result_set->next())
        {
            ServiceDescription service(
                result_set->getUInt("id"),
                result_set->getUInt("galaxy_id"),
                result_set->getString("name"),
                result_set->getString("type"),
                result_set->getString("version"),
                result_set->getString("address"),
                result_set->getUInt("tcp_port"),
                result_set->getUInt("udp_port"),
                result_set->getUInt("ping_port"));
            service.status(result_set->getInt("status"));
            service.last_pulse(result_set->getString("last_pulse_timestamp"));

            service_list.push_back(move(service));
        }

		d_kernel_->GetDatabase()->destroyResult(result);
        
    }
    catch(sql::SQLException &e)
    {
        LOG(error) << "SQLException at " << __FILE__ << " (" << __LINE__ << ": " << __FUNCTION__ << ")";
        LOG(error) << "MySQL Error: (" << e.getErrorCode() << ": " << e.getSQLState() << ") " << e.what();
    }
	*/
    return service_list;
	
}

std::string Datastore::prepareTimestampForStorage(const std::string& timestamp) const
{
    std::stringstream ss;

    ss.imbue(std::locale(ss.getloc(), new boost::posix_time::time_facet("%Y%m%d%H%M%S%F")));

    static boost::mutex mutex;

    boost::lock_guard<boost::mutex> lk(mutex);

    ss << boost::posix_time::time_from_string(timestamp);

    return ss.str();
}
