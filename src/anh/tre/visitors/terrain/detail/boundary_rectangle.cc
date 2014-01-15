// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "boundary_rectangle.h"

using namespace swganh::tre;

void BoundaryRectangle::Deserialize(swganh::ByteBuffer& buffer)
{
    x1 = buffer.read<float>();
    z1 = buffer.read<float>();

    x2 = buffer.read<float>();
    z2 = buffer.read<float>();

    feather_type = buffer.read<uint32_t>();
    feather_amount = buffer.read<float>();

    //Make sure x1, z1 is lower left and x2,z2 is top right
    float temp;
    if (x1 > x2)
    {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }

    if (z1 > z2)
    {
        temp = z1;
        z1 = z2;
        z2 = temp;
    }
}

bool BoundaryRectangle::IsContained(float px, float pz)
{
    //std::cout << "BREC::ISCONTAINED("<< px << "," << pz <<")" << std::endl;
    if (x2 >= px && x1 <= px && z2 >= pz && z1 <= pz)
        return true;

    return false;
}

float BoundaryRectangle::Process(float px, float pz)
{
    //std::cout << "BREC::PROCESS("<< px << "," << pz <<")" << std::endl;
    float result;

    if (!IsContained(px, pz))
        result = 0.0f;
    else
    {
        float min_distx = px - x1;
        float max_distx = x2 - px;
        float min_distz = pz - z1;
        float max_distz = z2 - pz;
        float x_length = x2 - x1;
        float length = z2 - z1;

        if (x_length < length)
            length = x_length;

        float feather_length = (float)( feather_amount * length * 0.5);
        float feather_result = feather_length;

        float newX0 = x1 + feather_length;
        float newX1 = x2 - feather_length;
        float newZ0 = z1 + feather_length;
        float newZ1 = z2 - feather_length;

        if (px < newX1 || px > newX0 || pz < newZ1 || pz > newZ0)
            return 1.0f;

        if (min_distx < feather_length)
            feather_result = min_distx;
        if (max_distx < feather_result)
            feather_result = max_distx;
        if (min_distz < feather_result)
            feather_result = min_distz;
        if (max_distz < feather_result)
            feather_result = max_distz;

        result = feather_result / feather_length;
    }

    return result;
}