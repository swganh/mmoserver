/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_WEAPON_H
#define ANH_ZONESERVER_TANGIBLE_WEAPON_H

#include "Item.h"

//=============================================================================

enum WeaponGroup
{
	WeaponGroup_Unarmed			= 1,
	WeaponGroup_1h				= 2,
	WeaponGroup_2h				= 4,
	WeaponGroup_Polearm			= 8,
	WeaponGroup_Rifle			= 16,
	WeaponGroup_Pistol			= 32,
	WeaponGroup_Carbine			= 64,
	WeaponGroup_Flamethrower	= 128,
	WeaponGroup_Saber			= 256,
	WeaponGroup_Launcher		= 512
};

//=============================================================================

class Weapon : public Item
{
	friend class ItemFactory;

	public:

		Weapon();
		virtual ~Weapon();

		void	setGroup(uint32 group){ mGroup = group; }
		uint32	getGroup(){ return mGroup; }

		void	sendAttributes(PlayerObject* playerObject);

	private:

		uint32	mGroup;
		uint8	mDamageType;
};

//=============================================================================

#endif

