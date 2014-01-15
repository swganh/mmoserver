// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "height_constant.h"

using namespace swganh::tre;

void HeightConstant::Deserialize(swganh::ByteBuffer& buffer)
{
    this->transform_type = buffer.read<uint32_t>();
    this->height_val = buffer.read<float>();
}

void HeightConstant::GetBaseHeight(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals)
{
    //std::cout << "AHCN::PROCESS("<< x << "," << z <<")" << std::endl;

    if (transform_value == 0)
        return;

    float result;

    switch (transform_type)
    {
    case 1:
        result = transform_value * height_val + base_value;
        break;
    case 2:
        result = base_value - transform_value * height_val;
        break;
    case 3:
        result = base_value + (base_value * height_val - base_value) * transform_value;
        break;
    case 4:
        result = 0;
        break;
    default:
        result = (float)((1.0 - transform_value) * base_value + transform_value * height_val);
    }

    base_value = result;
}