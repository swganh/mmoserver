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

#include "Object.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "DatabaseManager/Database.h"

#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace glm::gtx;

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
    mDirection = glm::quat();
    mPosition  = glm::vec3();

	mObjectController.setObject(this);
}

//=============================================================================

Object::Object(uint64 id,uint64 parentId,BString model,ObjectType type)
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

glm::vec3 Object::getWorldPosition() const 
{
    const Object* root_parent = getRootParent();

    // Is this object the root? If so it's position is the world position.
    if (this == root_parent) {
        return mPosition;
    }

    // Get the length of the object's vector inside the root parent (generally a building).
    float length = glm::length(mPosition);

    // Determine the translation angle.
    float theta = glm::angle(root_parent->mDirection) - glm::fastAtan(mPosition.x, mPosition.z);

    // Calculate and return the object's position relative to root parent's position in the world.
    return glm::vec3(
        root_parent->mPosition.x + (sin(theta) * length),
        root_parent->mPosition.y + mPosition.y,
        root_parent->mPosition.z - (cos(theta) * length)
        );				
}

//=============================================================================

// @TODO: This is a dependency on WorldManager that could be avoided by having an
//        Object instance hold a reference to it's parent.
// objects reference their parents - we just do not know who is the final (permissiongiving) container
// as it is it will return either the player or the building owning the item regardless in what container it is

const Object* Object::getRootParent() const 
{
    // If there's no parent id then this is the root object.
    if (! getParentId()) 
	{
        return this;
    }

    // Otherwise get the parent for this object and call getRootParent on it.
    Object* parent = gWorldManager->getObjectById(getParentId());
    assert(parent && "Unable to find root parent in WorldManager");

    return parent->getRootParent();
}

//=============================================================================

void Object::rotate(float degrees) {
    // Rotate the item left by the specified degrees
    mDirection = glm::rotate(mDirection, degrees, glm::vec3(0.0f, 1.0f, 0.0f));
}

//=============================================================================

void Object::rotateLeft(float degrees) {
    rotate(-degrees);
}

//=============================================================================

void Object::rotateRight(float degrees) {
    rotate(degrees);
}

//=============================================================================

void Object::faceObject(Object* target_object) {	
    facePosition(target_object->mPosition);
}

//=============================================================================

void Object::facePosition(const glm::vec3& target_position) {	
    // Create a mirror direction vector for the direction we want to face.
    glm::vec3 direction_vector = glm::normalize(target_position - mPosition);
    direction_vector.x = -direction_vector.x;

    // Create a lookat matrix from the direction vector and convert it to a quaternion.
    mDirection = glm::toQuat(glm::lookAt(
        direction_vector, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
        ));

    // If in the 3rd quadrant the signs need to be flipped.
    if (mDirection.y <= 0.0f && mDirection.w >= 0.0f) {
        mDirection.y = -mDirection.y;
        mDirection.w = -mDirection.w;
    }
}

//=============================================================================

void Object::move(const glm::quat& direction, float distance) {
    // Create a vector of the length we want pointing down the x-axis.
    glm::vec3 movement_vector(0.0f, 0.0f, distance);

    // Rotate the movement vector by the direction it should be facing.
    movement_vector = direction * movement_vector;

    // Add the movement vector to the current position to get the new position.
    mPosition += movement_vector;
}

//=============================================================================

void Object::moveForward(float distance) {
    move(mDirection, distance);
}

//=============================================================================

void Object::moveBack(float distance) {  
    move(mDirection, -distance);
}

//=============================================================================

float Object::rotation_angle() const {	
  glm::quat tmp = mDirection;

  if (tmp.y < 0.0f && tmp.w > 0.0f) {
    tmp.y *= -1;
    tmp.w *= -1;
  }

  return glm::angle(tmp);
}

//=============================================================================

bool Object::removeKnownObject(Object* object)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(this);
	if(player)
	{
		if(player->getTargetId() == object->getId())
			player->setTarget(0);
	}

	if(object->getType() == ObjType_Player)
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(object);
		PlayerObjectSet::iterator it = mKnownPlayers.find(player);

		if(it != mKnownPlayers.end())
		{
			//we might be its target
			if(player->getTargetId() == this->getId())
				player->setTarget(0);

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
	BString		value;

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

void Object::setAttribute(BString key,std::string value)
{
	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it == mAttributeMap.end())
	{
		gLogger->log(LogManager::DEBUG,"Object::setAttribute: could not find %s",key.getAnsi());
		return;
	}

	(*it).second = value;
}

//=========================================================================
//set the attribute and alter the db

void Object::setAttributeIncDB(BString key,std::string value)
{
	if(!hasAttribute(key))
	{
		addAttributeIncDB(key,value);
	}

	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it == mAttributeMap.end())
	{
		gLogger->log(LogManager::DEBUG,"Object::setAttribute: could not find %s",key.getAnsi());
		return;
	}

	(*it).second = value;

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->log(LogManager::DEBUG,"Object::addAttribute DB: no such attribute in the attribute table :%s",key.getAnsi());
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

void Object::addAttribute(BString key,std::string value)
{
	mAttributeMap.insert(std::make_pair(key.getCrc(),value));
	mAttributeOrderList.push_back(key.getCrc());
}

//=============================================================================
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addAttributeIncDB(BString key,std::string value)
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
		gLogger->log(LogManager::DEBUG,"Object::addAttribute DB: no such attribute in the attribute table :%s",key.getAnsi());
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

bool Object::hasAttribute(BString key) const
{
	if(mAttributeMap.find(key.getCrc()) != mAttributeMap.end())
		return(true);

	return(false);
}

//=============================================================================

void Object::removeAttribute(BString key)
{
	AttributeMap::iterator it = mAttributeMap.find(key.getCrc());

	if(it != mAttributeMap.end())
		mAttributeMap.erase(it);
	else
		gLogger->log(LogManager::DEBUG,"Object::removeAttribute: could not find %s",key.getAnsi());
}

//=========================================================================

//=========================================================================
//set the attribute and alter the db

void Object::setInternalAttributeIncDB(BString key,std::string value)
{
	if(!hasInternalAttribute(key))
	{
		addInternalAttributeIncDB(key,value);
	}

	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it == mInternalAttributeMap.end())
	{
		gLogger->log(LogManager::DEBUG,"Object::setAttribute: could not find %s",key.getAnsi());
		return;
	}

	(*it).second = value;

	uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
	if(!attributeID)
	{
		gLogger->log(LogManager::DEBUG,"Object::addAttribute DB: no such attribute in the attribute table :%s",key.getAnsi());
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

void	Object::setInternalAttribute(BString key,std::string value)
{
	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it == mInternalAttributeMap.end())
	{
		gLogger->log(LogManager::DEBUG,"Object::setInternalAttribute: could not find %s",key.getAnsi());
		return;
	}

	(*it).second = value;
}

//=============================================================================
//adds the attribute to the objects attribute list AND to the db - it needs a valid entry in the attribute table for that

void Object::addInternalAttributeIncDB(BString key,std::string value)
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
		gLogger->log(LogManager::DEBUG,"Object::addAttribute DB: no such attribute in the attribute table :%s",key.getAnsi());
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

void Object::addInternalAttribute(BString key,std::string value)
{
	mInternalAttributeMap.insert(std::make_pair(key.getCrc(),value));
}

//=============================================================================

bool Object::hasInternalAttribute(BString key)
{
	if(mInternalAttributeMap.find(key.getCrc()) != mInternalAttributeMap.end())
		return(true);

	return(false);
}

//=============================================================================

void Object::removeInternalAttribute(BString key)
{
	AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

	if(it != mInternalAttributeMap.end())
		mInternalAttributeMap.erase(it);
	else
		gLogger->log(LogManager::DEBUG,"Object::removeInternalAttribute: could not find %s",key.getAnsi());
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
		gLogger->log(LogManager::DEBUG,"Object::addKnownObject %I64u couldnt be added to %I64u - already in it", object->getId(), this->getId());
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

	/*
	ObjectIDSet::iterator IDIt				= mKnownObjectsIDs.begin();
	
	while(IDIt != mKnownObjectsIDs.end())
	{		
		Object* object = gWorldManager->getObjectById(*IDIt);
		if(!object)
		{
			(*IDIt)++;
		}
		ObjectIDSet::iterator itID = mKnownObjectsIDs.find(object->getId());

		if(itID != mKnownObjectsIDs.end())
		{
			mKnownObjectsIDs.erase(itID);
		}
		else
		{
			IDIt++;
		}		
	
	}
	*/
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

