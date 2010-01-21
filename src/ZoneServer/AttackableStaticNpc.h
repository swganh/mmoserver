/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ATTACKABLESTATICNPC_H
#define ANH_ZONESERVER_ATTACKABLESTATICNPC_H

#include "NPCObject.h"

//=============================================================================

class AttackableStaticNpc : public NPCObject
{
	friend class PersistentNpcFactory;
	friend class NonPersistentNpcFactory;

	public:

		AttackableStaticNpc();
		virtual ~AttackableStaticNpc();

		virtual void prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void respawn(void);

		void	setDeathEffect(uint32 effectId) {mDeathEffectId = effectId;}
		void	playDeathAnimation(void);

	private:
		void	spawn(void);

		uint32 mDeathEffectId;

};

//=============================================================================

#endif

