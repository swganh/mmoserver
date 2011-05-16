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

#ifndef ANH_ZONESERVER_RESOURCETYPE_H
#define ANH_ZONESERVER_RESOURCETYPE_H

#include "Utils/bstring.h"
#include "Utils/typedefs.h"


//=============================================================================

class ResourceType
{
    friend class ResourceManager;

public:

    ResourceType();
    ~ResourceType();

    uint32	getId() {
        return mId;
    }
    void	setId(uint32 id) {
        mId = id;
    }
    BString	getName() {
        return mName;
    }
    void	setName(const BString name) {
        mName = name;
    }
    BString	getDescriptor() {
        return mDescriptor;
    }
    void	setDescriptor(const BString desc) {
        mDescriptor = desc;
    }
    uint16	getCategoryId() {
        return mCatId;
    }
    void	setCategoryId(uint16 cat) {
        mCatId = cat;
    }
    BString	getModel() {
        return mModel;
    }
    void	setModel(const BString model) {
        mModel = model;
    }
    BString	getContainerModel();
    BString	getTypeName() {
        return mTypeName;
    }
    void	setTypeName(const BString name) {
        mTypeName = name;
    }
    uint32	getCategoryBazaar() {
        return mCategoryBazaar;
    }
    void	setCategoryBazaar(uint32 cat) {
        mCategoryBazaar = cat;
    }
    BString	getResourceType() {
        return mResourceType;
    }
    void	setResourceType(const BString type) {
        mResourceType = type;
    }

private:

    uint32	mId;
    BString	mName;
    BString	mTypeName;
    BString	mDescriptor;
    BString	mModel;
    uint16	mCatId;
    uint32  mCategoryBazaar;
    BString	mResourceType;
};

//=============================================================================

#endif

