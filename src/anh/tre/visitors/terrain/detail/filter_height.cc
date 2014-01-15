// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "filter_height.h"

using namespace swganh::tre;

void HeightFilter::Deserialize(swganh::ByteBuffer& buffer)
{
    minHeight = buffer.read<float>();
    maxHeight = buffer.read<float>();

    feather_type = buffer.read<uint32_t>();
    feather_amount = buffer.read<float>();
}

float HeightFilter::Process(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals)
{
    //std::cout << "FHGT::PROCESS("<< x << "," << z <<")" << std::endl;
    float result;

    if ((base_value > minHeight) && (base_value < maxHeight))
    {
        float feather_result = (float)((maxHeight - minHeight) * feather_amount * 0.5);

        if (minHeight + feather_result <= base_value)
        {
            if (maxHeight - feather_result >= base_value)
            {
                result = 1.0;
            }
            else
                result = (maxHeight - base_value) / feather_result;

        }
        else
        {
            result = (base_value - minHeight) / feather_result;
        }
    }
    else
        result = 0;

    return result;
}