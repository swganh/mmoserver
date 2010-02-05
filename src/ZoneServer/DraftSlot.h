/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_DRAFTSLOT_H
#define ANH_ZONESERVER_DRAFTSLOT_H

#include "Utils/typedefs.h"


//=============================================================================

enum DSType
{
	DST_Empty				= 0,	
	DST_IdentComponent		= 2,
	DST_Resource			= 4,
	DST_SimiliarComponent	= 5
};

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
		
		//the amount necessary to fill the slot
		uint32	getNecessaryAmount(){ return mAmount; }
		void	setNecessaryAmount(uint32 amount){ mAmount = amount; }
		
		//the type of res / component to fill
		uint8	getType(){ return mType; }
		void	setType(uint8 type){ mType = type; }
		
		uint8	getOptional(){ return mOptional; }
		void	setOptional(uint8 optional){ mOptional = optional; }
		

	private:

		string	mName;
		string	mFile;
		string	mResourceName;
		uint32	mAmount;
		
		uint8	mType;
		uint8	mOptional;
};

#endif


