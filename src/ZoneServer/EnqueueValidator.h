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

#ifndef ANH_ZONESERVER_ENQUEUE_VALIDATOR_H
#define ANH_ZONESERVER_ENQUEUE_VALIDATOR_H

#include "Utils/typedefs.h"

class ObjectController;
class ObjectControllerCmdProperties;

enum LocomotionValidator
{
	kLocoValidStanding							=	0,
	kLocoValidSneaking							=	1,
	kLocoValidWalking							=	2,
	kLocoValidRunning							=	3,
	kLocoValidKneeling							=	4,
	kLocoValidCrouchSneaking					=	5,
	kLocoValidCrouchWalking						=	6,
	kLocoValidProne								=	7,
	kLocoValidCrawling							=	8,
	kLocoValidClimbingStationary				=	9,
	kLocoValidClimbing							=	10,
	kLocoValidHovering							=	11,
	kLocoValidFlying							=	12,
	kLocoValidLyingDown							=	13,
	kLocoValidSitting							=	14,
	kLocoValidSkillAnimating					=	15,
	kLocoValidDrivingVehicle					=	16,
	kLocoValidRidingCreature					=	17,
	kLocoValidKnockedDown						=	18,
	kLocoValidIncapacitated						=	19,
	kLocoValidDead								=	20,
	kLocoValidBlocking							=	21,
};
//=============================================================================

//=======================================================================

class EnqueueValidator
{
public:
    explicit EnqueueValidator(ObjectController* controller);
    virtual ~EnqueueValidator();

    virtual bool validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties) = 0;

	uint32	inline getLowestCommonBit(uint64 playerMask, uint64 cmdPropertiesMask)
	{
		// checks each bit and returns the value
    	bool bFound = false;
    	uint32 i = 0;
    	for (; i < 64 && !bFound; ++i) {
    		bFound = (playerMask & (cmdPropertiesMask << i)) != 0;
    	}
    	if (bFound) {
    		return i;
    	}
    	return 0;
	}
	uint32 getLocoValidator(uint64 locomotion);

protected:
    ObjectController* mController;
};

//=======================================================================

#endif


