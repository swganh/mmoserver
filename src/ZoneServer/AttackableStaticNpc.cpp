/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "AttackableStaticNpc.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

//=============================================================================

AttackableStaticNpc::AttackableStaticNpc() : NPCObject(), mDeathEffectId(144)
{
	mNpcFamily	= NpcFamily_AttackableObject;

	// Use default radial.
	mRadialMenu = RadialMenuPtr(new RadialMenu());
	// mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default); 
	// mRadialMenu->addItem(2,0,radId_examine,radAction_Default); 
}

//=============================================================================

AttackableStaticNpc::~AttackableStaticNpc()
{
	mRadialMenu.reset();
}

//=============================================================================

void AttackableStaticNpc::playDeathAnimation(void)
{
	if (mDeathEffectId != 0)
	{
		string effect = gWorldManager->getClientEffect(mDeathEffectId);
		gMessageLib->sendPlayClientEffectObjectMessage(effect,"",this);
	}
}

//=============================================================================

void AttackableStaticNpc::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	// gLogger->logMsgF("AttackableStaticNpc::prepareCustomRadialMenu Entering" ,MSG_NORMAL);

	// For test.
	// PlayerObject* playerObject = dynamic_cast<PlayerObject*>(creatureObject);
	// gMessageLib->sendUpdatePvpStatus(this, playerObject);

	mRadialMenu.reset();
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	if (this->checkPvPState(CreaturePvPStatus_Attackable))
	{
		gLogger->logMsgF("AttackableStaticNpc::prepareCustomRadialMenu IS attackable" ,MSG_NORMAL);

		// mRadialMenu = RadialMenuPtr(new RadialMenu());
		mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default); 
		mRadialMenu->addItem(2,0,radId_examine,radAction_Default); 
	}
	else
	{
		gLogger->logMsgF("AttackableStaticNpc::prepareCustomRadialMenu is NOT attackable" ,MSG_NORMAL);
		mRadialMenu->addItem(1,0,radId_examine,radAction_Default); 
	}
}

