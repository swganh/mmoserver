/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ResourceType.h"


//=============================================================================

ResourceType::ResourceType()
{
}

//=============================================================================

ResourceType::~ResourceType()
{
}

//=============================================================================

BString ResourceType::getContainerModel()
{
    // gotta put shared_resource_container_ into the name
    BStringVector splits;
    BString tmpStr;
    tmpStr.setLength(512);

    int elements = mModel.split(splits,'/');

    tmpStr = splits[0].getAnsi();

    for(int i = 1; i < elements - 1; i++)
        tmpStr << "/" << splits[i].getAnsi();

    tmpStr << "/shared_resource_container_" << splits[elements-1].getAnsi();

    return tmpStr;
}





