// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <anh/byte_buffer.h>
#include <iostream>

namespace swganh
{
namespace tre
{

	enum LayerType
	{
		LAYER_TYPE_CONTAINER = 1,
		LAYER_TYPE_BOUNDARY,
		LAYER_TYPE_BOUNDARY_POLYGON, //Special due to water
		LAYER_TYPE_HEIGHT,
		LAYER_TYPE_FILTER
	};

	class Layer
	{
	public:
        
        virtual ~Layer() {}

		virtual void SetData(swganh::ByteBuffer& buffer)
		{
			enabled = buffer.read<uint32_t>() == 1;
		}

		virtual LayerType GetType() = 0;
		
		virtual void Deserialize(swganh::ByteBuffer& buffer) = 0;

		bool enabled;

	};

}
}
