// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "boundary_polyline.h"

using namespace swganh::tre;

void BoundaryPolyline::Deserialize(swganh::ByteBuffer& buffer)
{
    unsigned int sizeTemp = buffer.read<uint32_t>();

    // Initialize min and max values
    min_x = FLT_MAX;
    max_x = FLT_MIN;
    min_z = FLT_MAX;
    max_z = FLT_MIN;

    for(unsigned int j = 0; j < sizeTemp; j++)
    {
        float tempX = buffer.read<float>();
        float tempZ = buffer.read<float>();

        verts.push_back(glm::vec2(tempX, tempZ));

        // Track max values
        if (tempX > max_x)
            max_x = tempX;
        else if (tempX < min_x)
            min_x = tempX;

        if (tempZ > max_z)
            max_z = tempZ;
        else if (tempZ < min_z)
            min_z = tempZ;
    }

    feather_type = buffer.read<uint32_t>();
    feather_amount = buffer.read<float>();
    line_width = buffer.read<float>();

    // Account for line width
    min_x = min_x - line_width;
    max_x = max_x + line_width;
    min_z = min_z - line_width;
    max_z = max_z + line_width;
}

bool BoundaryPolyline::IsContained(float px, float pz)
{
    return false;
}

float BoundaryPolyline::Process(float px, float pz)
{
    //std::cout << "BPLN::PROCESS("<< px << "," << pz <<")" << std::endl;

    if (px < min_x)
        return 0.0;

    if (px > max_x || pz < (double)min_z )
        return 0.0;

    if ( pz > max_z )
        return 0.0;

    double line2 = line_width * line_width;

    double new_line = line2;
    double distz, distx, dist;

    double result = 0;

    for (unsigned int i = 0; i < verts.size(); ++i)
    {
        glm::vec2 point = verts.at(i);

        distz = pz - point.y;
        distx = px - point.x;
        dist = pow(distx,1) + pow(distz,2);
        if ( dist < line2 )
            line2 = dist;
    }

    double x_dist, diff, new_x, new_z, new_dist, z_dist;

    for (unsigned int i = 0; i < verts.size() - 1; ++i)
    {
        const glm::vec2 point = verts.at(i);
        const glm::vec2 point2 = verts.at(i + 1);

        x_dist = point2.x - point.x;
        z_dist = point2.y - point.y;
        diff = ((pz -  point.y) * z_dist + (px - point.x) * x_dist) / (z_dist * z_dist + x_dist * x_dist);

        if ( diff >= 0.0 )
        {
            if ( diff <= 1.0 )
            {
                new_x = px - (x_dist * diff + point.x);
                new_z = pz - (z_dist * diff +  point.y);
                new_dist = pow(new_z,2) + pow(new_x,2);

                if ( new_dist < line2 )
                {
                    line2 = new_dist;
                }
            }
        }

    }

    if ( line2 >= new_line )
        return 0.0;

    double new_feather = (1.0 - feather_amount) * line_width;

    if ( line2 >= pow(new_feather,2) )
        result = 1.0 - (sqrt(line2) - new_feather) / (line_width - new_feather);
    else
        result = 1.0;

    return (float) result;
}