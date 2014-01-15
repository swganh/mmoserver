// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "layer.h"
#include <vector>

namespace swganh
{
namespace tre
{
	class BoundaryLayer;
	class FilterLayer;
	class HeightLayer;

	class ContainerLayer : public Layer
	{
	public:
        
        virtual ~ContainerLayer() {}

		virtual void Deserialize(swganh::ByteBuffer& buffer)
		{
			//Read ADTA
			invert_boundaries = buffer.read<uint32_t>() == 1;
			invert_filters = buffer.read<uint32_t>() == 1;
		}

		virtual LayerType GetType() { return LAYER_TYPE_CONTAINER; }

		void InsertLayer(Layer* layer)
		{
			switch(layer->GetType())
			{
			case LAYER_TYPE_CONTAINER:
				children.push_back((ContainerLayer*)layer);
				break;
			case LAYER_TYPE_BOUNDARY:
			case LAYER_TYPE_BOUNDARY_POLYGON:
				boundaries.push_back((BoundaryLayer*)layer);
				break;
			case LAYER_TYPE_HEIGHT:
				heights.push_back((HeightLayer*)layer);
				break;
			case LAYER_TYPE_FILTER:
				filters.push_back((FilterLayer*)layer);
			}
		}

		std::vector<ContainerLayer*> children;
		std::vector<BoundaryLayer*> boundaries;
		std::vector<FilterLayer*> filters;
		std::vector<HeightLayer*> heights;

		bool invert_boundaries, invert_filters;

	private:
	};

}
}
