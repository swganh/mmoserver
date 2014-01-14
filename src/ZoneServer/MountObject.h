/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_MOUNT_OBJECT_H
#define ANH_ZONESERVER_MOUNT_OBJECT_H

#include "Ham.h"
#include "MovingObject.h"
#include "SkillManager.h"
#include "CreatureEnums.h"
#include "CreatureObject.h"
#include <map>
#include <list>

//=============================================================================


//=============================================================================


class MountObject : public CreatureObject
{
	public:


		MountObject(){;}
		~MountObject(){;}

		virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void handleObjectMenuSelect(uint8 messageType,Object* srcObject);


		

	protected:


	public:

};

//=============================================================================

#endif
