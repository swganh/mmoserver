// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <string>
#include <istream>
#include <vector>

namespace swganh {
namespace tre {

    class ConfigReader
    {
    public:
        explicit ConfigReader(std::string filename);

        const std::vector<std::string>& GetTreFilenames();

    public:
        void ParseConfig();

        /**
         * Safe getline that handles endline's properly cross platform.
         * 
         * @param input
         * @param output
         * @return
         */
        std::istream& Getline(std::istream& input, std::string& output);

        std::vector<std::string> tre_filenames_;
        std::string config_filename_;
    };

}}  // namespace swganh::tre
