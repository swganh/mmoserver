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
#ifndef ANH_ZONESERVER_POSTURE_STATE_H
#define ANH_ZONESERVER_POSTURE_STATE_H

#include "ZoneServer/declspec.h"
#include "abstractstate.h"


class PostureState :
	public IState
{
public:
	PostureState(void);
	virtual ~PostureState(void);
	/* Activates the Enter process for the character Posture
	*  
	*/
	virtual bool Enter(CreatureObject* obj);
	/* Activates the Exit process for the given Posture
	*  
	*/
	virtual bool Exit(CreatureObject* obj);
	/* Determines if the player can transition to the Posture
	*  
	*/
	virtual bool CanTransition(CreatureObject* obj);
};
/*	Upright Posture
*
*/
class PostureUpright:
	public PostureState
{
public:
	virtual bool Enter(CreatureObject* obj);
	virtual bool Exit(CreatureObject* obj);
	virtual bool CanTransition(CreatureObject* obj);
};
#endif
///*	Crouched Posture
//*
//*/
//class PostureCrouched:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Prone Posture
//*
//*/
//class PostureProne:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Sneaking Posture
//*
//*/
//class PostureSneaking:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Blocking Posture
//*
//*/
//class PostureBlocking:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Climbing Posture
//*
//*/
//class PostureClimbing:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Flying Posture
//*
//*/
//class PostureFlying:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	LyingDown Posture
//*
//*/
//class PostureLyingDown:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Sitting Posture
//*
//*/
//class PostureSitting:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	SkillAnimating Posture
//*
//*/
//class PostureSkillAnimating:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	DrivingVehicle Posture
//*
//*/
//class PostureDrivingVehicle:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	RidingCreature Posture
//*
//*/
//class PostureRidingCreature:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	KnockedDown Posture
//*
//*/
//class PostureKnockedDown:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Incapacitated Posture
//*
//*/
//class PostureIncapacitated:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
///*	Dead Posture
//*
//*/
//class PostureDead:
//	public PostureState
//{
//public:
//	virtual bool Enter(Object* obj);
//	virtual bool Exit(Object* obj);
//	virtual bool CanTransition(Object* obj);
//};
