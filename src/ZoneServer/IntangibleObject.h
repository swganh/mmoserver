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

#ifndef ANH_ZONESERVER_INTANGIBLE_OBJECT_H
#define ANH_ZONESERVER_INTANGIBLE_OBJECT_H

#include "Object.h"

//Forward Declerations
class PlayerObject;

//=============================================================================
//
// Intangible groups
//
enum IntangibleGroup
{
    ItnoGroup_None					= 0,
    ItnoGroup_Vehicle				= 1,
    ItnoGroup_Mount					= 2,
    ItnoGroup_Pet					= 3,
    ItnoGroup_Droid					= 4
};

class IntangibleObject : public Object
{
    friend class ObjectFactory;
    friend class VehicleControllerFactory;

public:
    IntangibleObject();
    ~IntangibleObject();

    BString				getName() const {
        return mName;
    }
    void				setName(const int8* name) {
        mName = name;
    }
    BString				getNameFile() const {
        return mNameFile;
    }
    void				setNameFile(const int8* file) {
        mNameFile = file;
    }
    BString				getCustomName() const {
        return mCustomName;
    }
    void				setCustomName(const int8* name) {
        mCustomName = name;
    }
    BString				getDetailFile() {
        return mDetailFile;
    }
    void				setDetailFile(const char* detail_file) {
        mDetailFile = detail_file;
    }
    BString				getDetail()	{
        return mDetail;
    }
    void				setDetail(const char* detail) {
        mDetail = detail;
    }
    float				getComplexity() {
        return mComplexity;
    }
    void				setComplexity(float complexity) {
        mComplexity = complexity;
    }
    int32				getVolume() {
        return mVolume;
    }
    void				setVolume(const int32 volume) {
        mVolume = volume;
    }
    int					getItnoGroup() {
        return mItnoGroup;
    }
    void				setItnoGroup(int itno_group) {
        mItnoGroup = itno_group;
    }
    BString				getPhysicalModelString() {
        return mPhysicalModel;
    }
    void				setPhysicalModelString(BString physical_model) {
        mPhysicalModel = physical_model;
    }
protected:
    float				mComplexity;
    int32				mVolume;
    BString				mCustomName;
    BString				mName;
    BString				mNameFile;
    BString				mDetailFile;			//ASCII
    BString				mDetail;				//Description - ASCII
    BString				mPhysicalModel;			//Model string for the physical part
    int					mItnoGroup;

private:

};

#endif
