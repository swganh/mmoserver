/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
    BString		taunts[5];
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
    void	setupTutorialTaunts(uint64 playerId, uint64 baseTauntPeriod, BString taunt1, BString taunt2, BString taunt3, BString taunt4, BString taunt5);

private:
    void	addTutorialPlayer(uint64 playerId, TutorialTauntConfigData* configData);
    void	randomChatWithPlayer(PlayerObject* player, TutorialTauntConfigData* configData);

    TutorialPlayers		mTutorialPlayers;

};

//=============================================================================

#endif

