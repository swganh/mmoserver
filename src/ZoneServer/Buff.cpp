#include "Buff.h"
#include "MessageLib\MessageLib.h"
#include "WorldManager.h"
#include "MissionManager.h"


#define	 gWorldManager	WorldManager::getSingletonPtr()


Buff::	Buff(CreatureObject* Target, CreatureObject* Instigator, uint NoOfTicks, uint64 Tick, uint32 Icon, uint64 CurrentGlobalTick):mTarget(Target),mInstigator(Instigator),mNoTicks(NoOfTicks),mTick(Tick),mIcon(Icon),mStartTime(CurrentGlobalTick)
{
	mCurrentTick = 0;
	mMarkedForDeletion = false;
	mChild = 0;
	mDoInit = true;
}

//-------------------------------------------------------
//a buff was loaded from the db on character load
//

Buff* Buff::FromDB(BuffDBItem* Item, uint64 CurrentGlobalTick)
{
	CreatureObject* target = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(Item->mTargetId));
	CreatureObject* instigator = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(Item->mInstigatorId));
	Buff* temp = new Buff(target, instigator, Item->mMaxTicks, Item->mTickLength, Item->mIconCRC, Item->mStartGlobalTick);

	if(temp->mNoTicks > 0)
	{
		temp->mNextTickLength = temp->mTick;
		//Take the time already ran off the buff
		temp->mTick -= (Item->mStartGlobalTick - Item->mPausedGlobalTick) % temp->mTick;

		temp->mNoTicks = Item->mMaxTicks-Item->mCurrentTick; //Remove Used Ticks
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
	if(mNoTicks > mCurrentTick)  //If we aren't on the final tick
	{
		//Modify Attributes
		AttributeList::iterator It = Attributes.begin();
		while(It != Attributes.end())
		{
			BuffAttribute* temp = (*It);
			ModifyAttribute(temp->GetType(), temp->GetTickValue());
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
			//gMessageLib->sendSystemMessage(Player, "Initialising Buff");
			if(mNoTicks < 1)
			{
				if(mIcon > 0) //if internal buff
				{
					gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)(mTick/1000));
				}
			} 
			else 
			{
				if(mIcon > 0) //if internal buff
				{
					gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)((mNoTicks*mTick)/1000));
				}
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
			ModifyAttribute(temp->GetType(), temp->GetInitialValue());
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
				gMessageLib->sendPlayerAddBuff(Player, mIcon, (float)(mTick/1000));
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
		gLogger->logMsgF("Buff::FinalChanges :Creature Object nonexistant)\n",MSG_HIGH);
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

void Buff::ModifyAttribute(BuffAttributeEnum Type, int32 Value)
{
	switch(Type)
	{
	
	case Health:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Modifier, Value);
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
			this->mTarget->getHam()->updatePropertyValue(HamBar_Strength, HamProperty_Modifier, Value);
		}
	break;
	
	case Constitution:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Constitution, HamProperty_Modifier, Value);
		}
	break;
	
	case Action:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Modifier, Value);
		}
	break;
	
	case Quickness:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Quickness, HamProperty_Modifier, Value);
		}
	break;
	
	case Stamina:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Stamina, HamProperty_Modifier, Value);
		}
	break;

	case Mind:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Modifier, Value);
		}
	break;
	
	case Focus:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Focus, HamProperty_Modifier, Value);
		}
	break;
	
	case Willpower:
		{
			this->mTarget->getHam()->updatePropertyValue(HamBar_Willpower, HamProperty_Modifier, Value);
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
			this->mTarget->modifySkillModValue(16, Value);
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
}

void Buff::EraseAttributes()
{
	Attributes.clear();
}