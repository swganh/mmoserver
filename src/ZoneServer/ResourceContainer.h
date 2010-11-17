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

#ifndef ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H
#define ANH_ZONESERVER_TANGIBLE_RESOURCECONTAINER_H

#include "Utils/typedefs.h"
#include "TangibleObject.h"

class Resource;
class PlayerObject;

//=============================================================================

class ResourceContainer : public TangibleObject
{
    friend class ObjectFactory;
    friend class ResourceContainerFactory;

public:

    ResourceContainer();
    ~ResourceContainer();

    virtual void	setParentIdIncDB(uint64 parentId);
    virtual void	upDateFactoryVolume(BString amount);
    virtual void	updateWorldPosition();

    void prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount);


    uint64		getResourceId();
    void		setResourceId(uint64 id);
    Resource*	getResource();
    void		setResource(Resource* resource);
    uint32		getAmount();
    void		setAmount(uint32 amount);
    uint32		getMaxAmount();
    void		setMaxAmount(uint32 max);

    void		sendAttributes(PlayerObject* playerObject);
    BString		getBazaarName();
    BString		getBazaarTang();
    uint32		getCategoryBazaar();

private:

    uint64		mResourceId;
    uint32		mAmount;
    uint32		mMaxAmount;
    Resource*	mResource;
};

//=============================================================================

#endif

