// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <stdio.h>
#include <vector>
#include <map>
#include <cstdint>
#include <memory>
#include <functional>

#include <anh/byte_buffer.h>
#include <anh/tre/visitors/visitor_interface.h>


namespace swganh
{
namespace tre
{
	class iff_file
	{
	public:

		static void loadIFF(swganh::ByteBuffer& input, std::shared_ptr<VisitorInterface> visitor=nullptr);

	private:
		static bool isNodeNameCharacter_(char c);
		static unsigned int getNameSize_(char namedata[], unsigned int size);
		static bool isFolderNode_(std::string& name);
		static std::string getIFFName_(swganh::ByteBuffer& input);
	};
}
}
