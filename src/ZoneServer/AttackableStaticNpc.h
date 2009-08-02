/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

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
		// void	filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player);
		// bool	preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player);
		// void	postProcessfilterConversation(ActiveConversation* av, ConversationPage* page, PlayerObject* player);
		void	setDeathEffect(uint32 effectId) {mDeathEffectId = effectId;}
		void	playDeathAnimation(void);

	private:
		uint32 mDeathEffectId;

};

//=============================================================================

#endif

