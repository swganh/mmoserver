// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <map>
#include "anh/byte_buffer.h"
#include "layer.h"

namespace swganh
{
namespace tre
{
	class Fractal;

	class HeightLayer : public Layer
	{
	public:

		virtual LayerType GetType() { return LAYER_TYPE_HEIGHT; }
		
		virtual void GetBaseHeight(float x, float z, float transform_value, float& base_value, std::map<uint32_t, Fractal*>& fractals) = 0;
	};
	
}
}
