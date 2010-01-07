/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CONTAINER_H
#define ANH_ZONESERVER_TANGIBLE_CONTAINER_H

#include "TangibleObject.h"
#include "ObjectFactoryCallback.h"

class CreatureObject;

//=============================================================================

class Container : public TangibleObject
{
	friend class ContainerObjectFactory;

	public:

		Container();
		~Container();

		CreatureObject* getParent(){ return mParent; }
		void		setParent(CreatureObject* creature){ mParent = creature; }
		uint32		getObjectLoadCounter(){ return mObjectLoadCounter; }
		void		setObjectLoadCounter(uint32 count){ mObjectLoadCounter = count; }

		ObjectList*	getObjects(){ return &mObjects; }

		void		addObject(Object* object){ mObjects.push_back(object); }
		void		removeObject(Object* object);
		void		deleteObject(Object* object);

	private:
		uint32				mObjectLoadCounter;
		CreatureObject*		mParent;
		ObjectList			mObjects;
};

//=============================================================================

#endif

