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

#include "StructureManager.h"

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
    mCommandMap.insert(std::make_pair(Structure_Query_NoBuildRegionData,&StructureManager::_HandleNoBuildRegionData));


}
