/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MountObject.h"
#include "MessageLib/MessageLib.h"
#include "PlayerObject.h"

		//=============================================================================
// known objects are those that are in the SI NEAR to our object and have been created
// all known objects that are NOT found in the next SI update will be destroyed as out of range

void MountObject::addKnownObject(Object* object)
{
	if(this->getId() == object->getId())
	{
		//assert(false);
		return;
	}
	if(checkKnownObjects(object))
	{
		gLogger->logMsgF("Object::addKnownObject %I64u couldnt be added to %I64u - already in it", MSG_NORMAL, object->getId(), this->getId());
		return;
	}

	if(object->getType() == ObjType_Player)
	{
		mKnownPlayers.insert(dynamic_cast<PlayerObject*>(object));
	}
	else
	{
		mKnownObjects.insert(object);
		mKnownObjectsIDs.insert(object->getId());
	}
}


bool MountObject::removeKnownObject(Object* object)
{
	if(object->getType() == ObjType_Player)
	{
		PlayerObjectSet::iterator it = mKnownPlayers.find(dynamic_cast<PlayerObject*>(object));

		if(it != mKnownPlayers.end())
		{
			mKnownPlayers.erase(it);

			return(true);
		}
	}
	else
	{

		ObjectSet::iterator it = mKnownObjects.find(object);

		if(it != mKnownObjects.end())
		{

			gLogger->logMsgF("Object::removeKnownObject removing %I64u from %I64u", MSG_NORMAL, object->getId(), this->getId());
			mKnownObjects.erase(it);

			ObjectIDSet::iterator itID = mKnownObjectsIDs.find(object->getId());

			if(itID != mKnownObjectsIDs.end())
			{
				mKnownObjectsIDs.erase(itID);
				return(true);
			}
			else
			{
				gLogger->logMsgF("Object::removeKnownObject %I64u couldnt be removed from %I64u - not found", MSG_NORMAL, object->getId(), this->getId());
			}
		

			return(true);
		}
		else
			gLogger->logMsgF("Object::removeKnownObject %I64u couldnt be removed from %I64u - not found", MSG_NORMAL, object->getId(), this->getId());

		
	}

	return(false);
}


//=============================================================================

void MountObject::destroyKnownObjects()
{
	ObjectSet::iterator			objIt		= mKnownObjects.begin();
	PlayerObjectSet::iterator	playerIt	= mKnownPlayers.begin();

	ObjectIDSet::iterator IDIt				= mKnownObjectsIDs.begin();
		
	// objects
	while(IDIt	!= mKnownObjectsIDs.end())
	{
		Object* object = gWorldManager->getObjectById(*IDIt);
		if(!object)
		{
			gLogger->logMsgF("Object::removeKnownObject %I64u couldnt be removed from %I64u - not found", MSG_NORMAL, (*IDIt), this->getId());	
			(*IDIt)++;
		}
		else
		{
			mKnownObjectsIDs.erase(IDIt++);	
		}
		
	}

	while(objIt != mKnownObjects.end())
	{
		(*objIt)->removeKnownObject(this);
		mKnownObjects.erase(objIt++);
	}

	// players
	while(playerIt != mKnownPlayers.end())
	{
		PlayerObject* targetPlayer = (*playerIt);

		gMessageLib->sendDestroyObject(mId,targetPlayer);

		targetPlayer->removeKnownObject(this);
		mKnownPlayers.erase(playerIt++);

		
	}
}