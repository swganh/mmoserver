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

#ifndef ANH_ZONESERVER_OBJECTCONTAINER_H
#define ANH_ZONESERVER_OBJECTCONTAINER_H

#include <vector>
#include "Object_Enums.h"
#include "Object.h"
#include "WorldConfig.h"
#include "WorldManager.h"
//#include "TangibleObject.h"


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


class ObjectContainer :	public Object
{
	
	friend class ItemFactory;
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
public:

	ObjectContainer(uint64 id,uint64 parentId,const BString model,ObjectType type);
	ObjectContainer();
	virtual ~ObjectContainer();

	//handles Object ready in case our item is in the container
	void				handleObjectReady(Object* object,DispatchClient* client);

	uint64				getObjectMainParent(Object* object);

	ObjectIDList*		getObjects() { return &mData; }
	Object*				getObjectById(uint64 id);
		
	/// =====================================================
	/// adds an Object to the cObjectContainer
	///	returns false if the container was full and the item not added
		
	bool				addObject(Object* data);
	bool				addObjectSecure(Object* data);
		
	bool				checkForObject(Object* object);
		
	bool				deleteObject(Object* data);
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
	
	//===========================================================================================
	//gets the contents of containers including their subcontainers
	uint16				getContentSize(uint16 iteration)
	{
		/*uint16 content = mData.size();

		if(iteration > gWorldConfig->getPlayerContainerDepth())
		{
			return content;
		}
			
		ObjectIDList*			ol = getObjects();
		ObjectIDList::iterator	it = ol->begin();

		while(it != ol->end())
		{
			ObjectContainer* tO = dynamic_cast<ObjectContainer*>(gWorldManager->getObjectById(*it));
			if(!tO)
			{
				assert(false);
			}

			content += tO->getContentSize(iteration+1);

			it++;
		}
		return content;*/
		return 1;
	}

private:

    ObjectIDList			mData;
    uint16					mCapacity;

};

//=============================================================================


#endif

