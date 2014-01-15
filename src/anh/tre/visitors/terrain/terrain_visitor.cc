// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "terrain_visitor.h"

#include "detail/fractal.h"
#include "detail/layer.h"

#include "detail/container_layer.h"

#include "detail/layer_loader.h"

#include "anh/logger.h"
#include "detail/header.h"

using namespace swganh::tre;

TerrainVisitor::TerrainVisitor()
{
    header = new TrnHeader();
	working_fractal_ = nullptr;
}

TerrainVisitor::~TerrainVisitor()
{
    delete header;

/*    for(auto& fractal : fractals_)
    {
        delete fractal.second;
    }

    for(auto& layer : layers_)
    {
        delete layer;
    }
	*/
}

void TerrainVisitor::visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data)
{
    if(name == "MFAMDATA")
    {
        //Reading a new fractal
        working_fractal_ = new Fractal(data);
        this->fractals_.insert(FractalMap::value_type(working_fractal_->fractal_id, working_fractal_));
    }
    else if(name == "0001DATA" && working_fractal_ != nullptr)
    {
        //Filling in data for a fractal
        working_fractal_->Deserialize(data);
        working_fractal_ = nullptr;
    }
    else if(working_layer_ != nullptr && name == "0001DATA")
    {
        //Loading basic layer data for the layer
        working_layer_->SetData(data);
    }
    else if(working_layer_ != nullptr && (name == "DATAPARM" || name == "DATA" || name == "ADTA"))
    {
        //Loading in layer specific layer data
        working_layer_->Deserialize(data);
        working_layer_ = nullptr;
    }
    else if(name == "0014DATA")
    {
        //Loading header data
        header->Deserialize(data);
    }
}

void TerrainVisitor::visit_folder(uint32_t depth, std::string name, uint32_t size)
{
    //Get our layer stack back to where it should be
    while(layer_stack_.size() > 0 && layer_stack_.top().second > depth)
    {
        layer_stack_.pop();
    }

    //If we can create the layer, we have an implementation for it
    Layer* test_layer_ = LayerLoader(name);
    if(test_layer_ != nullptr)
    {
        //We created a layer, so set it as the working layer
        working_layer_ = test_layer_;

        //Hook the layer into either the top level layer list, or it's parent
        if(layer_stack_.size() == 0 && working_layer_->GetType() == LAYER_TYPE_CONTAINER)
        {
            layers_.push_back((ContainerLayer*)working_layer_);
        }
        else if(layer_stack_.top().first->GetType() == LAYER_TYPE_CONTAINER)
        {
            ((ContainerLayer*)layer_stack_.top().first)->InsertLayer(working_layer_);
        }

        //Add the layer to the stack
        auto entry = std::make_pair<Layer*, uint32_t>(std::forward<Layer*>(working_layer_), std::forward<uint32_t>(depth));
        layer_stack_.push(std::move(entry));
    }
}
