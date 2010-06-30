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
#include "NoBuildRegionFactory.h"
#include "ObjectFactoryCallback.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

NoBuildRegionFactory::NoBuildRegionFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

NoBuildRegionFactory::~NoBuildRegionFactory(void)
{
	_destroyDatabindings();
}
//=============================================================================

void NoBuildRegionFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case 1:
		{
			NoBuildRegion* noBuildRegion = _createNoBuildRegion(result);

			if(noBuildRegion->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(noBuildRegion,asyncContainer->mClient);
			else
			{

			}
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}
//=============================================================================

void NoBuildRegionFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteProcedureAsync(this, new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,1,client), 
									"CALL sp_PlanetNoBuildRegions(%"PRIu64")",id);
}
//=============================================================================

NoBuildRegion* NoBuildRegionFactory::_createNoBuildRegion(DatabaseResult* result)
{
	NoBuildRegion*	noBuildRegion = new NoBuildRegion();

	uint64 count = result->getRowCount();

	result->GetNextRow(mNoBuildRegionBinding,noBuildRegion);

	noBuildRegion->setLoadState(LoadState_Loaded);

	return noBuildRegion;
}
//=============================================================================
void NoBuildRegionFactory::_setupDatabindings()
{
	mNoBuildRegionBinding = mDatabase->CreateDataBinding(7);
	mNoBuildRegionBinding->addField(DFT_uint32,offsetof(NoBuildRegion,mRegionId),4,1);
	mNoBuildRegionBinding->addField(DFT_bstring,offsetof(NoBuildRegion,mRegionName),64,2);
	mNoBuildRegionBinding->addField(DFT_float,offsetof(NoBuildRegion,mPosition.x),4,3);
	mNoBuildRegionBinding->addField(DFT_float,offsetof(NoBuildRegion,mPosition.z),4,4);
	mNoBuildRegionBinding->addField(DFT_float,offsetof(NoBuildRegion,mWidth),4,5);
	mNoBuildRegionBinding->addField(DFT_float,offsetof(NoBuildRegion,mHeight),4,6);
	mNoBuildRegionBinding->addField(DFT_uint8,offsetof(NoBuildRegion,mBuild),4,7);
	mNoBuildRegionBinding->addField(DFT_uint8,offsetof(NoBuildRegion,mNoBuildType),4,8);
}

//=============================================================================

void NoBuildRegionFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mNoBuildRegionBinding);
}

//=============================================================================
NoBuildPlanetList* NoBuildRegionFactory::getNoBuildPlanetList(uint32 planet)
{

}