#include "logger.h"

#include <fstream>

#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/core/null_deleter.hpp>

logger::logger_type& logger::get()
{
	return boost::log::sources::aux::logger_singleton<logger>::get();
}

logger::logger_type logger::construct_logger()
{
	auto core = boost::log::core::get();
	auto backend = boost::make_shared<boost::log::sinks::text_ostream_backend>();

	//backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::log::empty_deleter()));
  backend->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

	backend->add_stream(boost::make_shared<std::ofstream>("logs/swganh.log"));

	backend->auto_flush(true);

	auto sink = boost::make_shared<boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>>(backend);

	core->add_sink(sink);

	return logger_type();
}
