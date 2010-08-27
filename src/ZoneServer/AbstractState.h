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
#include "CreatureObject.h"
#include "CreatureEnums.h"

class IState
{
public:
	virtual ~IState(void){};	

	/* Activates the Enter process for the given state
	*  
	*/
	virtual bool Enter(CreatureObject* obj) = 0 ;
	/* Activates the Exit process for the given state
	*  
	*/
	virtual bool Exit(CreatureObject* obj) = 0;
	/* Determines if the player can transition to the state
	*  
	*/
	virtual bool CanTransition(CreatureObject* obj) = 0;
	/* sets the current state
	*  
	*/
	//virtual void setCurrentState(Object* obj) = 0;
	/* gets the ID for the given state
	*  
	*/
	virtual uint64 GetID(IState* state){return state->mStateID;}


	//virtual bool hidden(){return mHidden;}

protected:
	uint64					mStateID;
	bool					mHidden;
};
