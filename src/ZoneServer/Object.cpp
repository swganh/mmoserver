/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Object.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "DatabaseManager/Database.h"


//=============================================================================

Object::Object()
: mModel("")
, mLoadState(LoadState_Loading)
, mId(0)
, mParentId(0)
, mPrivateOwner(0)
, mEquipSlots(0)
, mSubZoneId(0)
, mTypeOptions(0)
, mDataTransformCounter(0)
, mMovementMessageToggle(true)
{
	mDirection = Anh_Math::Quaternion();
	mPosition  = Anh_Math::Vector3();

	mObjectController.setObject(this);
}

//=============================================================================

Object::Object(uint64 id,uint64 parentId,string model,ObjectType type)
: mModel(model)
, mLoadState(LoadState_Loading)
, mType(type)
, mId(id)
, mParentId(parentId)
, mPrivateOwner(0)
, mEquipSlots(0)
, mSubZoneId(0)
, mTypeOptions(0)
, mDataTransformCounter(0)
, mMovementMessageToggle(true)
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
// returns true when item *is* found

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


string Object::getBazaarName()
{
    return "";
}


string Object::getBazaarTang()
{
    return "";
}


ObjectController* Object::getController()
{
    return &mObjectController;
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

//=========================================================================
//set the attribute and alter the db

void Object::setAttributeIncDB(string key,std::string value)
{
	if(!hasAttribute(key))
	{
		addAttributeIncDB(key,value);
	}

	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it == mAttributeMap.end())
	{
		gLogger->logMsgF("Object::setAttribute: could not find %s",MSG_HIGH,key.getAnsi());
		return;
	}

	(*it).second = value;

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->logMsgF("Object::addAttribute DB: no such attribute in the attribute table :%s",MSG_HIGH,key.getAnsi());
		return;
	}

	int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
	sprintf(sql,"UPDATE item_attributes SET value='");

	sqlPointer = sql + strlen(sql);
	sqlPointer += gWorldManager->getDatabase()->Escape_String(sqlPointer,value.c_str(),value.length());
	sprintf(restStr,"'WHERE item_id=%I64u AND attribute_id=%u",this->getId(),attributeID);
	strcat(sql,restStr);

	
	//sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%"PRIu64" AND attribute_id=%u",value,this->getId(),attributeID);
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,sql);
}
	

//=============================================================================
//adds the attribute to the objects attribute list

void Object::addAttribute(string key,std::string value)
{
	mAttributeMap.insert(std::make_pair(key.getCrc(),value));
	mAttributeOrderList.push_back(key.getCrc());
}

//=============================================================================
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addAttributeIncDB(string key,std::string value)
{
	if(hasAttribute(key))
	{
		setAttributeIncDB(key,value);
		return;
	}

	mAttributeMap.insert(std::make_pair(key.getCrc(),value));
	mAttributeOrderList.push_back(key.getCrc());

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->logMsgF("Object::addAttribute DB: no such attribute in the attribute table :%s",MSG_HIGH,key.getAnsi());
		return;
	}
	int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
	sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,'",this->getId(),attributeID);

	sqlPointer = sql + strlen(sql);
	sqlPointer += gWorldManager->getDatabase()->Escape_String(sqlPointer,value.c_str(),value.length());
	sprintf(restStr,"',%u,0)",static_cast<uint32>(this->getAttributeMap()->size()));
	strcat(sql,restStr);
	
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,sql);

	//sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,%s,%u,0)",this->getId(),attributeID,value,mAttributeOrderList.size());
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

//=========================================================================
//set the attribute and alter the db

void Object::setInternalAttributeIncDB(string key,std::string value)
{
	if(!hasInternalAttribute(key))
	{
		addInternalAttributeIncDB(key,value);
	}

	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it == mInternalAttributeMap.end())
	{
		gLogger->logMsgF("Object::setAttribute: could not find %s",MSG_HIGH,key.getAnsi());
		return;
	}

	(*it).second = value;

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->logMsgF("Object::addAttribute DB: no such attribute in the attribute table :%s",MSG_HIGH,key.getAnsi());
		return;
	}

	int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
	sprintf(sql,"UPDATE item_attributes SET value='");

	sqlPointer = sql + strlen(sql);
	sqlPointer += gWorldManager->getDatabase()->Escape_String(sqlPointer,value.c_str(),value.length());
	sprintf(restStr,"'WHERE item_id=%I64u AND attribute_id=%u",this->getId(),attributeID);
	strcat(sql,restStr);

	
	//sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%"PRIu64" AND attribute_id=%u",value,this->getId(),attributeID);
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,sql);
}

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
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addInternalAttributeIncDB(string key,std::string value)
{
	if(hasInternalAttribute(key))
	{
		setInternalAttributeIncDB(key,value);
		return;
	}

	mInternalAttributeMap.insert(std::make_pair(key.getCrc(),value));

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->logMsgF("Object::addAttribute DB: no such attribute in the attribute table :%s",MSG_HIGH,key.getAnsi());
		return;
	}
	int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
	sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,'",this->getId(),attributeID);

	sqlPointer = sql + strlen(sql);
	sqlPointer += gWorldManager->getDatabase()->Escape_String(sqlPointer,value.c_str(),value.length());
	sprintf(restStr,"',%u,0)",static_cast<uint32>(this->mInternalAttributeMap.size()));
	strcat(sql,restStr);
	
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,sql);

	//sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,%s,%u,0)",this->getId(),attributeID,value,mAttributeOrderList.size());
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
// known objects are those that are in the SI NEAR to our object and have been created
// all known objects that are NOT found in the next SI update will be destroyed as out of range

void Object::addKnownObject(Object* object)
{
	if(this->getId() == object->getId())
	{
		//we cannot (should not) add ourselves to our owm KnownObjectsList!!!!!
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

//=============================================================================

bool Object::isOwnedBy(PlayerObject* player)
{
	return ((mPrivateOwner == player->getId()) || (mPrivateOwner == player->getGroupId()));
}

//=============================================================================
//assign the ResourceContainer a new parentid and send a containment when a target is given
//

void Object::setParentId(uint64 parentId,uint32 contaiment, PlayerObject* target, bool db)
{
	mParentId = parentId; 
	if(db)
	{
		this->setParentIdIncDB(parentId);		
	}

	if(target)
		gMessageLib->sendContainmentMessage(this->getId(),this->getParentId(),contaiment,target);

}

void Object::setParentId(uint64 parentId,uint32 contaiment, PlayerObjectSet*	knownPlayers, bool db)
{
	mParentId = parentId; 
	if(db)
	{
		this->setParentIdIncDB(parentId);		
	}

	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		if(player)
			gMessageLib->sendContainmentMessage(this->getId(),this->getParentId(),contaiment,player);

		playerIt++;
	}

}

