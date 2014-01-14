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
#include "Wearable.h"
#include "WorldConfig.h"
#include "WorldManager.h"
#include "UIManager.h"

#include "MessageLib/MessageLib.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

#include <boost/lexical_cast.hpp>

//======================================================================================================================
//
// set title
//

void ObjectController::_handleSetCurrentSkillTitle(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			newTitle;

	message->getStringUnicode16(newTitle);
	newTitle.convert(BSTRType_ANSI);

	SkillList* sList = playerObject->getSkills();
	SkillList::iterator sEnd = sList->end();
	for(SkillList::iterator it=sList->begin(); it != sEnd; ++it)
	{
		if((*it)->mIsTitle)
		{
			if(strcmp((*it)->mName.getAnsi(),newTitle.getAnsi()) == 0)
			{
				playerObject->setTitle(newTitle.getAnsi());
				gMessageLib->sendTitleUpdate(playerObject);
				break;
			}
		}
	}
}

//======================================================================================================================
//
// set language
//

void ObjectController::_handleSetSpokenLanguage(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	playerObject	= dynamic_cast<PlayerObject*>(mObject);
	string			tmpStr;

	message->getStringUnicode16(tmpStr);
	tmpStr.convert(BSTRType_ANSI);

	uint32 languageId = boost::lexical_cast<uint32>(tmpStr.getAnsi());
	playerObject->setLanguage(languageId);
	gMessageLib->sendLanguagePlay9(playerObject);
}

//======================================================================================================================
//
// set lfg
//

void ObjectController::_handleLfg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	playerObject->togglePlayerFlag(PlayerFlag_Lfg);

	gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================
//
// set helper
//

void ObjectController::_handleNewbieHelper(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	playerObject->togglePlayerFlag(PlayerFlag_NoobHelper);

	gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================
//
// set rp
//

void ObjectController::_handleRolePlay(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	playerObject->togglePlayerFlag(PlayerFlag_RolePlayer);

	gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================
//
// set afk
//

void ObjectController::_handleToggleAFK(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	playerObject->togglePlayerFlag(PlayerFlag_Afk);

	gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================
//
// set faction display
//

void ObjectController::_handleToggleDisplayFactionRank(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	playerObject->togglePlayerFlag(PlayerFlag_FactionRank);

	gMessageLib->sendUpdatePlayerFlags(playerObject);
}

//======================================================================================================================
//
// set anonymous
//

void ObjectController::_handleAnon(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	//PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	// simple toggle, TODO: delta updates, figure and bitmask to playerobject, store to db
}

//======================================================================================================================
//
// sends out badge information for the character sheet
//

void ObjectController::_handleRequestBadges(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);
	PlayerObject* targetObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));

	if(targetObject == NULL)
	{
		gLogger->logMsgF("ObjController::_handleRequestbages: could not find %"PRIu64"",MSG_NORMAL,targetId);
		return;
	}

	gMessageLib->sendBadges(targetObject,playerObject);
}

//======================================================================================================================
//
// request charactersheet info
//

void ObjectController::_handleRequestCharacterSheetInfo(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);

	gMessageLib->sendCharacterSheetResponse(playerObject);
}

//======================================================================================================================
//
// request biography
//

void ObjectController::_handleRequestBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* playerObject = dynamic_cast<PlayerObject*>(mObject);
	PlayerObject* targetObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));

	if(targetObject == NULL)
	{
		gLogger->logMsgF("ObjController::_handleRequestBiography: could not find %"PRIu64"",MSG_NORMAL,targetId);
		return;
	}

	gMessageLib->sendBiography(playerObject,targetObject);
}

//======================================================================================================================
//
// set biography
//

void ObjectController::_handleSetBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player	= dynamic_cast<PlayerObject*>(mObject);
	string			bio;
	int8			sql[5000],end[64],*sqlPointer;

	message->getStringUnicode16(bio);

	player->setBiography(bio);

	bio.convert(BSTRType_ANSI);
	sprintf(sql,"UPDATE character_biography SET biography ='");
	sprintf(end,"' WHERE character_id = %"PRIu64"",player->getId());
	sqlPointer = sql + strlen(sql);
	sqlPointer += mDatabase->Escape_String(sqlPointer,bio.getAnsi(),bio.getLength());
	strcat(sql,end);

	mDatabase->ExecuteSqlAsync(0,0,sql);
}

//======================================================================================================================
//
// admin command ?
//
void ObjectController::_handleEditBiography(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

}

//======================================================================================================================
//
// request character match
//

void ObjectController::_handleRequestCharacterMatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	player			= dynamic_cast<PlayerObject*>(mObject);
	string			dataStr;
	PlayerList		playersMatched;
	uint32			masksCount		= 0;
	uint32			playerFlags		= 0;
	uint32			mask2			= 0;
	uint32			mask3			= 0;
	uint32			mask4			= 0;
	uint32			factionCrc		= 0;
	int32			raceId			= 0;
	int8			titleStr[128];
	int8			unknown[64];
	uint32			elementCount	= 0;
	Skill*			skill			= NULL;

	message->getStringUnicode16(dataStr);

	if(dataStr.getLength())
		elementCount = swscanf(dataStr.getUnicode16(),L"%u %u %u %u %u %u %i %S %S",&masksCount,&playerFlags,&mask2,&mask3,&mask4,&factionCrc,&raceId,titleStr,unknown);

	if(elementCount != 9)
	{
		gLogger->logMsgF("ObjController::_handleRequestCharacterMatch: argument mismatch %"PRIu64"",MSG_NORMAL,player->getId());
		return;
	}

	if(strcmp(titleStr,"\"\"") != 0)
	{
		skill = gSkillManager->getSkillByName(titleStr);

		if(skill == NULL)
		{
			gLogger->logMsgF("ObjController::_handleRequestCharacterMatch: could not find matching skill for %s",MSG_NORMAL,titleStr);
			return;
		}
	}

	// for now check players in viewing range
	PlayerObjectSet* inRangePlayers	= player->getKnownPlayers();

	// and ourselve =)
	playersMatched.push_back(player);

	PlayerObjectSet::iterator it = inRangePlayers->begin();

	while(it != inRangePlayers->end())
	{
		PlayerObject* inRangePlayer = (*it);

		if(((playerFlags & inRangePlayer->getPlayerFlags()) == playerFlags)
			&&(raceId == -1 || raceId == inRangePlayer->getRaceId())
			&&(factionCrc == 0 || factionCrc == 1 || factionCrc == inRangePlayer->getFaction().getCrc()))
		{
			if(skill == NULL)
			{
				playersMatched.push_back(inRangePlayer);
			}
			else
			{
				if((skill->mIsProfession && strstr(inRangePlayer->getTitle().getAnsi(),titleStr))
					|| (strcmp(titleStr,inRangePlayer->getTitle().getAnsi()) == 0))
					playersMatched.push_back(inRangePlayer);
			}
		}

		++it;
	}

	gMessageLib->sendCharacterMatchResults(&playersMatched,player);
}

//======================================================================================================================
//
// match
//

void ObjectController::_handleMatch(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject*	matchObject	= dynamic_cast<PlayerObject*>(mObject);
	string			matchfield;
	uint32			i1,i2,i3,i4,i5;

	message->getStringUnicode16(matchfield);

	swscanf(matchfield.getUnicode16(),L"%u %u %u %u %u",&i1,&i2,&i3,&i4,&i5);

	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncContainerPool.malloc()) ObjControllerAsyncContainer(OCQuery_Nope),
								"update character_matchmaking set match_1 = %u, match_2 = %u, match_3 = %u, match_4 = %u where character_id = %I64u",
								i2,i3,i4,i5,matchObject->getId());

	// update the players Object
	matchObject->setPlayerMatch(0,i2);
	matchObject->setPlayerMatch(1,i3);
	matchObject->setPlayerMatch(2,i4);
	matchObject->setPlayerMatch(3,i5);

	// now send the delta
	gMessageLib->sendMatchPlay3(matchObject);
}

//======================================================================================================================


