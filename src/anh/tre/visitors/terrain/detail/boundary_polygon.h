// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "boundary_layer.h"
#include <glm/glm.hpp>
#include <string>

namespace swganh
{
namespace tre
{

	/**
		Boundary polygon class. It is used to restrict the extent of any of its parent's
		layers. It is the only boundary that can have water.
	*/
	class BoundaryPolygon : public BoundaryLayer
	{
	public:

		virtual LayerType GetType() { return LAYER_TYPE_BOUNDARY_POLYGON; }

		virtual void Deserialize(swganh::ByteBuffer& buffer);
		virtual bool IsContained(float px, float pz);
		virtual float Process(float px, float pz);
		
		uint32_t use_water_height;
		float water_height;

	protected:
		std::vector<glm::vec2> verts;
		
		float water_shader_size;
		std::string water_shader;
		float max_x, max_z, min_x, min_z;
	};

}
}
