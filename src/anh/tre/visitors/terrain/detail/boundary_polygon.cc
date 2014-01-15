// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "boundary_polygon.h"

using namespace swganh::tre;

void BoundaryPolygon::Deserialize(swganh::ByteBuffer& buffer)
{
    uint32_t sizeTemp = buffer.read<uint32_t>();

    min_x = FLT_MAX;
    max_x = FLT_MIN;
    min_z = FLT_MAX;
    max_z = FLT_MIN;

    for(unsigned int j = 0; j < sizeTemp; j++)
    {
        float tempX = buffer.read<float>();
        float tempZ = buffer.read<float>();

        verts.push_back(glm::vec2(tempX, tempZ));

        // Track max and min values
        if (tempX > max_x)
            max_x = tempX;
        else if (tempX < min_x)
            min_x = tempX;

        if (tempZ > max_z)
            max_z = tempZ;
        else if (tempZ < min_z)
            min_z = tempZ;
    }

    this->feather_type = buffer.read<uint32_t>();
    this->feather_amount = buffer.read<float>();
    this->use_water_height = buffer.read<uint32_t>();
    this->water_height = buffer.read<float>();
    this->water_shader_size = buffer.read<float>();
    this->water_shader = buffer.read<std::string>(false, true);
}

bool BoundaryPolygon::IsContained(float x, float z)
{
    //std::cout << "BPOL::ISCONTAINED("<< x << "," << z <<")" << std::endl;

    int j;
    bool odd_nodes = false;
    float x1, x2;

    for ( unsigned int i = 0; i < verts.size(); ++i )
    {
        j = (i+1) % verts.size();

        if ( verts.at(i).x < verts.at(j).x)
        {
            x1 = verts.at(i).x;
            x2 = verts.at(j).x;
        }
        else
        {
            x1 = verts.at(j).x;
            x2 = verts.at(i).x;
        }

        /* First check if the ray is possible to cross the line */
        if ( x > x1 && x <= x2 && ( z < verts.at(i).y || z <= verts.at(j).y ) )
        {
            static const float eps = 0.000001f;

            /* Calculate the equation of the line */
            float dx = verts.at(j).x - verts.at(i).x;
            float dz = verts.at(j).y - verts.at(i).y;
            float k;

            if ( fabs(dx) < eps )
            {
                k = std::numeric_limits<float>::infinity();
            }
            else
            {
                k = dz/dx;
            }

            float m = verts.at(i).y - k * verts.at(i).x;

            /* Find if the ray crosses the line */
            float z2 = k * x + m;
            if ( z <= z2 )
            {
                odd_nodes=!odd_nodes;
            }
        }
    }


    return odd_nodes;
}

float BoundaryPolygon::Process(float px, float pz)
{
    //std::cout << "BPOL::PROCESS("<< px << "," << pz <<")" << std::endl;
    float result;
    glm::vec2 last = verts.at(verts.size() - 1);
    bool odd_nodes = false;

    if (px < min_x || px > max_x || pz < min_z || pz > max_z)
        return 0.0f;

    if (verts.size() <= 0)
        return 0.0f;

    for (unsigned int i = 0; i < verts.size(); i++)
    {
        glm::vec2 point = verts.at(i);

        if ((point.y <= pz && pz < last.y) || (last.y <= pz && pz < point.y))
            if ((pz - point.y) * (last.x - point.x) / (last.y - point.y) + point.x > (double)px)
                odd_nodes = !odd_nodes;

        last = point;
    }

    double feather2, new_feather;

    if (odd_nodes)
    {
        if (feather_amount == 0)
            return 1.0f;

        feather2 = pow(feather_amount,2);
        new_feather = feather2;
        double diffz, diffx, dist;

        for (unsigned int i = 0; i < verts.size(); ++i)
        {
            glm::vec2 point = verts.at(i);

            diffz = pz - point.y;
            diffx = px - point.x;
            dist = pow(diffz,2) + pow(diffx,2);

            if ( dist < feather2 )
                feather2 = dist;
        }

        double ltp_x, ltp_z, ptl_z, ptl_x, diff, new_dist, newX, newZ;

        last = verts.at(verts.size() - 1);

        for (unsigned int i = 0; i < verts.size(); ++i)
        {
            glm::vec2 point = verts.at(i);

            ltp_x = last.x - point.x;
            ltp_z = last.y - point.y;
            ptl_z = point.y - last.y;
            ptl_x = point.x - last.x;
            diff = ((px - last.x) * ptl_x + (pz - last.y) * ptl_z) / (ltp_z * ptl_z + ltp_x * ltp_x);
            if ( diff >= 0.0 )
            {
                if ( diff <= 1.0 )
                {
                    newX = px - (ptl_x * diff + last.x);
                    newZ = pz - (ptl_z * diff + last.y);
                    new_dist = newZ * newZ + newX * newX;
                    if ( new_dist < feather2 )
                    {
                        feather2 = new_dist;
                    }
                }
            }

            last = point;
        }

        if ( feather2 >= new_feather - 0.00009999999747378752 && feather2 <= new_feather + 0.00009999999747378752 )
            result = 1.0;
        else
            result = (float)(sqrt(feather2) / feather_amount);
    }
    else
    {
        result = 0.0;
    }

    return result;
}