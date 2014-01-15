// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <string>


namespace swganh
{
namespace tre {
	class Layer;
}
}

swganh::tre::Layer* LayerLoader(std::string node_name);
