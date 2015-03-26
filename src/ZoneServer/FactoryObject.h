/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FACTORY_OBJECT_H
#define ANH_ZONESERVER_FACTORY_OBJECT_H

#include "PlayerStructure.h"
#include "BuildingEnums.h"
#include "ObjectFactory.h"

//=============================================================================



//=============================================================================

struct FactoryHopperItem
{
    uint64			HarvesterID;
    uint64			ResourceID;
    float			Quantity;
};

class FactoryObject :	public PlayerStructure, public DatabaseCallback
{
    friend class FactoryFactory;

public:

    FactoryObject();
    ~FactoryObject();

    virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    virtual void	handleObjectReady(Object* object,DispatchClient* client, uint64 hopper);

    FactoryFamily	getFactoryFamily() {
        return mFactoryFamily;
    }
    void			setFactoryFamily(FactoryFamily ff) {
        mFactoryFamily = ff;
    }

    uint32			getLoadCount() {
        return mTotalLoadCount;
    }
    uint32			decLoadCount() {
        return (mTotalLoadCount-1);
    }
    void			setLoadCount(uint32 count) {
        mTotalLoadCount = count;
    }

    bool			getActive() {
        return mActive;
    }
    void			setActive(bool value) {
        mActive = value;
    }

    virtual void	ResetRadialMenu() {
        mRadialMenu.reset();
        RadialMenu* radial	= NULL;
        RadialMenuPtr radialPtr(radial);
        mRadialMenu = radialPtr;
    }
    void			prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
    void			handleObjectMenuSelect(uint8 messageType,Object* srcObject);

    void			setSchematicCustomName(BString name) {
        mSchematicName = name;
    }
    BString			getSchematicCustomName() {
        return mSchematicName;
    }

    void			setSchematicName(BString name) {
        mSchematicSTF = name;
    }
    BString			getSchematicName() {
        return mSchematicSTF;
    }

    void			setSchematicFile(BString name) {
        mSchematicFile = name;
    }
    BString			getSchematicFile() {
        return mSchematicFile;
    }

    uint64			getManSchemID() {
        return mManSchematicID;
    }
    void			setManSchemID(uint64 value) {
        mManSchematicID = value;
    }

    uint64			getIngredientHopper() {
        return mIngredientHopper;
    }
    void			setIngredientHopper(uint64 value) {
        mIngredientHopper= value;
    }

    uint64			getOutputHopper() {
        return mOutputHopper;
    }
    void			setOutputHopper(uint64 value) {
        mOutputHopper= value;
    }

    uint32			getMask() {
        return mSchematicMask;
    }
    void			setMask(uint32 value) {
        mSchematicMask = value;
    }


private:

    bool			mActive;


    FactoryFamily	mFactoryFamily;

    uint32			mSchematicMask;

    uint32			mTotalLoadCount;
    uint64			mManSchematicID;

    uint64			mIngredientHopper;
    uint64			mOutputHopper;

    BString			mSchematicName;
    BString			mSchematicFile;
    BString			mSchematicSTF;

};

//=============================================================================

#endif
