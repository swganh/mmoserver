 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECTCONTAINER_H
#define ANH_ZONESERVER_OBJECTCONTAINER_H

#include "Utils/typedefs.h"
#include "Object_Enums.h"
#include "Object.h"
#include <vector>


//=============================================================================

typedef std::vector<Object*>			ObjectList;

//=============================================================================

/*
 - Base class for all containers / items that can be containers

 to offer methods to keep and organize a list of objects

*/


class ObjectContainer :	public Object
{
	

	public:

		ObjectContainer(uint64 id,uint64 parentId,const string model,ObjectType type);
		ObjectContainer();
		virtual ~ObjectContainer();

		ObjectList*		   getData() { return &mData; }
		Object*			   getDataById(uint64 id);
		bool		       addData(Object* Data);
		bool		       removeData(uint64 id);
		bool		       removeData(Object* Data);
		ObjectList::iterator removeData(ObjectList::iterator it);
		
		bool			   checkCapacity(){return(mCapacity > 0);}
		void			   setCapacity(uint64 cap){mCapacity = cap;}
		uint64			   getCapacity(){return mCapacity;}


private:



		ObjectList			mData;
		uint64				mCapacity;

		
		
};

//=============================================================================


#endif

