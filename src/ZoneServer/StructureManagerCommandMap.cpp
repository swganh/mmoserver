 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "WorldConfig.h"
#include "StructureManager.h"

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
	mCommandMap.insert(std::make_pair(Structure_UpdateCharacterLots,&StructureManager::_HandleUpdateCharacterLots));
	mCommandMap.insert(std::make_pair(Structure_UpdateStructureDeed,&StructureManager::_HandleStructureRedeedCallBack));
	mCommandMap.insert(std::make_pair(Structure_GetDestructionStructures,&StructureManager::_HandleStructureDestruction));
	mCommandMap.insert(std::make_pair(Structure_GetInactiveHarvesters,&StructureManager::_HandleGetInactiveHarvesters));
	mCommandMap.insert(std::make_pair(Structure_StructureTransfer_Lots_Recipient,&StructureManager::_HandleStructureTransferLotsRecipient));
	mCommandMap.insert(std::make_pair(Structure_Query_LoadDeedData,&StructureManager::_HandleQueryLoadDeedData));
	mCommandMap.insert(std::make_pair(Structure_Query_Remove_Permission,&StructureManager::_HandleRemovePermission));
	mCommandMap.insert(std::make_pair(Structure_Query_UpdateAdminPermission,&StructureManager::_HandleUpdateAdminPermission));
	mCommandMap.insert(std::make_pair(Structure_Query_Add_Permission,&StructureManager::_HandleAddPermission));
	mCommandMap.insert(std::make_pair(Structure_Query_LoadstructureItem,&StructureManager::_HandleNonPersistantLoadStructureItem));
	mCommandMap.insert(std::make_pair(Structure_Query_Check_Permission,&StructureManager::_HandleCheckPermission));
	mCommandMap.insert(std::make_pair(Structure_UpdateAttributes,&StructureManager::_HandleUpdateAttributes));


	
}