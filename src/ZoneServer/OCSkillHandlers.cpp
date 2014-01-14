/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "BankTerminal.h"
#include "CraftingTool.h"
#include "CurrentResource.h"
#include "Item.h"
#include "NPCObject.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "SurveyTool.h"
#include "TravelMapHandler.h"
#include "UIManager.h"
#include "Wearable.h"
#include "WorldConfig.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"




//======================================================================================================================
//
// teach
//

void ObjectController::_handleTeach(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	
	// check if our attribute targetplayer exists
	// check if our target is a player
	// parse the string if a known skill was selected
	// check what skills we can teach
	// check which of these skills the target might learn
	
	PlayerObject*	teacherObject	= dynamic_cast<PlayerObject*>(mObject);
	PlayerObject*	pupilObject	= dynamic_cast<PlayerObject*> (teacherObject->getTarget());

	// check if we have a target
	if(!pupilObject	)
	{
		gMessageLib->sendSystemMessage(teacherObject,L"","teaching","no_target");
		return;
	}

	if(pupilObject == teacherObject)
	{
		// target self:(
		gMessageLib->sendSystemMessage(teacherObject,L"","teaching","no_teach_self");
		return;
	}

	if((teacherObject->getGroupId() == 0)||(teacherObject->getGroupId() != pupilObject	->getGroupId()))
	{
		gMessageLib->sendSystemMessage(teacherObject,L"","teaching","not_in_same_group","","",L"",0,"","",L"",pupilObject->getId());
		return;
	}


	//check if our pupil already gets taught
	if (!pupilObject->getTrade()->getTeacher())
	{
		pupilObject->getTrade()->setTeacher(teacherObject);
		gSkillManager->teach(pupilObject,teacherObject,"");
	}
	else
	{
		gMessageLib->sendSystemMessage(teacherObject,L"","teaching","student_has_offer_to_learn","","",L"",0,"","",L"",pupilObject->getId());
	}

}

//======================================================================================================================
//
// do teach 
//

//======================================================================================================================
/*
void ObjectController::teach(PlayerObject* pupilObject,PlayerObject* teacherObject)
{
	// pupil and teacher bozh exist and are grouped
	// we will now compare the teachers skill list to the pupils skill list 
	// and assemble a list with skill the pupil does not have but were she/he has the prerequesits

	SkillList*	teacherSkills = teacherObject->getSkills();
	SkillList::iterator teacherIt = teacherSkills->begin();
	//SkillTeachContainer* teachContainer = new SkillTeachContainer();
	BStringVector availableSkills;
	uint32 nr = 0;
	while(teacherIt != teacherSkills->end())
	{
		//does the pupil have this skill?
		if (!pupilObject->checkSkill((*teacherIt)->mId))
		{
			//since the pupil doesnt have it ... are the requirements met?
			if ((pupilObject->checkteachSkill((*teacherIt)->mId)))
			{

				//yes the requirements are met :)
				//make sure though that its no novice profession
				//make sure xp requirements are met :P
				if(!strstr(gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi(),"novice"))
				{
					// oki were save to go no novice profession :))
					// add it to the teachable skills list 
					//however be aware of 
					//1) dont add the basic skill nodes for languages - only speak and comprehend
					//1) dont add the basic skill nodes for other skills - like basic_species_human
					if(strstr(gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi(),"language"))
					{

						if(!(!strstr(gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi(),"comprehend"))^!(!strstr(gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi(),"speak")))
						{
							//still have to make sure not to teach shyriiwook and  lekku
							if (gSkillManager->checkLearnSkill((*teacherIt)->mId,pupilObject))
							{
								//teachContainer->addSkill(nr,(*teacherIt)->mId);
								int8 str[64];
								sprintf(str,"@%s:%s","skl_n",gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi());
								availableSkills.push_back(str);
								nr++;
							}
						}
					}
					else
					{
						if(!strstr(gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi(),"species"))
						{
							if ((pupilObject->getXpAmount(gSkillManager->getSkillById((*teacherIt)->mId)->mXpType) >= gSkillManager->getSkillById((*teacherIt)->mId)->mXpCost))
							{
								//teachContainer->addSkill(nr,(*teacherIt)->mId);
								int8 str[64];
								sprintf(str,"@%s:%s","skl_n",gSkillManager->getSkillById((*teacherIt)->mId)->mName.getAnsi());
								availableSkills.push_back(str);
								nr++;
							}
						}
					}	
				}
			}
		}
		teacherIt++;
	}
	if(nr > 0)
	{
		gUIManager->createNewSkillSelectBox(teacherObject,"handleSkillteach","select skill","Select a skill to teach",availableSkills,teacherObject,SUI_LB_OK,pupilObject);
	}
	else
	{
		gMessageLib->sendSystemMessage(teacherObject,L"","teaching","no_skills");

		pupilObject->getTrade()->setTeacher(NULL);
	}
}
*/