/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "Buff.h"

#include "BuffDBItem.h"
#include "CreatureObject.h"
#include "MissionManager.h"
#include "PlayerObject.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"

BuffAttribute::BuffAttribute(BuffAttributeEnum Type, int32 InitialValue, int32	TickValue, int32 FinalValue)
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
    return new BuffAttribute((BuffAttributeEnum)item->mType, item->mInitialValue, item->mTickValue, item->mFinalValue);
}


BuffAttributeEnum BuffAttribute::GetType()
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
			It++;
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
	{ //If Buff is on final tick

		FinalChanges();
		/*if(mDowner!=0)
		{
			gWorldManager->addBuffToProcess(&mDowner);
		}*/
		mMarkedForDeletion = true;
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
		gLogger->logMsgF("Buff::FinalChanges :Creature Object nonexistant)",MSG_HIGH);
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
	gMissionManager->missionCompleteEntertainer(Player);


}

//=============================================================================
//
//

Buff::~Buff()
{
}

//=============================================================================
//
//

int32 Buff::ModifyAttribute(BuffAttributeEnum Type, int32 Value, bool damage, bool debuff)
{
	//damage is dealt by a debuff like poison for example this can incapacitate so
	//a debuff from a doctorbuff should not be able to incapacitate you

	int32 cV = Value;
	
	switch(Type)
	{

		case Health:
			{
				cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Modifier, Value, damage);
			}
		break;

	case Food_Filling:
	{
		if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->mTarget))
		{
			playerObject->getStomach()->incFood(Value);
			gMessageLib->sendFoodUpdate(playerObject);
		}
	}
	break;

	case Drink_Filling:
	{
		if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->mTarget))
		{
			playerObject->getStomach()->incDrink(Value);
			gMessageLib->sendDrinkUpdate(playerObject);
		}
	}
	break;

	case Strength:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Strength, HamProperty_Modifier, Value);
		}
	break;

	case Constitution:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Constitution, HamProperty_Modifier, Value);
		}
	break;

	case Action:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Modifier, Value);
		}
	break;

	case Quickness:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Quickness, HamProperty_Modifier, Value);
		}
	break;

	case Stamina:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Stamina, HamProperty_Modifier, Value);
		}
	break;

	case Mind:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Modifier, Value);
		}
	break;

	case Focus:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Focus, HamProperty_Modifier, Value);
		}
	break;

	case Willpower:
		{
			cV = this->mTarget->getHam()->updatePropertyValue(HamBar_Willpower, HamProperty_Modifier, Value);
		}
	break;

	case Burst_Run_Timer:{}break;
	case Burst_Run_Cost_Reduction:{}break;
	case To_Hit_Chance:{}break;
	case Crafting_Assembly:{}break;
	case Entertainer_Buff_Timer_Reduction:{}break;
	case Timer_Reduction_on_Fire_DOT:{}break;
	case Timer_Reduction_on_Bleed_DOT:{}break;
	case Timer_Reduction_on_Poison_DOT:{}break;
	case Timer_Reduction_on_Disease_DOT:{}break;
	case DOT_Avoidance_Chance_Fire:{}break;
	case DOT_Avoidance_Chance_Bleed:{}break;
	case DOT_Avoidance_Chance_Poison:{}break;
	case DOT_Avoidance_Chance_Disease:{}break;
	case Heat_Damage_Reduction_Percent:{}break;
	case Experimentation_Chance:{}break;
	case Damage_Reduction_Percent:{}break;
	case Constitution_Regen_Percent:{}break;
	case Incap_Timer_Reduction:{}break;
	case Heal_Time_Reduction_Percentage:{}break;
	case Accuracy:{}break;
	case Carbine_Accuracy:{}break;
	case Carbine_Speed:{}break;
	case Mask_Scent:
		{
			
			if(mTarget->getSkillModValue(16) != 0)
			{
				mTarget->modifySkillModValue(16, Value);
			} else { //we don't have skill mods for Mask Scent, hence erase attributes
				EraseAttributes();
			}
		}
	break;
	case Onehandmelee_Accuracy:{}break;
	case Onehandmelee_Speed:{}break;
	case Pistol_Accuracy:{}break;
	case Pistol_Speed:{}break;
	case Polearm_Accuracy:{}break;
	case Polearm_Speed:{}break;
	case Rifle_Accuracy:{}break;
	case Rifle_Speed:{}break;
	case Thrown_Accuracy:{}break;
	case Thrown_Speed:{}break;
	case Twohandmelee_Accuracy:{}break;
	case Twohandmelee_Speed:{}break;
	case Unarmed_Accuracy:{}break;
	case Unarmed_Speed:{}break;
	case Spice_Downer_Time_Reduction:{}break;
	case Enhance_Wookie_Roar_Ability:{}break;
	case Mission_Timer:	{ } break;
	default:
		{
		}
		break;
	}
	return(cV);
}

void Buff::EraseAttributes()
{
	Attributes.clear();
}

void Buff::SetParent(Buff* value)
{
    mParent=value;
}
