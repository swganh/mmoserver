/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Object.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "PlayerObject.h"

//=============================================================================

Object::Object() :
mId(0),
mParentId(0),
mModel(""),
mLoadState(LoadState_Loading),
mTypeOptions(0),
mSubZoneId(0),
mEquipSlots(0),
mPrivateOwner(0)
{
	mDirection = Anh_Math::Quaternion();
	mPosition  = Anh_Math::Vector3();

	mObjectController.setObject(this);
}

//=============================================================================

Object::Object(uint64 id,uint64 parentId,string model,ObjectType type) : 
mId(id),
mParentId(parentId),
mModel(model),
mType(type),
mLoadState(LoadState_Loading),
mTypeOptions(0),
mSubZoneId(0),
mEquipSlots(0),
mPrivateOwner(0)
{
	mObjectController.setObject(this);
}

//=============================================================================

Object::~Object()
{
	mKnownObjects.clear();
	mKnownPlayers.clear();

	mAttributeMap.clear();
	mInternalAttributeMap.clear();
	mAttributeOrderList.clear();
}

//=============================================================================

bool Object::removeKnownObject(Object* object)
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
			mKnownObjects.erase(it);

			return(true);
		}
	}

	return(false);
}

//=============================================================================

bool Object::checkKnownObjects(Object* object) const
{
	if(object->getType() == ObjType_Player)
	{
		PlayerObjectSet::const_iterator it = mKnownPlayers.find(dynamic_cast<PlayerObject*>(object));

		if(it != mKnownPlayers.end())
		{
			return(true);
		}
	}
	else
	{
		ObjectSet::const_iterator it = mKnownObjects.find(object);

		if(it != mKnownObjects.end())
		{
			return(true);
		}
	}

	return(false);
}
//=============================================================================



bool Object::checkKnownPlayer(PlayerObject* player)
{
	PlayerObjectSet::iterator it = mKnownPlayers.find(player);
	return (it != mKnownPlayers.end());
}

//=============================================================================

void Object::sendAttributes(PlayerObject* playerObject)
{
	if(playerObject->getConnectionState() != PlayerConnState_Connected)
		return;

	if(!mAttributeMap.size() || mAttributeMap.size() != mAttributeOrderList.size())
		return;

	Message*	newMessage;
	string		value;

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);

	gMessageFactory->addUint32(mAttributeMap.size()); 

	AttributeMap::iterator			mapIt;
	AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

	while(orderIt != mAttributeOrderList.end())
	{
		mapIt = mAttributeMap.find(*orderIt);
		//see if we have to format it properly
		
		gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));
		value = (*mapIt).second.c_str();
		if(gWorldManager->getAttributeKey((*mapIt).first).getCrc() == BString("duration").getCrc())
		{
			uint32 time;
			sscanf(value.getAnsi(),"%u",&time);	
			//uint32 hour = (uint32)time/3600;
			//time = time - hour*3600;
			uint32 minutes = (uint32)time/60;
			uint32 seconds = time - minutes*60;
			int8 valueInt[64];
			sprintf(valueInt,"%um %us",minutes,seconds);
			value = valueInt;

		}
			//gLogger->logMsgF("Object::sendAttribute: %s : %s",MSG_HIGH,gWorldManager->getAttributeKey((*mapIt).first).getAnsi(),value.getAnsi());	
		
		
		value.convert(BSTRType_Unicode16);
		gMessageFactory->addString(value);

		++orderIt;
	}

	//these should not be necessary in precu they start appearing in cu!!!
	//gMessageFactory->addUint32(0xffffffff);

	newMessage = gMessageFactory->EndMessage();

	//must in fact be send as unreliable for attributes to show during the crafting process!!!
	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),CR_Client,9);
}

//=========================================================================

void Object::setAttribute(string key,std::string value)
{
	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it == mAttributeMap.end())
	{
		gLogger->logMsgF("Object::setAttribute: could not find %s",MSG_HIGH,key.getAnsi());
		return;
	}

	(*it).second = value;
}

//=============================================================================

void Object::addAttribute(string key,std::string value)
{
	mAttributeMap.insert(std::make_pair(key.getCrc(),value));
	mAttributeOrderList.push_back(key.getCrc());
}

//=============================================================================

bool Object::hasAttribute(string key) const
{
	if(mAttributeMap.find(key.getCrc()) != mAttributeMap.end())
		return(true);

	return(false);
}

//=============================================================================

void Object::removeAttribute(string key)
{
	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it != mAttributeMap.end())
		mAttributeMap.erase(it);
	else
		gLogger->logMsgF("Object::removeAttribute: could not find %s",MSG_HIGH,key.getAnsi());
}

//=========================================================================

void	Object::setInternalAttribute(string key,std::string value)
{
	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it == mInternalAttributeMap.end())
	{
		gLogger->logMsgF("Object::setInternalAttribute: could not find %s",MSG_HIGH,key.getAnsi());
		return;
	}

	(*it).second = value;
}

//=============================================================================

void Object::addInternalAttribute(string key,std::string value)
{
	mInternalAttributeMap.insert(std::make_pair(key.getCrc(),value));
}

//=============================================================================

bool Object::hasInternalAttribute(string key)
{
	if(mInternalAttributeMap.find(key.getCrc()) != mInternalAttributeMap.end())
		return(true);

	return(false);
}

//=============================================================================

void Object::removeInternalAttribute(string key)
{
	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it != mInternalAttributeMap.end())
		mInternalAttributeMap.erase(it);
	else
		gLogger->logMsgF("Object::removeInternalAttribute: could not find %s",MSG_HIGH,key.getAnsi());
}


//=============================================================================

bool Object::addKnownObjectSafe(Object* object)
{
	if(!checkKnownObjects(object))
	{
		addKnownObject(object);

		return(true);
	}

	return(false);
}

//=============================================================================

void Object::addKnownObject(Object* object)
{
	if(object->getType() == ObjType_Player)
	{
		mKnownPlayers.insert(dynamic_cast<PlayerObject*>(object));
	}
	else
	{
		mKnownObjects.insert(object);
	}
}

//=============================================================================

void Object::destroyKnownObjects()
{
	ObjectSet::iterator			objIt		= mKnownObjects.begin();
	PlayerObjectSet::iterator	playerIt	= mKnownPlayers.begin();

	// objects
	while(objIt != mKnownObjects.end())
	{
		(*objIt)->removeKnownObject(this);
		objIt = mKnownObjects.erase(objIt);
	}

	// players
	while(playerIt != mKnownPlayers.end())
	{
		PlayerObject* targetPlayer = (*playerIt);

		gMessageLib->sendDestroyObject(mId,targetPlayer);

		targetPlayer->removeKnownObject(this);
		playerIt = mKnownPlayers.erase(playerIt);

		// (by Eru)
		// When I see a call to destroyKnownObjects(), I do NOT expect that code to delete or manipulate data belonging to other objects,
		// apart from the "theKnownObjects".


		//we need to update the mInRangeObjects too as the list is potentially old
		//and we might be on it
		
		// CODE REMOVED BY ERU
		/*
		ObjectSet* set =	targetPlayer->getController()->getInRangeObjects();
		ObjectSet::iterator osetsave = targetPlayer->getController()->getInRangeObjectsIterator();
		ObjectSet::iterator osetIt = set->find(this);
		
		
		if(osetIt != set->end())
		{
			if(osetsave == osetIt)
				osetsave++;

			 set->erase(this);

		}
		*/
	}
}

//=============================================================================

bool Object::isOwnedBy(PlayerObject* player)
{
	return ((mPrivateOwner == player->getId()) || (mPrivateOwner == player->getGroupId()));
}

