// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "filter_fractal.h"

#include "fractal.h"

using namespace swganh::tre;

void FractalFilter::Deserialize(swganh::ByteBuffer& buffer)
{
    fractal_id = buffer.read<uint32_t>();

    feather_type = buffer.read<uint32_t>();
    feather_amount = buffer.read<float>();

    min = buffer.read<float>();
    max = buffer.read<float>();

    step = buffer.read<float>();
}

float FractalFilter::Process(float x, float z, float transform_value, float& base_value, std::map<uint32_t,Fractal*>& fractals)
{
    //std::cout << "FFRA::PROCESS("<< x << "," << z <<")" << std::endl;

    Fractal* fractal = fractals.find(fractal_id)->second;

    float noise_result = fractal->getNoise(x, z) * step;
    float result = 0;

    if (noise_result > min && noise_result < max)
    {
        float feather_result = (float)((max - min) * feather_amount * 0.5);

        if (min + feather_result <= noise_result)
        {
            if (max - feather_result >= noise_result)
                result = 1.0;
            else
                result = (max - noise_result) / feather_result;
        }
        else
            result = (noise_result - min) / feather_result;
    }
    else
        result = 0;

    return result;
}