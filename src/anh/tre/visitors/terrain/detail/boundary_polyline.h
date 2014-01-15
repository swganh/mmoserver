// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "boundary_layer.h"
#include <glm/glm.hpp>

namespace swganh
{
namespace tre
{

	/**
		Boundary poly line class. It is used to restrict the extent of any of its parent's
		layers.
	*/
	class BoundaryPolyline : public BoundaryLayer
	{
	public:

		virtual void Deserialize(swganh::ByteBuffer& buffer);
		virtual bool IsContained(float px, float pz);
		virtual float Process(float px, float pz);
		
	protected:
		std::vector<glm::vec2> verts;
		float line_width;
		float max_x, max_z, min_x, min_z;
	};

}
}
