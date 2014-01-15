// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "object_visitor.h"

#include "../../iff/iff.h"

#include <anh/tre/resource_manager.h>

using namespace swganh::tre;
using namespace std;
using namespace std::placeholders;

AttributeHandlerIndex ObjectVisitor::attributeHandler_;

ObjectVisitor::ObjectVisitor()
    : VisitorInterface(), has_aggregate_(false), loaded_reference_(false)
{
    if(attributeHandler_.empty())
    {
        attributeHandler_.insert(make_pair(string("animationMapFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("appearanceFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("arrangementDescriptorFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("clearFloraRadius"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("clientDataFile"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("cockpitFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionActionBlockFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionActionFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionActionPassFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionHeight"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionLength"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionMaterialBlockFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionMaterialFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionMaterialPassFlags"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("collisionRadius"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("containerType"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("containerVolumeLimit"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("detailedDescription"), bind(&ObjectVisitor::_handleClientString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("gameObjectType"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("hasWings"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("interiorLayoutFileName"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("locationReservationRadius"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("lookAtText"), bind(&ObjectVisitor::_handleClientString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("movementDatatable"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("niche"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("noBuildRadius"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("numberOfPoles"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("objectName"), bind(&ObjectVisitor::_handleClientString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("onlyVisibleInTools"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("playerControlled"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("portalLayoutFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("race"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("radius"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("scale"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("scaleThresholdBeforeExtentTest"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("sendToClient"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("slopeModAngle"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("slopeModPercent"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("slotDescriptorFilename"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("snapToTerrain"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("species"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("stepHeight"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("structureFootprintFileName"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("swimHeight"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("targetable"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("terrainModificationFileName"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("tintPalette"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("useStructureFootprintOutline"), bind(&ObjectVisitor::_handleBool, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("warpTolerance"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("waterModPercent"), bind(&ObjectVisitor::_handleFloat, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("weaponEffect"), bind(&ObjectVisitor::_handleString, _1, _2, _3)));
        attributeHandler_.insert(make_pair(string("weaponEffectIndex"), bind(&ObjectVisitor::_handleInt, _1, _2, _3)));

        //attributeHandler_.insert(make_pair(string("attackType"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("defaultValue"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("maxValueExclusive"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("minValueInclusive"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("palettePathName"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("defaultPaletteIndex"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("cameraHeight"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("variableName"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("sourceVariable"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("dependentVariable"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("gender"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("acceleration"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("speed"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("turnRate"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("postureAlignToTerrain"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("collisionOffsetX"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("collisionOffsetZ"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("certificationsRequired"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("customizationVariableMapping"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("paletteColorCustomizationVariables"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("rangedIntCustomizationVariables"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("constStringCustomizationVariables"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("socketDestinations"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("surfaceType"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));

        //SCHEMATICS STUFF BELOW HERE

        //name is used for both "slots" and "attributes"
        //attributeHandler_.insert(make_pair(string("name"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));

        //slots marks the beginning of the slots section
        //attributeHandler_.insert(make_pair(string("slots"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("hardpoint"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));

        //attributes marks the beginning of the attributes section
        //attributeHandler_.insert(make_pair(string("attributes"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("experiment"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
        //attributeHandler_.insert(make_pair(string("value"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));

        //marks the end of the draft schematic.
        //attributeHandler_.insert(make_pair(string("craftedSharedTemplate"), bind(&ObjectVisitor::_handleUnhandled, _1, _2, _3)));
    }
}

void ObjectVisitor::visit_folder(uint32_t depth, std::string name, uint32_t size)
{}

void ObjectVisitor::visit_data(uint32_t depth, std::string name, uint32_t size, swganh::ByteBuffer& data)
{
    if(name == "XXXX")
    {
        _handleXXXX(data);
    }
    else if(name == "DERVXXXX")
    {
        _handleDERVXXXX(data);
    }
}

void ObjectVisitor::_handleXXXX(swganh::ByteBuffer& buf)
{
    if(buf.size() > 0)
    {
        if(buf.peek<char>() != 1)
        {
            std::string attributeName = buf.read<std::string>(false,true);
            AttributeHandlerIndexIterator it = attributeHandler_.find(attributeName);

            if(it != attributeHandler_.cend())
            {
                it->second(this, attributeName, buf);
            }
        }
    }
}

void ObjectVisitor::_handleDERVXXXX(swganh::ByteBuffer& buf)
{
    parentFiles.insert(buf.read<std::string>(false,true));
}

void ObjectVisitor::_handleClientString(ObjectVisitor* dst, string& name, swganh::ByteBuffer& buf)
{
    if(buf.read<char>())
    {
        shared_ptr<ClientString> cs = make_shared<ClientString>();
        if(buf.read<char>())
        {
            cs->file = buf.read<std::string>(false,true);
            if(buf.read<char>())
            {
                cs->entry = buf.read<std::string>(false,true);
            }
        }
        dst->attributes_.insert(AttributeMap::value_type(move(name), std::make_shared<boost::any>(cs)));
    }
}

void ObjectVisitor::_handleString(ObjectVisitor* dst, string& name, swganh::ByteBuffer& buf)
{
    if(buf.read<char>())
    {
        dst->attributes_.insert(AttributeMap::value_type(move(name), std::make_shared<boost::any>(buf.read<std::string>(false,true))));
    }
}

void ObjectVisitor::_handleInt(ObjectVisitor* dst, string& name, swganh::ByteBuffer& buf)
{
    if(buf.read<char>())
    {
        buf.read<char>();
        uint32_t buffer = buf.read<uint32_t>();
        dst->attributes_.insert(AttributeMap::value_type(move(name), std::make_shared<boost::any>(buffer)));
    }
}

void ObjectVisitor::_handleFloat(ObjectVisitor* dst, string& name, swganh::ByteBuffer& buf)
{
    if(buf.read<char>())
    {
        buf.read<char>();
        float buffer = buf.read<float>();
        dst->attributes_.insert(AttributeMap::value_type(move(name), std::make_shared<boost::any>(buffer)));
    }
}

void ObjectVisitor::_handleBool(ObjectVisitor* dst, string& name, swganh::ByteBuffer& buf)
{
    if(buf.read<char>())
    {
        dst->attributes_.insert(AttributeMap::value_type(move(name), std::make_shared<boost::any>((buf.read<char>()) ? true : false)));
    }
}

void ObjectVisitor::_handleUnhandled(ObjectVisitor* dst, std::string& name, swganh::ByteBuffer& buf)
{
    //@TODO: PRINT A MESSAGE
}

void ObjectVisitor::load_aggregate_data(swganh::tre::ResourceManager* f)
{
    if(!has_aggregate_)
    {
        AttributeMap aggregateAttributeMap;

        std::for_each(parentFiles.begin(), parentFiles.end(), [&] (std::string parentFile)
        {
            auto subI = f->GetResourceByName<ObjectVisitor>(parentFile);

            subI->load_aggregate_data(f);

            //Now we continue to build up our map.
            std::for_each(subI->attributes_.begin(), subI->attributes_.end(), [&] (AttributeMap::value_type pair)
            {
                AttributeMap::iterator lb = aggregateAttributeMap.lower_bound(pair.first);
                if(lb != aggregateAttributeMap.end() && !(aggregateAttributeMap.key_comp()(pair.first, lb->first)))
                {
                    lb->second = pair.second;
                }
                else
                {
                    aggregateAttributeMap.insert(lb, AttributeMap::value_type(pair.first, pair.second));
                }
            });
        });

        std::for_each(attributes_.begin(), attributes_.end(), [&] (AttributeMap::value_type pair)
        {
            AttributeMap::iterator lb = aggregateAttributeMap.lower_bound(pair.first);
            if(lb != aggregateAttributeMap.end() && !(aggregateAttributeMap.key_comp()(pair.first, lb->first)))
            {
                lb->second = pair.second;
            }
            else
            {
                aggregateAttributeMap.insert(lb, AttributeMap::value_type(pair.first, pair.second));    // Use lb as a hint to insert,
                // so it can avoid another lookup
            }
        });

        attributes_ = std::move(aggregateAttributeMap);
        has_aggregate_ = true;
    }
}

bool ObjectVisitor::has_attribute(const std::string& key)
{
    return attributes_.find(key) != attributes_.end();
}