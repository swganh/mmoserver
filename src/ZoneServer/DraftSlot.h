/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_DRAFTSLOT_H
#define ANH_ZONESERVER_DRAFTSLOT_H

#include "Utils/typedefs.h"


//=============================================================================

class DraftSlot
{
	friend class SchematicManager;

	public:

		DraftSlot();
		~DraftSlot();

		string	getComponentName(){ return mName; }
		void	setComponentName(string name){ mName = name; }
		string	getComponentFile(){ return mFile; }
		void	setComponentFile(string file){ mFile = file; }
		string	getResourceName(){ return mResourceName; }
		void	setResourceName(string res){ mResourceName = res; }
		uint32	getAmount(){ return mAmount; }
		void	setAmount(uint32 amount){ mAmount = amount; }
		uint8	getType(){ return mType; }
		void	setType(uint8 type){ mType = type; }
		uint8	getOptional(){ return mOptional; }
		void	setOptional(uint8 optional){ mOptional = optional; }
		

	private:

		string	mName;
		string	mFile;
		string	mResourceName;
		uint32	mAmount;
		//type gives the type of resource
		//2 is identical component
		//4 is resource
		//5 is similiar component
		uint8	mType;
		uint8	mOptional;
};

#endif


