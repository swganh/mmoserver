// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "layer_loader.h"

#include <anh/byte_buffer.h>

#include "container_layer.h"
#include "boundary_circle.h"
#include "boundary_polygon.h"
#include "boundary_polyline.h"
#include "boundary_rectangle.h"
#include "filter_fractal.h"
#include "filter_height.h"
#include "filter_slope.h"
#include "height_constant.h"
#include "height_fractal.h"

using namespace swganh::tre;

/**
	This class is used to load a particular implementation based on it's node name.
	We should consider using a map here instead as this is slightly inefficient.
*/
Layer* LayerLoader(std::string node_name)
{
    if(node_name == "LAYRFORM")
        return new ContainerLayer();
    else if(node_name == "BCIRFORM")
        return new BoundaryCircle();
    else if(node_name == "BPOLFORM")
        return new BoundaryPolygon();
    else if(node_name == "BPLNFORM")
        return new BoundaryPolyline();
    else if(node_name == "BRECFORM")
        return new BoundaryRectangle();
    else if(node_name == "FFRAFORM")
        return new FractalFilter();
    else if(node_name == "FHGTFORM")
        return new HeightFilter();
    else if(node_name == "FSLPFORM")
        return new SlopeFilter();
    else if(node_name == "AHCNFORM")
        return new HeightConstant();
    else if(node_name == "AHFRFORM")
        return new HeightFractal();
    else
        return nullptr;
}