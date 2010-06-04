
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

#include <vector>
#include "Common/MessageDispatchCallback.h"

#define gMedicManager MedicManager::getSingletonPtr()

class CreatureObject;
class Database;
class Message;
class MessageDispatch;
class ObjectControllerCommandMap;
class ObjectControllerCmdProperties;
class PlayerObject;

class MedicManager
{
public:
	~MedicManager();

	static MedicManager*	getSingletonPtr() { return mSingleton; }
	static MedicManager*	Init(MessageDispatch* dispatch) 
	{
		if(!mInsFlag)
		{
			mSingleton = new MedicManager(dispatch);
			mInsFlag = true;
			return mSingleton;
		} else {
			return mSingleton;
		}
	}

	bool CheckMedicine(PlayerObject* Medic, PlayerObject* Target, ObjectControllerCmdProperties* cmdProperties, std::string Type);
	bool CheckMedicRange(PlayerObject* Medic, PlayerObject* Target, float healRange);
	
	bool HealDamage(PlayerObject* Medic, PlayerObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties, std::string healType);
	bool HealDamageRanged(PlayerObject* Medic, PlayerObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties);
	bool HealWound(PlayerObject* Medic, PlayerObject* Target, uint64 WoundPackobjectID, ObjectControllerCmdProperties* cmdProperties, std::string healType);

	void startInjuryTreatmentEvent(PlayerObject* Medic);
	void startQuickHealInjuryTreatmentEvent(PlayerObject* Medic);
	void startWoundTreatmentEvent(PlayerObject* Medic);
	bool Diagnose(PlayerObject* Medic, PlayerObject* Target);
	void successForage(PlayerObject* player);
	//helpers
	std::string handleMessage(Message* message, std::string regexPattern);
	int32  CalculateBF(PlayerObject* Medic, PlayerObject* Target, int32 maxhealamount);
	int32 CalculateHealWound(PlayerObject* Medic, PlayerObject* Target, int32 woundHealPower, std::string healType);



private:
	static MedicManager*	mSingleton;
	static bool				mInsFlag;
	MessageDispatch*		Dispatch;

	MedicManager(MessageDispatch* dispatch);	
};
