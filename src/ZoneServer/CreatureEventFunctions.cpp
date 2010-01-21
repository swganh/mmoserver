/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "CreatureObject.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "MathLib/Quaternion.h"

//=============================================================================
//
// incap recovery
//

void CreatureObject::onIncapRecovery(const IncapRecoveryEvent* event)
{
	// gLogger->logMsg("CreatureObject::onIncapRecovery");

	if (mPosture == CreaturePosture_Dead)
	{
		// Forget it, you are dead!
		return;
	}

	if (this->getType() == ObjType_Player)
	{
		mCurrentIncapTime = 0;
		gMessageLib->sendIncapTimerUpdate(this);

		// update the posture
		mPosture = CreaturePosture_Upright;

		// reset ham regeneration
		mHam.updateRegenRates();
		if (mHam.getTaskId() == 0)
		{
			mHam.setTaskId(gWorldManager->addCreatureHamToProccess(&mHam));
		}

		updateMovementProperties();

		gMessageLib->sendPostureAndStateUpdate(this);

		if(PlayerObject* player = dynamic_cast<PlayerObject*>(this))
		{
			gMessageLib->sendUpdateMovementProperties(player);
			gMessageLib->sendSelfPostureUpdate(player);
		}
	}
	else if (this->getType() == ObjType_Creature)
	{
		// Placeholder for debug purpose only.
	}
	
}

//=============================================================================


