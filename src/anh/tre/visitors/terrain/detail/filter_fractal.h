// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "filter_layer.h"

namespace swganh
{
namespace tre
{

	/**
		Fractal Filter. Used to filter fractal data for the parent container.
	*/
	class FractalFilter : public FilterLayer
	{
	public:
		
		virtual void Deserialize(swganh::ByteBuffer& buffer);

		virtual float Process(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals);

	private:
		uint32_t fractal_id;
		float min, max, step;
	};

}
}
