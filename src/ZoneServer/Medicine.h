
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#pragma once
#include "Item.h"
class Medicine : public Item
{
	friend class ItemFactory;
public:
	virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
	virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
	void			handleStimpackMenuSelect(uint8 messageType, PlayerObject* player);
	bool			ConsumeUse(PlayerObject* playerObject);
	uint			getUsesRemaining();
	uint			getSkillRequired();
	uint			getHealthHeal();
	uint			getActionHeal();
	Medicine(void);
	~Medicine(void);
};
