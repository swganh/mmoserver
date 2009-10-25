/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MedicManager.h"

#include "Inventory.h"
#include "Medicine.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "WorldManager.h"

#include "MessageLib/MessageLib.h"


#ifndef min
#define min(a,b)(((a)<(b))?(a):(b))
#endif

bool			MedicManager::mInsFlag = false;
MedicManager*	MedicManager::mSingleton = NULL;


MedicManager::MedicManager(MessageDispatch* dispatch)
{
    Dispatch = dispatch;
}


MedicManager::~MedicManager()
{
}


bool MedicManager::Diagnose(PlayerObject* Medic, PlayerObject* Target)
{
	//TODO: Allow Pet Diagnosis

	if(!Medic->verifyAbility(0xDC7CF134))
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}

	//TODO: Remove Magic Number
	if(Medic->mPosition.distance2D(Target->mPosition) > 6)
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_b7");
		return false;
	}

	gUIManager->createNewDiagnoseListBox(Medic, Medic, Target);
	return true;
}


bool MedicManager::HealDamage(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties)
{
    return HealDamage(Medic, Target, 0, cmdProperties);
}


bool MedicManager::HealDamage(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties)
{
	//TODO: Major - Allow for Ranged Stims if we are beyond healing range of standard stims

	Medicine* Stim = NULL;

	gLogger->logMsg("Heal Damage Called\n", FOREGROUND_BLUE);

	//If we don't have an OC Controller Cmd Property (ie we have been called by using an item) - go get one
	if(cmdProperties == 0)
	{
		gLogger->logMsg("We need to get Object Properties\n", FOREGROUND_BLUE);

		CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(0x0A9F00A0);

		if(it == gObjControllerCmdPropertyMap.end())
		{
			//Cannot find properties
			gLogger->logMsg("Failed to get Object Properties\n", FOREGROUND_RED);
			return false;
		} else {
			cmdProperties = ((*it).second);
		}
	}

	PlayerObject* PlayerTarget = dynamic_cast<PlayerObject*>(Target);
	bool player = false;
	bool self = false;

	if(PlayerTarget == 0)
	{
		//We are looking at a critter, not a player
		gLogger->logMsg("Heal is targetting a Critter\n", FOREGROUND_BLUE);
	} else {
		player = true;
		self = (Medic->getId() == PlayerTarget->getId());
		gLogger->logMsg("Heal is targetting a player\n", FOREGROUND_BLUE);
	}

	//Get Medic Skill Mods
	uint32 healingskill = Medic->getSkillModValue(SMod_healing_injury_treatment);
	//uint32 healingspeed = Medic->getSkillModValue(SMod_healing_injury_speed);

	//If Currently in Delay Period
		//TODO
		//gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_must_wait");

	//is target within 6m TODO: Remove Magic Number
	if(Medic->mPosition.distance2D(Target->mPosition) > 6)
	{
		gLogger->logMsg("Heal Target is out of rang\ne", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","no_line_of_site");
		return false;
	}
	gLogger->logMsg("Heal Target is within range\n", FOREGROUND_BLUE);

	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->logMsg("Medic does not have ability\n", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}
	gLogger->logMsg("Medic has Ability Rights\n", FOREGROUND_BLUE);

	//Do PVP Alignments Match
	if(Medic->getPvPStatus() != Target->getPvPStatus())
	{
		//send pvp_no_help
		gLogger->logMsg("PVP Flag not right\n", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","pvp_no_help");
		return false;
	} else {
		if(player)
		{
			//TODO: PVP Flags match, but Status is not correct
			/*if(strcmp(Medic->getFaction(), Target->getFaction()) != 0)
			{
			}*/
		}
	}

	gLogger->logMsg("PVP Flags OK\n", FOREGROUND_BLUE);

	//Does Target Need Healing
	int TargetHealth = Target->getHam()->mHealth.getCurrentHitPoints();
	int TargetAction = Target->getHam()->mAction.getCurrentHitPoints();
	int TargetMaxHealth = Target->getHam()->mHealth.getMaxHitPoints();
	int TargetMaxAction = Target->getHam()->mAction.getMaxHitPoints();

	if(!(TargetHealth < TargetMaxHealth))
	{
		if(!(TargetAction < TargetMaxAction))
		{
			gLogger->logMsg("Target doesn't need healing\n", FOREGROUND_RED);
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_63");
			return false;
		}
	}
	gLogger->logMsg("Target Needs Healing\n", FOREGROUND_BLUE);

	//If we weren't triggered by a stim ie. from a command
	if(StimPackObjectID == 0)
	{
		//Look through inventory to find a StimPack
		Inventory* inventory = dynamic_cast<Inventory*>(Medic->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

		ObjectList::iterator It = inventory->getObjects()->begin();
		while(It != inventory->getObjects()->end())
		{
			Item* item = dynamic_cast<Item*>(*It);
			if(item == 0)
			{
				It++;
				continue;
			}

			switch(item->getItemType())
			{
			case ItemType_Stimpack_A:
			case ItemType_Stimpack_B:
			case ItemType_Stimpack_C:
			case ItemType_Stimpack_D:
			case ItemType_Stimpack_E:
				StimPackObjectID = item->getId();
				Stim = dynamic_cast<Medicine*>(item);
				break;
			default:
				break;
			}

			if(StimPackObjectID != 0)
			{
				break;
			} else {
				It++;
			}
		}

		//Check if a Stim was found
		if(Stim == 0)
		{
			gLogger->logMsg("No Stim Found\n", FOREGROUND_RED);
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_60");
			return false;
		}
	} else {
		gLogger->logMsg("We already have a Stim Selected\n", FOREGROUND_BLUE);
		Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));
	}
	gLogger->logMsg("Stim ID Found OK\n", FOREGROUND_BLUE);

	//Is the stim suitable for skill level
	if(healingskill < Stim->getSkillRequired())
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","insufficient_skill_heal");
		return false;
	}
	//Get Heal Strength
	//maxheal = stimpower * [(100 + InjuryTreatSkill + BEClothes + food/buff) / 100] * MedCityBonus (1.1)

	uint Strength = Stim->getHealthHeal() * ((100 + healingskill) / 100);

	//Adjust for Target BF
	int32 BF = Target->getHam()->getBattleFatigue();
	if(BF > 250)
	{
		Strength -= (Strength * (BF-250) / 1000);

		if(BF > 500) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_medium");
		} else if(BF > 750) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_high");
		} else {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_low");
		}
	}

	int StrengthHealth = min((int)Strength, TargetMaxHealth-TargetHealth);
	int StrengthAction = min((int)Strength, TargetMaxAction-TargetAction);

	Target->getHam()->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints, StrengthHealth);
	Target->getHam()->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, StrengthAction);

	if(Stim->ConsumeUse(Medic))
	{
		Inventory* inventory = dynamic_cast<Inventory*>(Medic->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		inventory->deleteObject(Stim);
	}

	//Add XP as Total Heal / 4 if not targetting self
	if(!self)
		gSkillManager->addExperience(XpType_medical, (int)((StrengthHealth + StrengthAction)/4), Medic);

	if(self) //If targetting self
	{
		if(StrengthHealth > 0)
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_01", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_10", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_11", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
			}
		}
		gMessageLib->sendCombatAction(Medic, Target, BString::CRC("heal_self"), 1, 1, 1);

	} else { //if targetting something else
		if(StrengthHealth > 0)
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_06", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_08", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
				}
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_13", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_16", "", "", L"", StrengthHealth, "", "", "", 0, 0, 0, "", "", L"");
				}
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_14", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_17", "", "", L"", StrengthAction, "", "", "", 0, 0, 0, "", "", L"");
				}
			}
		}
		gMessageLib->sendCombatAction(Medic, Target, BString::CRC("heal_other"), 0, 0, 1);
	}

	//Get Cooldown Timer (Minimum 4 secs)
	//cooldown = max(4, ( ( 100-foodbuff)/100 ) x  ( ( 100 -  InjuryTreatmentSpeed  ) /100 ) x base healing speed )
	return true;
}
