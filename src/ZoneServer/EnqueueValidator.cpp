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

#include "EnqueueValidator.h"
#include "ObjectController.h"
#include "CreatureEnums.h"

EnqueueValidator::EnqueueValidator(ObjectController* controller)
: mController(controller)
{}

EnqueueValidator::~EnqueueValidator()
{}

uint32 EnqueueValidator::getLocoValidator(uint64 locomotion)
{
	// this is needed because of how SOE does their locomotion validation message
	uint32 locoValidator = 0;
	switch(locomotion)
	{
		case kLocomotionStanding: locoValidator = kLocoValidStanding; break;
		case kLocomotionSneaking: locoValidator = kLocoValidSneaking; break;
		case kLocomotionWalking: locoValidator = kLocoValidWalking; break;
		case kLocomotionRunning: locoValidator = kLocoValidRunning; break;
		case kLocomotionKneeling: locoValidator = kLocoValidKneeling; break;
		case kLocomotionCrouchSneaking: locoValidator = kLocoValidCrouchWalking; break;
		case kLocomotionCrouchWalking: locoValidator = kLocoValidProne; break;
		case kLocomotionProne: locoValidator = kLocoValidProne; break;
		case kLocomotionCrawling: locoValidator = kLocoValidCrawling; break;
		case kLocomotionClimbingStationary: locoValidator = kLocoValidClimbingStationary; break;
		case kLocomotionClimbing: locoValidator = kLocoValidClimbing; break;
		case kLocomotionHovering: locoValidator = kLocoValidHovering; break;
		case kLocomotionFlying: locoValidator = kLocoValidFlying; break;
		case kLocomotionLyingDown: locoValidator = kLocoValidLyingDown; break;
		case kLocomotionSitting: locoValidator = kLocoValidSitting; break;
		case kLocomotionSkillAnimating: locoValidator = kLocoValidSkillAnimating; break;
		case kLocomotionDrivingVehicle: locoValidator = kLocoValidDrivingVehicle; break;
		case kLocomotionRidingCreature: locoValidator = kLocoValidRidingCreature; break;
		case kLocomotionKnockedDown: locoValidator = kLocoValidKnockedDown; break;
		case kLocomotionIncapacitated: locoValidator = kLocoValidIncapacitated; break;
		case kLocomotionDead: locoValidator = kLocoValidDead; break;
		case kLocomotionBlocking: locoValidator = kLocoValidBlocking; break;
	}

	return locoValidator;
}