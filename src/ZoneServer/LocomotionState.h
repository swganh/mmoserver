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

#pragma once
#ifndef ANH_ZONESERVER_LOCOMOTION_STATE_H
#define ANH_ZONESERVER_LOCOMOTION_STATE_H

#include "ZoneServer/declspec.h"
#include "AbstractState.h"

class ZONE_API LocomotionState :
	public IState
{
public:
	LocomotionState(void);
	virtual ~LocomotionState(void);
	/* Activates the Enter process for the character Locomotion
	*  
	*/
	virtual bool Enter(CreatureObject* obj);
	/* Activates the Exit process for the given Locomotion
	*  
	*/
	virtual bool Exit(CreatureObject* obj);
	/* Determines if the player can transition to the Locomotion
	*  
	*/
	virtual bool CanTransition(CreatureObject* obj);
};
#endif
/* Standing Locomotion
*
*/
class ZONE_API LocomotionStanding:
	public LocomotionState
{
public:
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
///*	Sneaking Locomotion
//*
//*/
//class LocomotionSneaking:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Walking Locomotion
//*
//*/
//class LocomotionWalking:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Running Locomotion
//*
//*/
//class LocomotionRunning:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
//class LocomotionKneeling:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	CrouchSneaking Locomotion
//*
//*/
//class LocomotionCrouchSneaking:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	CrouchWalking Locomotion
//*
//*/
//class LocomotionCrouchWalking:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Prone Locomotion
//*
//*/
//class LocomotionProne:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Crawling Locomotion
//*
//*/
//class LocomotionCrawling:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	ClimbingStationary Locomotion
//*
//*/
//class LocomotionClimbingStationary:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Climbing Locomotion
//*
//*/
//class LocomotionClimbing:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Hovering Locomotion
//*
//*/
//class LocomotionHovering:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Flying Locomotion
//*
//*/
//class LocomotionFlying:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	LyingDown Locomotion
//*
//*/
//class LocomotionLyingDown:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Sitting Locomotion
//*
//*/
//class LocomotionSitting:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	SkillAnimating Locomotion
//*
//*/
//class LocomotionSkillAnimating:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	DrivingVehicle Locomotion
//*
//*/
//class LocomotionDrivingVehicle:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	RidingCreature Locomotion
//*
//*/
//class LocomotionRidingCreature:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	KnockedDown Locomotion
//*
//*/
//class LocomotionKnockedDown:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Incapacitated Locomotion
//*
//*/
//class LocomotionIncapacitated:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Dead Locomotion
//*
//*/
//class LocomotionDead:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Blocking Locomotion
//*
//*/
//class LocomotionBlocking:
//	public LocomotionState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};