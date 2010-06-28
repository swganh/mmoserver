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
#include "Buff.h"

#include "BuffDBItem.h"
#include "CreatureObject.h"
#include "MissionManager.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "SkillEnums.h"

#include "MessageLib/MessageLib.h"

BuffAttribute::BuffAttribute(uint64 Type, int32 InitialValue, int32	TickValue, int32 FinalValue)
: mAttribute(Type)
, mInitialValue(InitialValue)
, mTickValue(TickValue)
, mFinalValue(FinalValue)
{
	// TODO we need to test for total wounds and stuff to make sure that total wounds / health damage do *NOT*
	// exceed the *UNBUFFED* values!!!!
}


BuffAttribute::~BuffAttribute()
{}


BuffAttribute* BuffAttribute::FromDB(BuffAttributeDBItem* item)
{
    return new BuffAttribute(item->mType, item->mInitialValue, item->mTickValue, item->mFinalValue);
}


uint64 BuffAttribute::GetType()
{
    return mAttribute;
}


int32 BuffAttribute::GetInitialValue()
{
    return mInitialValue;
}


int32 BuffAttribute::GetTickValue()
{
    return mTickValue;
}


int32 BuffAttribute::GetFinalValue()
{
    return mFinalValue;
}

Buff::Buff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick)
: mChild(0)
, mParent(0)
, mInstigator(Instigator)
, mTarget(Target)
, mTick(Tick)
, mStartTime(CurrentGlobalTick)
, mCurrentTick(0)
, mIcon(Icon)
, mNoTicks(NoOfTicks)
, mDoInit(true)
, mMarkedForDeletion(false)
{}


Buff* Buff::SimpleBuff(CreatureObject* Target, CreatureObject* Instigator, uint64 Duration, uint32 Icon, uint64 CurrentGlobalTick)
{
    return new Buff(Target, Instigator, 0, Duration, Icon, CurrentGlobalTick);
}


Buff* Buff::TickingBuff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick)
{
    return new Buff(Target, Instigator, NoOfTicks, Tick, Icon, CurrentGlobalTick);
}


//-------------------------------------------------------
//a buff was loaded from the db on character load
//

Buff* Buff::FromDB(BuffDBItem* Item, uint64 CurrentGlobalTick)
{
	CreatureObject* target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(Item->mTargetId));
	CreatureObject* instigator = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(Item->mInstigatorId));
	Buff* temp = new Buff(target, instigator, static_cast<uint>(Item->mMaxTicks), Item->mTickLength, Item->mIconCRC, Item->mStartGlobalTick);

	if(temp->mNoTicks > 0)
	{
		temp->mNextTickLength = temp->mTick;
		//Take the time already ran off the buff
		temp->mTick -= (Item->mStartGlobalTick - Item->mPausedGlobalTick) % temp->mTick;

		temp->mNoTicks = static_cast<uint32>(Item->mMaxTicks-Item->mCurrentTick); //Remove Used Ticks
		temp->mCurrentTick = 0; //Set Tick to start
		temp->mStartTime = CurrentGlobalTick; //Reset start time

	}
	else
	{
		//Take the time already ran off the buff
		temp->mTick -= (Item->mPausedGlobalTick - Item->mStartGlobalTick);
		temp->mStartTime = CurrentGlobalTick; //Reset start time
	}

	temp->mDoInit = false;
	temp->mDBID = Item->mBuffId;
	return temp;
}


//-------------------------------------------------------
//an update event to our buff
//

uint64 Buff::Update(uint64 CurrentTime, void* ref)
{
	//Todo
	// we need a way to mark damaging buffs from nondamaging buffs
	// ie docbuff restoring original ham values vs poison
	if(mNoTicks > mCurrentTick)  //If we aren't on the final tick
	{
		//Modify Attributes
		AttributeList::iterator It = Attributes.begin();
		while(It != Attributes.end())
		{
			BuffAttribute* temp = (*It);
			ModifyAttribute(temp->GetType(), temp->GetTickValue());	 
			//were not interested in the return value here as ticking buffs are not applying final changes to restore an attribute
			++It;
		}

		//Increment the Tick Counter
		IncrementTick();

		//Recreate Timer
		if(mNextTickLength != 0)
			return mNextTickLength;
		else
			return mTick;

	}
	else
	{ //If  Buff is on final tick

		FinalChanges();
		if(mChild!=0)
		{
			gWorldManager->addBuffToProcess(mChild);
		}
		mMarkedForDeletion = true;
		mTarget->CleanUpBuffs();
		return 0;
	}
}

//=============================================================================
//
//

void Buff::ReInit()
{
	if(mDoInit)
	{
		InitialChanges();
		mDoInit = false;
	}
	else
		InitializeIcons();
}


void Buff::AddAttribute(BuffAttribute* Attribute)
{
    Attributes.push_back(Attribute);
}


void Buff::SetID(uint64 value)
{
    mID=value;
}


void Buff::SetChildBuff(Buff* value)
{
    mChild=value;
    if(value!=0) value->SetParent(this);
}


void Buff::setTarget(CreatureObject* creature)
{
    mTarget=creature;
}


bool Buff::GetIsMarkedForDeletion()
{
    return mMarkedForDeletion;
}


uint64 Buff::GetID()
{
    return mID;
}


uint64 Buff::GetDBID()
{
    return mDBID;
}


CreatureObject*	Buff::GetTarget()
{
    return mTarget;
}


CreatureObject*	Buff::GetInstigator()
{
    return mInstigator;
}


uint64 Buff::GetTickLength()
{
    return mTick;
}


uint32 Buff::GetNoOfTicks()
{
    return mNoTicks;
}


uint32 Buff::GetCurrentTickNumber()
{
    return mCurrentTick;
}


uint32 Buff::GetIcon()
{
    return mIcon;
}


string Buff::GetName()
{
    return mName;
}


uint64 Buff::GetStartGlobalTick()
{
    return mStartTime;
}


void Buff::SetInit(bool init)
{
    mDoInit = init;
}


void Buff::IncrementTick()
{
    mCurrentTick++;
}

uint64 Buff::GetRemainingTime(uint64 CurrentTime)
 {
	if(mNoTicks==0)
	{
		return (mStartTime+mTick)-CurrentTime;
	} else {
		if(mNoTicks > mCurrentTick)
		{
			return (mStartTime+(mTick *(mNoTicks - mCurrentTick)))-CurrentTime;
		}
	}
 
     return 0;
 }


//=============================================================================
//
//

void Buff::InitialChanges()
{
	if(mDoInit)
	{
		PlayerObject* Player = dynamic_cast<PlayerObject*>(mTarget);
		if(Player)
		{
		
			if(mIcon > 0) //if internal buff
			{
				gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)(mTick/1000));
			}
		
		}
		else
		{
			//if Creature/NPC
		}
		AttributeList::iterator It = Attributes.begin();
		while(It != Attributes.end())
		{
			BuffAttribute* temp = (*It);
			//make sure we know if the ham has altered our value!
			temp->SetInitialValue(ModifyAttribute(temp->GetType(), temp->GetInitialValue()));
			if(temp->GetFinalValue())
				temp->SetFinalValue(-temp->GetInitialValue());

			It++;
		}
		mDoInit=false;
	}
}

//=============================================================================
//just send the icons as we just reloaded
//

void Buff::InitializeIcons()
{
	PlayerObject* Player = dynamic_cast<PlayerObject*>(mTarget);
	if(Player)
	{
		//gMessageLib->sendSystemMessage(Player, "Initialising Buff");
		if(mNoTicks < 1)
		{
			if(mIcon > 0) //if internal buff
			{
				//make sure the time is updated in case we travel with the shuttle on the planet
				uint64 time = gWorldManager->GetCurrentGlobalTick() - mStartTime;
				gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)((mTick-time)/1000));
			}
		}
		else
		{
			if(mIcon > 0) //if internal buff
			{
				gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)(((mNoTicks-mCurrentTick)*mTick)/1000));
			}
		}
	}
}

//=============================================================================
// last changes of the buff before it wears off
// typically that means that the initial values get restored

void Buff::FinalChanges()
{
	CreatureObject*	creature = dynamic_cast<PlayerObject*>(mTarget);
	if(!creature)
	{
		gLogger->log(LogManager::DEBUG,"Buff::FinalChanges :Creature Object nonexistant)");
		mMarkedForDeletion =true;
		EraseAttributes();
		return;
	}

	AttributeList::iterator It = Attributes.begin();
	while(It != Attributes.end())
	{
		BuffAttribute* temp = (*It);
		ModifyAttribute(temp->GetType(), temp->GetFinalValue());
		It++;
	}

	PlayerObject* Player = dynamic_cast<PlayerObject*>(mTarget);
	if(Player)
	{
		if(mIcon > 0)
		{
			gMessageLib->sendPlayerRemoveBuff(Player, mIcon);
		}
	}

	//Complete entertainer mission
	gMissionManager->missionCompleteEntertainer(Player,this);


}

//=============================================================================
//
//

int32 Buff::ModifyAttribute(uint64 Type, int32 Value, bool damage, bool debuff)
{
	//damage is dealt by a debuff like poison for example this can incapacitate so
	//a debuff from a doctorbuff should not be able to incapacitate you

	int32 cV = Value;
	
	switch(Type)
	{
		case attr_health:
			{
				cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Modifier, Value, damage);
			}
		break;

	case attr_strength:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Strength, HamProperty_Modifier, Value);
		}
	break;

	case attr_constitution:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Constitution, HamProperty_Modifier, Value);
		}
	break;

	case attr_action:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Modifier, Value);
		}
	break;

	case attr_quickness:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Quickness, HamProperty_Modifier, Value);
		}
	break;

	case attr_stamina:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Stamina, HamProperty_Modifier, Value);
		}
	break;

	case attr_mind:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Modifier, Value);
		}
	break;

	case attr_focus:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Focus, HamProperty_Modifier, Value);
		}
	break;

	case attr_willpower:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Willpower, HamProperty_Modifier, Value);
		}
	break;
	case bio_comp_mask_scent:
		{
			if(mTarget->getSkillModValue(SMod_mask_scent) != 0)
			{
				mTarget->modifySkillModValue(SMod_mask_scent, Value);
			} else { //we don't have skill mods for Mask Scent, hence erase attributes
				EraseAttributes();
			}
		}
	break;
	case accuracy:
		{
			mTarget->modifySkillModValue(SMod_ranged_accuracy, Value);
			mTarget->modifySkillModValue(SMod_melee_accuracy, Value);
		}
		break;
	case bleed_resist : {gLogger->log(LogManager::NOTICE,"Unhandled Attribute: bleed_resist in Buff.cpp");} break;
	case blind_defense : {
			mTarget->modifySkillModValue(SMod_blind_defense, Value);
		}
		break;
	case block : 
		{
			mTarget->modifySkillModValue(SMod_block, Value);
		}
		break;
	case burst_run : {
			mTarget->modifySkillModValue(SMod_burst_run, Value);
		} break;
	case camouflage : {
			if(mTarget->getSkillModValue(SMod_camouflage) != 0)
				mTarget->modifySkillModValue(SMod_camouflage, Value);
		} break;
	case creature_action : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: creature_action in Buff.cpp");} break;
	case creature_health : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute creature_health in Buff.cpp");} break;
	case creature_mind : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute creature_mind in Buff.cpp");} break; 
	case creature_tohit : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute creature_tohit in Buff.cpp");} break;
	case dizzy_defense : {
			mTarget->modifySkillModValue(SMod_dizzy_defense, Value);
		} break;
	case dodge : {
			mTarget->modifySkillModValue(SMod_dodge, Value);
		} break;
	case experiment_bonus : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: experimentation_bonus in Buff.cpp");} break; 
	case fire_resist : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: fire_resist in Buff.cpp");} break;
	case food_reduce : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute in Buff.cpp");} break; 
	case general_assembly : {
			mTarget->modifySkillModValue(SMod_general_assembly, Value);
		} break;
	case harvesting : {
		if(mTarget->getSkillModValue(SMod_creature_harvesting) != 0)
			mTarget->modifySkillModValue(SMod_creature_harvesting, Value);
		} break;
	case healer_speed : {
		if(mTarget->getSkillModValue(SMod_healing_wound_speed) != 0)
			mTarget->modifySkillModValue(SMod_healing_wound_speed, Value);
		if(mTarget->getSkillModValue(SMod_healing_injury_speed) != 0)
			mTarget->modifySkillModValue(SMod_healing_injury_speed, Value);
		if(mTarget->getSkillModValue(SMod_healing_range_speed) != 0)
			mTarget->modifySkillModValue(SMod_healing_range_speed, Value);
		} break;
	case healing_dance_wound :  {
		if(mTarget->getSkillModValue(SMod_healing_dance_wound) != 0)
			mTarget->modifySkillModValue(SMod_healing_dance_wound, Value);
		} break;
	case healing_music_wound :  {
		if(mTarget->getSkillModValue(SMod_healing_music_wound) != 0)
			mTarget->modifySkillModValue(SMod_healing_music_wound, Value);
		} break;
	case healing_wound_treatment :  {
		if(mTarget->getSkillModValue(SMod_healing_wound_treatment) != 0)
			mTarget->modifySkillModValue(SMod_healing_wound_treatment, Value);
		} break;
	case incap_recovery : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: incap_recovery in Buff.cpp");} break; 
	case innate_regeneration :  {
		if(mTarget->getSkillModValue(SMod_private_innate_regeneration) != 0)
			mTarget->modifySkillModValue(SMod_private_innate_regeneration, Value);
		} break; 
	case innate_roar :  {
		if(mTarget->getSkillModValue(SMod_private_innate_roar) != 0)
			mTarget->modifySkillModValue(SMod_private_innate_roar, Value);
		} break;
	case intimidate_defense :  {
			mTarget->modifySkillModValue(SMod_intimidate_defense, Value);
		} break;
	case knockdown_defense :  {
			mTarget->modifySkillModValue(SMod_knockdown_defense, Value);
		} break;
	case melee_defense :  {
			mTarget->modifySkillModValue(SMod_melee_defense, Value);
		} break;
	case mind_heal : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: mind_heal in Buff.cpp");} break; 
	case mitigate_damage : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: mitigate_damage in Buff.cpp");} break; 
	case poison_disease_resist : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: poison_disease_resist in Buff.cpp");} break;
	case ranged_defense :  {
			mTarget->modifySkillModValue(SMod_ranged_defense, Value);
		} break;
	case reduce_clone_wounds : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: reduce_clone_wounds in Buff.cpp");} break;
	case reduce_spice_downtime : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: reduce_spice_downtime in Buff.cpp");} break;
	case stun_defense :  {
			mTarget->modifySkillModValue(SMod_stun_defense, Value);
		} break;
	case surveying :  {
		if(mTarget->getSkillModValue(SMod_surveying) != 0)
			mTarget->modifySkillModValue(SMod_surveying, Value);
		} break;
	case tame_bonus :  {
		if(mTarget->getSkillModValue(SMod_tame_bonus) != 0)
			mTarget->modifySkillModValue(SMod_tame_bonus, Value);
		} break;
	case trapping :  {
		if(mTarget->getSkillModValue(SMod_trapping) != 0)
			mTarget->modifySkillModValue(SMod_trapping, Value);
		} break; 
	case unarmed_damage :  {
			mTarget->modifySkillModValue(SMod_unarmed_damage, Value);
		} break;
	case xp_increase : {gLogger->log(LogManager::DEBUG,"Unhandled Attribute: xp_increase in Buff.cpp");} break; 
	default:
		{
		}
		break;
	}
	return(cV);
}

void Buff::EraseAttributes()
{
	AttributeList::iterator it = Attributes.begin();

	while(it!= Attributes.end())
	{
		SAFE_DELETE(*it);
		it=Attributes.erase(it);
	}
}

void Buff::SetParent(Buff* value)
{
    mParent=value;
}
