// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "logger.h"

#include <fstream>

#include <boost/log/core.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/support/date_time.hpp>



namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace logging = boost::log;

logger::logger_type& logger::get()
{
    return boost::log::sources::aux::logger_singleton<logger>::get();
}

void logger::setFile(std::string file)
{
	

	auto core = boost::log::core::get();
	auto backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();

    backend->add_stream(boost::make_shared<std::ofstream>(file ));
    backend->auto_flush(true);

    auto sink = boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);

	core->add_sink(sink);

	boost::shared_ptr<logging::attribute> pTimeStamp(new attrs::local_clock());

	logging::attribute lpTimeStamp	= attrs::local_clock();
	logging::attribute lpCounter	= attrs::counter<unsigned int>(1);
	logging::core::get()->add_global_attribute("LineID",  lpCounter	);
    logging::core::get()->add_global_attribute("TimeStamp",(lpTimeStamp));
	
	sink->set_formatter(
		expr::stream
		<< "(" << std::setw(8) << std::setfill('0') << expr::attr< unsigned int >("LineID") <<")"
		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
		<< ": <" << logging::trivial::severity
		<< "> " << expr::smessage

		);
	
	
}

logger::logger_type logger::construct_logger()
{
    auto core = boost::log::core::get();
    auto backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();

    backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::log::empty_deleter()));
    //backend->add_stream(boost::make_shared<std::ofstream>(logger::file ));

    backend->auto_flush(true);

    auto sink = boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);

    core->add_sink(sink);

	boost::shared_ptr<logging::attribute> pTimeStamp(new attrs::local_clock());

	logging::attribute lpTimeStamp	= attrs::local_clock();
	logging::attribute lpCounter	= attrs::counter<unsigned int>(1);
	logging::core::get()->add_global_attribute("LineID",  lpCounter	);
    logging::core::get()->add_global_attribute("TimeStamp",(lpTimeStamp));
	
	sink->set_formatter(
		expr::stream
		<< "(" << std::setw(5) << std::setfill('0') << expr::attr< unsigned int >("LineID") <<")"
		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%d %H:%M:%S")
		<< ": <" << logging::trivial::severity
		<< "> " << expr::smessage

		);
	

    return logger_type();
}

//}
