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
#include "CraftingTool.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ContainerManager.h"
#include "ZoneOpcodes.h"
#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "DatabaseManager/Database.h"

#include <glm/gtx/transform2.hpp>

//=============================================================================

Object::Object()
    : mMovementMessageToggle(true)
	, mModel("")
    , mLoadState(LoadState_Loading)
    , mId(0)
    , mParentId(0)
    , mPrivateOwner(0)
    , mEquipSlots(0)
    , mSubZoneId(0)
    , mTypeOptions(0)
    , mDataTransformCounter(0)
{
    mDirection = glm::quat();
    mPosition  = glm::vec3();

    mObjectController.setObject(this);
}

//=============================================================================

Object::Object(uint64 id,uint64 parentId,BString model,ObjectType type)
    : mMovementMessageToggle(true)
    , mModel(model)
    , mLoadState(LoadState_Loading)
    , mType(type)
    , mId(id)
    , mParentId(parentId)
    , mPrivateOwner(0)
    , mEquipSlots(0)
    , mSubZoneId(0)
    , mTypeOptions(0)
    , mDataTransformCounter(0)
{
    mObjectController.setObject(this);

}

//=============================================================================

Object::~Object()
{
    mAttributeMap.clear();
	mInternalAttributeMap.clear();
	mAttributeOrderList.clear();

	ObjectIDList::iterator	objectIt = mData.begin();

	while(objectIt != mData.end())
	{
		//make sure we dont have invalid objects
		
	 	Object* object = gWorldManager->getObjectById((*objectIt));
		if(!object)
		{
			DLOG(INFO) << "ObjectContainer::remove Object : No Object!!!!";
			assert(false && "ObjectContainer::~ObjectContainer WorldManager unable to find object instance");
			objectIt = removeObject(objectIt);			
			continue;
		}

		//take care of a crafting tool
		if(CraftingTool* tool = dynamic_cast<CraftingTool*>(object))
		{
			if(tool->getCurrentItem())
			{
				gWorldManager->removeBusyCraftTool(tool);

			}
		}

		//just deinitialization/removal out of mainObjectMap .. NO removal out of si/cells
		gWorldManager->destroyObject(object);

		objectIt = removeObject(objectIt);
	}
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
    float theta = glm::angle(root_parent->mDirection) - glm::atan(mPosition.x, mPosition.z);

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
//  @TODO: what if the player is in a building ???
const Object* Object::getRootParent() const
{
    // If there's no parent id then this is the root object.
    if (! getParentId())
    {
        return this;
    }

    // Otherwise get the parent for this object and call getRootParent on it.
    Object* parent = gWorldManager->getObjectById(getParentId());
    
	if(!parent)
	{
		return this;
	}

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

BString Object::getBazaarName()
{
    return "";
}


BString Object::getBazaarTang()
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
        DLOG(INFO) << "Object::setAttribute: could not find " << key.getAnsi();
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
        DLOG(INFO) << "Object::setAttribute: could not find " << key.getAnsi();
        return;
    }

    (*it).second = value;

    uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
    if(!attributeID)
    {
        DLOG(INFO) << "Object::addAttribute DB: no such attribute in the attribute table :" << key.getAnsi();
        return;
    }

    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"UPDATE item_attributes SET value='");

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getDatabase()->escapeString(sqlPointer,value.c_str(),value.length());
    sprintf(restStr,"'WHERE item_id=%"PRIu64" AND attribute_id=%u",this->getId(),attributeID);
    strcat(sql,restStr);

    //sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%"PRIu64" AND attribute_id=%u",value,this->getId(),attributeID);
    gWorldManager->getDatabase()->executeSqlAsync(0,0,sql);

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
        DLOG(INFO) << "Object::addAttribute DB: no such attribute in the attribute table : " << key.getAnsi();
        return;
    }
    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,'",this->getId(),attributeID);

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getDatabase()->escapeString(sqlPointer,value.c_str(),value.length());
    sprintf(restStr,"',%u,0)",static_cast<uint32>(this->getAttributeMap()->size()));
    strcat(sql,restStr);

    gWorldManager->getDatabase()->executeSqlAsync(0,0,sql);

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
        DLOG(INFO) << "Object::removeAttribute: could not find " << key.getAnsi();
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
        DLOG(INFO) << "Object::setAttribute: could not find " << key.getAnsi();
        return;
    }

    (*it).second = value;

    uint32 attributeID = gWorldManager->getAttributeId(key.getCrc());
    if(!attributeID)
    {
        DLOG(INFO) << "Object::addAttribute DB: no such attribute in the attribute table :" << key.getAnsi();
        return;
    }

    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"UPDATE item_attributes SET value='");

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getDatabase()->escapeString(sqlPointer,value.c_str(),value.length());
    sprintf(restStr,"'WHERE item_id=%"PRIu64" AND attribute_id=%u",this->getId(),attributeID);
    strcat(sql,restStr);

    //sprintf(sql,"UPDATE item_attributes SET value='%s' WHERE item_id=%"PRIu64" AND attribute_id=%u",value,this->getId(),attributeID);
    gWorldManager->getDatabase()->executeSqlAsync(0,0,sql);
  
}

void	Object::setInternalAttribute(BString key,std::string value)
{
    AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

    if(it == mInternalAttributeMap.end())
    {
        DLOG(INFO) << "Object::setInternalAttribute: could not find " << key.getAnsi();
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
        DLOG(INFO) << "Object::addAttribute DB: no such attribute in the attribute table : " << key.getAnsi();
        return;
    }
    int8 sql[512],*sqlPointer,restStr[128];
//	int8 sql[1024]
    sprintf(sql,"INSERT INTO item_attributes VALUES(%"PRIu64",%u,'", this->getId(), attributeID);

    sqlPointer = sql + strlen(sql);
    sqlPointer += gWorldManager->getDatabase()->escapeString(sqlPointer, value.c_str(), value.length());
    sprintf(restStr,"',%u,0)",static_cast<uint32>(this->mInternalAttributeMap.size()));
    strcat(sql,restStr);

    gWorldManager->getDatabase()->executeSqlAsync(0, 0, sql);

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
        DLOG(INFO) << "Object::removeInternalAttribute: could not find " << key.getAnsi();
}




//=============================================================================

bool Object::isOwnedBy(PlayerObject* player)
{
	return ((mPrivateOwner == player->getId()) || (mPrivateOwner == player->getGroupId()));
}


bool Object::registerWatcher(Object* object)
{
	if(!checkContainerKnownObjects(object))
	{
		addContainerKnownObject(object);

        return(true);
    }

    return(false);
}

bool Object::registerStatic(Object* object)
{
	if(!checkStatics(object))
	{
		PlayerObject* player = dynamic_cast<PlayerObject*>(object);
		if(player)
		{
			mKnownStaticPlayers.insert(player);
		}
		else
			mKnownStatics.insert(object);

		return(true);
	}

	return(false);
}

//=============================================================================
// the knownObjects are the containers we are watching 
// the knownPlayers are the players watching us
// we*can* watch ourselves!! (when equipping/unequipping stuff for example)

void Object::addContainerKnownObject(Object* object)
{
	
	if(checkContainerKnownObjects(object))
	{
		DLOG(INFO) << "Object::addKnownObject " << object->getId() << " couldnt be added to " <<this->getId()<< " - already in it";
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

void Object::UnregisterAllWatchers()
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
		(*objIt)->unRegisterWatcher(this);
		mKnownObjects.erase(objIt++);
	}

	// players
	while(playerIt != mKnownPlayers.end())
	{			 
		PlayerObject* targetPlayer = (*playerIt);

		targetPlayer->unRegisterWatcher(this);
		mKnownPlayers.erase(playerIt++);

		
	}
}

//=============================================================================

bool Object::unRegisterWatcher(Object* object)
{
	DLOG(INFO) << "Object::unRegisterWatcher :: unregister " << object->getId() << " for " << this->getId();

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

			DLOG(INFO) << "Object::unRegisterWatcher :: " << object->getId() << " was successfully unregistered for " << this->getId();

			return(true);
		}
	}
	else
	{
		ObjectSet::iterator it = mKnownObjects.find(object);

		if(it != mKnownObjects.end())
		{
			mKnownObjects.erase(it);

			DLOG(INFO) << "Object::unRegisterWatcher :: " << object->getId() << " was successfully unregistered for " << this->getId();
			return(true);
		}
	}

	return(false);
}
//======================================================================================================================
//checks whether a player is registered as watcher be it over the si or constant
bool Object::checkRegisteredWatchers(PlayerObject* const player) const
{
	PlayerObjectSet::const_iterator it = mKnownPlayers.find(player);
	if(it == mKnownPlayers.end())
	{
		it = mKnownStaticPlayers.find(player);
		return (it != mKnownStaticPlayers.end());
	}
	return (it != mKnownPlayers.end());
}

bool Object::checkRegisteredWatchers(Object* const object) const
{
	ObjectSet::const_iterator it = mKnownObjects.find(object);
	if(it == mKnownObjects.end())
	{
		it = mKnownStatics.find(object);
		return (it != mKnownStatics.end());
	}
	return (it != mKnownObjects.end());
}


//=============================================================================
// checks whether an Object is registered as a static to us
bool Object::checkStatics(Object* object) const
{
	if(object->getType() == ObjType_Player)
	{
		PlayerObjectSet::const_iterator it = mKnownStaticPlayers.find(dynamic_cast<PlayerObject*>(object));
		return (it != mKnownStaticPlayers.end());
	}
	else
	{
		ObjectSet::iterator it = mKnownStatics.find(object);
		return (it != mKnownStatics.end());
	}
}

//=============================================================================
// returns true when item *is* found

bool Object::checkContainerKnownObjects(Object* object) const
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
//use for cells - players must enter them of course - it might be prudent to separate											
//players from items though
bool Object::addObjectSecure(Object* data) 
{ 
	
	if(hasObject(data->getId()))
	{
		assert(false);
		return false;
	}

	mData.push_back(data->getId()); 

	if(mCapacity)
	{
		return true;
	}
	else
	{
		DLOG(INFO) << "Object*::addObjectSecure No Capacity!!!!";
		return true;

	}
}

//===============================================
//use only when youre prepared to receive a false result with a not added item
//returns false when the item couldnt be added (container full)

bool Object::addObject(Object* data) 
{ 
	if(mCapacity)
	{
		mData.push_back(data->getId()); 
		return true;
	}
	else
	{
		DLOG(INFO) << "Object*::addObject No Capacity left for container ", this->getId();			
		return false;	
	}
}



//=============================================================================

Object* Object::getObjectById(uint64 id)
{
	ObjectIDList::iterator it = mData.begin();

	while(it != mData.end())
	{
		if((*it) == id) 
			return(gWorldManager->getObjectById((*it))); 

		++it;
	}
	DLOG(INFO) << "Object*::getDataById Data " << id << " not found";
	return NULL;
}

//=============================================================================
//just removes it out of the container - the object gets not deleted in the worldmanager
//
bool Object::removeObject(Object* data)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == data->getId())
		{
			it = mData.erase(it);
			return true;
		}
		++it;
	}
	DLOG(INFO) << "Object*::removeDataByPointer Data "<< data->getId() << " not found";
//	assert(false);
	return false;
}

//=============================================================================

bool Object::removeObject(uint64 id)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == id)
		{
			it = mData.erase(it);
			return true;
		}
		++it;
	}
	DLOG(INFO) << "Object*::removeDataById " << id <<" not found";
	return false;
}

//=============================================================================

ObjectIDList::iterator Object::removeObject(ObjectIDList::iterator it)
{
	it = mData.erase(it);
	return it;
}

//=============================================================================
// *this* is obviously a container that gets to hold the item we just created
// we need to create this item to registered players
// please note that the inventory and the datapad handle their own ObjectReady functions!!!!

void Object::handleObjectReady(Object* object,DispatchClient* client)
{
	
	//==========================
	// reminder: objects are owned by the global map, our item (container) only keeps references
	gWorldManager->addObject(object,true);

	//add it to the spatialIndex, too
	gSpatialIndexManager->createInWorld(object);

	this->addObject(object);
	
}



//=============================================================================================
// gets a headcount of all tangible (!!!) Objects in the container 
// including those contained in containers
uint16 Object::getHeadCount()
{
	uint16 count = 0;

	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		//do NOT count static tangibles like the playerStructureTerminal
		TangibleObject* to = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(*it));
		if(to && (!to->getStatic()))
		{
		 	count += to->getHeadCount();
			count += 1; //implememt items counting more than 1 at one time	
		}
		++it;
	}
	return count;

}
bool Object::checkCapacity(uint8 amount, PlayerObject* player)
{
	uint16 contentCount = getHeadCount();
	if(player&&(mCapacity-contentCount < amount))
	{
		gMessageLib->SendSystemMessage(L"",player,"container_error_message","container3");
	}

	return((mCapacity-contentCount) >= amount);
}


bool Object::hasObject(uint64 id)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == id)
		{
			return(true);
		}
		++it;
	}
	return(false);

}

bool Object::checkForObject(Object* object)
{
	ObjectIDList::iterator it = mData.begin();
	while(it != mData.end())
	{
		if((*it) == object->getId())
		{
			return(true);
		}
		++it;
	}
	return(false);
}
