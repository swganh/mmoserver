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
