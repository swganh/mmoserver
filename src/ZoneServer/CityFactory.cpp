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

#include "CityFactory.h"
#include "City.h"
#include "ObjectFactoryCallback.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool			CityFactory::mInsFlag    = false;
CityFactory*	CityFactory::mSingleton  = NULL;

//======================================================================================================================

CityFactory*	CityFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new CityFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

CityFactory::CityFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

CityFactory::~CityFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void CityFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case CityFQuery_MainData:
		{
			City* city = _createCity(result);

			if(city->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(city,asyncContainer->mClient);
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

void CityFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,CityFQuery_MainData,client),
							"SELECT cities.id,cities.city_name,planet_regions.region_name,planet_regions.region_file,planet_regions.x,planet_regions.z,"
							"planet_regions.width,planet_regions.height"
							" FROM cities"
							" INNER JOIN planet_regions ON (cities.city_region = planet_regions.region_id)"
							" WHERE (cities.id = %"PRIu64")",id);
}

//=============================================================================

City* CityFactory::_createCity(DatabaseResult* result)
{
	City*	city = new City();

	uint64 count = result->getRowCount();

	result->GetNextRow(mCityBinding,(void*)city);

	city->setLoadState(LoadState_Loaded);

	return city;
}

//=============================================================================

void CityFactory::_setupDatabindings()
{
	mCityBinding = mDatabase->CreateDataBinding(8);
	mCityBinding->addField(DFT_uint64,offsetof(City,mId),8,0);
	mCityBinding->addField(DFT_bstring,offsetof(City,mCityName),64,1);
	mCityBinding->addField(DFT_bstring,offsetof(City,mRegionName),64,2);
	mCityBinding->addField(DFT_bstring,offsetof(City,mNameFile),64,3);
	mCityBinding->addField(DFT_float,offsetof(City,mPosition.x),4,4);
	mCityBinding->addField(DFT_float,offsetof(City,mPosition.z),4,5);
	mCityBinding->addField(DFT_float,offsetof(City,mWidth),4,6);
	mCityBinding->addField(DFT_float,offsetof(City,mHeight),4,7);
}

//=============================================================================

void CityFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mCityBinding);
}

//=============================================================================

