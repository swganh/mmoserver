/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_SCHEMATICGROUP_H
#define ANH_ZONESERVER_SCHEMATICGROUP_H

#include "Utils/typedefs.h"
#include <vector>

class DraftSchematic;

typedef std::vector<DraftSchematic*>	SchematicsList;

//======================================================================

class SchematicGroup
{
	public:

		SchematicGroup();
		~SchematicGroup();

		uint32					mId;
		string					mName;
		SchematicsList			mSchematics;
};

#endif

