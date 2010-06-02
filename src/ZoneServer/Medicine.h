
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
	void			handleStimpackMenuSelect(uint8 messageType, PlayerObject* player, std::string medpackType);
	void			handleWoundPackMenuSelect(uint8 messageType, PlayerObject* player, std::string medpackType);
	bool			ConsumeUse(PlayerObject* playerObject);
	uint			getUsesRemaining();
	uint			getSkillRequired(string skill);
	uint			getHealthHeal();
	uint			getActionHeal();
	uint			getHealWoundAction();
	uint			getHealWoundConstitution();
	uint			getHealWoundHealth();
	uint			getHealWoundQuickness();
	uint			getHealWoundStamina();
	uint			getHealWoundStrength();
	uint			getHealWound(string attribute);

	Medicine(void);
	~Medicine(void);
};
