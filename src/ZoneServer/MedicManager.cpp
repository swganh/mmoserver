/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "MedicManager.h"
#include "InjuryTreatmentEvent.h"
#include "WoundTreatmentEvent.h"
#include "Inventory.h"
#include "Medicine.h"
#include "ObjectControllerCommandMap.h"
#include "ObjectControllerOpcodes.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "ForageManager.h"
#include "StructureManager.h"

#include "MessageLib/MessageLib.h"
#include "Utils/rand.h"


#ifndef min
#define min(a,b)(((a)<(b))?(a):(b))
#endif

bool			MedicManager::mInsFlag = false;
MedicManager*	MedicManager::mSingleton = NULL;

//consts
const char* const woundpack = "woundpack";
const char* const stim = "stim";
const char* const rangedstim = "ranged";
const char* const self = "self";
const char* const action = "action";
const char* const constitution = "constitution";
const char* const health = "health";
const char* const quickness = "quickness";
const char* const stamina = "stamina";
const char* const strength = "strength";



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

    if(glm::distance(Medic->mPosition, Target->mPosition) > distance)
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_b7");
		return false;
	}

	gUIManager->createNewDiagnoseListBox(Medic, Medic, Target);
	return true;
}

bool MedicManager::CheckMedicine(PlayerObject* Medic, PlayerObject* Target, ObjectControllerCmdProperties* cmdProperties, std::string medpackType)
{
	//This determines what type of medicine the player is using
	bool wound = false;
	uint32 opcode = 0;
	if (medpackType == action || medpackType == constitution || medpackType == health || medpackType == quickness || 
		medpackType == stamina || medpackType == strength || medpackType == woundpack)
		opcode = opOChealwound;
	else if (medpackType == rangedstim)
	{
		// CM
		//uint32 opcode == opOChealdamagerange; 
	}
	else
		opcode = opOChealdamage;


	Medicine* medicine = NULL;

	uint64 MedicinePackObjectID = 0;

	gLogger->log(LogManager::DEBUG,"Check the type of Medicine");

	//If we don't have an OC Controller Cmd Property (ie we have been called by using an item) - go get one
	if(cmdProperties == 0)
	{
		gLogger->log(LogManager::DEBUG,"We need to get Object Properties");

		CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opcode);

		if(it == gObjControllerCmdPropertyMap.end())
		{
			//Cannot find properties
			gLogger->log(LogManager::DEBUG,"Failed to get Object Properties");
			return false;
		} else {
			cmdProperties = ((*it).second);
		}
	}

	//If we weren't triggered by a stim ie. from a command
	if(MedicinePackObjectID == 0)
	{

		//TODO have an automated function that looks for the first item of a certain typein ALL containers

		//Look through inventory to find the correct MedicinePack
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
			//ItemType
			uint32 mItemType = item->getItemType();
			//check the opCode to see which medicine we need ??
			if (medpackType == stim)
			{
				switch(mItemType)
				{
				case ItemType_Stimpack_A:
				case ItemType_Stimpack_B:
				case ItemType_Stimpack_C:
				case ItemType_Stimpack_D:
				case ItemType_Stimpack_E:
					MedicinePackObjectID = item->getId();
					medicine = dynamic_cast<Medicine*>(item);
					break;
				}
			}
			else if (medpackType == rangedstim)
			{
				switch(mItemType)
				{
				case ItemType_Ranged_Stimpack_A:
				case ItemType_Ranged_Stimpack_B:
				case ItemType_Ranged_Stimpack_C:
				case ItemType_Ranged_Stimpack_D:
				case ItemType_Ranged_Stimpack_E:
					MedicinePackObjectID = item->getId();
					medicine = dynamic_cast<Medicine*>(item);
				default:
					break;
				}
			}
			else if (medpackType == action )
			{
				switch(mItemType)
				{
					//action
					case ItemType_Wound_Action_A:
					case ItemType_Wound_Action_B:			
					case ItemType_Wound_Action_C:
					case ItemType_Wound_Action_D:
					case ItemType_Wound_Action_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else if	(medpackType == constitution)
			{
				switch(mItemType)
				{
					//constitution
					case ItemType_Wound_Constitution_A:
					case ItemType_Wound_Constitution_B:			
					case ItemType_Wound_Constitution_C:
					case ItemType_Wound_Constitution_D:
					case ItemType_Wound_Constitution_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else if (medpackType == health)
			{
				switch(mItemType)
				{
					// health
					case ItemType_Wound_Health_A:
					case ItemType_Wound_Health_B:
					case ItemType_Wound_Health_C:
					case ItemType_Wound_Health_D:
					case ItemType_Wound_Health_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else if (medpackType == quickness)
			{
				switch(mItemType)
				{
					// quickness
					case ItemType_Wound_Quickness_A:
					case ItemType_Wound_Quickness_B:
					case ItemType_Wound_Quickness_C:
					case ItemType_Wound_Quickness_D:
					case ItemType_Wound_Quickness_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else if (medpackType == stamina)
			{
				switch(mItemType)
				{
					// stamina
					case ItemType_Wound_Stamina_A:
					case ItemType_Wound_Stamina_B:
					case ItemType_Wound_Stamina_C:
					case ItemType_Wound_Stamina_D:
					case ItemType_Wound_Stamina_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else if (medpackType == strength)
			{
				switch(mItemType)
				{
					// strength
					case ItemType_Wound_Strength_A:
					case ItemType_Wound_Strength_B:
					case ItemType_Wound_Strength_C:
					case ItemType_Wound_Strength_D:
					case ItemType_Wound_Strength_E:
						MedicinePackObjectID = item->getId();
						medicine = dynamic_cast<Medicine*>(item);
						wound = true;
					default:
						break;
				}
			}
			else
			{
				gLogger->log(LogManager::DEBUG, "Invalid Medicine Type");
			}

			if(medicine)
			{
				break;
			} 
			else 
			{
				It++;
			}
		}
		//Check if a Stim was found
		if(medicine == 0)
		{
			gLogger->log(LogManager::DEBUG,"No valid medicine Found");
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_60");
			return false;
		}
	} else 
	{
		gLogger->log(LogManager::DEBUG,"We already have medicine Selected");
		medicine = dynamic_cast<Medicine*>(gWorldManager->getObjectById(MedicinePackObjectID));
	}
	gLogger->log(LogManager::DEBUG,"Medicine ID Found OK");

	//Is the medicine suitable for skill level
	uint64 medicSkill;
	uint64 req = medicine->getSkillRequired("healing_ability");
	if (wound)
	{
		medicSkill = Medic->getSkillModValue(SMod_healing_wound_treatment);
	}
	else
	{
		medicSkill = Medic->getSkillModValue(SMod_healing_ability);
	};
	
	if(medicSkill < req)
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing","insufficient_skill_heal","",L"healingskill",L"req");
		gLogger->log(LogManager::DEBUG,"The selected medicine is too high level.");
		return false;
	}

	if (medpackType == rangedstim)
		return HealDamageRanged(Medic, Target, MedicinePackObjectID, cmdProperties);
	

	if (medpackType == stim)
		return HealDamage(Medic, Target, MedicinePackObjectID, cmdProperties);
	

	if (wound)
		return HealWound(Medic, Target, MedicinePackObjectID, cmdProperties, medpackType);

	return false;
}

bool MedicManager::HealDamage(PlayerObject* Medic, PlayerObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties)
{
	Medicine* Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));
	bool isSelf = (Medic->getId() == Target->getId());

	//Get Medic Skill Mods
	uint32 healingskill = Medic->getSkillModValue(SMod_healing_injury_treatment);

	//If Currently in Delay Period
	if(Medic->checkPlayerCustomFlag(PlayerCustomFlag_InjuryTreatment))
	{
		//Say you can't heal yet.
		gMessageLib->sendSystemMessage(Medic,L"","healing_response", "healing_must_wait");
		return false;
	}

	if (!MedicManager::CheckMedicRange(Medic, Target, (float)6.0))
	{
		return false;
	}

	//If we don't have an OC Controller Cmd Property (ie we have been called by using an item) - go get one
	if(cmdProperties == 0)
	{
		gLogger->log(LogManager::DEBUG,"We need to get Object Properties");

		CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opOChealdamage);

		if(it == gObjControllerCmdPropertyMap.end())
		{
			//Cannot find properties
			gLogger->log(LogManager::DEBUG,"Failed to get Object Properties");
			return false;
		} else {
			cmdProperties = ((*it).second);
		}
	}

	if (!CheckMedicRange(Medic, Target, (float)6.0))
		return false;

	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->log(LogManager::DEBUG,"Medic does not have ability");
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}
	gLogger->log(LogManager::DEBUG,"Medic has Ability Rights");

	//Does Target Need Healing
	int TargetHealth = Target->getHam()->mHealth.getCurrentHitPoints();
	int TargetAction = Target->getHam()->mAction.getCurrentHitPoints();
	int TargetMaxHealth = Target->getHam()->mHealth.getMaxHitPoints();
	int TargetMaxAction = Target->getHam()->mAction.getMaxHitPoints();

	if(!(TargetHealth < TargetMaxHealth))
	{
		if(!(TargetAction < TargetMaxAction))
		{
			if (isSelf){
				gLogger->log(LogManager::DEBUG,"You don't need healing");
				gMessageLib->sendSystemMessage(Medic,L"","healing","no_damage_to_heal_self");
				return false;
			}
			if (!isSelf) {
				gLogger->log(LogManager::DEBUG,"Target does not need healing");
				gMessageLib->sendSystemMessage(Medic,L"","healing","no_damage_to_heal_target");
				return false;
			}
		}
	}
	gLogger->log(LogManager::DEBUG,"Target Needs Healing");


	//Get Heal Strength
	//TODO - BEClothing, and Med Center/city bonuses.
	int healthpower = Stim->getHealthHeal();
	int actionpower = Stim->getActionHeal();
	//uint BEClothes = NULL;
	//uint MedCityBonus = NULL;
	uint maxhealhealth = healthpower * ((100 + healingskill) / 100);
	uint maxhealaction = actionpower * ((100 + healingskill) / 100);


	//Adjust for Target BF
	maxhealhealth = MedicManager::CalculateBF(Medic, Target, maxhealhealth);
	maxhealaction = MedicManager::CalculateBF(Medic, Target, maxhealaction);
	
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
	if(!isSelf)
		gSkillManager->addExperience(XpType_medical, (int)((StrengthHealth + StrengthAction)/4), Medic);

	if(isSelf) //If targetting self
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

	} 
	else 
	{ //if targetting something else
		if(StrengthHealth > 0)
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_06", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_08", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_13", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_16", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
			}
		} 
		else 
		{
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_14", "", "", L"", StrengthAction,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_17", "", "", L"", StrengthAction,"","",L"",0,Medic->getId());
			}
		}
		//Anim
		gMessageLib->sendCreatureAnimation(Medic, BString("heal_other"));

		//CE
		gMessageLib->sendPlayClientEffectLocMessage("clienteffect/healing_healdamage.cef",Target->mPosition,Target);
	}
	return true;
}

bool MedicManager::HealDamageRanged(PlayerObject* Medic, PlayerObject* Target, uint64 StimPackObjectID, ObjectControllerCmdProperties* cmdProperties)
{
	bool isSelf = false;
	Medicine* Stim = dynamic_cast<Medicine*>(gWorldManager->getObjectById(StimPackObjectID));

	isSelf = (Medic->getId() == Target->getId());

	//Get Medic Skill Mods
	uint32 healingskill = Medic->getSkillModValue(SMod_healing_range);

	//If Currently in Delay Period
	
	if(Medic->checkPlayerCustomFlag(PlayerCustomFlag_InjuryTreatment))
	{
		//Say you can't heal yet.
		gMessageLib->sendSystemMessage(Medic,L"","healing_response", "healing_must_wait");
		return false;
	}
	//check range
	if (!CheckMedicRange(Medic, Target, (float)32.0))
		return false;
	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->log(LogManager::DEBUG,"Medic does not have ability");
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_heal");
		return false;
	}
	gLogger->log(LogManager::DEBUG,"Medic has Ability Rights");

	//Does Target Need Healing
	int TargetHealth = Target->getHam()->mHealth.getCurrentHitPoints();
	int TargetAction = Target->getHam()->mAction.getCurrentHitPoints();
	int TargetMaxHealth = Target->getHam()->mHealth.getMaxHitPoints();
	int TargetMaxAction = Target->getHam()->mAction.getMaxHitPoints();

	if(!(TargetHealth < TargetMaxHealth))
	{
		if(!(TargetAction < TargetMaxAction))
		{
			gLogger->log(LogManager::DEBUG,"Target doesn't need healing");
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_63");
			return false;
		}
	}
	gLogger->log(LogManager::DEBUG,"Target Needs Healing");

	
	//Get Heal Strength
	//TODO - BEClothing, and Med Center/city bonuses.
	int healthpower = Stim->getHealthHeal();
	int actionpower = Stim->getActionHeal();
	uint BEClothes = NULL;
	uint MedCityBonus = NULL;
	uint maxhealhealth = healthpower * ((100 + healingskill + BEClothes) / 100) * MedCityBonus;
	uint maxhealaction = actionpower * ((100 + healingskill + BEClothes) / 100) * MedCityBonus;


	//Adjust for Target BF
	maxhealhealth = MedicManager::CalculateBF(Medic, Target, maxhealhealth);
	maxhealaction = MedicManager::CalculateBF(Medic, Target, maxhealaction);

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
	if(!isSelf)
		gSkillManager->addExperience(XpType_medical, (int)((StrengthHealth + StrengthAction)/4), Medic);

	if(isSelf) //If targetting self
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
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_08", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_04", "", "", L"", StrengthAction);
			} else {
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_13", "", "", L"", StrengthHealth,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_16", "", "", L"", StrengthHealth,"","",L"",0,Medic->getId());
			}
		} else {
			if(StrengthAction > 0)
			{
				gMessageLib->sendSystemMessage(Medic, L"", "healing_response", "healing_response_14", "", "", L"", StrengthAction,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_17", "", "", L"", StrengthAction,"","",L"",0,Medic->getId());
			}
		}
		gMessageLib->sendCombatAction(Medic, Target, BString::CRC("throw_grenade_medium_healing"), 0, 0, 1);
	}

	return true;
}
//HealWounds 
bool MedicManager::HealWound(PlayerObject* Medic, PlayerObject* Target, uint64 WoundPackobjectID, ObjectControllerCmdProperties* cmdProperties, std::string healType)
{
	bool isSelf = false;
	Medicine* WoundPack = dynamic_cast<Medicine*>(gWorldManager->getObjectById(WoundPackobjectID));
	isSelf = (Medic->getId() == Target->getId());

	uint32 healingskill = Medic->getSkillModValue(SMod_healing_wound_treatment);
	
	if(Medic->checkPlayerCustomFlag(PlayerCustomFlag_WoundTreatment))
	{
		gMessageLib->sendSystemMessage(Medic,L"","healing_response", "enhancement_must_wait");
		return false;
	}
	if (!CheckMedicRange(Medic, Target, (float)6.0))
		return false;
	//Does Medic have ability
	if(!Medic->verifyAbility(cmdProperties->mAbilityCrc))
	{
		gLogger->log(LogManager::DEBUG,"Medic does not have ability");
		gMessageLib->sendSystemMessage(Medic,L"","healing_response","cannot_enhance");
		return false;
	}
	gLogger->log(LogManager::DEBUG,"Medic has Ability Rights");

	int TargetWounds = 0;

	int32 WoundHealPower = 0;
	int32 maxwoundheal = 0;
	Ham* ham = Target->getHam();
	//check the wounds type
	if (healType == action)
	{
		TargetWounds = ham->mAction.getWounds();
		WoundHealPower = WoundPack->getHealWoundAction();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Action ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_50","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_53", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_56", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());
			}
		}
	}
	else if	(healType == constitution)
	{
		TargetWounds = ham->mConstitution.getWounds();
		WoundHealPower = WoundPack->getHealWoundConstitution();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Constitution ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_20","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_32", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_41", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());
			}
		}
	}
	else if (healType == health)
	{
		TargetWounds = ham->mHealth.getWounds();
		WoundHealPower = WoundPack->getHealWoundHealth();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Health ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_49","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_52", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_55", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());
			}
		}
	}
	else if (healType == quickness)
	{
		TargetWounds = ham->mQuickness.getWounds();
		WoundHealPower = WoundPack->getHealWoundQuickness();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Quickness ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_21","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_33", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_42", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());	
			}
		}
	}
	else if (healType == stamina)
	{
		TargetWounds = ham->mStamina.getWounds();
		WoundHealPower = WoundPack->getHealWoundStamina();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Stamina ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_22","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_34", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_43", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());
			}
		}
	}
	else if (healType == strength)
	{
		TargetWounds = ham->mStrength.getWounds();
		WoundHealPower = WoundPack->getHealWoundStrength();
		maxwoundheal = MedicManager::CalculateBF(Medic, Target, WoundHealPower);
		maxwoundheal = maxwoundheal * ((100 + healingskill) / 100);
		maxwoundheal = min(maxwoundheal, TargetWounds);
		ham->updatePropertyValue(HamBar_Strength ,HamProperty_Wounds, -maxwoundheal);
		if (maxwoundheal > 0)
		{
			//success message
			if (isSelf)
				gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_19","","",L"",maxwoundheal);
			else{
				gMessageLib->sendSystemMessage(Medic ,L"", "healing_response", "healing_response_31", "", "", L"", maxwoundheal,"","",L"",Target->getId());
				gMessageLib->sendSystemMessage(Target, L"", "healing_response", "healing_response_46", "", "", L"", maxwoundheal,"","",L"",0,Medic->getId());	
			}
		}
	}

	if(maxwoundheal <= 0)
	{
		if (isSelf){
			gLogger->log(LogManager::DEBUG,"You don't need wound healing");
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_67");
			return false;
		}
		if (!isSelf) {
			gLogger->log(LogManager::DEBUG,"Unable to find any wounds which you can heal");
			gMessageLib->sendSystemMessage(Medic,L"","healing_response","healing_response_64");
			return false;
		}
	}

	//Get Wound Heal Strength
	//TODO - BEClothing, and Med Center/city bonuses.
	//These will probably affect the skills

	//uint BEClothes = NULL;
	//uint MedCityBonus = NULL;

	if (!isSelf)
	{
		//Anim
		gMessageLib->sendCreatureAnimation(Medic, BString("heal_other"));

		//CE
		gMessageLib->sendPlayClientEffectLocMessage("clienteffect/healing_healwound.cef",Target->mPosition,Target);
		//XP
		gSkillManager->addExperience(XpType_medical, (int)((maxwoundheal)*2.5), Medic);
	}
	else
	{
		//Anim
		gMessageLib->sendCreatureAnimation(Medic, BString("heal_self"));

		//CE
		gMessageLib->sendPlayClientEffectLocMessage("clienteffect/healing_healwound.cef",Target->mPosition,Target);
	}
		

	if(WoundPack->ConsumeUse(Medic))
	{
		Inventory* inventory = dynamic_cast<Inventory*>(Medic->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		inventory->deleteObject(WoundPack);
	}

	return true;
}
//Check Heal Range
bool MedicManager::CheckMedicRange(PlayerObject* Medic, PlayerObject* Target, float healRange)
{
	float distance = gWorldConfig->getConfiguration("Player_heal_distance", healRange);

    if(glm::distance(Medic->mPosition, Target->mPosition) > distance)
	{
		gLogger->log(LogManager::DEBUG,"Heal Target is out of range");
		gMessageLib->sendSystemMessage(Medic,L"","healing","no_line_of_sight");
		return false;
	}
	gLogger->log(LogManager::DEBUG,"Heal Target is within range");
	return true;
}
//InjuryTreatment Event

int32 MedicManager::CalculateBF(PlayerObject* Medic, PlayerObject* Target, int32 maxhealamount)
{
	int32 BF = Target->getHam()->getBattleFatigue();
	if(BF > 250)
	{
		maxhealamount -= (maxhealamount * (BF-250) / 1000);
		if (Target && Medic->getId() != Target->getId())
		{
			if(BF > 500) {
				gMessageLib->sendSystemMessage(Target,L"","healing","shock_effect_medium_target");
			} else if(BF > 750) {
				gMessageLib->sendSystemMessage(Target,L"","healing","shock_effec_high_target");
			} else {
				gMessageLib->sendSystemMessage(Target,L"","healing","shock_effect_low_target");
			}
		}
		else
		{
			if(BF > 500) {
				gMessageLib->sendSystemMessage(Medic,L"","healing","shock_effect_medium");
			} else if(BF > 750) {
				gMessageLib->sendSystemMessage(Medic,L"","healing","shoc_effect_high");
			} else {
				gMessageLib->sendSystemMessage(Medic,L"","healing","shock_effect_low");
			}
		}
	}
	return maxhealamount;
}
void MedicManager::startInjuryTreatmentEvent(PlayerObject* Medic)
{
		uint healingspeed = Medic->getSkillModValue(SMod_healing_injury_speed);
		int delay = (int)((( 100 - (float)healingspeed ) / 100 ) * 10000); 
		uint64 cooldown = std::max(4000, delay);
		uint64 now = gWorldManager->GetCurrentGlobalTick();

		Medic->getController()->addEvent(new InjuryTreatmentEvent(now + cooldown), cooldown);
		Medic->togglePlayerCustomFlagOn(PlayerCustomFlag_InjuryTreatment);
}
void MedicManager::startWoundTreatmentEvent(PlayerObject* Medic)
{
		uint healingspeed = Medic->getSkillModValue(SMod_healing_wound_speed);
		int delay = (int)((( 100 - (float)healingspeed ) / 100 ) * 10000); 
		uint64 cooldown = std::max(4000, delay);
		uint64 now = gWorldManager->GetCurrentGlobalTick();

		Medic->getController()->addEvent(new WoundTreatmentEvent(now + cooldown), cooldown);
		Medic->togglePlayerCustomFlagOn(PlayerCustomFlag_WoundTreatment);
}
//Foraging
void MedicManager::successForage(PlayerObject* player)
{
	//Chance of success = sqrt(skill)/20 + 0.15
	//Chance in down = chance/2

	//First lets calc our chance to 'win'
	//This is the magic formula!	
	double chance = std::sqrt((double)player->getSkillModValue(20))/20 + 0.15;

	if(!gStructureManager->checkCityRadius(player)) 
		chance = chance*50;
	else
		chance = chance*100;

	if((gRandom->getRand() % 100) <= chance)
	{
		// YOU WIN!
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_success");
	}
	else
	{
		//YOU LOSE! GOOD DAY SIR!
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_fail");
	}

	player->setForaging(false);
}