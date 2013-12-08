// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#ifndef ANH_LOGGER_H
#define ANH_LOGGER_H

#include <iosfwd>
#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>

using namespace boost::log;

enum severity_level
{
	event,
	info,
	warning,
	error,
	fatal
};

/*
template<typename CharT, typename TraitsT>
inline std::basic_ostream<CharT, TraitsT>& operator<<(
	std::basic_ostream<CharT, TraitsT>& stream, ::severity_level level)
{
	static const char* const str[] =
	{
		"event",
		"info",
		"warning",
		"error",
		"fatal"
	};
	if (static_cast<std::size_t>(level) < (sizeof(str) / sizeof(*str))) {
		stream << str[level];
	} else {
		stream << static_cast<int>(level);
	}

	return stream;
}
*/
struct logger
{
	typedef boost::log::sources::severity_logger_mt< severity_level> logger_type;

	static logger_type& get();
	static logger_type construct_logger();

    enum registration_line_t { registration_line = __LINE__ };
    static const char* registration_file() { return __FILE__; }
};

#define LOG(level)\
	BOOST_LOG_SEV(::logger::get(),\
		(::level))

#ifdef _DEBUG
#define DLOG(level)	LOG(level)
#else
#define DLOG(level) if (true); else LOG(level)
#endif

#endif

