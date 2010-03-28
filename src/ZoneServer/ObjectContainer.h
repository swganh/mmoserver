 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECTCONTAINER_H
#define ANH_ZONESERVER_OBJECTCONTAINER_H

#include "Utils/typedefs.h"
#include "Object_Enums.h"
#include "Object.h"
#include "WorldConfig.h"
#include "WorldManager.h"
//#include "TangibleObject.h"
#include <vector>


//=============================================================================

typedef std::list<Object*>	ObjectList;
//=============================================================================

/*
 - Base class for all containers / items that can be containers

 to offer methods to keep and organize a list of objects
 used by inventory, backpacks, datapads, manufacturingschematics, furniture
 still to add to players for equippedobjects 

*/
class PlayerObject;


class ObjectContainer :	public Object, public ObjectFactoryCallback
{
	
	friend class ItemFactory;
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
	public:

		ObjectContainer(uint64 id,uint64 parentId,const string model,ObjectType type);
		ObjectContainer();
		virtual ~ObjectContainer();

		//handles Object ready in case our item is container
		void				handleObjectReady(Object* object,DispatchClient* client);

		uint64				getObjectMainParent(Object* object);

		ObjectIDList*		getObjects() { return &mData; }
		Object*				getObjectById(uint64 id);
		bool				addObject(Object* Data);
		bool				addObjectSecure(Object* Data);
		bool				addObjectSecure(Object* Data, PlayerObject* player);
		void				addObjectSecure(Object* object, PlayerObjectSet* inRangePlayers, PlayerObject* player = NULL);
		
		//creates the object for the provided player/s
		bool				addObject(Object* Data,PlayerObject* player);
		bool				addObject(Object* Data,PlayerObjectSet*	knownPlayers);
		
		bool				checkForObject(Object* object);
		
		
		bool				deleteObject(Object* data);

		bool				removeObject(uint64 id);
		bool				removeObject(uint64 id, PlayerObject* player);
		bool				removeObject(uint64 id, PlayerObjectSet*	knownPlayers);
		
		bool				removeObject(Object* Data);
		bool				removeObject(Object* Data, PlayerObject* player);
		bool				removeObject(Object* Data, PlayerObjectSet*	knownPlayers);

		ObjectIDList::iterator removeObject(ObjectIDList::iterator it);
		ObjectIDList::iterator removeObject(ObjectIDList::iterator it, PlayerObject*	player);
		ObjectIDList::iterator removeObject(ObjectIDList::iterator it, PlayerObjectSet*	knownPlayers);
		
		//we need to check the content of our children, too!!!!
		virtual bool		checkCapacity(){return((mCapacity-mData.size()) > 0);}
		virtual bool		checkCapacity(uint8 amount, PlayerObject* player = NULL);
		void				setCapacity(uint16 cap){mCapacity = cap;}
		uint16				getCapacity(){return mCapacity;}
		uint16				getHeadCount();
		
		uint16				getContentSize(uint16 iteration)
		{
			uint16 content = mData.size();

			if(iteration > gWorldConfig->getPlayerContainerDepth())
			{
				return content;
			}
			
			ObjectIDList*			ol = getObjects();
			ObjectIDList::iterator	it = ol->begin();

			while(it != ol->end())
			{
				ObjectContainer* tO = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById((*it)));
				if(!tO)
				{
					assert(false);
				}

				content += tO->getContentSize(iteration+1);

				it++;
			}
			return content;
		}


private:



		ObjectIDList			mData;
		uint16					mCapacity;

		
		
};

//=============================================================================


#endif

