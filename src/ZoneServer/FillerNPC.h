/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_FILLERNPC_H
#define ANH_ZONESERVER_FILLERNPC_H

#include "NPCObject.h"


//=============================================================================

// This is so very special, that I can't find word....
// But bare with me... there is a plan to have a "description block" that tells what a npc is setup for, like combat, taunts in Tutorial, quests or whatever.
// Right now I'm playing with the lower level build stones.

class	TutorialTauntConfigData
{
	public:
		int64		when;
		uint64		tauntBasePeriodTime;
		string		taunts[5];
};

typedef std::map<uint64, TutorialTauntConfigData*>	TutorialPlayers;

class FillerNPC : public NPCObject
{
	friend class PersistentNpcFactory;
	friend class NonPersistentNpcFactory;

	public:

		FillerNPC();
		~FillerNPC();

		void	filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player);
		bool	preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player);
		void	postProcessfilterConversation(ActiveConversation* av, ConversationPage* page, PlayerObject* player);

		virtual void	handleEvents(void);
		virtual uint64	handleState(uint64 timeOverdue);



		void	removeTutorialPlayer(uint64 playerId);
		void	setupTutorialTaunts(uint64 playerId, uint64 baseTauntPeriod, string taunt1, string taunt2, string taunt3, string taunt4, string taunt5);

	private:
		void	addTutorialPlayer(uint64 playerId, TutorialTauntConfigData* configData);
		void	randomChatWithPlayer(PlayerObject* player, TutorialTauntConfigData* configData);
		
		TutorialPlayers		mTutorialPlayers;

};

//=============================================================================

#endif

