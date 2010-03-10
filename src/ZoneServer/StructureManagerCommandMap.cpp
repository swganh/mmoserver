 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "WorldConfig.h"
#include "StructureManager.h"

#include "PlayerStructureTerminal.h"
#include "FactoryFactory.h"
#include "nonPersistantObjectFactory.h"
#include "HarvesterObject.h"
#include "HouseObject.h"
#include "FactoryObject.h"
#include "Inventory.h"
#include "DataPad.h"
#include "ResourceContainer.h"
#include "ResourceType.h"
#include "ObjectFactory.h"
#include "ManufacturingSchematic.h"
#include "PlayerObject.h"
#include "PlayerStructure.h"
#include "QuadTree.h"
#include "WorldManager.h"
#include "ZoneTree.h"
#include "MessageLib/MessageLib.h"

#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "Utils/rand.h"
//======================================================================================================================

bool								StructureManagerCommandMapClass::mInsFlag = false;
StructureManagerCommandMapClass*	StructureManagerCommandMapClass::mSingleton = NULL;

//=

StructureManagerCommandMapClass* StructureManagerCommandMapClass::Init()
{
	if(mInsFlag == false)
	{
		mSingleton = new StructureManagerCommandMapClass();
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

StructureManagerCommandMapClass::StructureManagerCommandMapClass()
{
	mCommandMap.insert(std::make_pair(Structure_Query_Hopper_Permission_Data,&StructureManager::_HandleQueryHopperPermissionData));
	mCommandMap.insert(std::make_pair(Structure_Query_Admin_Permission_Data,&StructureManager::_HandleQueryAdminPermissionData));
	mCommandMap.insert(std::make_pair(Structure_Query_Ban_Permission_Data,&StructureManager::_HandleQueryBanPermissionData));
	mCommandMap.insert(std::make_pair(Structure_Query_Entry_Permission_Data,&StructureManager::_HandleQueryEntryPermissionData));
//	mCommandMap.insert(std::make_pair(opAttributeAttr_Health,&Food::_handleHealth_Buff));
//	mCommandMap.insert(std::make_pair(opAttributeAttr_Mind,&Food::_handleMind_Buff));
	
	
}