// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "height_fractal.h"
#include "fractal.h"

using namespace swganh::tre;

void HeightFractal::Deserialize(swganh::ByteBuffer& buffer)
{
    this->fractal_id = buffer.read<uint32_t>();
    this->transform_type = buffer.read<uint32_t>();
    this->height_val = buffer.read<float>();
}

void HeightFractal::GetBaseHeight(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals)
{
    //std::cout << "AHFR::PROCESS("<< x << "," << z <<")" << std::endl;

    Fractal* fractal = fractals.find(fractal_id)->second;

    float noise_result = fractal->getNoise(x, z) * height_val;

    float result;

    switch (transform_type)
    {
    case 1:
        result = base_value + noise_result * transform_value;
        break;
    case 2:
        result = base_value - noise_result * transform_value;
        break;
    case 3:
        result = base_value + (noise_result * base_value - base_value) * transform_value;
        break;
    case 4:
        result = base_value;
        break;
    default:
        result = base_value + (noise_result - base_value) * transform_value;
        break;
    }

    base_value = result;
}