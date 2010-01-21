/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FACTORY_BASE_H
#define ANH_ZONESERVER_FACTORY_BASE_H

#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>
#include <map>


//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class InLoadingContainer;
class Type1_QueryContainer;
class Object;
class Item;
class QueryContainerBase;
class SpawnData;

//=============================================================================

typedef std::map<uint64,InLoadingContainer*>	ObjectLoadMap;

//=============================================================================

class FactoryBase : public DatabaseCallback
{
	public:

		FactoryBase(Database* database);

		virtual void	handleDatabaseJobComplete(void* ref,DatabaseResult* result) = 0;
		virtual void	requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client) = 0;

		virtual			~FactoryBase();

		bool			releaseQueryContainerPoolMemory(){ return(mQueryContainerPool.release_memory()); }
		bool			releaseILCPoolMemory(){ return(mILCPool.release_memory()); }

	protected:

		void				_buildAttributeMap(Object* object,DatabaseResult* result);

		InLoadingContainer* _getObject(uint64 id);
		bool				_removeFromObjectLoadMap(uint64 id);

		boost::pool<boost::default_user_allocator_malloc_free>		mILCPool;
		boost::pool<boost::default_user_allocator_malloc_free>		mQueryContainerPool;

		ObjectLoadMap			mObjectLoadMap;

		DataBinding*			mAttributeBinding;
		Database*				mDatabase;
};

//=============================================================================

class NonPersistantQueryContainerBase
{
	public:

		NonPersistantQueryContainerBase(ObjectFactoryCallback* ofCallback,uint32 queryType,DispatchClient* client,Item* newItem) :
				mClient(client),mOfCallback(ofCallback),mQueryType(queryType),mItem(newItem){}

		DispatchClient*			mClient;
		ObjectFactoryCallback*	mOfCallback;
		Object*					mObject;
		uint32					mQueryType;
		Item*					mItem;
};

//=============================================================================

class QueryNonPersistentItemFactory
{
	public:

		QueryNonPersistentItemFactory(ObjectFactoryCallback* ofCallback,uint32 queryType, uint64 id = 0) :
		  mOfCallback(ofCallback),mQueryType(queryType),mId(id) {}

		ObjectFactoryCallback*	mOfCallback;
		Object*					mObject;
		uint32					mQueryType;
		uint64					mId;
};

//=============================================================================

class QueryContainerBase
{
	public:

		QueryContainerBase(ObjectFactoryCallback* ofCallback,uint32 queryType,DispatchClient* client, uint64 id = 0) :
			mClient(client),mOfCallback(ofCallback),mQueryType(queryType),mId(id){}

		DispatchClient*			mClient;
		ObjectFactoryCallback*	mOfCallback;
		Object*					mObject;
		uint32					mQueryType;
		uint64					mId;
};

//=============================================================================

class InLoadingContainer
{
	public:

		InLoadingContainer(Object* object,ObjectFactoryCallback* ofCallback,DispatchClient* client,uint32 loadCounter = 1)
						  : mObject(object),mOfCallback(ofCallback),mClient(client),mLoadCounter(loadCounter), mManSchem(false){}

		Object*					mObject;
		ObjectFactoryCallback*	mOfCallback;
		DispatchClient*			mClient;
		uint32					mLoadCounter;
		bool					mManSchem;
};

//=============================================================================

class Type1_QueryContainer
{
	public:

		Type1_QueryContainer(){}

		uint64	mId;
		string	mString;
};

//=============================================================================

class Attribute_QueryContainer
{
	public:

		Attribute_QueryContainer(){}

		string	mKey;
		string	mValue;
		uint8	mInternal;
};

//=============================================================================

enum NonPersistentQuery
{
	NPQuery_MainData	= 1,
	NPQuery_Attributes	= 2
};

//=============================================================================

#endif
