// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "filter_slope.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MODIFIER 0.005555555690079927

using namespace swganh::tre;

void SlopeFilter::Deserialize(swganh::ByteBuffer& buffer)
{
    default_value = 1.5707964f;

    min_angle = buffer.read<float>();
    max_angle = buffer.read<float>();

    SetMinAngle((float)(M_PI * min_angle * MODIFIER));
    SetMaxAngle((float)(M_PI * max_angle * MODIFIER));

    feather_type = buffer.read<uint32_t>();
    feather_amount = buffer.read<float>();

    if (feather_amount <= 0)
        feather_amount = 0.0f;
    else if (feather_amount >= 1)
        feather_amount = 1.0f;
}

void SlopeFilter::SetMinAngle(float new_angle)
{
    if (new_angle >= 0)
    {
        if (new_angle <= default_value)
        {
            min_angle = new_angle;
            max = std::sin(default_value - new_angle);
        }
        else
        {
            min_angle = default_value;
            max = 0;
        }
    }
    else
    {
        min_angle = 0;
        max = std::sin(default_value);
    }
}

void SlopeFilter::SetMaxAngle(float new_angle)
{
    if (new_angle >= 0)
    {
        if (new_angle <= default_value)
        {
            max_angle = new_angle;
            min = std::sin(default_value - new_angle);
        }
        else
        {
            max_angle = default_value;
            min = 0;
        }
    }
    else
    {
        max_angle = 0;
        min = std::sin(default_value);
    }
}

float SlopeFilter::Process(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals)
{
    //std::cout << "FSLP::PROCESS("<< x << "," << z <<")" << std::endl;
    float result;

    if (base_value > min && base_value < max)
    {
        float feather_result = (float)( max - min * feather_amount * 0.5);

        if (min + feather_result <= base_value)
        {
            if (max - feather_result >= base_value)
            {
                result = 1.0;
            }
            else
            {
                result = (max - base_value) / feather_result;
            }
        }
        else
            result = (base_value - min) / feather_result;
    }
    else
        result = 0;

    return result;
}