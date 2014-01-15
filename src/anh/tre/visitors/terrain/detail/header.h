// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <string>
#include <anh/byte_buffer.h>

namespace swganh
{
namespace tre
{

struct TrnHeader
{
	void Deserialize(swganh::ByteBuffer buffer)
	{
		filename = buffer.read<std::string>(false, true);
		map_width = buffer.read<float>();
		chunk_width = buffer.read<float>();
		tiles_per_chunk = buffer.read<uint32_t>();
		use_global_water_height = buffer.read<uint32_t>();
		global_water_height = buffer.read<float>();
		water_shader_size = buffer.read<float>();
		water_shader_name = buffer.read<std::string>(false, true);
		seconds_per_world_cycle = buffer.read<float>();
	}

	std::string filename;
	float map_width;
	float chunk_width;
	uint32_t   tiles_per_chunk;
	uint32_t   use_global_water_height;
	float global_water_height;
	float water_shader_size;
	std::string water_shader_name;
	float seconds_per_world_cycle;

	/* 
	Values not loaded because server doesn't need them
	
	float Collidable_MinDist;
	float Collidable_MaxDist;
	float Collidable_TileSize;
	float Collidable_TileBorder;
	float Collidable_Seed;

	float NonCollidable_MinDist;
	float NonCollidable_MaxDist;
	float NonCollidable_TileSize;
	float NonCollidable_TileBorder;
	float NonCollidable_Seed;

	float NearRadial_MinDist;
	float NearRadial_MaxDist;
	float NearRadial_TileSize;
	float NearRadial_TileBorder;
	float NearRadial_Seed;

	float FarRadial_MinDist;
	float FarRadial_MaxDist;
	float FarRadial_TileSize;
	float FarRadial_TileBorder;
	float FarRadial_Seed;
	*/
};

}
}
