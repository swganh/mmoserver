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

#ifndef ANH_ZONESERVER_OBJECT_H
#define ANH_ZONESERVER_OBJECT_H

#include <vector>
#include <map>
#include <memory>
#include <set>
#include <list>

#include <boost/thread/mutex.hpp>
#include <boost/lexical_cast.hpp>

#include <tbb/atomic.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Utils/EventHandler.h"

#include "Common\Crc.h"
#include "Utils/typedefs.h"

#include "anh/logger.h"

#include "ZoneServer/ObjectController/ObjectController.h"
#include "ZoneServer/GameSystemManagers/Radial Manager/RadialMenu.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UICallback.h"
#include "ZoneServer/Objects/Object_Enums.h"

#include "anh/event_dispatcher/event_dispatcher.h"

//=============================================================================

class Object;
class PlayerObject;
class CreatureObject;

typedef std::map<uint32,std::string>	AttributeMap;
typedef std::shared_ptr<RadialMenu>	RadialMenuPtr;
// typedef std::vector<uint64>				ObjectIDList;
typedef std::list<uint64>				ObjectIDList;
typedef std::list<Object*>				ObjectList;
typedef std::set<Object*>				ObjectSet;
typedef std::set<uint64>				ObjectIDSet;
typedef std::set<PlayerObject*>			PlayerObjectSet;
typedef std::set<uint64>				PlayerObjectIDSet;
typedef std::list<uint32>				AttributeOrderList;
typedef std::set<uint64>				Uint64Set;
//=============================================================================

#define DISPATCH(BIG, LITTLE) if(auto dispatcher = GetEventDispatcher()) \
{dispatcher->Dispatch(std::make_shared<BIG ## Event>(#BIG "::" #LITTLE, std::static_pointer_cast<BIG>(shared_from_this())));}

typedef swganh::event_dispatcher::ValueEvent<Object*> ObjectEvent;

/*
 - Base class for all gameobjects
 */

class Object : public UICallback, public Anh_Utils::EventHandler, public ObjectFactoryCallback, public std::enable_shared_from_this<Object>
{
	friend class PlayerObjectFactory;
	friend class InventoryFactory;
	friend class NonPersistentItemFactory;
	friend class ItemFactory;

public:

	Object();
	Object(uint64 id,uint64 parentId,const BString model,ObjectType type);

	ObjectLoadState				getLoadState(){ return mLoadState; }
	void						setLoadState(ObjectLoadState state){ mLoadState = state; }

	uint64						getId() const { return mId; }
	void						setId(uint64 id){ mId = id; }

	uint64						getParentId() const { return mParentId; }
	void						setParentId(uint64 parentId){ mParentId = parentId; }
		
	swganh::event_dispatcher::EventDispatcher* Object::GetEventDispatcher();
	
	//=============================================================================
	//just sets a new ParentID and sends Containment to TargetObject
    virtual void				setParentIdIncDB(uint64 parentId) {
        mParentId = parentId;
    }		
		
	BString						getModelString(){ return mModel; }
	
	/*@brief	this will get the old fashioned Object type 
	*			phase out and use the 
	*/
	ObjectType					getType() const { return mType; }
	uint32						getTypeOptions() const { return mTypeOptions; }
	
	/*@brief	this will get the Objects type-CRC (YALP OERC etc)
	*			use instead of ObjectType
	*/
	CRC_Type					getObjectType() const { return object_type_; }
		
	void						setModelString(const BString model){ mModel = model; }
	void						setType(ObjectType type){ mType = type; }
	void						setTypeOptions(uint32 options){ mTypeOptions = options; }


	RadialMenuPtr				getRadialMenu(){ return mRadialMenu; }
    virtual void				ResetRadialMenu() {}//	RadialMenu* radial	= NULL;RadialMenuPtr radialPtr(radial);	mRadialMenu = radialPtr;}

	//clientprefab Menu List
	MenuItemList*				getMenuList(){ return mMenuItemList; }
	void						setMenuList(MenuItemList* list){ mMenuItemList = list; }

	virtual void				handleUIEvent(uint32 action,int32 element,BString inputStr = "",UIWindow* window = nullptr, std::shared_ptr<WindowAsyncContainerCommand> AsyncContainer = nullptr) {}

	virtual void				prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount){}
	virtual void				prepareCustomRadialMenuInCell(CreatureObject* creatureObject, uint8 itemCount){}
	virtual	void				handleObjectMenuSelect(uint8 messageType,Object* srcObject){}

	virtual void				updateWorldPosition(){}

	virtual void				sendAttributes(PlayerObject* playerObject);
	virtual BString				getBazaarName();
	virtual BString				getBazaarTang();

	ObjectController*			getController();

	// common attributes, send to the client
	AttributeMap*				getAttributeMap(){ return &mAttributeMap; }
	//template<typename T> T		getAttribute(BString key) const;
	template<typename T> T		getAttribute(std::string) const;
	template<typename T> T		getAttribute(uint32 keyCrc) const;
	void						setAttribute(std::string key,std::string value);

	void						setAttributeIncDB(std::string key,std::string value);
	void						addAttribute(std::string key,std::string value);
	void						addAttributeIncDB(std::string key,std::string value);
	bool						hasAttribute(std::string key) const;
	void						removeAttribute(std::string key);
	AttributeOrderList*			getAttributeOrder(){ return &mAttributeOrderList; }

	// internal attributes, only used server side
	AttributeMap*				getInternalAttributeMap(){ return &mInternalAttributeMap; }
	template<typename T> T		getInternalAttribute(BString key);
	void						setInternalAttribute(BString key,std::string value);
	void						addInternalAttribute(BString key,std::string value);
	void						setInternalAttributeIncDB(BString key,std::string value);
	void						addInternalAttributeIncDB(BString key,std::string value);
	bool						hasInternalAttribute(BString key);
	void						removeInternalAttribute(BString key);

	// subzone this is used by the client to identify subzones
	uint32						getSubZoneId() const { return mSubZoneId; }
	void						setSubZoneId(uint32 id){ mSubZoneId = id; }

	uint32						getGridBucket() const { return zmapCellID; }
	void						setGridBucket(uint32 id){ zmapCellID = id; }

	//===========================================================================
	// equip management
		
	//equip slots set the equipmanagerslots an item occupies when equipped
	uint64						getEquipSlotMask(){ return mEquipSlots; }
	void						setEquipSlotMask(uint64 slotMask){ mEquipSlots = slotMask; }
		
	//equip restrictions are the equipmanagers restrictions based on race or gender
	uint64						getEquipRestrictions(){ return mEquipRestrictions; }
	void						setEquipRestrictions(uint64 restrictions){ mEquipRestrictions = restrictions; }

	uint32						getDataTransformCounter(){ return mDataTransformCounter; }
	uint32						incDataTransformCounter(){ return ++mDataTransformCounter; }
	void						setDataTransformCounter(uint32 restrictions){ mDataTransformCounter= restrictions; }

	/*@brief getRegisteredWatchers() gets all watchers registered to us
	*
	*/
	PlayerObjectSet*		    getRegisteredWatchers() { return &mKnownPlayers; }

	void						UnregisterAllWatchers();
	
	/*@brief checks whether a player is a registered watcher of ours static or otherwise
	*/
	bool						checkRegisteredWatchers(PlayerObject* const player) const;
	
	/*@brief	registers a Watcher (Player) to a container in case it isnt already registered
	*/
	bool						registerWatcher(PlayerObject* const player);
	
	/*@brief a watcher gets unregistered
	*/
	bool						unRegisterWatcher(PlayerObject* object);
	
	//virtual void				addContainerKnownObject(Object* object);

	/*
	*static knownObject is the inventory, the hair, etc - these dont need to be altered by the SI
	*/
	bool						checkStatics(Object* object) const;
	PlayerObjectSet*		    getRegisteredStaticWatchers() { return &mKnownStaticPlayers; }
	ObjectSet*					getStatitcs() { return &mKnownStatics; }
	bool						registerStatic(Object* object);
		

		
	virtual ~Object();

    /*! Retrieve the world position of an object. Important for ranged lookups that need
        *  to include objects inside and outside of buildings.
        *
        * \returns glm::vec3 The world position of an object.
        */
    glm::vec3 getWorldPosition() const;

    /*! Returns the current object's root (permission giving) parent. If the object is the root it returns itself.
        *
        * \returns const Object* Root parent for the current object.
        */
    const Object* getRootParent() const;
        

    /*! Rotates an object by the specified degrees.
        *
        * \param degrees The degree of rotation.
        */
    void rotate(float degrees);


    /*! Rotates an object left by the specified degrees.
        *
        * \param degrees The degree of rotation.
        */
    void rotateLeft(float degrees);
        
    /*! Rotates an object right by the specified degrees.
        *
        * \param degrees The degree of rotation.
        */
    void rotateRight(float degrees);        
		
	/*! Orients the current object so that it faces the object passed in.
		*
		* \param target_object The object the current object should face.
		*/
	void faceObject(Object* target_object);
		
	/*! Orients the current object so that it faces the position passed in.
		*
		* \param target_position The position the current object should face.
		*/
	void facePosition(const glm::vec3& target_position);
               
    /*! Moves an object along a directional facing by a certain distance.
        *
        * \param direction The direction to consider as the front facing
        * \param distance The distance to move (measured in meters).
        */
    void move(const glm::quat& direction, float distance);
                
    /*! Moves an object forward along it's own directional facing by a certain distance.
        *
        * \param distance The distance to move (measured in meters).
        */
    void moveForward(float distance);
        
    /*! Moves an object back along it's own directional facing by a certain distance.
        *
        * \param distance The distance to move (measured in meters).
        */
    void moveBack(float distance);


    /*! Determines the angle used by update transform messages for rotation.
        *
        * \returns Current rotation angle.
        */
    float rotation_angle() const;

    glm::quat   mDirection;
    glm::vec3   mPosition;
	//Anh_Math::Quaternion	mDirection;
	//Anh_Math::Vector3		mPosition;

	inline uint64			getPrivateOwner() { return mPrivateOwner; }
	inline void				setPrivateOwner(uint64 owner) { mPrivateOwner = owner; }
	bool					isOwnedBy(PlayerObject* player);
	const glm::vec3&		getLastUpdatePosition(){ return mLastUpdatePosition; }
	void					setLastUpdatePosition(const glm::vec3& pos ){mLastUpdatePosition = pos; }


	bool					movementMessageToggle(){mMovementMessageToggle = !mMovementMessageToggle; return mMovementMessageToggle;}


	//Set of Subcells we're in.
	Uint64Set				zmapSubCells;

	//handles Object ready in case our item is in the container
	void				handleObjectReady(Object* object,DispatchClient* client);

	/*@brief gets a reference to the List containing all the Containers Child Items
	*should be renamed to getContainerContent
	*/
	ObjectIDList*		getObjects() { return &mData; }

	ObjectIDList		getContainerContentCopy() { return mData; }

	Object*				getObjectById(uint64 id);
		
	/// =====================================================
	/// adds an Object to the ObjectContainer
	///	returns false if the container was full and the item not added
		
	bool				addObject(Object* data);
	bool				addObjectSecure(Object* data);
		
	bool				checkForObject(Object* object);
	bool				hasObject(uint64 id);
		
	bool				removeObject(uint64 id);
	bool				removeObject(Object* Data);
	ObjectIDList::iterator removeObject(ObjectIDList::iterator it);
		
		
	//we need to check the content of our children, too!!!!
	virtual bool		checkCapacity(){return((mCapacity-mData.size()) > 0);}
	virtual bool		checkCapacity(uint8 amount, PlayerObject* player = NULL);
	void				setCapacity(uint16 cap){mCapacity = cap;}
	uint16				getCapacity(){return mCapacity;}
	uint16				getHeadCount();


		

//		void						clearKnownObjects(){ mKnownObjects.clear(); mKnownPlayers.clear(); }
//		ObjectSet*					getContainerKnownObjects() { return &mKnownObjects; }
	
	/*	@brief	gets the Custom (non stf) name of an Object
	*			For players the Custom Name will be build out of first_name " " and last_name
	*			For other Objects it will be directly in the db / or set via appropriate functions
	*/
	std::u16string		getCustomName() const { auto lock = AcquireLock(); return getCustomName(lock); }
	std::u16string		getCustomName(boost::unique_lock<boost::mutex>& lock) const { return custom_name_; }

	/*	@brief	sets the Custom (non stf) name of an Object
	*			For players the Custom Name will be build out of first_name " " and last_name
	*			For other Objects it will be set directly for example on crafting or when placing a structure
	*/
    void				setCustomName(std::u16string name){ auto lock = AcquireLock(); setCustomName(lock, name); }
	void				setCustomName(boost::unique_lock<boost::mutex>& lock, std::u16string name);
	
	
	boost::unique_lock<boost::mutex> AcquireLock() const
	{
		return boost::unique_lock<boost::mutex>(object_mutex_);
	}

protected:

	mutable boost::mutex		object_mutex_;	

	ObjectIDList				mData;
	uint16						mCapacity;

	bool						mMovementMessageToggle;
	AttributeMap				mAttributeMap;
	AttributeOrderList			mAttributeOrderList;
	AttributeMap 				mInternalAttributeMap;
	ObjectIDSet					mKnownObjectsIDs;
	ObjectController			mObjectController;
	BString						mModel;

	std::u16string			custom_name_;

	MenuItemList*			mMenuItemList;
	RadialMenuPtr			mRadialMenu;

	ObjectLoadState			mLoadState;
	ObjectType				mType;
	CRC_Type				object_type_;

	uint64					mId;
	uint64					mParentId;
		
	// If object is used as a private object in an Instance, this references the instances (objects) owner
	uint64					mPrivateOwner; 
	uint64					mEquipRestrictions;
	uint64					mEquipSlots;
	uint32					mInMoveCount;
	uint32					mSubZoneId;
	uint32					mTypeOptions;
	uint32					mDataTransformCounter;
	uint32					zmapCellID;
private:
	glm::vec3		        mLastUpdatePosition;	// Position where SI was updated.

	//registered Players that are watching us
	PlayerObjectSet				mKnownPlayers;

	//registered Objects out of the SI
	ObjectSet					mKnownStatics;
	PlayerObjectSet				mKnownStaticPlayers;

};

//=============================================================================

/*template<typename T>
T	Object::getAttribute(BString key) const
{
    AttributeMap::const_iterator it = mAttributeMap.find(key.getCrc());

    if(it != mAttributeMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            DLOG(info) << "Object::getAttribute: cast failed " << key.getAnsi();
        }
    }
    else
        DLOG(info) << "Object::getAttribute: could not find " << key.getAnsi();

    return(T());
}
*/
//=============================================================================

template<typename T>
T	Object::getAttribute(std::string key) const
{
	AttributeMap::const_iterator it = mAttributeMap.find(common::memcrc(key));

	if(it != mAttributeMap.end())
	{
		try
		{
			return(boost::lexical_cast<T>((*it).second));
		}
		catch(boost::bad_lexical_cast &)
		{
			DLOG(info) << "Object::getAttribute: cast failed " << key;
		}
	}
	else
		DLOG(info) << "Object::getAttribute: could not find " << key;

	return(T());
}

//=============================================================================

template<typename T>
T	Object::getAttribute(uint32 keyCrc) const
{
    AttributeMap::iterator it = mAttributeMap.find(keyCrc);

    if(it != mAttributeMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            DLOG(info) << "Object::getAttribute: cast failed " << keyCrc;
        }
    }
    else
        DLOG(info) << "Object::getAttribute: could not find " << keyCrc;

    return(T());
}


//=============================================================================

template<typename T>
T	Object::getInternalAttribute(BString key)
{
    AttributeMap::iterator it = mInternalAttributeMap.find(key.getCrc());

    if(it != mInternalAttributeMap.end())
    {
        try
        {
            return(boost::lexical_cast<T>((*it).second));
        }
        catch(boost::bad_lexical_cast &)
        {
            DLOG(info) << "Object::getInternalAttribute: cast failed " << key.getAnsi();
        }
    }
    else
        DLOG(info) << "Object::getInternalAttribute: could not find " << key.getAnsi();

    return(T());
}

//=============================================================================

#endif

