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

#ifndef ANH_ZONESERVER_STRUCTUREMANAGER_H
#define ANH_ZONESERVER_STRUCTUREMANAGER_H

#include <vector>
#include <list>

#include "DatabaseManager/DatabaseCallback.h"
#include "ZoneServer/Objects/ObjectFactoryCallback.h"
#include "ZoneServer/Objects/Tangible Object/TangibleEnums.h"
#include "ZoneServer/WorldManager.h"
#include "anh/utils/Scheduler.h"
#include "ZoneServer\GameSystemManagers\UI Manager\UICallback.h"
#include "ZoneServer\GameSystemManagers\Structure Manager\StructureManagerTypes.h"

#define 	gStructureManager	StructureManager::getSingletonPtr()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class PlayerObject;
class FactoryObject;
class PlayerStructure;
class UIWindow;
class StructureManagerCommandMapClass;
class NoBuildRegion;
class ObjectController;


namespace Anh_Utils
{
// class Clock;
class Scheduler;
//class VariableTimeScheduler;
}
//======================================================================================================================

typedef std::list<uint64>				ObjectIDList;


//======================================================================================================================

class structure_async_container : public WindowAsyncContainerCommand
{
public :

	structure_async_container() {
		structure_id_ = 0;
	}
	~structure_async_container(){}

	uint64	getStructureId(){return structure_id_;}
	void	setStructureId(uint64 id){structure_id_ = id;}

private :

	uint64	structure_id_;

};


typedef		std::vector<StructureDeedLink*>		DeedLinkList;
typedef		std::vector<StructureItemTemplate*>	StructureItemList;
typedef		std::vector<NoBuildRegionTemplate*>	NoBuildRegionList;

//======================================================================================================================



//======================================================================================================================


//======================================================================================================================

class StructureManagerAsyncContainer
{
public:

    StructureManagerAsyncContainer(Structure_QueryType qt,DispatchClient* client) {
        mQueryType = qt;
        mClient = client;
    }
    ~StructureManagerAsyncContainer() {}

    Structure_QueryType			mQueryType;
    DispatchClient*				mClient;

    uint64						mStructureId;
    uint64						mPlayerId;
    uint64						mTargetId;

    uint8						b1;
    uint8						b2;

    int8						name[64];

    PlayerObject*				builder;
    StructureAsyncCommand		command;

};



//======================================================================================================================

class StructureManager : public UICallback, public swganh::database::DatabaseCallback,public ObjectFactoryCallback
{
    friend class ObjectFactory;
    friend class StructureManagerCommandMapClass;


public:
    //System


    static StructureManager*	getSingletonPtr() {
        return mSingleton;
    }
    static StructureManager*	Init(swganh::database::Database* database,MessageDispatch* dispatch);

    ~StructureManager();

    void					Shutdown();
    //inherited callbacks
    virtual void			handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result);

	virtual	void	handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer);
	//virtual void	handleUIEvent(std::u16string leftValue, std::u16string rightValue, UIWindow* window, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer);
	virtual void	handleUIEvent(std::u16string leftValue, std::u16string rightValue, UIWindow* window, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer);

    void					handleObjectReady(Object* object,DispatchClient* client);

    void					createNewFactorySchematicBox(PlayerObject* player, FactoryObject* factory);
    void					createNewStructureStatusBox(PlayerObject* player, PlayerStructure* structure);
    void					createRenameStructureBox(PlayerObject* player, PlayerStructure* structure);
    void					createPowerTransferBox(PlayerObject* player, PlayerStructure* structure);
    void					createPayMaintenanceTransferBox(PlayerObject* player, PlayerStructure* structure);
    void					createNewStructureDeleteConfirmBox(PlayerObject* player, PlayerStructure* structure);
    void					createNewStructureDestroyBox(PlayerObject* player, PlayerStructure* structure, bool redeed);

    void					updateKownPlayerPermissions(PlayerStructure* structure);

    //=========================================================

    StructureDeedLink*		getDeedData(uint32 type);	//returns the data associated with a certain deed

    StructureItemList*		getStructureItemList() {
        return(&mItemTemplate);
    }

    //=========================================================
    NoBuildRegionList*		getNoBuildRegionList() {
        return(&mNoBuildList);
    }

    //=========================================================

    //get db data

    //camps

    bool					checkCampRadius(PlayerObject* player);
    bool					checkCityRadius(PlayerObject* player);
    bool					checkinCamp(PlayerObject* player);
    //no build region
    bool					checkNoBuildRegion(const glm::vec3& vec3);
    bool					checkNoBuildRegion(PlayerObject* player);
    //bool					checkInNoBuildRadius(glm::vec3 dVec, glm::vec3 rVec);

    //PlayerStructures
    void					getDeleteStructureMaintenanceData(uint64 structureId, uint64 playerId);

    void					addNametoPermissionList(uint64 structureId, uint64 playerId, BString name, BString list);
    void					removeNamefromPermissionList(uint64 structureId, uint64 playerId, BString name, BString list);
    void					checkNameOnPermissionList(uint64 structureId, uint64 playerId, BString name, BString list, StructureAsyncCommand command);
    void					processVerification(StructureAsyncCommand command, bool owner);
    void					TransferStructureOwnership(StructureAsyncCommand command);

    //returns a confirmatioon code for structure destruction
    BString					getCode();

    //
    ObjectIDList*			getStrucureDeleteList() {
        return &mStructureDeleteList;
    }
    void					addStructureforDestruction(uint64 iD)
    {
        mStructureDeleteList.push_back(iD);
        gWorldManager->getPlayerScheduler()->addTask(fastdelegate::MakeDelegate(this,&StructureManager::_handleStructureObjectTimers),7,1000,NULL);
    }

    void					addStructureforConstruction(uint64 iD)
    {
        mStructureDeleteList.push_back(iD);
        gWorldManager->getPlayerScheduler()->addTask(fastdelegate::MakeDelegate(this,&StructureManager::_handleStructureObjectTimers),7,mBuildingFenceInterval,NULL);
    }

    void					addStructureforHopperUpdate(uint64 iD)
    {
        mStructureDeleteList.insert(mStructureDeleteList.begin(),iD);// .push_back(iD);
        gWorldManager->getPlayerScheduler()->addTask(fastdelegate::MakeDelegate(this,&StructureManager::_handleStructureObjectTimers),7,5000,NULL);
    }

    //check all active harvesters once in a while they might have been turned off by the db
    bool					_handleStructureDBCheck(uint64 callTime, void* ref);

    void					OpenStructureAdminList(uint64 structureId, uint64 playerId);
    void					OpenStructureEntryList(uint64 structureId, uint64 playerId);
    void					OpenStructureBanList(uint64 structureId, uint64 playerId);
    void					OpenStructureHopperList(uint64 structureId, uint64 playerId);


    uint32					getCurrentPower(PlayerObject* player);
    uint32					deductPower(PlayerObject* player, uint32 amount);

    //asynchronously updates the lot count of a player
    void					UpdateCharacterLots(uint64 charId);

    /// New Style OC functions
    // =======================================================================================
    /// This command is used to place structures
    /**
     *
     * The client attempts to place the structure
     *
     * @param object The object placing the structure (always a PlayerObject).
     * @param message The message from the client requesting this command.
     * @param cmd_properties Contextual information for use during processing this command.
     */
    bool HandlePlaceStructure(Object* object, Object* target,Message* message, ObjectControllerCmdProperties* cmdProperties);

private:

    //callback functions
    void				_HandleQueryHopperPermissionData(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleQueryAdminPermissionData(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleQueryEntryPermissionData(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleQueryBanPermissionData(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleUpdateCharacterLots(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleStructureRedeedCallBack(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleStructureDestruction(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleGetInactiveHarvesters(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleStructureTransferLotsRecipient(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleQueryLoadDeedData(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleRemovePermission(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleUpdateAdminPermission(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleAddPermission(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleNonPersistantLoadStructureItem(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleCheckPermission(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);
    void				_HandleUpdateAttributes(StructureManagerAsyncContainer* asynContainer,swganh::database::DatabaseResult* result);

    void				_HandleNoBuildRegionData(StructureManagerAsyncContainer* asyncContainer, swganh::database::DatabaseResult* result);


    StructureManager(swganh::database::Database* database,MessageDispatch* dispatch);

    bool	_handleStructureObjectTimers(uint64 callTime, void* ref);

    static StructureManager*	mSingleton;
    static bool					mInsFlag;

    swganh::database::Database*					mDatabase;
    MessageDispatch*			mMessageDispatch;

    DeedLinkList				mDeedLinkList;
    StructureItemList			mItemTemplate;
    ObjectIDList				mStructureDeleteList;
    uint32						mBuildingFenceInterval;

    NoBuildRegionList			mNoBuildList;

};

typedef void														(StructureManager::*funcStructureManagerPointer)(StructureManagerAsyncContainer*,swganh::database::DatabaseResult*);
typedef std::map<Structure_QueryType,funcStructureManagerPointer>	StructureManagerCommandMap;
#define gStructureManagerCmdMap										((StructureManagerCommandMapClass::getSingletonPtr())->mCommandMap)

class StructureManagerCommandMapClass
{
public:

    static StructureManagerCommandMapClass*		getSingletonPtr() {
        return mSingleton;
    }
    static StructureManagerCommandMapClass*		Init();


    ~StructureManagerCommandMapClass();
    StructureManagerCommandMapClass();

    StructureManagerCommandMap					mCommandMap;

private:

    static bool									mInsFlag;
    static StructureManagerCommandMapClass*		mSingleton;

};

#endif



