/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_GROUP_OBJECT_H
#define ANH_ZONESERVER_GROUP_OBJECT_H

#include "PlayerObject.h"




//=============================================================================

typedef std::vector<uint64>				Uint64List;



//=============================================================================

class GroupObject 
{

	public:

		GroupObject(uint64 id){mId = id;}
		~GroupObject(){}

		
		uint64				getId()	{ return mId;}
		void				setId(uint64 id){mId = id;}

		Uint64List*			getPlayerList()	{ return &mPlayerList;}
		void				addPlayer(uint64 id){ mPlayerList.push_back(id);}
		void				removePlayer(uint64 id)
							{
								Uint64List::iterator it = mPlayerList.begin();
								while(it != mPlayerList.end())
								{
									if((*it) == id)
										it = mPlayerList.erase(it);
									if(it != mPlayerList.end())
										it++;
										
								}
							}

	private:
		
		Uint64List		mPlayerList;
		uint64			mId;
};



//=============================================================================

#endif



