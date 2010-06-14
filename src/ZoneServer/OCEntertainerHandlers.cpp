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
#include "EntertainerManager.h"
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
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/atMacroString.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"




//======================================================================================================================
//
// stop listening
//

void ObjectController::_handlestoplistening(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*	callingObject	= dynamic_cast<PlayerObject*>(mObject);
	gEntertainerManager->stopListening((PlayerObject*)mObject);

}

//======================================================================================================================
//
// stop watching
//

void ObjectController::_handlestopwatching(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*	callingObject	= dynamic_cast<PlayerObject*>(mObject);
	gEntertainerManager->stopWatching((PlayerObject*)mObject);

}

//======================================================================================================================
//
// watch
//

void ObjectController::_handlewatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	// lets get the target player
	message->setIndex(32);
	PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getUint64()));
	if(!targetPlayer)
	{
		gMessageLib->sendSystemMessage(targetPlayer,L"","performance","dance_watch_npc");
		gLogger->log(LogManager::DEBUG,"OC :: handle startwatch No entertainer");
		return;
	}
	gEntertainerManager->startWatching((PlayerObject*)mObject, targetPlayer);

}

//======================================================================================================================
//
// listen
//

void ObjectController::_handlelisten(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);

	// lets get the target player
	message->setIndex(32);
	PlayerObject* targetPlayer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getUint64()));

	if(!targetPlayer)
	{
		gMessageLib->sendSystemMessage(targetPlayer,L"","performance","music_listen_npc");
		return;
	}

	gEntertainerManager->startListening((PlayerObject*)mObject, targetPlayer);

}

//======================================================================================================================
//
// stop dance
//

void ObjectController::_handlestopdance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if (entertainer->getPerformingState() == PlayerPerformance_Dance)
	{

		//remove the Entertainer Tick
		gWorldManager->removeEntertainerToProcess(entertainer->getEntertainerTaskId());

		//go through the list of spectators and stop the performance
		gEntertainerManager->stopEntertaining(entertainer);

	}

}

//======================================================================================================================
//
// stop music
//

void ObjectController::_handlestopmusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if (entertainer->getPerformingState() == PlayerPerformance_Music)
	{
		//remove the Entertainer Tick
		gWorldManager->removeEntertainerToProcess(entertainer->getEntertainerTaskId());

		//go through the list of spectators and stop the performance
		gEntertainerManager->stopEntertaining(entertainer);

	}
}

//======================================================================================================================
//
// flourish
//
void ObjectController::_handlePauseDance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if(entertainer->getPerformingState() != PlayerPerformance_Dance)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","dance_fail");
		return;
	}

	entertainer->setPerformancePaused(Pause_Start);
}

void ObjectController::_handlePauseMusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if(entertainer->getPerformingState() != PlayerPerformance_Music)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","music_fail");
		return;
	}

	gMessageLib->sendperformFlourish(entertainer, 0);

}

void ObjectController::_handleflourish(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	//are we performing???
	if(entertainer->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_performing");
		return;
	}
	uint8 flourishMax = 8;

	if(entertainer->getPerformingState() == PlayerPerformance_Dance)
		flourishMax = 9;

	//find out what flourish we are supposed to play
	string dataStr;
	message->getStringUnicode16(dataStr);
	uint32 mFlourishId;
	swscanf(dataStr.getUnicode16(),L"%u",&mFlourishId);

	if((mFlourishId < 1)||(mFlourishId > flourishMax))
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_valid");
		return;
	}

	//give notice

	gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_perform");
	gEntertainerManager->flourish(entertainer,mFlourishId);
}

//======================================================================================================================
// change the dance
//======================================================================================================================

void ObjectController::_handleChangeDance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if(entertainer->getPerformingState() != PlayerPerformance_Dance)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","dance_must_be_performing_self");
		return;
	}

	SkillCommandList*	dancerSkillCommands = entertainer->getSkillCommands();
	SkillCommandList::iterator dancerIt = dancerSkillCommands->begin();

	string dataStr;
	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);

	int8 danceStr[32];
	sprintf(danceStr,"startdance+%s",dataStr.getAnsi());

	bool found = false;
	if(dataStr.getLength() >0 )
	{
		//check if we are able to perform this dance
		while(dancerIt != dancerSkillCommands->end())
		{
			string mDanceString = gSkillManager->getSkillCmdById((*dancerIt));
			//look for our selected dance
			if(BString(danceStr).getCrc() == mDanceString.getCrc() ){
				//yay we are able to perform this dance :)
				gEntertainerManager->changeDance(entertainer,dataStr);
				found = true;
				return;
			}

			dancerIt++;
		}
	}

	if (found == false)
	{
		BStringVector availableCommands;
		uint32 nr = 0;
		dancerIt = dancerSkillCommands->begin();
		while(dancerIt != dancerSkillCommands->end())
		{
			//look for all the dances
			if(strstr(gSkillManager->getSkillCmdById((*dancerIt)).getAnsi(),"startdance+"))
			{
				// add it to our dancelist
				int8 str[128];

				sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*dancerIt).getAnsi());
				availableCommands.push_back(str);
				nr++;
			}

			dancerIt++;
		}

		if(nr > 0)
		{
			gUIManager->createNewListBox(entertainer,"handleselectdance","select dance","Select a dance",availableCommands,entertainer,SUI_Window_SelectDance_Listbox,SUI_LB_OK);
		}
		else
		{
			gMessageLib->sendSystemMessage(entertainer,L"","performance","dance_fail");
		}
	}
}

//======================================================================================================================
// Add or remove somebody to or from our denyservice List
//======================================================================================================================

void ObjectController::_handleDenyService(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);
	//either add or remove a player from the list if one is added to the commandline
	//or show a list box with all Players on the List to select one to be removed

	string dataStr;
	dataStr.setType(BSTRType_Unicode16);
	PlayerObject* outcast;
	//find out the id of the player if he should happen to be near
	message->setIndex(32);
	outcast = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(message->getUint64()));
	if(!outcast)
	{
		//if the poor sod is not known to the client by either being offline or somewhere else
		message->getStringUnicode16(dataStr);
		dataStr.convert(BSTRType_ANSI);
		//whatever they may have entered we will now have to check if it exists...
		//entertainers / players targetedwill not be send by the client
		//let the entertainermanager check it
		if (dataStr.getLength() >0)
		{
			gEntertainerManager->verifyOutcastName(entertainer,dataStr.getAnsi());
			return;
		}

	}
	else
	if(outcast)
	{
		//poor sod is near and will now be added to our list (or removed)
		gEntertainerManager->toggleOutcastId(entertainer,outcast->getId(),outcast->getFirstName());
		return;
	}

	//No valid player : show us a list of all players on our List
	gEntertainerManager->showOutcastList(entertainer);

}

//======================================================================================================================
// change the music
//======================================================================================================================

void ObjectController::_handleChangeMusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);

	if(entertainer->getPerformingState() != PlayerPerformance_Music)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","music_must_be_performing_self");
		return;
	}

	SkillCommandList*	entertainerSkillCommands = entertainer->getSkillCommands();
	SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();

	string dataStr;
	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);
	int8 musicStr[32];
	sprintf(musicStr,"startmusic+%s",dataStr.getAnsi());

	bool found = false;
	if(dataStr.getLength() >0 )
	{
		//check if we are able to perform this dance
		while(entertainerIt != entertainerSkillCommands->end())
		{
			string mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
			//look for our selected dance
			if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
			{
				//yay we are able to perform this dance :)
				gEntertainerManager->changeMusic(entertainer,dataStr);
				found = true;
				return;

			}

			entertainerIt++;
		}
	}

	//show list box with all available dances
	if (found == false)
	{
		BStringVector availableCommands;
		uint32 nr = 0;
		entertainerIt = entertainerSkillCommands->begin();
		while(entertainerIt != entertainerSkillCommands->end())
		{
			//look for all the dances
			if(strstr(gSkillManager->getSkillCmdById((*entertainerIt)).getAnsi(),"startmusic+"))
			{
				// add it to our dancelist
				int8 str[128];
				sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*entertainerIt).getAnsi());
				availableCommands.push_back(str);
				nr++;
			}

			entertainerIt++;
		}

		if(nr > 0)
		{
			gUIManager->createNewListBox(entertainer,"handleselectmusic","select music","@performance:music_no_music_param",availableCommands,entertainer,SUI_Window_SelectMusic_Listbox,SUI_LB_OK);
		}
		else
		{
			gMessageLib->sendSystemMessage(entertainer,L"","performance","music_fail");
		}
	}
	//no need for skillboxes we can do that directly
	gEntertainerManager->changeMusic(entertainer,musicStr);
}

//======================================================================================================================
// start dance
//======================================================================================================================

void ObjectController::_handlestartdance(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//show list box with all available dances
	PlayerObject*	performer	= dynamic_cast<PlayerObject*>(mObject);
	if(performer->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(performer,L"","performance","already_performing_self");
		return;
	}

	
	if(performer->getSamplingState())
	{
		performer->getSampleData()->mPendingSample = false;
		gMessageLib->sendSystemMessage(performer,L"","survey","sample_cancel");
	}

	if(performer->checkStatesEither(CreatureState_Combat | CreatureState_Tumbling | CreatureState_Swimming | CreatureState_Crafting))
	{
		gMessageLib->sendSystemMessage(performer,L"", "error_message", "wrong_state");
		return;
	}

	SkillCommandList*	dancerSkillCommands = performer->getSkillCommands();
	SkillCommandList::iterator dancerIt = dancerSkillCommands->begin();

	string dataStr;
	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);

	int8 danceStr[32];
	sprintf(danceStr,"startdance+%s",dataStr.getAnsi());

	bool found = false;
	if(dataStr.getLength() >0 )
	{
		//check if we are able to perform this dance
		while(dancerIt != dancerSkillCommands->end())
		{
			string mDanceString = gSkillManager->getSkillCmdById((*dancerIt));
			//look for our selected dance
			if(BString(danceStr).getCrc() == mDanceString.getCrc() ){
				//yay we are able to perform this dance :)
				performer->setPerformingState(PlayerPerformance_Dance);
				gEntertainerManager->startDancePerformance(performer,dataStr);
				found = true;
				return;

			}

			dancerIt++;
		}
	}

	if (found == false)
	{
		BStringVector availableCommands;
		uint32 nr = 0;
		dancerIt = dancerSkillCommands->begin();
		while(dancerIt != dancerSkillCommands->end())
		{
			//look for all the dances
			if(strstr(gSkillManager->getSkillCmdById((*dancerIt)).getAnsi(),"startdance+"))
			{
				// add it to our dancelist
				int8 str[128];

				sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*dancerIt).getAnsi());
				availableCommands.push_back(str);
				nr++;
			}

			dancerIt++;
		}

		if(nr > 0)
		{
			gUIManager->createNewListBox(performer,"handleselectdance","select dance","Select a dance",availableCommands,performer,SUI_Window_SelectDance_Listbox,SUI_LB_OK);
		}
		else
		{
			gMessageLib->sendSystemMessage(performer,L"","performance","dance_fail");
		}
	}

}

//======================================================================================================================
// start music
//======================================================================================================================

void ObjectController::_handlestartmusic(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	performer	= dynamic_cast<PlayerObject*>(mObject);
	if(performer->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(performer,L"","performance","already_performing_self");
		return;

	}

	if(performer->checkStatesEither(CreatureState_Combat | CreatureState_Tumbling | CreatureState_Swimming))
	{
		gMessageLib->sendSystemMessage(performer,L"", "error_message", "wrong_state");
		return;
	}

	//show list box with all available dances

	SkillCommandList*	entertainerSkillCommands = performer->getSkillCommands();
	SkillCommandList::iterator entertainerIt = entertainerSkillCommands->begin();

	string dataStr;
	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);
	int8 musicStr[32];
	sprintf(musicStr,"startmusic+%s",dataStr.getAnsi());

	bool found = false;
	if(dataStr.getLength() >0 )
	{
		//check if we are able to perform this piece of music
		while(entertainerIt != entertainerSkillCommands->end())
		{
			string mEntertainerString = gSkillManager->getSkillCmdById((*entertainerIt));
			//look for our selected dance
			if(BString(musicStr).getCrc() == mEntertainerString.getCrc() )
			{
				//yay we are able to perform this dance :)
				performer->setPerformingState(PlayerPerformance_Music);
				gEntertainerManager->startMusicPerformance(performer,dataStr);
				found = true;
				return;

			}

			entertainerIt++;
		}
	}

	if (found == false)
	{
		BStringVector availableCommands;
		uint32 nr = 0;
		entertainerIt = entertainerSkillCommands->begin();
		while(entertainerIt != entertainerSkillCommands->end())
		{
			//look for all the dances
			if(strstr(gSkillManager->getSkillCmdById((*entertainerIt)).getAnsi(),"startmusic+"))
			{
				// add it to our dancelist
				int8 str[128];
				sprintf(str,"@%s:%s","cmd_n",gSkillManager->getSkillCmdById(*entertainerIt).getAnsi());
				availableCommands.push_back(str);
				nr++;
			}

			entertainerIt++;
		}

		if(nr > 0)
		{
			gUIManager->createNewListBox(performer,"handleselectmusic","select music","@performance:music_no_music_param",availableCommands,performer,SUI_Window_SelectMusic_Listbox,SUI_LB_OK);
		}
		else
		{
			gMessageLib->sendSystemMessage(performer,L"","performance","music_fail");
		}
	}
}

//======================================================================================================================
//stops the band (instruments) from playing
//======================================================================================================================

void ObjectController::_handleStopBand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	performer	= dynamic_cast<PlayerObject*>(mObject);

	if(performer->getGroupId() == 0)
	{
		gMessageLib->sendSystemMessage(performer,L"You are not in a band.");
		return;

	}

	gEntertainerManager->StopBand(performer);
}

//======================================================================================================================
//starts the band (instruments)
//======================================================================================================================

void ObjectController::_handleStartBand(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	performer	= dynamic_cast<PlayerObject*>(mObject);

	if(performer->checkStatesEither(CreatureState_Combat | CreatureState_Tumbling | CreatureState_Swimming))
	{
		gMessageLib->sendSystemMessage(performer,L"", "error_message", "wrong_state");
		return;
	}

	if(performer->getPerformingState() != PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(performer,L"","performance","already_performing_self");
		return;

	}

	if(performer->getGroupId() == 0)
	{
		gMessageLib->sendSystemMessage(performer,L"You are not in a band.");
		return;

	}

	//get song
	string dataStr;
	message->getStringUnicode16(dataStr);
	dataStr.convert(BSTRType_ANSI);

	gEntertainerManager->StartBand(performer, dataStr);
}

//======================================================================================================================
//performs a bandflourish
//======================================================================================================================

void ObjectController::_handleBandFlourish(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	entertainer	= dynamic_cast<PlayerObject*>(mObject);
	//gMessageLib->sendSystemMessage(performer,L"","performance","music_stop_band_self");

	if(entertainer->getGroupId() == 0)
	{
		gMessageLib->sendSystemMessage(entertainer,L"You are not in a band.");
		return;

	}

	uint8 flourishMax = 8;
	//are we performing???
	if(entertainer->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_performing");
		return;
	}
	if(entertainer->getPerformingState() == PlayerPerformance_Dance)
		flourishMax = 9;


	string dataStr;
	message->getStringUnicode16(dataStr);
	//dataStr.convert(BSTRType_ANSI);
	//printf(" flourish : %s",dataStr.getAnsi());
	wchar_t temp[64];

	swscanf(dataStr.getUnicode16(),L"%s",&temp);

	if(wcsncmp(temp, L"on", sizeof(temp)) == 0)
	{
		if(entertainer->checkSkill(10))
		{
			if(entertainer->getAcceptBandFlourishes() == false)
			{
				entertainer->setAcceptBandFlourishes(true);

				gMessageLib->sendSystemMessage(entertainer,L"","performance","band_flourish_on");
			}
			else
				gMessageLib->sendSystemMessage(entertainer,L"","performance","band_flourish_status_on");
		}
	}
	else if(wcsncmp(temp, L"off", sizeof(temp)) == 0)
	{
		if(entertainer->checkSkill(10))
		{
			if(entertainer->getAcceptBandFlourishes() == true)
			{
				entertainer->setAcceptBandFlourishes(false);

				gMessageLib->sendSystemMessage(entertainer,L"","performance","band_flourish_off");
			}
			else
				gMessageLib->sendSystemMessage(entertainer,L"","performance","band_flourish_status_off");
		}
	}
	else
	{
		uint32 FlourishId;
		swscanf(dataStr.getUnicode16(), L"%d", &FlourishId);

		if((FlourishId < 1)||(FlourishId > flourishMax))
		{
			gMessageLib->sendSystemMessage(entertainer,L"","performance","flourish_not_valid");
			return;
		}

		gEntertainerManager->BandFlourish(entertainer, FlourishId);
	}
}

//=============================================================================================================================
//
// image design
//

void ObjectController::_handleImageDesign(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	PlayerObject* designObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
	PlayerObject*	imageDesigner	= dynamic_cast<PlayerObject*>(mObject);

	if(!designObject)
		return;

	if(!imageDesigner)
		return;

	if(designObject->getPosture() == CreaturePosture_Dead)
	{
		gMessageLib->sendSysMsg(imageDesigner,"image_designer","target_dead",NULL,designObject);
		//gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","target_dead");
		return;
	}

	if(designObject->isIncapacitated())
	{
		gMessageLib->sendSysMsg(imageDesigner,"image_designer","target_dead",NULL,designObject);
		return;
	}

	if(!imageDesigner->checkSkill(SMSkill_NoviceEntertainer))
	{
		gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","not_an_image_designer");
		return;
	}
    //Sch we need to add more states and checks - Rouse
	if(imageDesigner->checkStatesEither(CreatureState_Combat | CreatureState_Tumbling | CreatureState_Swimming | CreatureState_Crafting))
	{
		gMessageLib->sendSystemMessage(imageDesigner,L"You cannot perform that action on this target");
		return;
	}

	if(imageDesigner->getImageDesignSession() != IDSessionNONE)
	{
		if(imageDesigner->getImageDesignSession() == IDSessionID)
			gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","already_image_designing");
		else
			gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","already_being_image_designed");
		return;
	}

	if(designObject->getImageDesignSession() != IDSessionNONE)
	{
		if(designObject->getImageDesignSession() == IDSessionID)
			gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","target_is_image_designing");
		else
			gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","outstanding_offer");
		return;
	}

	if(glm::distance(designObject->getWorldPosition(), imageDesigner->getWorldPosition()) > 16)
	{
		gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","out_of_range");
		return;
	}

	if((designObject != imageDesigner) && (designObject->getGroupId() != imageDesigner->getGroupId() ))
	{
		gMessageLib->sendSystemMessage(imageDesigner,L"","image_designer","not_in_same_group");
		return;
	}

	designObject->SetImageDesignSession(IDSessionPREY);
	designObject->setIDPartner(imageDesigner->getId());
	imageDesigner->SetImageDesignSession(IDSessionID);
	imageDesigner->setIDPartner(designObject->getId());

	gMessageLib->sendImageDesignStartMessage(imageDesigner,designObject);

}

void ObjectController::handleImageDesignChangeMessage(Message* message,uint64 targetId)
{
	message->getUint32();

	uint64 dsgObjectId	= message->getUint64();//Id of the one being Ided
	uint64 idObjectId	= message->getUint64(); // our object id?

	uint64 buildingId	= message->getUint64();//probably the buildings ID

	uint32 ColorCounter;
	uint32 AttributeCounter;
	string hair;
	string holoEmote;

	PlayerObject*	messageGenerator	=	dynamic_cast<PlayerObject*>(mObject);
	PlayerObject*	imageDesigner		=	dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(dsgObjectId));
	PlayerObject*	customer			=	dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(idObjectId));


	if(!customer)
		return;

	if(!imageDesigner)
		return;

	uint8 designerCommit,statMigration,flagHair;
	uint32 customerAccept;

	uint16 unknown1;
	uint32 smTimer;//timer statMigration
	uint32 sessionId;
	uint32 creditsDemanded,creditsOffered;
	uint32 skillLevel1,skillLevel2,skillLevel3,skillLevel4;


	message->getUint8(flagHair);
	message->getStringAnsi(hair);


	message->getUint16(unknown1); //string only relevant to ID, plain uint16 for customer
	message->getUint32(smTimer);//bitmap timer for statMigration(only 4 minutes available)
	message->getUint32(sessionId);//some sort of Id session identification?
	message->getUint32(creditsDemanded);
	message->getUint32(creditsOffered);

	message->getUint8(designerCommit);//comited by designer
	message->getUint32(customerAccept);//comitted by customer
	message->getUint8(statMigration);

	//we could test here for cheaters :P
	message->getUint32(skillLevel1);
	message->getUint32(skillLevel2);
	message->getUint32(skillLevel3);
	message->getUint32(skillLevel4);

	message->getUint32(AttributeCounter);
	string attribute;
	float value;
	for(uint32 i=0; i<AttributeCounter;i++)
	{
		message->getStringAnsi(attribute);
		value = message->getFloat();

		customer->UpdateIdAttributes(attribute,value);
	}

	uint32 colorvalue;
	message->getUint32(ColorCounter);
	for(uint32 i=0; i<ColorCounter;i++)
	{
		message->getStringAnsi(attribute);
		colorvalue = message->getUint32();

		customer->UpdateIdColors(attribute,static_cast<uint16>(colorvalue));
	}

	message->getStringAnsi(holoEmote);

	if(((imageDesigner == customer) || customerAccept) && designerCommit)
	{
		if(imageDesigner->getImageDesignSession() == IDSessionNONE)
			return;

		imageDesigner->setIDPartner(0);
		customer->setIDPartner(0);

		imageDesigner->SetImageDesignSession(IDSessionNONE);
		customer->SetImageDesignSession(IDSessionNONE);
		//changelists get deleted
		gEntertainerManager->commitIdChanges(customer,imageDesigner,hair,creditsOffered, statMigration,holoEmote,flagHair);
	}
	
	if((imageDesigner == messageGenerator) && designerCommit)
	{
		uint32 idTimer	= gWorldConfig->getConfiguration<uint32>("Player_Timer_IDSessionTimeOut",(uint32)60000);
		messageGenerator->setImageDesignerTaskId(gWorldManager->addImageDesignerToProcess(messageGenerator,idTimer));
		gLogger->log(LogManager::DEBUG,"Added ID Tick Control !!!");
	}

	//if(imageDesigner->getImageDesignSession() == IDSessionPREY)
		//gMessageLib->sendIDChangeMessage(customer,customer,imageDesigner,hair, sessionId,creditsOffered, creditsDemanded,customerAccept,designerCommit,statMigration,smTimer,flagHair,buildingId,holoEmote);

	//if(imageDesigner->getImageDesignSession() == IDSessionID)
	//{
	gMessageLib->sendIDChangeMessage(customer,imageDesigner,customer,hair, sessionId,creditsOffered, creditsDemanded,customerAccept,designerCommit,statMigration,smTimer,flagHair,buildingId,holoEmote);
	//}

}

void ObjectController::handleImageDesignStopMessage(Message* message,uint64 targetId)
{

	message->getUint32(); // unknown
	uint64 dsgObjectId = message->getUint64();//Id of the one being Ided
	uint64 idObjectId = message->getUint64(); // our object id?
	message->getUint64();

	//the one who send the message - either id or customer
	PlayerObject*	messageGenerator	=	dynamic_cast<PlayerObject*>(mObject);

	//the imagedesigner 
	PlayerObject*	imageDesigner		=	dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(dsgObjectId));//
	
	//the customer
	PlayerObject*	customer			=	dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(idObjectId));


	if(!customer)
	{
		return;
	}

	if(!imageDesigner)
		return;
	uint8 flag1,flag2,flag3;


	//always remember id and prey are swapped here
	message->getUint8(flag1);
	string hair;
	message->getStringAnsi(hair);
	uint16 unknown1;
	uint32 counter1,counter2,creditsDemanded,creditsOffered;

	message->getUint16(unknown1);
	message->getUint32(counter1);
	message->getUint32(counter2);//some sort of Id session identification?
	message->getUint32(creditsDemanded);
	message->getUint32(creditsOffered);

	uint32 unknown2;
	message->getUint8(flag2);
	message->getUint32(unknown2);
	message->getUint8(flag3);

	uint32 skillLevel1,skillLevel2,skillLevel3,skillLevel4;

	message->getUint32(skillLevel1);
	message->getUint32(skillLevel2);
	message->getUint32(skillLevel3);
	message->getUint32(skillLevel4);

	if(messageGenerator->getImageDesignSession() == IDSessionPREY)
	{
		gMessageLib->sendIDEndMessage(imageDesigner,customer,imageDesigner,hair, counter2,creditsOffered, 0,unknown2,flag2,flag3,counter1);
		gMessageLib->sendSystemMessage(imageDesigner,L"The Customer cancelled the session.");
	}

	if(messageGenerator->getImageDesignSession() == IDSessionID)
	{
		gMessageLib->sendIDEndMessage(customer,customer,imageDesigner,hair, counter2,creditsOffered, 0,unknown2,flag2,flag3,counter1);
		
	}

	imageDesigner->setIDPartner(0);
	customer->setIDPartner(0);
	imageDesigner->SetImageDesignSession(IDSessionNONE);
	customer->SetImageDesignSession(IDSessionNONE);



}

//=============================================================================================================================
void ObjectController::_handleRequestStatMigrationData(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);
	string dataStr;
	message->getStringUnicode16(dataStr);

	uint32 value1,value2,value3,value4,value5,value6,value7,value8,value9;

	swscanf(dataStr.getUnicode16(),L"%u %u %u %u %u %u %u %u %u",&value1,&value2,&value3,&value4,&value5,&value6,&value7,&value8,&value9);

	we->getHam()->setTargetStatValue(HamBar_Health,value1);
	we->getHam()->setTargetStatValue(HamBar_Strength,value2);
	we->getHam()->setTargetStatValue(HamBar_Constitution,value3);

	we->getHam()->setTargetStatValue(HamBar_Action,value4);
	we->getHam()->setTargetStatValue(HamBar_Quickness,value5);
	we->getHam()->setTargetStatValue(HamBar_Stamina,value6);

	we->getHam()->setTargetStatValue(HamBar_Mind,value7);
	we->getHam()->setTargetStatValue(HamBar_Focus,value8);
	we->getHam()->setTargetStatValue(HamBar_Willpower,value9);

	//add it to the db
	int8 sql[400];
	sprintf(sql,"call sp_CharacterStatMigrationCreate (%"PRIu64",%u,%u,%u,%u,%u,%u,%u,%u,%u,0)",we->getId(),value1,value2,value3,value4,value5,value6,value7,value8,value9);
	ObjControllerAsyncContainer* asyncContainer;
	asyncContainer = new ObjControllerAsyncContainer(OCQuery_Nope);
	mDatabase->ExecuteProcedureAsync(this,asyncContainer,sql);

	//We need to check to see if we're in the tutorial. If so these changes are INSTANT!
	if(gWorldConfig->isTutorial())
	{
		Ham* pHam = we->getHam();
		uint32 currentAmount = pHam->getTotalHamCount();

		uint32 nextAmount = pHam->getTargetStatValue(HamBar_Health);
		nextAmount += pHam->getTargetStatValue(HamBar_Strength);
		nextAmount += pHam->getTargetStatValue(HamBar_Constitution);
		nextAmount += pHam->getTargetStatValue(HamBar_Action);
		nextAmount += pHam->getTargetStatValue(HamBar_Quickness);
		nextAmount += pHam->getTargetStatValue(HamBar_Stamina);
		nextAmount += pHam->getTargetStatValue(HamBar_Mind);
		nextAmount += pHam->getTargetStatValue(HamBar_Focus);
		nextAmount += pHam->getTargetStatValue(HamBar_Willpower);

		if(currentAmount == nextAmount)
		{
			int32 value;
			
			value = pHam->getTargetStatValue(HamBar_Health) - pHam->getPropertyValue(HamBar_Health,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Health,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Strength) - pHam->getPropertyValue(HamBar_Strength,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Strength,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Constitution) - pHam->getPropertyValue(HamBar_Constitution,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Constitution,HamProperty_BaseHitpoints,value,true);



			value = pHam->getTargetStatValue(HamBar_Action) - pHam->getPropertyValue(HamBar_Action,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Action,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Quickness) - pHam->getPropertyValue(HamBar_Quickness,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Quickness,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Stamina) - pHam->getPropertyValue(HamBar_Stamina,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Stamina,HamProperty_BaseHitpoints,value,true);



			value = pHam->getTargetStatValue(HamBar_Mind) - pHam->getPropertyValue(HamBar_Mind,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Mind,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Focus) - pHam->getPropertyValue(HamBar_Focus,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Focus,HamProperty_BaseHitpoints,value,true);

			value = pHam->getTargetStatValue(HamBar_Willpower) - pHam->getPropertyValue(HamBar_Willpower,HamProperty_BaseHitpoints);
			pHam->updatePropertyValue(HamBar_Willpower,HamProperty_BaseHitpoints,value,true);

			//now the db
			ObjControllerAsyncContainer* asyncContainer2;
					
			int8 sql[1024];
			asyncContainer2 = new ObjControllerAsyncContainer(OCQuery_Null);
			sprintf(sql,"UPDATE swganh.character_attributes SET health_max = %i, strength_max = %i, constitution_max = %i, action_max = %i, quickness_max = %i, stamina_max = %i, mind_max = %i, focus_max = %i, willpower_max = %i where character_id = %"PRIu64"",pHam->getTargetStatValue(HamBar_Health),pHam->getTargetStatValue(HamBar_Strength),pHam->getTargetStatValue(HamBar_Constitution), pHam->getTargetStatValue(HamBar_Action),pHam->getTargetStatValue(HamBar_Quickness),pHam->getTargetStatValue(HamBar_Stamina),pHam->getTargetStatValue(HamBar_Mind) ,pHam->getTargetStatValue(HamBar_Focus) ,pHam->getTargetStatValue(HamBar_Willpower) ,we->getId());
			mDatabase->ExecuteSqlAsync(this,asyncContainer2,sql);

			asyncContainer2 = new ObjControllerAsyncContainer(OCQuery_Null);
			sprintf(sql,"UPDATE swganh.character_attributes SET health_current = %i, strength_current = %i, constitution_current = %i, action_current = %i, quickness_current = %i, stamina_current = %i, mind_current = %i, focus_current = %i, willpower_current = %i where character_id = %"PRIu64"",pHam->getTargetStatValue(HamBar_Health),pHam->getTargetStatValue(HamBar_Strength),pHam->getTargetStatValue(HamBar_Constitution), pHam->getTargetStatValue(HamBar_Action),pHam->getTargetStatValue(HamBar_Quickness),pHam->getTargetStatValue(HamBar_Stamina),pHam->getTargetStatValue(HamBar_Mind) ,pHam->getTargetStatValue(HamBar_Focus) ,pHam->getTargetStatValue(HamBar_Willpower) ,we->getId());
			mDatabase->ExecuteSqlAsync(this,asyncContainer2,sql);
		}
	}
}

//=============================================================================================================================
void ObjectController::_handleStatMigration(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	int8 sql[500];

	ObjControllerAsyncContainer* asyncContainer;

	asyncContainer = new ObjControllerAsyncContainer(OCQuery_StatRead);
	asyncContainer->playerObject = we;

	sprintf(sql,"SELECT target_health, target_strength, target_constitution, target_action, target_quickness, target_stamina, target_mind, target_focus, target_willpower FROM swganh.character_stat_migration where character_id = %"PRIu64, we->getId());
	mDatabase->ExecuteSqlAsync(this,asyncContainer,sql);

}


void ObjectController::_handlePlayHoloEmote(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	HoloStruct* myEmote = gEntertainerManager->getHoloEmoteByCRC(we->getHoloEmote());

	if(!myEmote)
	{
		gMessageLib->sendSystemMessage(we,L"","image_designer","no_holoemote");
		return;
	}

	if(we->getHoloCharge()<= 0)
	{
		gMessageLib->sendSystemMessage(we,L"","image_designer","no_charges_holoemote");
		return;
	}

	int8 cmdLine[32];
	sscanf(dataStr.getAnsi(),"%s",cmdLine);

	if(!strcmp(cmdLine,"remove"))
	{
		//remove from playerObject
		gMessageLib->sendSystemMessage(we,L"","image_designer","remove_holoemote");
		we->setHoloCharge(0);
		we->setHoloEmote(0);
		//dont forget to remove from db, too
		return;
	}

	bool lotsOfStuff = false;

	if(!strcmp(cmdLine,"help"))
	{
		if(!strcmp(myEmote->pEmoteName,"all"))
		{
			lotsOfStuff = true;
		}

		//just give help
		int8 sql[512], sql1[512];

		if(lotsOfStuff)
		{
			sprintf(sql1,"Your Holo-Emote generator can play all Holo-Emotes available. You have %u charges remaining."
			"\xa To play your Holo-Emote type \x2fholoemote \x3cname\x3e.\xa To delete your Holo-Emote type \x2fholoemote delete. "
			"\xa Purchasing a new Holo-Emote will automatically delete your current Holo-Emote.",we->getHoloCharge());

			sprintf(sql,"%s \xa \xa The available Holo-Emote names are: \xa \xa"
			"Beehive \x9 \x9 Blossom \x9 Brainstorm \xa"
			"Bubblehead \x9 Bullhorns \x9 Butterflies \xa"
			"Champagne \x9 Haunted \x9 Hearts \xa"
			"Hologlitter \x9 \x9 Holonotes \x9 Imperial \xa"
			"Kitty \x9 \x9 \x9 Phonytail \x9 Rebel \xa"
			"Sparky",sql1);
		}
		else
		{
			sprintf(sql,"Your current Holo Emote is %s.\xa You have %u charges remaining."
			"\xa To play your Holo-Emote type \x2fholoemote %s.\xa To delete your Holo-Emote type \x2fholoemote delete. "
			"\xa Purchasing a new Holo-Emote will automatically delete your current Holo-Emote.",myEmote->pEmoteName,we->getHoloCharge(),myEmote->pEmoteName);
		}

		

		gUIManager->createNewMessageBox(NULL,"holoHelpOff","Holo-Emote Help",sql,we);

		return;
	}

	HoloStruct* requestedEmote = gEntertainerManager->getHoloEmoteIdbyName(cmdLine);

	if(!requestedEmote)
	{
		gMessageLib->sendSystemMessage(we,L"","image_designer","holoemote_help");
		return;
	}

	//its *not* all
	//only play if we own the relevant generator and havnt requested holoemote all
	if(((requestedEmote->pId == myEmote->pId)||(myEmote->pId == 0))&& (requestedEmote->pId != 0))
	{			
		if(we->decHoloCharge())
		{
			string effect = gWorldManager->getClientEffect(requestedEmote->pId);
			gMessageLib->sendPlayClientEffectObjectMessage(effect,"head",we);
			int8 sql[256];
			sprintf(sql,"update swganh.character_holoemotes set charges = charges-1 where character_id = %I64u", we->getId());
			mDatabase->ExecuteSqlAsync(this,new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_Nope),sql);
		}
		else
		{
			gMessageLib->sendSystemMessage(we,L"","image_designer","no_charges_holoemote");
			return;
		}
	}
	else
	{
		gMessageLib->sendSystemMessage(we,L"","image_designer","holoemote_help");
		return;
	}
	
}

//======================================================================================================================

void ObjectController::_handleDistract(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);
	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_distract");
	sprintf(effectStr,"clienteffect/entertainer_distract_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

//======================================================================================================================

void ObjectController::_handleFireJet(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_fire_jets");
	sprintf(effectStr,"clienteffect/entertainer_fire_jets_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

void ObjectController::_handleDazzle(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_dazzle");
	sprintf(effectStr,"clienteffect/entertainer_Dazzle_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

void ObjectController::_handleColorLights(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_color_lights");
	sprintf(effectStr,"clienteffect/entertainer_color_lights_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

void ObjectController::_handleSmokeBomb(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_smoke_bomb");
	sprintf(effectStr,"clienteffect/entertainer_smoke_bomb_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

void ObjectController::_handleSpotLight(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);
	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}
	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}
	int8 effectStr[64];
	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_spot_light");
	sprintf(effectStr,"clienteffect/entertainer_spot_light_level_%u.cef",effect);
	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",we);

}

void ObjectController::_handleVentriloquism(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//start parsing the commandline
	//either we need to give help
	//or we have a request for a specific emote
	string dataStr;
	message->getStringUnicode16(dataStr);

	// Have to convert BEFORE using toLower, since the conversion done there is removed It will assert().
	// Either do the conversion HERE, or better fix the toLower so it handles unicode also.
	dataStr.convert(BSTRType_ANSI);
	dataStr.toLower();

	PlayerObject*	we	= dynamic_cast<PlayerObject*>(mObject);

	if(!we)
		return;

	if (targetId == 0)
	{
		targetId = we->getTargetId();
	}

	//the target
	CreatureObject* targetObject = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId));
	if(!targetObject)
	{
		targetObject = we;
	}

	if(we->getPerformingState() == PlayerPerformance_None)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_not_performing");
		return;
	}

	uint32 effect;
	uint32 skill;
	uint32 highest;


	sscanf(dataStr.getAnsi(),"%u",&effect);

	//check for skillmod - tied to Mind afaik
	skill = we->getSkillModValue(SMod_healing_dance_mind);
	highest = 0;

	if(skill>= 10)
		highest = 1;

	if(skill>= 30)
		highest = 2;

	if(skill>= 75)
		highest = 3;

	if(!dataStr.getLength())
		effect = highest;

	if (effect > highest)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_level_too_high","","",L"",0,"","",L"");
		return;
	}

	if(highest == 0)
	{
		gMessageLib->sendSystemMessage(we,L"","performance","effect_lack_skill_self","","",L"",0,"","",L"");
		return;
	}

	int8 effectStr[64];

	gMessageLib->sendSystemMessage(we,L"","performance","effect_perform_ventriloquism");

	sprintf(effectStr,"clienteffect/entertainer_ventriloquism_level_%u.cef",effect);

	gMessageLib->sendPlayClientEffectObjectMessage(effectStr,"",targetObject);
}

