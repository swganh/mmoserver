/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#include "PostureState.h"


PostureState::PostureState(void)
{
}


PostureState::~PostureState(void)
{
}

bool PostureState::Enter(CreatureObject* obj)
{
	return true;
}
bool PostureState::Exit(CreatureObject* obj)
{
	return true;
}
bool PostureState::CanTransition(CreatureObject* obj)
{
	return true;
}


bool PostureUpright::Enter(CreatureObject* obj)
{
	obj->setPosture(this->mStateID);
	obj->updateMovementProperties();

	PlayerObject*  player = dynamic_cast<PlayerObject*>(obj);

	if(player)
	{
		// see if we need to get out of sampling mode
		if(player->getSamplingState())
		{
			gMessageLib->SendSystemMessage(::common::OutOfBand("survey", "sample_cancel"), player);
			player->setSamplingState(false);
		}

		if(player->checkPlayerCustomFlag(PlayerCustomFlag_LogOut))
		{
			player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
			gMessageLib->SendSystemMessage(::common::OutOfBand("logout", "aborted"), player);	
		}

		player->toggleStateOff(CreatureState_SittingOnChair);

		player->setPosture(CreaturePosture_Upright);
		player->getHam()->updateRegenRates();
		player->updateMovementProperties();

		gMessageLib->sendUpdateMovementProperties(player);
		gMessageLib->sendPostureAndStateUpdate(player);
		gMessageLib->sendSelfPostureUpdate(player);

		//if player is seated on an a chair, hack-fix clientside bug by manually sending client message
		bool IsSeatedOnChair = player->checkState(CreatureState_SittingOnChair);
		if(IsSeatedOnChair)
		{
			gMessageLib->SendSystemMessage(::common::OutOfBand("shared", "player_stand"), player);	
		}
	}

	return true;
}
bool PostureUpright::Exit(CreatureObject* obj)
{
	return true;
}
bool PostureUpright::CanTransition(CreatureObject* obj)
{
	return true;
}