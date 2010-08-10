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

#include "QuadTreeNode.h"
#include "WorldManager.h"
#include "Common/LogManager.h"
#include "MathLib/Rectangle.h"
#include "MathLib/Circle.h"

#include <cassert>

//======================================================================================================================
//
// Constructor
//

QuadTreeNode::QuadTreeNode(float lowX,float lowZ,float width,float height) :
Rectangle(lowX,lowZ,width,height),mSubNodes(NULL)
{
}

//======================================================================================================================
//
// Deconstructor
//

QuadTreeNode::~QuadTreeNode()
{
	// if its a branch, free our children
	if(mSubNodes)
	{
		for(uint8 i = 0;i < 4;i++)
		{
			delete(mSubNodes[i]);
		}

		free(mSubNodes);
	}
}

//======================================================================================================================
//
// grow the tree by one level
//

void QuadTreeNode::subDivide()
{
	// this is a leaf, so make it a branch and grow 4 leafs
	if(!mSubNodes)
	{
		// make them a quarter size of their parent
		float width		= mWidth * 0.5f;
		float height	= mHeight * 0.5f;

		// create them
		mSubNodes = (QuadTreeNode**)::malloc(4 * sizeof(QuadTreeNode*));

		mSubNodes[0] = new QuadTreeNode(mPosition.x,mPosition.z + height,width,height);
		mSubNodes[1] = new QuadTreeNode(mPosition.x + width,mPosition.z + height,width,height);
		mSubNodes[2] = new QuadTreeNode(mPosition.x + width,mPosition.z,width,height);
		mSubNodes[3] = new QuadTreeNode(mPosition.x,mPosition.z,width,height);
	}
	// its a branch, so traverse its children
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			mSubNodes[i]->subDivide();
		}
	}
}

//======================================================================================================================
//
// insert an object
//

int32 QuadTreeNode::addObject(Object* object)
{
	// Validate input. Should be interesting to see.
	assert(object && "QuadTreeNode::addObject this method does not accept NULL objects");
	assert(object->getId() && "QuadTreeNode::addObject this method requires an object with a valid id");

	// gLogger->log(LogManager::DEBUG,"Trying to add Object %"PRIu64" @ %.2f %.2f ",  object->getId(), object->mPosition.x, object->mPosition.z);

	// its a leaf, add it
	if(!mSubNodes)
	{
		// make sure it doesn't already exists
		StdObjectMap::iterator it = mObjects.find(object->getId());

		if (it == mObjects.end())
		{
			mObjects.insert(std::make_pair(object->getId(),object));
			// gLogger->log(LogManager::DEBUG,"QuadTreeNode::addObject: INSERTED OBJECT with id = %"PRIu64"",  object->getId());
		}
		else
		{
			gLogger->log(LogManager::DEBUG,"QuadTreeNode::addObject: INSERTED OBJECT already exist = %"PRIu64"",  object->getId());
			return(2);
		}

		return(1);
	}
	// its a branch, see to which children it goes
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			// found the one it belongs in
			if(mSubNodes[i]->checkBounds(object))
			{
				// add it and break out
				mSubNodes[i]->addObject(object);

				return(0);
			}
		}
	}
	assert(false && "QuadTreeNode::addObject unable to add object to a node");
	return(0);
}

//======================================================================================================================
//
// checks if an object belongs into this node
//

bool QuadTreeNode::checkBounds(Object* object)
{
	if(object->mPosition.x >= mPosition.x && object->mPosition.x < mPosition.x + mWidth
	&& object->mPosition.z >= mPosition.z && object->mPosition.z < mPosition.z + mHeight)
	{
		return(true);
	}

	return(false);
}

//======================================================================================================================
//
// gather all objects in range of object(all objects from the intersecting leafs)
// given resultList as the visitor and a shape for intersection
//

void QuadTreeNode::getObjectsInRange(Object* object,ObjectSet* resultSet,uint32 typeMask,Anh_Math::Shape* shape)
{
	// this is a leaf,add the contents
	if(!mSubNodes)
	{
		StdObjectMap::iterator it = mObjects.begin();

		while(it != mObjects.end())
		{
			if ((*it).first)
			{
				Object* currentObject = (*it).second;
				
				// don't add ourself
				if(currentObject != object && ((currentObject->getType() & typeMask) == static_cast<uint32>(currentObject->getType())))
				{
					// gLogger->log(LogManager::DEBUG,"QuadTreeNode::getObjectsInRange FINDING object with id = %"PRIu64"",  currentObject->getId());
					resultSet->insert(currentObject);
				}					
			}
			else
			{
				gLogger->log(LogManager::CRITICAL,"QuadTreeNode::getObjectsInRange ERROR INVALID ID\n");
				assert(false && "QuadTreeNode::getObjectsInRange ERROR INVALID ID");
			}
			++it;
		}
	}
	// traverse the intersecting sub branches
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			if(mSubNodes[i]->intersects(shape))
			{
				mSubNodes[i]->getObjectsInRange(object,resultSet,typeMask,shape);
			}
		}
	}
}

//used by camps to get all contained objects out of the host node
void QuadTreeNode::getObjectsInRangeContains(Object* object,ObjectSet* resultSet,uint32 typeMask,Anh_Math::Shape* shape)
{
	// this is a leaf,add the contents
	if(!mSubNodes)
	{
		StdObjectMap::iterator it = mObjects.begin();

		while(it != mObjects.end())
		{
			Object* currentObject = (*it).second;

			// don't add ourself
			if(currentObject != object && ((currentObject->getType() & typeMask) == static_cast<uint32>(currentObject->getType())))
			{
				if(ObjectContained(shape,currentObject))
					resultSet->insert(currentObject);
			}

			++it;
		}
	}
	// traverse the intersecting sub branches
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			if(mSubNodes[i]->intersects(shape))
			{
				mSubNodes[i]->getObjectsInRangeContains(object,resultSet,typeMask,shape);
			}
		}
	}
}


//======================================================================================================================
//
// checks if a node intersects with a given region
//

bool QuadTreeNode::intersects(Anh_Math::Shape* shape)
{
	// rectangular
	if(Anh_Math::Rectangle* rectangle = dynamic_cast<Anh_Math::Rectangle*>(shape))
	{
		const glm::vec3& rectPos = rectangle->getPosition();

		// check intersection
		if(rectPos.x > mPosition.x + mWidth  || rectPos.x + rectangle->getWidth()  < mPosition.x
		|| rectPos.z > mPosition.z + mHeight || rectPos.z + rectangle->getHeight() < mPosition.z)
		{
			return(false);
		}

		return(true);
	}
	// circle
	else if(dynamic_cast<Anh_Math::Circle*>(shape))
	{
		// TODO
		return(false);
	}

	return(false);
}

bool QuadTreeNode::ObjectContained(Anh_Math::Shape* shape, Object* object)
{
	// rectangular
	if(Anh_Math::Rectangle* rectangle = dynamic_cast<Anh_Math::Rectangle*>(shape))
	{
		const glm::vec3& rectPos = rectangle->getPosition();

		// check intersection
		if(rectPos.x > object->mPosition.x   || rectPos.x + rectangle->getWidth()  < object->mPosition.x
		|| rectPos.z > object->mPosition.z  || rectPos.z + rectangle->getHeight() < object->mPosition.z)
		{
			return(false);
		}

		return(true);
	}
	// circle
	else if(dynamic_cast<Anh_Math::Circle*>(shape))
	{
		// TODO
		return(false);
	}

	return(false);
}


//======================================================================================================================
//
// removes an object
//

int32 QuadTreeNode::removeObject(Object* object)
{
	// Validate input. Should be interesting to see.
	assert(object && "QuadTreeNode::removeObject this method does not accept NULL objects");
	assert(object->getId() && "QuadTreeNode::removeObject this method requires an object with a valid id");

	// make sure its a leaf
	if(!mSubNodes)
	{
		// make sure it doesn't already exists
		StdObjectMap::iterator it = mObjects.find(object->getId());

		if (it != mObjects.end())
		{
			// gLogger->log(LogManager::DEBUG,"QuadTreeNode::removeObject REMOVE object with id = %"PRIu64"",  object->getId());

			mObjects.erase(it);

			return(1);
		}
		gLogger->log(LogManager::DEBUG,"QuadTreeNode::removeObject ERROR FAILED to REMOVE object with id = %"PRIu64"",  object->getId());
		return(2);
	}
	// traverse our children
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			// found the one it should be in
			if(mSubNodes[i]->checkBounds(object))
			{
				// remove it and break out
				mSubNodes[i]->removeObject(object);

				return(0);
			}
		}
	}
	assert(false && "QuadTreeNode::removeObject unable to remove object");
	return(0);
}

//======================================================================================================================
//
// update an objects position in the tree, TODO: optimize
//

// int32 QuadTreeNode::addMyObject(Object* object);

int32 QuadTreeNode::updateObject(Object* object, const glm::vec3& newPosition)
{
	// Validate input. Should be interesting to see.
	assert(object && "QuadTreeNode::updateObject this method does not accept NULL objects");
	assert(object->getId() && "QuadTreeNode::updateObject this method requires an object with a valid id");

	// shouldnt be called on leafs
	if(mSubNodes)
	{
		// gLogger->log(LogManager::DEBUG,"Remove Object %"PRIu64" @ %.2f %.2f ",  object->getId(), object->mPosition.x, object->mPosition.z);
		removeObject(object);

		object->mPosition = newPosition;

		// gLogger->log(LogManager::DEBUG,"Add Object %"PRIu64" @ %.2f %.2f ",  object->getId(), object->mPosition.x, object->mPosition.z);
		addObject(object);
		// addMyObject(object);
	}

	return(0);
}

//======================================================================================================================

//======================================================================================================================
//
// FOR TEST
// insert an object
//

/*
int32 QuadTreeNode::addMyObject(Object* object)
{
	// Validate input. Should be interesting to see.
	assert(object);
	assert(object->getId());

	// its a leaf, add it
	if(!mSubNodes)
	{
		// make sure it doesn't already exists
		StdObjectMap::iterator it = mObjects.find(object->getId());

		if (it == mObjects.end())
		{
			mObjects.insert(std::make_pair(object->getId(),object));
			gLogger->log(LogManager::DEBUG,"QuadTreeNode::addMyObject: INSERTED OBJECT with id = %"PRIu64"",  object->getId());
		}
		else
		{
			gLogger->log(LogManager::DEBUG,"QuadTreeNode::addMyObject: ERROR INSERTED OBJECT already exist = %"PRIu64"",  object->getId());
			return(2);
		}

		return(1);
	}
	// its a branch, see to which children it goes
	else
	{
		for(uint8 i = 0;i < 4;i++)
		{
			// found the one it belongs in
			if(mSubNodes[i]->checkBounds(object))
			{
				// add it and break out
				mSubNodes[i]->addMyObject(object);

				return(0);
			}
		}
	}
	assert(false);
	return(0);
}
*/
