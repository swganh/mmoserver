/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Trainer.h"
#include "ActiveConversation.h"
#include "Bank.h"
#include "CellObject.h"
#include "Conversation.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "QuadTree.h"
#include "SkillManager.h"
#include "WorldManager.h"
#include "UIManager.h"
#include "WorldConfig.h"
#include "Tutorial.h"
#include "ZoneTree.h"

#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"

//=============================================================================

Trainer::Trainer() : NPCObject(), mPlayerGotRequirementsForMasterSkill(false)
{
	mNpcFamily	= NpcFamily_Trainer;

	mRadialMenu = RadialMenuPtr(new RadialMenu());

	mRadialMenu->addItem(1,0,radId_converseStart,radAction_Default); 
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

Trainer::~Trainer()
{
}

//=============================================================================

void Trainer::filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player)
{
	bool requirementsMet;

	std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();

	// gLogger->logMsgF("Trainer::filterConversationOptions: page->mId = %u",MSG_NORMAL,page->mId);
	switch(page->mId)
	{
		// currently learnable skills
		case 2:
		{
			// gLogger->logMsg("Trainer::filterConversationOptions: Currently learnable skills (case 2)");
			while(optionIt != page->mOptions.end())
			{
				Skill* skill = gSkillManager->getSkillByName((*optionIt)->mStfVariable);
				requirementsMet = false;

				// its no skill option
				if (!skill)
				{
					// gLogger->logMsg("Trainer::filterConversationOptions: Its no skill option");
					requirementsMet = true;
				}

				// see if we have it already and check xp
				// Note: We do not handle lack of SP as an error now... that's done later in this chain of training events.
				else if(player->checkSkill(skill->mId))
				{
					// We already have this skill.
					requirementsMet = false;
				}
				else if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
				{
					// We do not have enough of XP for training this skill.
					// gLogger->logMsg("Trainer::filterConversationOptions: Missing XP");
					requirementsMet = false;
				}
				else
				{
					// If we end up here, we have at least some skill to train.
					// gotAllSkills = false;

					// no skill requirements
					if (!skill->mSkillsRequired.size())
					{
						// gLogger->logMsg("Trainer::filterConversationOptions: Novice");
						// avaliableSkills++;		// We got a "Novice" skill.
						requirementsMet = true;
					}
					// see if we got the required skills
					else
					{
						SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
						requirementsMet = true;
						
						// avaliableSkills++;
						while(reqSkillIt != skill->mSkillsRequired.end())
						{
							// we don't have the requirements
							if(!player->checkSkill(*reqSkillIt))
							{
								requirementsMet = false;
								// avaliableSkills--;
								break;
							}
							++reqSkillIt;
						}
					}
				}

				// add it to the send list
				if (requirementsMet)
				{
					filteredOptions->push_back(*optionIt);
					// gLogger->logMsg("Trainer::filterConversationOptions: pushed something");
				}
				++optionIt;
			}
		}
		break;

		// It will cost %DI credits to train in %TO. Would you like to continue? 
		case 3:
		{
			// gLogger->logMsg("Trainer::filterConversationOptions: It will cost %DI credits to train in %TO. Would you like to continue?");
			while(optionIt != page->mOptions.end())
			{
				// gLogger->logMsg("Trainer::filterConversationOptions: pushed something");
				filteredOptions->push_back(*optionIt);
				++optionIt;
			}
		}
		break;
		
		// What skills will I be able to learn next?
		case 4:
		{
			// gLogger->logMsg("Trainer::filterConversationOptions: What skills will I be able to learn next?");
 			while(optionIt != page->mOptions.end())
			{
				Skill* skill = gSkillManager->getSkillByName((*optionIt)->mStfVariable);
				requirementsMet = false;

				// its no skill option
				if (!skill)
				{
					// gLogger->logMsg("Trainer::filterConversationOptions: Its no skill option");
					requirementsMet = true;
				}

				// see if we have it already
				else if (player->checkSkill(skill->mId))
				{
					requirementsMet = false;
				}
				else
				{
					// no skill requirements
					if (!skill->mSkillsRequired.size())
					{
						requirementsMet = true;
					}
					// see if we got the required skills
					else
					{
						SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
						requirementsMet = true;

						while(reqSkillIt != skill->mSkillsRequired.end())
						{
							// we don't have the requirements
							if(!player->checkSkill(*reqSkillIt))
							{
								requirementsMet = false;
								break;
							}
							++reqSkillIt;
						}
					}
				}

				// add it to the send list
				if(requirementsMet)
				{
					// gLogger->logMsg("Trainer::filterConversationOptions: pushed next skill");
					filteredOptions->push_back(*optionIt);
				}
				++optionIt;
			}
		}
		break;

		// by default don't filter
		default:
		{
			// gLogger->logMsg("Trainer::filterConversationOptions: default");
			while(optionIt != page->mOptions.end())
			{
				// gLogger->logMsg("Trainer::filterConversationOptions: pushed something");
				filteredOptions->push_back(*optionIt);
				++optionIt;
			}
		}
		break;
	}
}

//=============================================================================

// Return pageLink for next conversation.
uint32 Trainer::handleConversationEvent(ActiveConversation* av,ConversationPage* page,ConversationOption* option,PlayerObject* player)
{
	// gLogger->logMsgF("Trainer::conversationEvent: page->mId = %u",MSG_NORMAL,page->mId);
		
	std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();
	uint32 pageLink = page->mId;

	switch(page->mId)
	{
		// Select currently learnable skills
		case 2:
		{
			mPlayerGotRequirementsForMasterSkill = false;
			bool noviceSkill = false;
			bool gotAllSkills = true;	// As long as we have not detected any missing skills.
			bool gotRequirementsForSomeSkill = false;
			uint32 skillsToLearn = 0;

			uint32 numberOfSkills = 0;
			uint32 knownSkills = 0;

			// gLogger->logMsg("Trainer::conversationEvent: Currently learnable skills (case 2)");
			while (optionIt != page->mOptions.end())
			{
				Skill* skill = gSkillManager->getSkillByName((*optionIt)->mStfVariable);
				numberOfSkills++;

				// its no skill option
				if (!skill)
				{
					numberOfSkills--;
					// gLogger->logMsg("Trainer::conversationEvent: Its no skill option");
				}

				// see if we have it already and check xp
				// Note: We do not handle lack of SP as an error now... that's done later in this chain of events.
				else if (player->checkSkill(skill->mId))
				{
					// We already have this skill.
					knownSkills++;
					// gLogger->logMsg("Trainer::conversationEvent: We already have this skill");
				}
				else if (!skill->mSkillsRequired.size())
				{
					// No skill requirements, we got a "Novice" skill from a basic profession.
					// gLogger->logMsg("Trainer::conversationEvent: Novice");
					skillsToLearn++;
					gotAllSkills = false;	// Do not have all skills if we end up here.
					noviceSkill = true;
				}
				else
				{
					// gLogger->logMsg("Trainer::conversationEvent: Do not have all skills if we end up here");
					gotAllSkills = false;	// Do not have all skills if we end up here.

					// see if we got the required skills
					SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
					bool requirementsMet = true;
					skillsToLearn++;

					while (reqSkillIt != skill->mSkillsRequired.end())
					{
						if (!player->checkSkill(*reqSkillIt))
						{
							// we don't have the requirements
							// gLogger->logMsg("Trainer::conversationEvent: We don't have the requirements");
							requirementsMet = false;
							break;
						}
						++reqSkillIt;
					}
					
					if (requirementsMet)
					{
						// gLogger->logMsg("Trainer::conversationEvent: We DO have the requirements, check if we have the xp for next level");
						gotRequirementsForSomeSkill = true;

						// We DO have the requirements, check if we have the xp for next level.
						if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
						{
							// We do not have enough of XP for training this skill.
							// gLogger->logMsg("Trainer::conversationEvent: Missing XP");
							skillsToLearn--;
						}
					}
					else
					{
						skillsToLearn--;
					}
				}
				++optionIt;
			}

			// Now we must return status of this session.
			if (skillsToLearn == 0)
			{
				// Either we have them all, i.e. we are a Master, or we lack xp for training OR we lack the required skills to train it.
				if (gotAllSkills)
				{
					// We are a Master!
					// gLogger->logMsg("Trainer::conversationEvent: We are a Master!");
					pageLink = 9;
				}
				else if (gotRequirementsForSomeSkill)
				{
					// We lack xp for training.
					// gLogger->logMsg("Trainer::conversationEvent: We lack xp for training");
					pageLink = 8;
				}
				else
				{
					// This is not a valid state any more.
					// We do not have requirements for any skill, sounds like we are attempting an elite trainers novice skill.
					// gLogger->logMsg("Trainer::conversationEvent: ERROR: Sounds like we are attempting an elite trainers novice skill");
					pageLink = 12;
				}
			}
			else
			{
				if (noviceSkill) // avaliableSkills should be 1
				{
					// gLogger->logMsg("Trainer::conversationEvent: We train to Novice.");
					// Not implemented yet. Special handling for the Space Station Tutorial Only. 
				}
				else
				{

					if ((knownSkills + 1) ==  numberOfSkills)
					{
						// We have all skills except the last one, we are going for a Master skill...
						mPlayerGotRequirementsForMasterSkill = true;
					}

					// Let's select a skill to train. This is the default path.
					// retLinkPage = 0;	// Use caller default linkPage.
					// gLogger->logMsg("Trainer::conversationEvent: Default, we have some skills to train.");
				}
			}
		}
		break;


		// Skill is selected.
		case 3:
		{
			// gLogger->logMsg("Trainer::conversationEvent: Skill selected");
			bool failed = true;	
			Skill* skill = gSkillManager->getSkillByName(av->getTOStfVariable());

			// its no skill option
			if (!skill)
			{
				// This is a system error.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: It's no skill option\n");
			}
			// no skill requirements, Novice Skills have no requirements.
			/* 
			else if (!skill->mSkillsRequired.size())
			{
				// This is a system error.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: No skill requirements");
				break;
			}
			*/ 
			else if(player->checkSkill(skill->mId))
			{
				// This is a system failure.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: Player already have the skill\n");
			}
			else if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
			{
				// This is a system failure.
				gLogger->logMsgF("Trainer::conversationEvent: ERROR: Player need %u XP, but only have %u",MSG_NORMAL,player->getXpAmount(skill->mXpType), skill->mXpCost);
			}
			// see if we got the required skills
			else
			{
				SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
				failed = false;
				while(reqSkillIt != skill->mSkillsRequired.end())
				{
					// we don't have the requirements
					if (!player->checkSkill(*reqSkillIt))
					{
						// This is a system failure.
						gLogger->logMsg("Trainer::conversationEvent: ERROR: Player does not have all requirements\n");
						failed = true;
						break;
					}
					++reqSkillIt;
				}
			}
			if (failed)
			{
				// This is a system failure.
				gMessageLib->sendSystemMessage(player,L"","teaching","learning_failed");
				pageLink = 0;
				break;
			}

			// Here we start the non-fatal error processing.
			// gLogger->logMsg("Trainer::conversationEvent: Start the non-fatal error processing");
			if (player->getSkillPointsLeft() < skill->mSkillPointsRequired)
			{
				// Player lack skillpoints for training this skill.
				// gLogger->logMsgF("Trainer::conversationEvent: Player need %u skillpoints, but only have %u",MSG_NORMAL,skill->mSkillPointsRequired, player->getSkillPointsLeft());
				pageLink = 10;	// pageLink for this condition.
				break;
			}

			if (gWorldConfig->isTutorial())
			{
				// We do not charge any cost for training in the Tutorial.
				pageLink = 15;
			}
			else if ((dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits() < av->getDI())
				 && (dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->getCredits() < av->getDI())
				)
			{
				// Player lack credits in both inventory and bank.

				pageLink = 11;	// pageLink for this situation.
				// gLogger->logMsgF("Trainer::conversationEvent: Player needs %u credits, but only have %u as cash",MSG_NORMAL, av->getDI(),
				// 				dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits());
				// System message: You lack the %DI credits required for training in %TO. 
				gMessageLib->sendSystemMessage(player,L"","skill_teacher","prose_nsf",av->getTOStfFile(),av->getTOStfVariable(),L"",av->getDI());

				break;
			}
			else
			{
				// Let's start credit transaction for this skill...
				// Do that in next state...
			}
		}		
		break;

		// View info about skills
		case 4:
		{
			bool requirementsMet;
			bool noviceSkill = false;
			bool gotAllSkills = true;	// As long as we have not detected any missing skills.

			// gLogger->logMsg("Trainer::conversationEvent: View info about skills");
			while(optionIt != page->mOptions.end())
			{
				Skill* skill = gSkillManager->getSkillByName((*optionIt)->mStfVariable);
				requirementsMet = false;

				// its no skill option
				if (!skill)
				{
					// gLogger->logMsg("Trainer::conversationEvent: Its no skill option");
					requirementsMet = true;
				}

				// see if we have it already
				else if(player->checkSkill(skill->mId))
				{
					// We already have this skill.
					requirementsMet = false;
				}
				else
				{
					// If we end up here, we have at least some skill to train.
					gotAllSkills = false;

					// no skill requirements
					if (!skill->mSkillsRequired.size())
					{
						// gLogger->logMsg("Trainer::conversationEvent: Novice");
						noviceSkill = true;
						requirementsMet = true;
					}
					// see if we got the required skills
					else
					{
						SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
						requirementsMet = true;
						
						while(reqSkillIt != skill->mSkillsRequired.end())
						{
							// we don't have the requirements
							if(!player->checkSkill(*reqSkillIt))
							{
								requirementsMet = false;
								break;
							}
							++reqSkillIt;
						}
					}
				}
				++optionIt;
			}

			// Now we must return status of this session.
			// We have them all, i.e. we are a Master.
			if (gotAllSkills)
			{
				// We are a Master!
				// gLogger->logMsg("Trainer::conversationEvent: We are a Master!");
				pageLink = 9;
			}
			else
			{
				if (noviceSkill) // avaliableSkills should be 1
				{
					// gLogger->logMsg("Trainer::conversationEvent: We train to Novice.");
					// Not implemented yet. Special handling for the Space Station Tutorial Only. 
				}
				else
				{
					// Let's select a skill to train. This is the default path.
					// retLinkPage = 0;	// Use caller default linkPage.
					// gLogger->logMsg("Trainer::conversationEvent: Default, we have some skills to train.");
				}
			}
		}
		break;


		// Train the skill
		case 5:
		{
			bool failed = true;

			// gLogger->logMsg("Trainer::conversationEvent: Learn request");
			Skill* skill = gSkillManager->getSkillByName(av->getTOStfVariable());

			// its no skill option
			if (!skill)
			{
				// This is a system error.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: It's no skill option\n");
			}
			else if(player->checkSkill(skill->mId))
			{
				// This is a system failure.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: Player already have the skill\n");
			}
			else if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
			{
				// This is a system failure.
				gLogger->logMsgF("Trainer::conversationEvent: ERROR: Player need %u XP, but only have %u",MSG_NORMAL,player->getXpAmount(skill->mXpType), skill->mXpCost);
			}
			// see if we got the required skills
			else
			{
				failed = false;
				SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();

				while(reqSkillIt != skill->mSkillsRequired.end())
				{
					// We don't have the requirements
					if (!player->checkSkill(*reqSkillIt))
					{
						// This is a system failure.
						gLogger->logMsg("Trainer::conversationEvent: ERROR: Player does not have all requirements\n");
						failed = true;
						break;
					}
					++reqSkillIt;
				}
			}
			if (!failed)
			{
				// Let's train this skill...
				// gLogger->logMsg("Trainer::conversationEvent: Processing %DI credit payment for %TO training.");
				gMessageLib->sendSystemMessage(player,L"","skill_teacher","prose_pay",av->getTOStfFile(),av->getTOStfVariable(),L"",av->getDI());

				// if (strstr(skill->mName.getAnsi(),"master"))
				if (mPlayerGotRequirementsForMasterSkill)
				{
					// We are going to train to Master...
					// gLogger->logMsg("Trainer::conversationEvent: We are going to train to Master...");
					// postProcessfilter(av, page, player, 13);
					postProcessfilter(av, player, 13);
					pageLink = 0;	// Terminate conversation
				}

				if (!(dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->updateCredits(-skill->mMoneyRequired)))
				{
					if (!(dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->updateCredits(-skill->mMoneyRequired)))
					{	
						// This is a system error.
						gMessageLib->sendSystemMessage(player,L"","skill_teacher","prose_nsf");
						gLogger->logMsg("Trainer::conversationEvent: ERROR: Error verifying credits\n");
						pageLink = 0;
					}
				}
			}
			else
			{
				// This is a system error.
				gMessageLib->sendSystemMessage(player,L"","teaching","learning_failed");
				// gLogger->logMsg("Trainer::conversationEvent: Error verifying credits");
				pageLink = 0;
			}
		}		
		break;


		// Train the skill in the Tutorial
		// NOTE: This case is never called, not even from Tutorial.
		case 15:
		{
			bool failed = true;

			// gLogger->logMsg("Trainer::conversationEvent: Learn request");
			Skill* skill = gSkillManager->getSkillByName(av->getTOStfVariable());

			// its no skill option
			if (!skill)
			{
				// This is a system error.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: It's no skill option\n");
			}
			// no skill requirements, Novice Skills have no requirements.
			else if(player->checkSkill(skill->mId))
			{
				// This is a system failure.
				gLogger->logMsg("Trainer::conversationEvent: ERROR: Player already have the skill\n");
			}
			else if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
			{
				// This is a system failure.
				gLogger->logMsgF("Trainer::conversationEvent: ERROR: Player need %u XP, but only have %u",MSG_NORMAL,player->getXpAmount(skill->mXpType), skill->mXpCost);
			}
			// see if we got the required skills
			else
			{
				failed = false;
				SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();

				while(reqSkillIt != skill->mSkillsRequired.end())
				{
					// We don't have the requirements
					if (!player->checkSkill(*reqSkillIt))
					{
						// This is a system failure.
						gLogger->logMsg("Trainer::conversationEvent: ERROR: Player does not have all requirements\n");
						failed = true;
						break;
					}
					++reqSkillIt;
				}
			}
			if (!failed)
			{
				// We do not charge any cost for training in the Tutorial.
				gLogger->logMsg("Trainer::conversationEvent: No charge for training in Tutorial.\n");
			}
			else
			{
				// This is a system error.
				gMessageLib->sendSystemMessage(player,L"","teaching","learning_failed");
				// gLogger->logMsg("Trainer::conversationEvent: Error verifying credits");
				pageLink = 0;
			}
		}		
		break;

		case 7:
		{
			// Selected more info about this skill.
			Skill* skill = gSkillManager->getSkillByName(av->getTOStfVariable());
			if (skill)
			{

				int8 str[800];

				BStringVector populatedListBox;
				int8 strCaption[128];
				sprintf(strCaption,"@%s:%s","skl_n", skill->mName.getAnsi());

				sprintf(str,"MONETARY COST");
				populatedListBox.push_back(str);

				if (gWorldConfig->isTutorial() && (!skill->mSkillsRequired.size()))
				{
					sprintf(str,"%u credits", 0);
				}
				else
				{
					sprintf(str,"%u credits", skill->mMoneyRequired);
				}
				populatedListBox.push_back(str);

				sprintf(str,"SKILL POINT COST");
				populatedListBox.push_back(str);

				sprintf(str,"%u points", skill->mSkillPointsRequired);
				populatedListBox.push_back(str);

				sprintf(str,"REQUIRED SKILLS");
				populatedListBox.push_back(str);

				SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
				while (reqSkillIt != skill->mSkillsRequired.end())
				{
					// Add all skills required.
					// if (!player->checkSkill(*reqSkillIt))
					{
						// We don't have this required skill
						sprintf(str,"@%s:%s","skl_n", gSkillManager->getSkillById(*reqSkillIt)->mName.getAnsi());
						populatedListBox.push_back(str);
					}
					++reqSkillIt;
				}

				sprintf(str,"XP COST");
				populatedListBox.push_back(str);
				
				// gLogger->logMsgF("Trainer::conversationEvent: Skill type = %u named %s",MSG_NORMAL, skill->mXpType, gSkillManager->getXPTypeById(skill->mXpType).getAnsi());

				if (skill->mXpCost > 0)
				{
					sprintf(str,"%s = %u", gSkillManager->getXPTypeExById(skill->mXpType).getAnsi(), skill->mXpCost);
				}
				else
				{
					sprintf(str,"%s", gSkillManager->getXPTypeExById(skill->mXpType).getAnsi());
				}
				populatedListBox.push_back(str);

				sprintf(str,"You do not have this skill.\n\nDescription:\n%s", gSkillManager->getSkillInfoById(skill->mId).getAnsi());
				
				gUIManager->createNewListBox(NULL,"",strCaption,str,populatedListBox,player);
				// gUIManager->createNewListBox(NULL,"",strCaption,"@ui_attrib_mod:description \n\n@ui_attrib_mod:description \n@skl_d:combat_1hsword_ability_01",populatedListBox,player);
				// gUIManager->createNewListBox(NULL,"",strCaption,"@ui_attrib_mod:description \n@skl_d:combat_1hsword_ability_01 Testing",populatedListBox,player);
				// pageLink = 4;	// Back to where we come from... 
			}
			else
			{
				gLogger->logMsg("Trainer: No skill avaliable\n");
			}
		}
		break;

		default:
		{
			// gLogger->logMsg("Trainer::conversationEvent: Default Nothing");
		}
	}
	return pageLink;
}

//=============================================================================

// Post process conversation.
void Trainer::postProcessfilterConversation(ActiveConversation* av, ConversationPage* page, PlayerObject* player)
{
	// gLogger->logMsgF("Trainer::postProcessfilterConversation: page->mId = %u",MSG_NORMAL,page->mId);
	postProcessfilter(av, player, page->mId);
}

//=============================================================================

void Trainer::postProcessfilter(ActiveConversation* av, PlayerObject* player, uint32 pageId)
{
	// gLogger->logMsgF("Trainer::postProcessfilter: pageId = %u",MSG_NORMAL, pageId);
		
	switch (pageId)
	{
		// We train our new skill.
		case 5:
			// Fall through...
		case 13:
		{
			// gLogger->logMsg("Trainer::postProcessfilter: We train our new skill.");


			// Let the player pay for the training.
			// gLogger->logMsgF("Trainer::postProcessfilter: Player now have %u credits left in cash", MSG_NORMAL,
								// dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->getCredits());

			// System message: You successfully make a payment of %DI credits to %TO.
			// gLogger->logMsg("Trainer::postProcessfilter: You successfully make a payment of %DI credits to %TO");
			if (av->getNpc()->getFirstName().getLength())
			{
				// The NPC have a name.
				/*
				char *formatString = {" %s (%s)"};

				uint32 length = av->getNpc()->getFirstName().getDataLength() + av->getNpc()->getTitle().getDataLength() + strlen(formatString);
				char *buffer = new char(length);

				sprintf(buffer,formatString, av->getNpc()->getFirstName().getAnsi(), av->getNpc()->getTitle().getAnsi());
				string npcDesription(buffer);

				gLogger->logMsgF("Trainer::conversationEvent: Name-string = %s", MSG_NORMAL, npcDesription.getAnsi());
				*/
				gMessageLib->sendSystemMessage(player,
												L"",
												"base_player",
												"prose_pay_acct_success",
												"",
												"",
												av->getNpc()->getFirstName(),		// TODO: Use the complete descripton "Opsa Venfo (a scout trainer)"
												av->getDI()
												);
			}
			else
			{
				gMessageLib->sendSystemMessage(player,
												L"",
												"base_player",
												"prose_pay_acct_success",
												"mob/creature_names",
												av->getNpc()->getTitle(),
												L"",
												av->getDI()
												);
			}
		}
		// Fall through...

		case 15:
		{

			Skill* skill = gSkillManager->getSkillByName(av->getTOStfVariable());


			// Train the skill	
			gSkillManager->learnSkill(skill->mId,player,true);

			gMessageLib->sendPlayMusicMessage(1956,player);
			// gLogger->logMsgF("Trainer::postProcessfilter(: Do send that music.", MSG_NORMAL);

			if (pageId == 13)
			{
				char elements[5][32];
				memset(elements, 0, sizeof(elements));
				char *masterStringSingleName = {"WOW! You have learned everything I have to teach... I suppose that now it's time for you to find your own path. Farewell, %s."};
				char *masterStringDoubleName = {"WOW! You have learned everything I have to teach... I suppose that now it's time for you to find your own path. Farewell, %s %s."};

				// string aMess("@skill_teacher:no_qualify");
				char *buffer = new char[strlen(masterStringDoubleName) + player->getFirstName().getLength() + player->getLastName().getLength()];
				if (player->getLastName().getLength())
				{
					sprintf(buffer,masterStringDoubleName, player->getFirstName().getAnsi(), player->getLastName().getAnsi());
				}
				else
				{
					sprintf(buffer,masterStringSingleName, player->getFirstName().getAnsi());
				}
				string aMess(buffer);
				aMess.convert(BSTRType_Unicode16);
				if (!gWorldConfig->isInstance())
				{
					gMessageLib->sendSpatialChat(this,aMess,elements);
				}
				else
				{
					gMessageLib->sendSpatialChat(this, aMess, elements, player);
				}
				delete buffer;
			}
			// TODO: Update with our new abilities.
		}	
		break;

		default:
		{
			// Do nothing.
		}
		break;
	}
}


//=============================================================================

// Pre process conversation.
// Every npc-implementation have to make their special version of this method, if needed. That why we pass the Conversation-object.
// Invoked before we start any actual interaction with the NPC. This is the place where we can abort the conversation, if needed.

bool Trainer::preProcessfilterConversation(ActiveConversation* av,Conversation* conversation,PlayerObject* player)
{
	bool continueConversation = true;
	ConversationPage *page = conversation->getPage(2);	// Page 2 is where WE have the interesting skill-options to check.
	
	// gLogger->logMsgF("Trainer::preProcessfilterConversation: page->mId = %u",MSG_NORMAL,page->mId);

	setLastConversationTarget(player->getId());

	std::vector<ConversationOption*>::iterator optionIt = page->mOptions.begin();

	switch(page->mId)
	{
		// Lets see if we have the requriments met by this trainer.
		case 2:
		{
			bool noviceSkill = false;
			bool gotAllSkills = true;	// As long as we have not detected any missing skills.
			uint32 skillsToLearn = 0;
			BStringVector skillsRequired;

			bool gotRequirementsForSomeSkill = false;

			while (optionIt != page->mOptions.end())
			{
				Skill* skill = gSkillManager->getSkillByName((*optionIt)->mStfVariable);
				// requirementsMet = false;

				// its no skill option
				if (!skill)
				{
					// gLogger->logMsg("Trainer::preProcessfilterConversation: Its no skill option");
				}

				// see if we have it already and check xp
				// Note: We do not handle lack of SP as an error now... that's done later in this chain of events.
				else if (player->checkSkill(skill->mId))
				{
					// We already have this skill.
					// gLogger->logMsg("Trainer::preProcessfilterConversation: We already have this skill");
				}
				else if (!skill->mSkillsRequired.size())
				{
					// No skill requirements, we got a "Novice" skill from a basic profession.
					// gLogger->logMsg("Trainer::preProcessfilterConversation: Novice");
					skillsToLearn++;
					gotAllSkills = false;	// Do not have all skills if we end up here.
					noviceSkill = true;
				}
				else
				{
					// gLogger->logMsg("Trainer::preProcessfilterConversation: Do not have all skills if we end up here");
					gotAllSkills = false;	// Do not have all skills if we end up here.

					// see if we got the required skills
					SkillsRequiredList::iterator reqSkillIt = skill->mSkillsRequired.begin();
					bool requirementsMet = true;
					skillsToLearn++;

					int8 str[128];
					
					while (reqSkillIt != skill->mSkillsRequired.end())
					{
						if (!player->checkSkill(*reqSkillIt))
						{
							// we don't have the requirements
							// gLogger->logMsg("Trainer::preProcessfilterConversation: We don't have the requirements");
							requirementsMet = false;
							
							// We should only add requirements for Novice skills (and then only for Elite Professions).
							if (strstr(gSkillManager->getSkillById(skill->mId)->mName.getAnsi(),"novice"))
							{
								// add it to our list
								sprintf(str,"@%s:%s","skl_n", gSkillManager->getSkillById(*reqSkillIt)->mName.getAnsi());
								// gLogger->logMsgF("Trainer::conversationEvent: Skills missing = %u %s",MSG_NORMAL, *reqSkillIt, gSkillManager->getSkillById(*reqSkillIt)->mName.getAnsi());
								skillsRequired.push_back(str);
							}
							else
							{
								break;
							}
						}
						++reqSkillIt;
					}
					
					if (requirementsMet)
					{
						// gLogger->logMsg("Trainer::preProcessfilterConversation: We DO have the requirements, check if we have the xp for next level");
						gotRequirementsForSomeSkill = true;

						// We DO have the requirements, check if we have the xp for next level.
						if (player->getXpAmount(skill->mXpType) < skill->mXpCost)
						{
							// We do not have enough of XP for training this skill.
							// gLogger->logMsg("Trainer::conversationEvent: Missing XP");
							skillsToLearn--;
						}
					}
					else
					{
						skillsToLearn--;
					}
				}
				++optionIt;
			}

			// Now we must return status of this session.
			if (skillsToLearn == 0)
			{
				// Either we have them all, i.e. we are a Master, or we lack xp for training OR we lack the required skills to train it.
				if (gotAllSkills)
				{
					// We are a Master!
					// gLogger->logMsg("Trainer::preProcessfilterConversation: We are a Master!");
					if (!gWorldConfig->isInstance())
					{
						gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(16)); // giveup
					}
					else
					{
						gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(16), player); // giveup
					}
					
					char elements[5][32];
					memset(elements, 0, sizeof(elements));
					// string aMess("@skill_teacher:no_qualify");
					string aMess("I'm sorry, but I cannot teach you anymore. You have already learned everything I have to teach.");
					aMess.convert(BSTRType_Unicode16);
					// gMessageLib->sendSpatialChat(this,aMess,elements);
					if (!gWorldConfig->isInstance())
					{
						gMessageLib->sendSpatialChat(this,aMess,elements);
					}
					else
					{
						gMessageLib->sendSpatialChat(this, aMess, elements, player);
					}

					continueConversation = false;
				}
				else if (gotRequirementsForSomeSkill)
				{
					// We lack xp for training.
					// gLogger->logMsg("Trainer::preProcessfilterConversation: We lack xp for training");
				}
				else
				{
					// We do not have requirements for any skill, sounds like we are attempting an elite trainers novice skill.
					// gLogger->logMsg("Trainer::preProcessfilterConversation: sounds like we are attempting an elite trainers novice skill");

					// createNewMessageBox(UICallback* callback,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,ui_window_types windowType = SUI_Window_MessageBox,uint8 mbType = SUI_MB_OK);
					
					// gUIManager->createNewMessageBox(NULL,"no_qualify_title","@skill_teacher:no_qualify_title","@skill_teacher:no_qualify_prompt",player,SUI_Window_MessageBox, SUI_MB_OK);
					// BStringVector skillsRequired;

					// add it to our list
					// int8 str[128];
					// sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*dancerIt).getAnsi());
					// sprintf(str,"@skl_n:crafting_artisan_engineering_04");
					// skillsRequired.push_back(str);

					if (!gWorldConfig->isInstance())
					{
						gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(27)); // poke
					}
					else
					{
						gMessageLib->sendCreatureAnimation(av->getNpc(),gWorldManager->getNpcConverseAnimation(27), player); // poke
					}

					char elements[5][32];
					memset(elements, 0, sizeof(elements));

					// string aMess("@skill_teacher:no_qualify");
					string aMess("You do not qualify for any of my teachings. Come back and speak with me later if you acquire the following skills...");
					aMess.convert(BSTRType_Unicode16);
					// gMessageLib->sendSpatialChat(this,aMess,elements);
					if (!gWorldConfig->isInstance())
					{
						gMessageLib->sendSpatialChat(this,aMess,elements);
					}
					else
					{
						gMessageLib->sendSpatialChat(this, aMess, elements, player);
					}

					

					gUIManager->createNewListBox(NULL,"","@skill_teacher:no_qualify_title","@skill_teacher:no_qualify_prompt",skillsRequired,player);
					continueConversation = false;
				}
			}
			else
			{
				if (noviceSkill) // avaliableSkills should be 1
				{
					if (gWorldConfig->isTutorial())
					{
						// gLogger->logMsg("Trainer::preProcessfilterConversation: We train to Novice.");
						// Not implemented yet. Special handling for the Space Station Tutorial Only. 
					}
				}
				else
				{
					// Let's select a skill to train. This is the default path.
					// retLinkPage = 0;	// Use caller default linkPage.
					// gLogger->logMsg("Trainer::preProcessfilterConversation: We have some skills to train.");
				}
			}
		}
		break;
		
		default:
		{
			// gLogger->logMsg("Trainer::preProcessfilterConversation: Default, bad page input");
		}
		break;
	}
	return continueConversation;
}

void Trainer::prepareConversation(PlayerObject* player)
{

	// Let's turn to the player asking for my attention.

	/*
	gLogger->logMsgF("%"PRIu64"",MSG_NORMAL, this->getId());
	gLogger->logMsgF("%f %f %f",MSG_NORMAL, player->mPosition.mX, player->mPosition.mY, player->mPosition.mZ);
	gLogger->logMsgF("\n%f %f %f",MSG_NORMAL, this->mPosition.mX, this->mPosition.mY, this->mPosition.mZ);
	
	gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, player->mDirection.mX, player->mDirection.mY, player->mDirection.mZ, player->mDirection.mW);
	gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, this->mDirection.mX, this->mDirection.mY, this->mDirection.mZ, this->mDirection.mW);
	*/
	
	float x = player->mPosition.mX - this->mPosition.mX;
	float z = player->mPosition.mZ - this->mPosition.mZ;
	float h = sqrt(x*x + z*z);
	
	// gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, x, z, x/h, z/h);
	// We need to use half the angle when storing the new direction.

	// gLogger->logMsgF("mY: %f %f %f %f",MSG_NORMAL, x/h, (float)asin((float)(x/h)), (float)0.5f*asin((float)(x/h)), (float)sin(0.5f*asin((float)(x/h))));
	// gLogger->logMsgF("mW: %f %f %f %f",MSG_NORMAL, z/h, (float)acos((float)(z/h)), (float)0.5f*acos((float)(z/h)), (float)cos(0.5f*acos((float)(z/h))));

	if ((z/h) < 0.0)
	{	
		if (x/h < 0.0)
		{
			this->mDirection.mW = static_cast<float>(cos((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
			this->mDirection.mY = static_cast<float>(sin((3.14159354 * 0.5) + 0.5f*acos(-z/h)));
		}
		else
		{
			this->mDirection.mY = sin(0.5f*acos(z/h));
			this->mDirection.mW = cos(0.5f*acos(z/h));
		}
	}
	else
	{
		this->mDirection.mY = sin(0.5f*asin(x/h));	
		this->mDirection.mW = cos(0.5f*acos(z/h));
	}
	// gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, this->mDirection.mX, this->mDirection.mY, this->mDirection.mZ, this->mDirection.mW);
	
	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (!gWorldConfig->isInstance())
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this);
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
		else
		{
			gMessageLib->sendDataTransform(this);
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
	else
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this, player);
			gMessageLib->sendUpdateTransformMessageWithParent(this, player);
		}
		else
		{
			gMessageLib->sendDataTransform(this, player);
			gMessageLib->sendUpdateTransformMessage(this, player);
		}
	}
	// gLogger->logMsgF("%f %f %f %f",MSG_NORMAL, this->mDirection.mX, this->mDirection.mY, this->mDirection.mZ, this->mDirection.mW);

	setLastConversationTarget(player->getId());

	// Start the conversation timeout in case we disconnect.
	gWorldManager->addNpcConversation(300000, this);	// Trainer will restore it's facing after 5 min.
}

void Trainer::stopConversation(PlayerObject* player)
{
	if (player)
	{
		if (gWorldConfig->isInstance())
		{
			// Inform Tutorial about stop of npc conversations.
			player->getTutorial()->npcConversationHasEnded(this->getId());
		}

		if (player->getId() == getLastConversationTarget())
		{
			gWorldManager->addNpcConversation(15000, this);	// Trainer will restore it's facing 15 seconds after finished conversation.
		}
		else if (gWorldConfig->isInstance())
		{
			if (player->getGroupId())
			{
				if (PlayerObject* lastPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(getLastConversationTarget())))
				{
					if (lastPlayer->getGroupId() != player->getGroupId())
					{
						// Last target was not in my group (so it's another instance).
						gWorldManager->addNpcConversation(15000, this);	// Trainer will restore it's facing 15 seconds after finished conversation.
					}
				}
			}
			else
			{
				// We are alone in the instance.
				gWorldManager->addNpcConversation(15000, this);	// Trainer will restore it's facing 15 seconds after finished conversation.
			}
		}
	}
}

void Trainer::restorePosition(PlayerObject* player)
{
	// gLogger->logMsg("trainer::restore position");
	restoreDefaultDirection();

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (!gWorldConfig->isInstance())
	{
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this);
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
		else
		{
			gMessageLib->sendDataTransform(this);
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
	else
	{
		// Le't see if player still around.
		if (player->isConnected())
		{
			if (this->getParentId())
			{
				// We are inside a cell.
				gMessageLib->sendDataTransformWithParent(this, player);
				gMessageLib->sendUpdateTransformMessageWithParent(this, player);
			}
			else
			{
				gMessageLib->sendDataTransform(this, player);
				gMessageLib->sendUpdateTransformMessage(this, player);
			}
		}
	}
}


void Trainer::respawn(void)
{
	gLogger->logMsgF("Trainer::respawn: Added new trainer for spawn, with id = %"PRIu64"", MSG_NORMAL, this->getId());

	// The cell we will spawn in.
	this->setParentId(getCellIdForSpawn());

	// Default spawn position.
	Anh_Math::Vector3 position(getSpawnPosition());

	// Respawn delay. If the creature have an unique delay, use that. Else use the one provided by the parent object.
	this->setRespawnDelay(0);
	if (this->hasInternalAttribute("creature_respawn_delay"))
	{
		uint64 respawnDelay = this->getInternalAttribute<uint64>("creature_respawn_delay");					
		// gLogger->logMsgF("creature_respawn_delay = %"PRIu64"", MSG_NORMAL, respawnDelay);
		// mRespawnDelay = respawnDelay;
		this->setRespawnDelay(respawnDelay);
	}

	// Let us get the spawn point.
	// Use the supplied direction?
	this->mDirection = getSpawnDirection();

	if (this->getParentId() == 0)
	{
		// Heightmap only works outside.
		position.mY = this->getHeightAt2DPosition(position.mX, position.mZ, true);
	}
	
	// gLogger->logMsgF("Setting up spawn of creature at %.0f %.0f %.0f", MSG_NORMAL, position.mX, position.mY, position.mZ);
	this->mPosition = this->getSpawnPosition();		// Default spawn position.

	// mSpawned = false;
	
	this->mHam.calcAllModifiedHitPoints();


	// All init is done, just the spawn in the world is left.
	this->spawn();
}

//=============================================================================
//
//	Spawn.
//

void Trainer::spawn(void)
{
	// gLogger->logMsgF("AttackableStaticNpc::spawn: Spawning creature %"PRIu64"", MSG_NORMAL, this->getId());
	// gLogger->logMsgF("Spawned static objects # %"PRIu64" (%"PRIu64")", MSG_NORMAL, gCreatureSpawnCounter, gCreatureSpawnCounter - gCreatureDeathCounter);

	// Update the world about my presence.
	
	if (this->getParentId())
	{
		// insert into cell
		this->setSubZoneId(0);

		if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
		{
			cell->addChild(this);
		}
		else
		{
			gLogger->logMsgF("Trainer::spawn: couldn't find cell %"PRIu64, MSG_HIGH, this->getParentId());
			
			// It's a serious isse that we need to investigate.
			assert(cell);
		}
	}
	else
	{
		if (QTRegion* region = gWorldManager->getSI()->getQTRegion(this->mPosition.mX, this->mPosition.mZ))
		{
			this->setSubZoneId((uint32)region->getId());
			region->mTree->addObject(this);
		}
	}

	// Add us to the world.
	gMessageLib->broadcastContainmentMessage(this->getId(),this->getParentId(),4,this);

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (gWorldConfig->isTutorial())
	{
		// We need to get the player object that is the owner of this npc.
		if (this->getPrivateOwner() != 0)
		{
			PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getPrivateOwner()));
			if (playerObject)
			{
				gLogger->logMsgF("Trainer::spawn: Spawned a private skill trainer.", MSG_NORMAL);
				if (this->getParentId())
				{
					// We are inside a cell.
					gMessageLib->sendDataTransformWithParent(this, playerObject);
					gMessageLib->sendUpdateTransformMessageWithParent(this, playerObject);
				}
				else
				{
					gMessageLib->sendDataTransform(this, playerObject);
					gMessageLib->sendUpdateTransformMessage(this, playerObject);
				}
			}
			else
			{
				assert(false);
				gLogger->logMsgF("Trainer::spawn: Failed to spawn a private skill trainer.", MSG_NORMAL);
			}
		}
	}
	else
	{
		gLogger->logMsgF("Trainer::spawn: Spawned a trainer.", MSG_NORMAL);
		if (this->getParentId())
		{
			// We are inside a cell.
			gMessageLib->sendDataTransformWithParent(this);
			gMessageLib->sendUpdateTransformMessageWithParent(this);
		}
		else
		{
			gMessageLib->sendDataTransform(this);
			gMessageLib->sendUpdateTransformMessage(this);
		}
	}
}
