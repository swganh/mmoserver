#include "BuildInfo.h"
#include "Utils/typedefs.h"

#include <fstream>

std::string GetBuildNumber()
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_num;

    if (build_num.length() == 0)
    {
        std::string build_string = GetBuildString();    
        build_num = ANH_VERSION_MAJOR"."ANH_VERSION_MINOR"."ANH_VERSION_PATCH"."+build_string.substr(0, build_string.find_first_of(" "));
    }

	return build_num;
}

std::string GetBuildTime()
{
    // @note: this string is special, it is set by subversion each time a
    // commit is made. Before returning the value to the caller, remove the extra
    // information that subversion adds.
    static std::string build_time;

    if (build_time.length() == 0)
    {
        std::string build_string = GetBuildString();    
        build_time = build_string.substr(build_string.find_first_of(" ")+1);
    }

	return build_time;
}

std::string GetBuildString()
{
    static std::string build_revision;
    static std::string build_timestamp;

    if (build_revision.length() == 0) 
    {
        std::ifstream version_file("VERSION");

        if (version_file.is_open())
        {
            std::getline(version_file, build_revision);
			std::getline(version_file, build_timestamp);
        }

        version_file.close();
    }

	return build_revision+" "+build_timestamp;
}

