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

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"




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
        gMessageLib->SendSystemMessage(::common::OutOfBand("teaching", "no_target"), teacherObject);
        return;
    }

    if(pupilObject == teacherObject)
    {
        // target self:(
        gMessageLib->SendSystemMessage(::common::OutOfBand("teaching", "no_teach_self"), teacherObject);
        return;
    }

    if((teacherObject->getGroupId() == 0)||(teacherObject->getGroupId() != pupilObject	->getGroupId()))
    {
        gMessageLib->SendSystemMessage(::common::OutOfBand("teaching", "not_in_same_group", 0, pupilObject->getId(), 0), teacherObject);
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
        gMessageLib->SendSystemMessage(::common::OutOfBand("teaching", "student_has_offer_to_learn", 0, pupilObject->getId(), 0), teacherObject);
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
