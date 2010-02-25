/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MedicManager.h"
#include "InjuryTreatmentEvent.h"
#include "Inventory.h"
#include "Medicine.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "WorldConfig.h"

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

	if(!Medic->verifyAbility(opOCdiagnose))
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}

	
	float distance = gWorldConfig->getConfiguration("Player_heal_distance",(float)6.0);

	if(Medic->mPosition.distance2D(Target->mPosition) > distance)
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_b7");
		return false;
	}

	gUIManager->createNewDiagnoseListBox(Medic, Medic, Target);
	return true;
}


bool MedicManager::CheckStim(PlayerObject* Medic, CreatureObject* Target, ObjectControllerCmdProperties* cmdProperties)
{

	//This determines what type of stim the player is using (either medic or CM)

	bool rangedstim = false;
	bool stimpack = false;

	Medicine* Stim = NULL;

	uint64 StimPackObjectID = 0;

	gLogger->logMsg("Heal Damage Called", FOREGROUND_BLUE);

	//If we don't have an OC Controller Cmd Property (ie we have been called by using an item) - go get one
	if(cmdProperties == 0)
	{
		gLogger->logMsg("We need to get Object Properties", FOREGROUND_BLUE);

		CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opOChealdamage);

		if(it == gObjControllerCmdPropertyMap.end())
		{
			//Cannot find properties
			gLogger->logMsg("Failed to get Object Properties", FOREGROUND_RED);
			return false;
		} else {
			cmdProperties = ((*it).second);
		}
	}

		//If we weren't triggered by a stim ie. from a command
	if(StimPackObjectID == 0)
	{


		//TODO have an automated function that looks for the first item of a certain typein ALL containers

		//Look through inventory to find a StimPack
		Inventory* inventory = dynamic_cast<Inventory*>(Medic->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

		ObjectIDList::iterator It = inventory->getObjects()->begin();
		while(It != inventory->getObjects()->end())
		{
			Item* item = dynamic_cast<Item*>(gWorldManager->getObjectById((*It)));
			if(!item)
			{
				//hell resource containers are no items!!!
				//they would cast as tangibles though
				//assert(false);
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
					stimpack = true;
					break;
				case ItemType_Ranged_Stimpack_A:
				case ItemType_Ranged_Stimpack_B:
				case ItemType_Ranged_Stimpack_C:
				case ItemType_Ranged_Stimpack_D:
				case ItemType_Ranged_Stimpack_E:
					StimPackObjectID = item->getId();
					Stim = dynamic_cast<Medicine*>(item);
					rangedstim = true;
				default:
					break;
			}

			if(Stim)
			{
				break;
			} 
			else 
			{
				It++;
			}
		}

		//Check if a Stim was found
		if(Stim == 0)
		{
			gLogger->logMsg("No Stim Found", FOREGROUND_RED);
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_60");
			return false;
		}
	} else 
	{
		gLogger->logMsg("We already have a Stim Selected", FOREGROUND_BLUE);
		Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));
	}
	gLogger->logMsg("Stim ID Found OK", FOREGROUND_BLUE);

	//Is the stim suitable for skill level

	uint64 healingskill = Medic->getSkillModValue(SMod_healing_ability);

	uint64 req = Stim->getSkillRequired();
	
	if(healingskill < req)
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing","insufficient_skill_heal","",L"healingskill",L"req");
		gLogger->logMsg("The selected stim is too high level.", FOREGROUND_RED);
		return false;
	}

	if (rangedstim == true){
	return HealDamageRanged(Medic, Target, StimPackObjectID, cmdProperties);
	}

	if (stimpack == true){
    return HealDamage(Medic, Target, StimPackObjectID, cmdProperties);
	}

	return false;
	
}


bool MedicManager::HealDamage(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject* PlayerTarget = dynamic_cast<PlayerObject*>(Target);
	bool player = false;
	bool self = false;
	bool critter = false;
	Medicine* Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));

	if(PlayerTarget == 0)
	{
		//We are looking at a critter, not a player
		critter = true;
		gLogger->logMsg("Heal is targetting a Critter", FOREGROUND_BLUE);
	} else {
		player = true;
		self = (Medic->getId() == PlayerTarget->getId());
		gLogger->logMsg("Heal is targetting a player", FOREGROUND_BLUE);
	}

	//Get Medic Skill Mods
	uint32 healingskill = Medic->getSkillModValue(SMod_healing_injury_treatment);

	//If Currently in Delay Period

	if(Medic->checkPlayerCustomFlag(PlayerCustomFlag_InjuryTreatment))
	{
		//Say you can't heal yet.
		gMessageLib->sendSystemMessage(Medic,L"","healing_response", "healing_must_wait");
		return false;
	}


	float distance = gWorldConfig->getConfiguration("Player_heal_distance",(float)6.0);

	if(Medic->mPosition.distance2D(Target->mPosition) > distance)
	{
		gLogger->logMsg("Heal Target is out of range", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing","no_line_of_sight");
		return false;
	}
	gLogger->logMsg("Heal Target is within range", FOREGROUND_BLUE);

	//If we don't have an OC Controller Cmd Property (ie we have been called by using an item) - go get one
	if(cmdProperties == 0)
	{
		gLogger->logMsg("We need to get Object Properties", FOREGROUND_BLUE);

		CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opOChealdamage);

		if(it == gObjControllerCmdPropertyMap.end())
		{
			//Cannot find properties
			gLogger->logMsg("Failed to get Object Properties", FOREGROUND_RED);
			return false;
		} else {
			cmdProperties = ((*it).second);
		}
	}


	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->logMsg("Medic does not have ability", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}
	gLogger->logMsg("Medic has Ability Rights", FOREGROUND_BLUE);

	//Do PVP Alignments Match - only check if targetting player.

	if(critter) {
		return false;
	}

	if(Medic->getPvPStatus() != PlayerTarget->getPvPStatus())
	{
		//send pvp_no_help
		gLogger->logMsg("PVP Flag not right", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing","pvp_no_help");
		return false;
	}

	gLogger->logMsg("PVP Flags OK", FOREGROUND_BLUE);

	//Does Target Need Healing
	int TargetHealth = Target->getHam()->mHealth.getCurrentHitPoints();
	int TargetAction = Target->getHam()->mAction.getCurrentHitPoints();
	int TargetMaxHealth = Target->getHam()->mHealth.getMaxHitPoints();
	int TargetMaxAction = Target->getHam()->mAction.getMaxHitPoints();

	if(!(TargetHealth < TargetMaxHealth))
	{
		if(!(TargetAction < TargetMaxAction))
		{
			if (self){
				gLogger->logMsg("You don't need healing", FOREGROUND_RED);
				gMessageLib->sendSystemMessage(Medic,L"","healing","no_damage_to_heal_self");
				return false;
			}
			if (!self) {
				gLogger->logMsg("Target does not need healing", FOREGROUND_RED);
				gMessageLib->sendSystemMessage(Medic,L"","healing","no_damage_to_heal_target");
				return false;
			}
		}
	}
	gLogger->logMsg("Target Needs Healing", FOREGROUND_BLUE);


	//Get Heal Strength
	//TODO - Food, BEClothing, and Med Center/city bonuses.
	int healthpower = Stim->getHealthHeal();
	int actionpower = Stim->getActionHeal();
	//uint BEClothes = NULL;
	//uint food = NULL;
	//uint MedCityBonus = NULL;
	uint maxhealhealth = healthpower * ((100 + healingskill) / 100);
	uint maxhealaction = healthpower * ((100 + healingskill) / 100);


	//Adjust for Target BF
	int32 BF = Target->getHam()->getBattleFatigue();
	if(BF > 250)
	{
		maxhealhealth -= (maxhealhealth * (BF-250) / 1000);
		maxhealaction -= (maxhealaction * (BF-250) / 1000);

		if(BF > 500) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_medium");
		} else if(BF > 750) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_high");
		} else {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_low");
		}
	}

	int StrengthHealth = min((int)maxhealhealth, TargetMaxHealth-TargetHealth);
	int StrengthAction = min((int)maxhealaction, TargetMaxAction-TargetAction);

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
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_01", "", "", L"", StrengthHealth);
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_10", "", "", L"", StrengthHealth);
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_11", "", "", L"", StrengthAction);
			}
		}

		//Anim
		gMessageLib->sendCreatureAnimation(Medic, BString("heal_self"));

		//CE
		gMessageLib->sendPlayClientEffectLocMessage("clienteffect/healing_healdamage.cef",Medic->mPosition,Medic);

	} else { //if targetting something else
		if(StrengthHealth > 0)
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_06", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_08", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
				}
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_13", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_16", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
				}
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_14", "", "", L"", StrengthAction,"","",L"",Target->getId());
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_17", "", "", L"", StrengthAction,"","",L"",0,Medic->getId());
				}
			}
		}
		//Anim

		gMessageLib->sendCreatureAnimation(Medic, BString("heal_other"));

		//CE
		gMessageLib->sendPlayClientEffectLocMessage("clienteffect/healing_healdamage.cef",PlayerTarget->mPosition,PlayerTarget);
	}
	return true;
}

bool MedicManager::HealDamageRanged(PlayerObject* Medic, CreatureObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject* PlayerTarget = dynamic_cast<PlayerObject*>(Target);
	bool player = false;
	bool self = false;
	Medicine* Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));

	if(PlayerTarget == 0)
	{
		//We are looking at a critter, not a player
		gLogger->logMsg("Heal is targetting a Critter", FOREGROUND_BLUE);
	} else {
		player = true;
		self = (Medic->getId() == PlayerTarget->getId());
		gLogger->logMsg("Heal is targetting a player", FOREGROUND_BLUE);
	}

	//Get Medic Skill Mods
	uint32 healingskill = Medic->getSkillModValue(SMod_healing_range);

	//If Currently in Delay Period

	
	if(Medic->checkPlayerCustomFlag(PlayerCustomFlag_InjuryTreatment))
	{
		//Say you can't heal yet. Doesn't seem to be a valid string entry for this message.
		return false;
	}
	

	float distance = gWorldConfig->getConfiguration("Player_heal_distance",(float)32.0);

	if(Medic->mPosition.distance2D(Target->mPosition) > distance)
	{
		gLogger->logMsg("Heal Target is out of range", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing","no_line_of_sight");
		return false;
	}
	gLogger->logMsg("Heal Target is within range", FOREGROUND_BLUE);

	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->logMsg("Medic does not have ability", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}
	gLogger->logMsg("Medic has Ability Rights", FOREGROUND_BLUE);

	//Do PVP Alignments Match
	if(Medic->getPvPStatus() != Target->getPvPStatus())
	{
		if (!self && PlayerTarget == 0){
			PlayerTarget = Medic;
		}
		else {
		//send pvp_no_help
		gLogger->logMsg("PVP Flag not right", FOREGROUND_RED);
		gMessageLib->sendSystemMessage(Medic,L"","healing","pvp_no_help");
		return false;
		}
	} else {
		if(player)
		{
			//TODO: PVP Flags match, but Status is not correct
			/*if(strcmp(Medic->getFaction(), Target->getFaction()) != 0)
			{
			}*/
		}
	}

	gLogger->logMsg("PVP Flags OK", FOREGROUND_BLUE);

	//Does Target Need Healing
	int TargetHealth = Target->getHam()->mHealth.getCurrentHitPoints();
	int TargetAction = Target->getHam()->mAction.getCurrentHitPoints();
	int TargetMaxHealth = Target->getHam()->mHealth.getMaxHitPoints();
	int TargetMaxAction = Target->getHam()->mAction.getMaxHitPoints();

	if(!(TargetHealth < TargetMaxHealth))
	{
		if(!(TargetAction < TargetMaxAction))
		{
			gLogger->logMsg("Target doesn't need healing", FOREGROUND_RED);
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_63");
			return false;
		}
	}
	gLogger->logMsg("Target Needs Healing", FOREGROUND_BLUE);

	
	//Get Heal Strength
	//TODO - Food, BEClothing, and Med Center/city bonuses.
	int healthpower = Stim->getHealthHeal();
	int actionpower = Stim->getActionHeal();
	uint BEClothes = NULL;
	uint food = NULL;
	uint MedCityBonus = NULL;
	uint maxhealhealth = healthpower * ((100 + healingskill + BEClothes + food) / 100) * MedCityBonus;
	uint maxhealaction = healthpower * ((100 + healingskill + BEClothes + food) / 100) * MedCityBonus;


	//Adjust for Target BF
	int32 BF = Target->getHam()->getBattleFatigue();
	if(BF > 250)
	{
		maxhealhealth -= (maxhealhealth * (BF-250) / 1000);
		maxhealaction -= (maxhealaction * (BF-250) / 1000);

		if(BF > 500) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_medium");
		} else if(BF > 750) {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_high");
		} else {
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","shock_effect_low");
		}
	}

	int StrengthHealth = min((int)maxhealhealth, TargetMaxHealth-TargetHealth);
	int StrengthAction = min((int)maxhealaction, TargetMaxAction-TargetAction);

	Target->getHam()->updatePropertyValue(HamBar_Health, HamProperty_CurrentHitpoints, StrengthHealth);
	Target->getHam()->updatePropertyValue(HamBar_Action, HamProperty_CurrentHitpoints, StrengthAction);
	//Cost.
	int cost = 50;
	

	int TargetMind = Target->getHam()->mMind.getCurrentHitPoints();
	int TargetMaxMind = Target->getHam()->mMind.getMaxHitPoints();

	if (TargetMind < cost) {
		gMessageLib->sendSystemMessage(Medic,L"","healing","not_enough_mind");
		return false;
	}

	Target->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_CurrentHitpoints, -cost);


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
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_01", "", "", L"", StrengthHealth);
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_10", "", "", L"", StrengthHealth);
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_11", "", "", L"", StrengthAction);
			}
		}
		gMessageLib->sendCombatAction(Medic, Target, BString::CRC("throw_grenade_near_healing"), 1, 1, 1);

	} else { //if targetting something else
		if(StrengthHealth > 0)
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_06", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_08", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
				}
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_13", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_16", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
				}
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_14", "", "", L"", StrengthAction,"","",L"",Target->getId());
				if(player)
				{
					gMessageLib->sendSystemMessage(PlayerTarget, L"", "healing_response", "healing_response_17", "", "", L"", StrengthAction,"","",L"",0,Medic->getId());
				}
			}
		}
		gMessageLib->sendCombatAction(Medic, Target, BString::CRC("throw_grenade_medium_healing"), 0, 0, 1);
	}

	return true;
}