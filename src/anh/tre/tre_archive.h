// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "tre_reader.h"

namespace swganh {
namespace tre {

    /**
     * TreArchive is a simple utility for accessing resource files from
     * a collection of .tre files.
     */
    class TreArchive
    {
    public:
        /**
         * Explicit constructor that accepts a collection of tre files.
         *
         * \param tre_files Collection of tre files.
         */
        explicit TreArchive(std::vector<std::unique_ptr<TreReader>>&& tre_files);

        /**
         * Explicit constructor that accepts a list of tre files to load.
         *
         * \param tre_filenames A collection of filenames to load.
         */
        explicit TreArchive(std::vector<std::string>&& tre_filenames);

        /**
         * Explicit constructor that accepts a configuration file listing the
         * the tre files to load.
         *
         * \param config_filename Path to configuration file (usually the live.cfg).
         */
        explicit TreArchive(std::string config_filename);

        ~TreArchive();

        bool Open();

        /**
         * Returns the size of the requested resource.
         *
         * Searches for the most recent version of the file and returns its size.
         *
         * \param resource_name The name of the resource.
         * \return The size of the requested resource.
         */
        uint32_t GetResourceSize(const std::string& resource_name) const;

        /**
         * Returns the requested resource in binary format.
         *
         * Searches for the most recent version of the requested file and 
         * returns it in binary format.
         *
         * \param resource_name The name of the resource.
         * \return The file in binary format (move constructable).
         */
        swganh::ByteBuffer GetResource(const std::string& resource_name);
        
        /**
         * Retrieves the requested resource in binary format.
         *
         * Searches for the most recent version of the requested file and 
         * returns it in binary format.
         *
         * \param resource_name The name of the resource.
         * \param buffer The buffer to store the resource.
         */
        void GetResource(const std::string& resource_name, swganh::ByteBuffer& buffer);
        
        /**
         * Returns a list of the available tre files.
         *
         * \return A list of available tre files.
         */
        std::vector<std::string> GetTreFilenames() const;
        
        /**
         * Returns a list of the of all available resources in the archive.
         *
         * \return A list of available resources.
         */
        std::vector<std::string> GetAvailableResources() const;
        
        /**
         * Returns a list of the of all available resources in the archive.
         *
         * Accepts a progress callback, useful for keeping UI threads informed of load progress.
         *
         * \param progress_callback A callback handler to be called with progress status
         * \return A list of available resources.
         */
        std::vector<std::string> GetAvailableResources(std::function<void (int total, int completed)> progress_callback) const;

    private:

        void CreateReaders(const std::vector<std::string>& resource_files);

		
        typedef std::vector<std::unique_ptr<TreReader>> ReaderList;
        ReaderList readers_;
		ResourceLookup lookup_;
    };
}}  // namespace swganh::tre
