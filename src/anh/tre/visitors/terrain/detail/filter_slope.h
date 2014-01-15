// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "filter_layer.h"

namespace swganh
{
namespace tre
{

	/**
		Slope Filter. Used to filter fractal data for the parent container.

		This can be used to restrict slope to a particular range.
	*/
	class SlopeFilter : public FilterLayer
	{
	public:
		
		virtual void Deserialize(swganh::ByteBuffer& buffer);

		virtual float Process(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals);

		void SetMinAngle(float new_angle);
		void SetMaxAngle(float new_angle);

	private:
		float min_angle, min, max_angle, max, default_value;
	};

}
}
