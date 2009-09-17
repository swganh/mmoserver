/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "QuadTreeNode.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "MathLib/Rectangle.h"
#include "MathLib/Circle.h"

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

		mSubNodes[0] = new QuadTreeNode(mPosition.mX,mPosition.mZ + height,width,height);
		mSubNodes[1] = new QuadTreeNode(mPosition.mX + width,mPosition.mZ + height,width,height);
		mSubNodes[2] = new QuadTreeNode(mPosition.mX + width,mPosition.mZ,width,height);
		mSubNodes[3] = new QuadTreeNode(mPosition.mX,mPosition.mZ,width,height);
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
	assert(object);
	assert(object->getId());

	// gLogger->logMsgF("Trying to add Object %llu @ %.2f %.2f ", MSG_NORMAL, object->getId(), object->mPosition.mX, object->mPosition.mZ);		

	// its a leaf, add it
	if(!mSubNodes)
	{
		// make sure it doesn't already exists
		StdObjectMap::iterator it = mObjects.find(object->getId());

		if (it == mObjects.end())
		{
			mObjects.insert(std::make_pair(object->getId(),object));
			// gLogger->logMsgF("QuadTreeNode::addObject: INSERTED OBJECT with id = %llu", MSG_NORMAL, object->getId());	
		}
		else
		{
			gLogger->logMsgF("QuadTreeNode::addObject: INSERTED OBJECT already exist = %llu", MSG_NORMAL, object->getId());	
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
	assert(false);
	return(0);
}

//======================================================================================================================
//
// checks if an object belongs into this node
//

bool QuadTreeNode::checkBounds(Object* object)
{
	if(object->mPosition.mX >= mPosition.mX && object->mPosition.mX < mPosition.mX + mWidth
	&& object->mPosition.mZ >= mPosition.mZ && object->mPosition.mZ < mPosition.mZ + mHeight)
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
				Object* currentObject = gWorldManager->getObjectById((*it).first);
				if (currentObject)
				{
					assert(currentObject == (*it).second);
					// Object* currentObject = (*it).second;

					// don't add ourself
					if(currentObject != object && ((currentObject->getType() & typeMask) == currentObject->getType()))
					{
						// gLogger->logMsgF("QuadTreeNode::getObjectsInRange FINDING object with id = %llu", MSG_NORMAL, currentObject->getId());		
						resultSet->insert(currentObject);
					}
				}
				else
				{
					// The object is gone... we need figure out why it's not deleted properly.
					gLogger->logMsgF("QuadTreeNode::getObjectsInRange ERROR INVALID DATA and ID. ID = %llu", MSG_NORMAL, (*it).first);		
				}
			}
			else
			{
				gLogger->logMsgF("QuadTreeNode::getObjectsInRange ERROR INVALID ID\n", MSG_NORMAL);		
				assert(false);
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
			if(currentObject != object && ((currentObject->getType() & typeMask) == currentObject->getType()))
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
		Anh_Math::Vector3* rectPos = rectangle->getPosition();

		// check intersection
		if(rectPos->mX > mPosition.mX + mWidth  || rectPos->mX + rectangle->getWidth()  < mPosition.mX
		|| rectPos->mZ > mPosition.mZ + mHeight || rectPos->mZ + rectangle->getHeight() < mPosition.mZ)
		{
			return(false);
		}

		return(true);
	}
	// circle
	else if(Anh_Math::Circle* circle = dynamic_cast<Anh_Math::Circle*>(shape))
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
		Anh_Math::Vector3* rectPos = rectangle->getPosition();

		// check intersection
		if(rectPos->mX > object->mPosition.mX   || rectPos->mX + rectangle->getWidth()  < object->mPosition.mX
		|| rectPos->mZ > object->mPosition.mZ  || rectPos->mZ + rectangle->getHeight() < object->mPosition.mZ)
		{
			return(false);
		}

		return(true);
	}
	// circle
	else if(Anh_Math::Circle* circle = dynamic_cast<Anh_Math::Circle*>(shape))
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
	assert(object);
	assert(object->getId());

	// make sure its a leaf
	if(!mSubNodes)
	{
		// make sure it doesn't already exists
		StdObjectMap::iterator it = mObjects.find(object->getId());

		if (it != mObjects.end())
		{
			// gLogger->logMsgF("QuadTreeNode::removeObject REMOVE object with id = %llu", MSG_NORMAL, object->getId());		

			mObjects.erase(it);

			return(1);
		}
		gLogger->logMsgF("QuadTreeNode::removeObject ERROR FAILED to REMOVE object with id = %llu", MSG_NORMAL, object->getId());		
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
	assert(false);
	return(0);
}

//======================================================================================================================
//
// update an objects position in the tree, TODO: optimize
//

// int32 QuadTreeNode::addMyObject(Object* object);

int32 QuadTreeNode::updateObject(Object* object,Anh_Math::Vector3 newPosition)
{
	// Validate input. Should be interesting to see.
	assert(object);
	assert(object->getId());

	// shouldnt be called on leafs
	if(mSubNodes)
	{
		// gLogger->logMsgF("Remove Object %llu @ %.2f %.2f ", MSG_NORMAL, object->getId(), object->mPosition.mX, object->mPosition.mZ);		
		removeObject(object);
	
		object->mPosition = newPosition;

		// gLogger->logMsgF("Add Object %llu @ %.2f %.2f ", MSG_NORMAL, object->getId(), object->mPosition.mX, object->mPosition.mZ);		
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
			gLogger->logMsgF("QuadTreeNode::addMyObject: INSERTED OBJECT with id = %llu", MSG_NORMAL, object->getId());	
		}
		else
		{
			gLogger->logMsgF("QuadTreeNode::addMyObject: ERROR INSERTED OBJECT already exist = %llu", MSG_NORMAL, object->getId());	
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