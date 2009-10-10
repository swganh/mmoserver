/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_STRUCTUREMANAGER_H
#define ANH_ZONESERVER_STRUCTUREMANAGER_H

#include <vector>

#include "DatabaseManager/DatabaseCallback.h"
#include "MathLib/Vector3.h"
#include "ObjectFactoryCallback.h"
#include "TangibleEnums.h"

#define 	gStructureManager	StructureManager::getSingletonPtr()

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class PlayerObject;

//======================================================================================================================

enum Structure_QueryType
{
	Structure_Query_NULL				=	0,
	Structure_Query_LoadDeedData		=	1,
	Structure_Query_LoadstructureItem	=	2,
	
	Structure_Query_delete				=	3,
	
};

//======================================================================================================================

struct attributeDetail
{
	string	value;
	uint32	attributeId;
};

//links deeds to structuredata
//TODO still needs to be updated to support several structure types for some placeables
//depending on customization

struct StructureDeedLink
{
	uint32	structure_type;
	uint32	skill_Requirement;
	uint32	item_type;
	string	structureObjectString;
	uint8	requiredLots;
	string	stf_file;
	string	stf_name;
	float	healing_modifier;
};

//templated items that need to be at certain spots on/in the structure
//like signs / campfires/elevator buttons and stuff
struct StructureItemTemplate
{
	uint32				CellNr;
	uint32				structure_id;
	uint32				item_type;
	string				structureObjectString;

	TangibleGroup		tanType;

	string				name;
	string				file;

	Anh_Math::Vector3	mPosition;
	Anh_Math::Vector3	mDirection;

	float			dw;
};

typedef		std::vector<StructureDeedLink*>		DeedLinkList;
typedef		std::vector<StructureItemTemplate*>	StructureItemList;

//======================================================================================================================



//======================================================================================================================


//======================================================================================================================

class StructureManagerAsyncContainer
{
public:

	StructureManagerAsyncContainer(Structure_QueryType qt,DispatchClient* client){ mQueryType = qt; mClient = client; }
	~StructureManagerAsyncContainer(){}

	Structure_QueryType			mQueryType;
	DispatchClient*				mClient;

	uint64						mStructureId;
	uint64						mPlayerId;

	PlayerObject*				builder;
	
};

//======================================================================================================================

class StructureManager : public DatabaseCallback,public ObjectFactoryCallback
{
	friend class ObjectFactory;
	

	public:
		//System

		static StructureManager*	getSingletonPtr() { return mSingleton; }
		static StructureManager*	Init(Database* database,MessageDispatch* dispatch);

		~StructureManager();

		void					Shutdown();

		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		void					handleObjectReady(Object* object,DispatchClient* client);

		//=========================================================

		StructureDeedLink*		getDeedData(uint32 type);	//returns the data associated with a certain deed

		StructureItemList*		getStructureItemList(){return(&mItemTemplate);}

		//=========================================================
		//get db data

		//camps

		bool					checkCampRadius(PlayerObject* player);
		bool					checkCityRadius(PlayerObject* player);
		bool					checkinCamp(PlayerObject* player);

		//PlayerStructures
		void					getDeleteStructureMaintenanceData(uint64 structureId, uint64 playerId);



	private:

		StructureManager(Database* database,MessageDispatch* dispatch);
	
		static StructureManager*	mSingleton;
		static bool					mInsFlag;

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;

		DeedLinkList				mDeedLinkList;
		StructureItemList			mItemTemplate;
	
};

#endif 

