/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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

#include "ZoneServer\Services\ham\ham_service.h"

#include <boost/asio/placeholders.hpp>

#include <map>

#include <ZoneServer\Services\scene_events.h>
#include <ZoneServer\Worldmanager.h>
//#include "swganh_core/simulation/scene_events.h"

#include <anh/event_dispatcher/event_dispatcher.h>
#include <anh/logger.h>

#include "ZoneServer\Objects\Creature Object\CreatureObject.h"

using namespace swganh::ham;
using namespace swganh::tre;
using namespace boost::posix_time;
using namespace swganh::event_dispatcher;

HamService::HamService(swganh::app::SwganhKernel* kernel) : kernel_(kernel) , timer_(kernel->GetCpuThreadPool())
{
	LOG(error) << "HamService::HamService register events";
	
    SetServiceDescription(swganh::service::ServiceDescription(
                              "Ham Service",
                              "ham",
                              "0.1",
                              "127.0.0.1",
                              0,
                              0,
                              0));

	//Start the timer
    timer_.expires_from_now(boost::posix_time::seconds(1));
    timer_.async_wait(boost::bind(&HamService::handleTick_, this, boost::asio::placeholders::error));
}

HamService::~HamService()
{}

void HamService::Initialize()
{}

void	HamService::SetHam(CreatureObject* creature, const uint16_t statIndex, const int32_t value)
{
	//NoNonsense checks
	//do NEVER send updates without a change - the clients list will desynchronize and refuse further updates
	//optimally a creature where this method is used isnt part of the simulation

	if(creature->GetStatBase(statIndex) != value)
		creature->SetStatBase(statIndex, value);

	if(creature->GetStatMax(statIndex) != value)
		creature->SetStatMax(statIndex, value);

	if(creature->GetStatCurrent(statIndex) != value)
		creature->SetStatCurrent(statIndex, value);

	if(creature->GetStatEncumberance(statIndex) != 0)
		creature->SetStatEncumberance(statIndex, 0);

	if(creature->GetStatWound(statIndex) != 0)
		creature->SetStatWound(statIndex, 0);
}

int32_t	HamService::ModifyMaxHam(CreatureObject* creature, const uint16_t statIndex, const int32_t update)
{
	LOG(info) << "HamService::ModifyMaxHam : update : " << update;
	
	//make sure we dont get negative hitpoints
	int32_t update_calculated = update;
    int32_t current_value = creature->GetStatCurrent(statIndex); 

    if((current_value + update) < 1)
        update_calculated = 1 - current_value;

   LOG(info) << "HamService::ModifyMaxHam : update calculated : " << update_calculated;
	creature->AddStatMax(statIndex, update_calculated);
	creature->AddStatCurrent(statIndex, update_calculated);
    
    return update_calculated;
}

bool HamService::checkMainPools(CreatureObject* creature, const int32 health,const int32 action, const int32 mind)
{
	if(creature->GetStatCurrent(HamBar_Health) < health || creature->GetStatCurrent(HamBar_Action) < action || creature->GetStatCurrent(HamBar_Mind) < mind )
		return false;
	return true;
}

bool HamService::checkMainPool(CreatureObject* creature, const uint16_t statIndex, const int32 value)
{
	if(creature->GetStatCurrent(statIndex) < value)
		return false;
	return true;
}

int32_t HamService::RemoveWound(CreatureObject* creature, uint16_t statIndex, uint32_t update)
{
	//always check against current hitpoints not the theoretical max
	//int32_t currentHam = creature->GetStatCurrent(statIndex); 
	int32_t currentWound = creature->GetStatWound(statIndex); 
	
	//never let us get below 0 Wound
	if(update > currentWound)	{
		update = currentWound;
	}

	//No neg wound application
	//No Nonsense Updates otherwise the client desyncs
	if(update <= 0)	{
		return 0;
	}

	creature->DeductStatWound(statIndex, update);
		
	//add us to regeneration
	addToRegeneration(creature->getId());

	//combatspam
	/*CombatSpamMessage spam;
    spam.damage = update;
    spam.file = "cbt_spam";
	spam.text = "wounded";
	creature->GetController()->Notify(&spam);
	*/

	return update;
	
}

int32_t  HamService::ApplyWound(CreatureObject* creature, uint16_t statIndex, uint32_t update)
{
	//always check against current hitpoints not the theoretical max
	int32_t currentHam = creature->GetStatCurrent(statIndex); 
	int32_t currentWound = creature->GetStatWound(statIndex); 
	
	//never let us get below 1 health because of wounds
	//otherwise we wont be able to UN incapacitate
	if(update >= currentHam-1)	{
		update = currentHam -1;
	}

	//No neg wound application
	//No Nonsense Updates otherwise the client desyncs
	if(update <= 0)	{
		return 0;
	}

	creature->AddStatWound(statIndex,update);

	//recalculate Hitpoints
	int32_t newHam = creature->GetStatMax(statIndex) - creature->GetStatEncumberance(statIndex) - creature->GetStatWound(statIndex);

	//and adapt them if necessary
	if(currentHam != newHam)	{
		creature->SetStatCurrent(statIndex,(newHam < 1 ? 1 : newHam));
	}

	return update;
}

int32_t  HamService::UpdateWound(CreatureObject* creature, uint16_t statIndex, int32_t update)
{
	if (update > 0)
		return ApplyWound(creature, statIndex, 0-update);
	
	if (update < 0)
		return RemoveWound(creature, statIndex, 0-update);

	return 0;
}

void HamService::calcCurrentHitPoints(CreatureObject* creature, const uint16_t statIndex)
{
    //a ham buff modifies the mMaxHitPoints in the Buff Object

    //go through all buffs and calculate the values freshly!!!!!
    /*
    ..
    */
	int32 modified_hitpoints_temp;
	int32 current_damage_temp;
	
	modified_hitpoints_temp = creature->GetStatMax(statIndex) - creature->GetStatWound(statIndex) - creature->GetStatEncumberance(statIndex);
	current_damage_temp		= modified_hitpoints_temp - creature->GetStatCurrent(statIndex);
	
	creature->SetStatCurrent(statIndex, modified_hitpoints_temp);

}

int32 HamService::getCurrentDamage(CreatureObject* creature, const uint16_t statIndex)
{	
	return(getModifiedHitPoints(creature, statIndex) - creature->GetStatCurrent(statIndex));
}

bool HamService::ApplyHamCosts(CreatureObject* creature, uint32_t healthCost, uint32_t actionCost, uint32_t mindCost)
{
	//always check against current hitpoints not the theoretical max
	int32_t currentHealth = creature->GetStatCurrent(0); 
	int32_t currentAction = creature->GetStatCurrent(3); 
	int32_t currentMind = creature->GetStatCurrent(6); 
	
	
	//never let us get below 1 we wont incapacitate over playing music lol
	if((healthCost >= (uint32_t) currentHealth) || (actionCost >= (uint32_t) currentAction) || (mindCost >= (uint32_t) currentMind))	{
		return false;
	}

	//No neg cost application
	//No Nonsense Updates otherwise the client desyncs
	if((healthCost == 0) && (actionCost == 0) && (mindCost == 0))	{
		return false;
	}

	currentHealth	-= healthCost;
	currentAction	-= actionCost;
	currentMind		-= mindCost;

	if(healthCost > 0)	{
		creature->SetStatCurrent(0, currentHealth);
	}

	if(actionCost >0)	{
		creature->SetStatCurrent(3, currentAction);
	}

	if(mindCost >0)	{
		creature->SetStatCurrent(6, currentMind);
	}

	//add us to regeneration
	addToRegeneration(creature->getId());
	
	return true;
	//combatspam
	/*CombatSpamMessage spam;
    spam.damage = update;
    spam.file = "cbt_spam";
	spam.text = "wounded";
	creature->GetController()->Notify(&spam);
	*/
	
}

bool HamService::ApplyHamCost(CreatureObject* creature, uint16_t statIndex, uint32_t cost)
{
	//always check against current hitpoints not the theoretical max
	int32_t currentHam = creature->GetStatCurrent(statIndex); 
	
	
	//never let us get below 1 we wont incapacitate over playing music lol
	if(cost >= (uint32_t) currentHam)	{
		return false;
	}

	//No neg cost application
	//No Nonsense Updates otherwise the client desyncs
	if(cost <= 0)	{
		return 0;
	}

	currentHam -= cost;
	creature->SetStatCurrent(statIndex, currentHam);

	//add us to regeneration
	addToRegeneration(creature->getId());
	
	return true;
		
}


bool HamService::UpdateBaseHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update)
{
	int32_t baseHam = creature->GetStatBase(statIndex); 
	int32_t u = update;

	DLOG (error) << "HamManager::UpdateBaseHitpoints : " << creature->getId();

	if(update == 0)	{
		DLOG (error) << "HamManager::UpdateBaseHitpoints : sanity out of range : update : " << update << " (!!!) "  << creature->getId();
		return false;
	}

	//sanity check 
	int32_t update_temp = update;
	if((update < 0) && ((baseHam + update) < 1))	{
		DLOG (error) << "HamManager::UpdateBaseHitpoints : sanity out of range : currentHam : " << baseHam << " update : " << update << " "  << creature->getId();
		update_temp = 1- baseHam;
		DLOG (error) << "HamManager::UpdateBaseHitpoints :  update changed to " << update_temp;
	}

	creature->SetStatBase(statIndex, (baseHam + update_temp));
	creature->SetStatCurrent(statIndex, (baseHam + update_temp));
	creature->SetStatMax(statIndex, (baseHam + update_temp));

	return true;
}

int32_t HamService::UpdateCurrentHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update)
{

	int32_t currentHam = creature->GetStatCurrent(statIndex); 
	int32_t u = update;
	int32_t modified_hitpoints_temp = getModifiedHitPoints(creature, statIndex);

	DLOG (error) << "HamManager::UpdateCurrentHitpoints : " << creature->getId();

	//sanity check for range
	if((update > 0) && (currentHam  >= modified_hitpoints_temp))	{
		DLOG (error) << "HamManager::UpdateCurrentHitpoints : sanity out of range : currentHam : " << currentHam << " update : " << update << " "  << creature->getId();
		DLOG (error) << "HamManager::UpdateCurrentHitpoints : currentham cant get bigger than max hitpoints - encumbrance - wounds ";
		return 0;
	}

	//never go below zero
	if((update < 0) && (currentHam += update) < 0)	{
		u = 0-currentHam;
	}
	else
	// never go higher than maxhitpoints - encumbrance and wounds
	if((update > 0) && (creature->GetStatCurrent(statIndex) + update) > modified_hitpoints_temp)	{
		u = modified_hitpoints_temp - currentHam;
	}

	//No Nonsense In case we get damaged and already are at zero the update will be canceled
	//In case we get spammed with zero updates we will break here
	//as the client desynchronizes with zero updates
	if(u == 0)	{
		DLOG (error) << "HamManager::UpdateCurrentHitpoints : sanity out of range : currentHam : " << currentHam << " update would be : " << u << " after recalc "  << creature->getId();
		return 0;
	}

	currentHam += u;
	creature->SetStatCurrent(statIndex, currentHam);

	//set us on the regeneration timer in case were not already on it
	if(update < 0)	{
		addToRegeneration(creature->getId());
	}
	
	return u;

}

int32_t HamService::UpdateBattleFatigue(CreatureObject* creature, const int32_t update)
{
	int32_t current_bf = creature->GetBattleFatigue();
	int32_t u = update;
	
	DLOG (error) << "HamManager::UpdateBattleFatiguee : " << creature->getId();

	//sanity check for range
	if((update > 0) && (current_bf + update  >= 1000))	{
		DLOG (error) << "HamManager::UpdateBattleFatiguee : sanity out of range : currentBf : " << current_bf << " update : " << update << " "  << creature->getId();
		return 0;
	}

	//never go below zero
	if((update < 0) && (current_bf += update) < 0)	{
		u = 0-current_bf;
	}
	
	//No Nonsense In case we get damaged and already are at zero the update will be canceled
	//In case we get spammed with zero updates we will break here
	//as the client desynchronizes with zero updates
	if(u == 0)	{
		DLOG (error) << "HamManager::UpdateBattleFatiguee : sanity out of range : currentHam : " << current_bf << " update would be : " << u << " after recalc "  << creature->getId();
		return 0;
	}

	creature->AddBattleFatigue(u);

	return u;

}

bool HamService::ApplyModifiedHamCosts(CreatureObject* creature, int32_t healthCost, int32_t actionCost, int32_t mindCost)
{

	int32_t currentHealth = creature->GetStatCurrent(0); 
	int32_t currentAction = creature->GetStatCurrent(3); 
	int32_t currentMind = creature->GetStatCurrent(6); 
	

	float modifiedHCost = healthCost * (1.0f - ((float)creature->GetStatCurrent(HamBar_Strength) / ((float)creature->GetStatMax(HamBar_Strength)+1400.0f)));
    float modifiedACost = actionCost * (1.0f - ((float)creature->GetStatCurrent(HamBar_Quickness) / ((float)creature->GetStatMax(HamBar_Quickness)+1400.0f)));
    float modifiedMCost = mindCost * (1.0f - ((float)creature->GetStatCurrent(HamBar_Focus) / ((float)creature->GetStatMax(HamBar_Focus) + 1400.0f)));

	//never let us get below 1 we wont incapacitate over playing music lol
	if(((uint32_t) modifiedHCost >= (uint32_t) currentHealth) || ((uint32_t) modifiedACost >= (uint32_t) currentAction) || ((uint32_t)modifiedMCost >= (uint32_t) currentMind))	{
		return false;
	}

	return ApplyHamCosts(creature, modifiedHCost, modifiedACost, modifiedMCost);
   
}

void HamService::addToRegeneration(uint64 id)
{
	auto find_itr = reg_.find(id);
	if(find_itr != reg_.end())	{
		DLOG (info) << "HamManager::UpdateCurrentHitpoints : " << id << "was already on the regenration timer";
		return;
	}
		
	DLOG (info) << "HamManager::UpdateCurrentHitpoints : " << id << "was added to the regenration timer";
	
	//currently hardcoded one regeneration event per second
	ptime next_time = second_clock::local_time() + seconds(1);
	reg_.insert(std::make_pair(id,std::make_pair(next_time , id)));
	
}

int32 HamService::getModifiedHitPoints(CreatureObject* creature, const uint16_t statIndex)
{
  
	return  (creature->GetStatMax(statIndex) - creature->GetStatEncumberance(statIndex) - creature->GetStatWound(statIndex));

}

uint32_t  HamService::regenerationModifier(CreatureObject* creature, uint16_t mainstatIndex)
{
	

	float modifier = 1;
	uint32_t result;
	uint32_t mDivider = 50;

	//todo either script it or get some sort of configuration manager
	switch(creature->states.getPosture())	{
		case CreaturePosture_Crouched : modifier = 1.25f;	break;
		case CreaturePosture_LyingDown :
		case CreaturePosture_Sitting : modifier = 1.75f;	break;
		case CreaturePosture_Incapacitated : 
		case CreaturePosture_Dead : modifier = 0.0f;	break;
		case CreaturePosture_KnockedDown : modifier = 0.75f;	break;
		default : modifier = 1;
	}

	//mainstatindex+2 is constitution for health; willpower for mind and STAMINA for action
	result = (uint32_t)((creature->GetStatCurrent(mainstatIndex+2) / mDivider) * modifier);
	
	LOG(error) << "HamService::regenerationModifier_ " << creature->getId() << " will heal for : " << result;
	
	// Test for creatures	
	if (creature->getObjectType() == SWG_PLAYER)	{
		return result;
	}

	return((result/10)+1);
}

bool HamService::regenerate_(uint64 id)
{
	CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(id));
	
	bool done = true;

	if(!creature)	{
		//bail out
		return true;
	}

	//regeneration only for health action mind
	//LOG (info) << "HamService::regenerate_ : " << creature->getId();
	
	for(uint16_t mainstatIndex = HamBar_Health; mainstatIndex <= HamBar_Willpower;mainstatIndex++)	{
		int32_t maxPool = creature->GetStatMax(mainstatIndex);
		if(creature->GetStatCurrent(mainstatIndex) < maxPool ){
			//LOG (error) << "HamService::regenerate_ : " << creature->getId() << " needs to be healed";
			int32_t newPool = UpdateCurrentHitpoints(creature, mainstatIndex, regenerationModifier(creature, mainstatIndex));
			if (newPool && creature->GetStatCurrent(mainstatIndex) < maxPool )	{
				done = false;
			}
		}
	}

	return done;
	
}

void HamService::handleTick_(const boost::system::error_code& e)
{
    ptime current_time = second_clock::local_time();

    boost::lock_guard<boost::mutex> lock(mutex_);
	auto iterator = reg_.begin();

	while(iterator != reg_.end())	{
		if(current_time > iterator->second.first)	{
			if(regenerate_(iterator->second.second)){
				reg_.erase(iterator++);
			}
			else
				iterator++;
		}
		else
			iterator++;
	}

	timer_.expires_from_now(boost::posix_time::seconds(1));
	timer_.async_wait(boost::bind(&HamService::handleTick_, this, boost::asio::placeholders::error));

}