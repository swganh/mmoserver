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

#include "ObjectControllerCommandMap.h"
#include "ObjectController.h"
#include "ObjectControllerOpcodes.h"
#include "ScriptEngine/ScriptEngine.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "Common/Message.h"

#include "OCStructureHandlers.h"

//======================================================================================================================

bool						ObjectControllerCommandMap::mInsFlag = false;
ObjectControllerCommandMap* ObjectControllerCommandMap::mSingleton = NULL;

//======================================================================================================================

ObjectControllerCommandMap::ObjectControllerCommandMap(Database* database) :
mDatabase(database)
{
	// setup cpp hooks
	_registerCppHooks();

  // Set up new style hooks
  RegisterCppHooks_();

	// load the property map
	mDatabase->ExecuteSqlAsync(this,NULL,"SELECT commandname,characterability,deny_in_states,healthcost,actioncost,mindcost,"
										 "animationCrc,addtocombatqueue,defaulttime,scripthook,requiredweapongroup,"
										 "cbt_spam,trail1,trail2,commandgroup,allowInPosture,"
										 "health_hit_chance,action_hit_chance,mind_hit_chance,"
										 "knockdown_chance,dizzy_chance,blind_chance,stun_chance,intimidate_chance,"
										 "posture_down_chance,extended_range,damage_multiplier,delay_multiplier"
										 " FROM command_table");
}

//======================================================================================================================

ObjectControllerCommandMap* ObjectControllerCommandMap::Init(Database* database)
{
	if(mInsFlag == false)
	{
		mSingleton = new ObjectControllerCommandMap(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

ObjectControllerCommandMap::~ObjectControllerCommandMap()
{
	mCommandMap.clear();

	CmdPropertyMap::iterator it = mCmdPropertyMap.begin();

	while(it != mCmdPropertyMap.end())
	{
		delete((*it).second);
		mCmdPropertyMap.erase(it++);
	}

	mInsFlag = false;
	delete(mSingleton);
}

//======================================================================================================================

void ObjectControllerCommandMap::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	ObjectControllerCmdProperties* commandProperties;

	DataBinding* binding = mDatabase->CreateDataBinding(28);
	binding->addField(DFT_bstring,offsetof(ObjectControllerCmdProperties,mCommandStr),64,0);
	binding->addField(DFT_bstring,offsetof(ObjectControllerCmdProperties,mAbilityStr),64,1);
	binding->addField(DFT_uint64,offsetof(ObjectControllerCmdProperties,mStates),8,2);
	binding->addField(DFT_int32,offsetof(ObjectControllerCmdProperties,mHealthCost),4,3);
	binding->addField(DFT_int32,offsetof(ObjectControllerCmdProperties,mActionCost),4,4);
	binding->addField(DFT_int32,offsetof(ObjectControllerCmdProperties,mMindCost),4,5);
	binding->addField(DFT_uint32,offsetof(ObjectControllerCmdProperties,mAnimationCrc),4,6);
	binding->addField(DFT_uint8,offsetof(ObjectControllerCmdProperties,mAddToCombatQueue),1,7);
	binding->addField(DFT_uint64,offsetof(ObjectControllerCmdProperties,mDefaultTime),8,8);
	binding->addField(DFT_bstring,offsetof(ObjectControllerCmdProperties,mScriptHook),32,9);
	binding->addField(DFT_uint32,offsetof(ObjectControllerCmdProperties,mRequiredWeaponGroup),4,10);
	binding->addField(DFT_bstring,offsetof(ObjectControllerCmdProperties,mCbtSpam),32,11);
	binding->addField(DFT_uint8,offsetof(ObjectControllerCmdProperties,mTrail1),1,12);
	binding->addField(DFT_uint8,offsetof(ObjectControllerCmdProperties,mTrail2),1,13);
	binding->addField(DFT_uint8,offsetof(ObjectControllerCmdProperties,mCmdGroup),1,14);
	binding->addField(DFT_uint32,offsetof(ObjectControllerCmdProperties,mPostureMask),4,15);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mHealthHitChance),4,16);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mActionHitChance),4,17);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mMindHitChance),4,18);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mKnockdownChance),4,19);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mDizzyChance),4,20);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mBlindChance),4,21);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mStunChance),4,22);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mIntimidateChance),4,23);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mPostureDownChance),4,24);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mExtendedRange),4,25);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mDamageMultiplier),4,26);
	binding->addField(DFT_float,offsetof(ObjectControllerCmdProperties,mDelayMultiplier),4,27);

	uint64 count = result->getRowCount();

	for(uint64 i = 0;i < count;i++)
	{
		commandProperties = new ObjectControllerCmdProperties();

		result->GetNextRow(binding,commandProperties);

		commandProperties->mCommandStr.toLower();
		commandProperties->mCmdCrc	= commandProperties->mCommandStr.getCrc();

		// setup a script hook
		mCmdScriptListener.registerFunction(commandProperties->mCommandStr.getAnsi());

		if(commandProperties->mAbilityStr.getLength())
		{
			commandProperties->mAbilityCrc = commandProperties->mAbilityStr.getCrc();
		}

		// if set, load up the script
		if(commandProperties->mScriptHook.getLength())
		{
			commandProperties->mScript = gScriptEngine->createScript();
			commandProperties->mScript->setFileName(commandProperties->mScriptHook.getAnsi());
			commandProperties->mScript->run();
		}

		mCmdPropertyMap.insert(std::make_pair(commandProperties->mCmdCrc,commandProperties));
	}

	mDatabase->DestroyDataBinding(binding);

	if(result->getRowCount())
		gLogger->log(LogManager::NOTICE,"Mapped functions.");
}

const CommandMap& ObjectControllerCommandMap::getCommandMap() {
  return command_map_;
}

//======================================================================================================================
//
// setup cpp hooks
//

void ObjectControllerCommandMap::_registerCppHooks()
{
  mCommandMap.insert(std::make_pair(opOCspatialchatinternal, std::bind(&ObjectController::_handleSpatialChatInternal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsocialinternal, std::bind(&ObjectController::_handleSocialInternal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetmoodinternal, std::bind(&ObjectController::_handleSetMoodInternal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCopencontainer, std::bind(&ObjectController::_handleOpenContainer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCclosecontainer, std::bind(&ObjectController::_handleCloseContainer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtransferitem, std::bind(&ObjectController::_handleTransferItem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtransferitemarmor, std::bind(&ObjectController::_handleTransferItem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtransferitemmisc, std::bind(&ObjectController::_handleTransferItemMisc, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtransferitemweapon, std::bind(&ObjectController::_handleTransferItem, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsitserver, std::bind(&ObjectController::_handleSitServer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstand, std::bind(&ObjectController::_handleStand, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCprone, std::bind(&ObjectController::_handleProne, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCkneel, std::bind(&ObjectController::_handleKneel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestquestimersandcounters, std::bind(&ObjectController::_handleRequestQuestTimersAndCounters, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCNPCConversationStart, std::bind(&ObjectController::_handleNPCConversationStart, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCNPCConversationStop, std::bind(&ObjectController::_handleNPCConversationStop, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCNPCConversationSelect, std::bind(&ObjectController::_handleNPCConversationSelect, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCPurchaseTicket, std::bind(&ObjectController::_handlePurchaseTicket, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgetattributesbatch, std::bind(&ObjectController::_handleGetAttributesBatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCServerDestroyObject, std::bind(&ObjectController::_handleServerDestroyObject, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCTarget, std::bind(&ObjectController::_handleTarget, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestDraftslotsBatch, std::bind(&ObjectController::_handleRequestDraftslotsBatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestResourceWeightsBatch, std::bind(&ObjectController::_handleRequestResourceWeightsBatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCSynchronizedUIListen, std::bind(&ObjectController::_handleSynchronizedUIListen, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetcurrentskilltitle, std::bind(&ObjectController::_handleSetCurrentSkillTitle, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestbadges, std::bind(&ObjectController::_handleRequestBadges, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetspokenlanguage, std::bind(&ObjectController::_handleSetSpokenLanguage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOClfg, std::bind(&ObjectController::_handleLfg, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCnewbiehelper, std::bind(&ObjectController::_handleNewbieHelper, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCroleplay, std::bind(&ObjectController::_handleRolePlay, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtoggleAwayFromKeyboard, std::bind(&ObjectController::_handleToggleAFK, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtoggleDisplayingFactionRank, std::bind(&ObjectController::_handleToggleDisplayFactionRank, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCanon, std::bind(&ObjectController::_handleAnon, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestsurvey, std::bind(&ObjectController::_handleRequestSurvey, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsurvey, std::bind(&ObjectController::_handleSurvey, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestcoresample, std::bind(&ObjectController::_handleRequestCoreSample, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsample, std::bind(&ObjectController::_handleSample, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	mCommandMap.insert(std::make_pair(opOCrequestbadges, std::bind(&ObjectController::_handleRequestBadges, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestwaypointatposition, std::bind(&ObjectController::_handleRequestWaypointAtPosition, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetwaypointactivestatus, std::bind(&ObjectController::_handleSetWaypointActiveStatus, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCwaypoint, std::bind(&ObjectController::_handleWaypoint, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetwaypointname, std::bind(&ObjectController::_handleSetWaypointName, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestcharactersheetinfo, std::bind(&ObjectController::_handleRequestCharacterSheetInfo, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrequestbiography, std::bind(&ObjectController::_handleRequestBiography, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsetbiography, std::bind(&ObjectController::_handleSetBiography, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCeditbiography, std::bind(&ObjectController::_handleEditBiography, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsurrenderskill, std::bind(&ObjectController::_handleSurrenderSkill, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCclientqualifiedforskill, std::bind(&ObjectController::_handleClientQualifiedForSkill, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCteach, std::bind(&ObjectController::_handleTeach, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCBoardTransport, std::bind(&ObjectController::_handleBoardTransport, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCNewbieSelectStartingLocation, std::bind(&ObjectController::_handleNewbieSelectStartingLocation, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	mCommandMap.insert(std::make_pair(opOCLogoutClient, std::bind(&ObjectController::_handleClientLogout, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCburstrun, std::bind(&ObjectController::_BurstRun, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	mCommandMap.insert(std::make_pair(opOCFactoryCrateSplit, std::bind(&ObjectController::_handleFactoryCrateSplit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCExtractObject, std::bind(&ObjectController::_ExtractObject, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCresourcecontainertransfer, std::bind(&ObjectController::_handleResourceContainerTransfer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCresourcecontainersplit, std::bind(&ObjectController::_handleResourceContainerSplit, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	


	

	//pets,mounts
	mCommandMap.insert(std::make_pair(opOCmount, std::bind(&ObjectController::_handleMount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdismount, std::bind(&ObjectController::_handleDismount, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	//social
	mCommandMap.insert(std::make_pair(opOCrequestcharactermatch, std::bind(&ObjectController::_handleRequestCharacterMatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtip, std::bind(&ObjectController::_handleTip, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCaddfriend, std::bind(&ObjectController::_handleAddFriend, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCremovefriend, std::bind(&ObjectController::_handleRemoveFriend, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCaddignore, std::bind(&ObjectController::_handleAddIgnore, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCremoveignore, std::bind(&ObjectController::_handleRemoveIgnore, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmatch, std::bind(&ObjectController::_handleMatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCfiendfriend, std::bind(&ObjectController::_handlefindfriend, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	//crafting
	mCommandMap.insert(std::make_pair(opOCRequestCraftingSession, std::bind(&ObjectController::_handleRequestCraftingSession, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCCancelCraftingSession, std::bind(&ObjectController::_handleCancelCraftingSession, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCSelectDraftSchematic, std::bind(&ObjectController::_handleSelectDraftSchematic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCnextcraftingstage, std::bind(&ObjectController::_handleNextCraftingStage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCcreateprototype, std::bind(&ObjectController::_handleCreatePrototype, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCcreatemanfschematic, std::bind(&ObjectController::_handleCreateManufactureSchematic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// combat
	mCommandMap.insert(std::make_pair(opOCduel, std::bind(&ObjectController::_handleDuel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCendduel, std::bind(&ObjectController::_handleEndDuel, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCpeace, std::bind(&ObjectController::_handlePeace, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdeathblow, std::bind(&ObjectController::_handleDeathBlow, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCloot, std::bind(&ObjectController::_handleLoot, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	//attackhandler are NOT par of the commandMap!!!


	// brawler
	mCommandMap.insert(std::make_pair(opOCberserk1, std::bind(&ObjectController::_handleBerserk1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCcenterofbeing, std::bind(&ObjectController::_handleCenterOfBeing, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCintimidate1, std::bind(&ObjectController::_handleIntimidate1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtaunt, std::bind(&ObjectController::_handleTaunt, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCwarcry1, std::bind(&ObjectController::_handleWarcry1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCberserk2, std::bind(&ObjectController::_handleBerserk2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCintimidate2, std::bind(&ObjectController::_handleIntimidate2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCwarcry2, std::bind(&ObjectController::_handleWarcry2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// marksman
	mCommandMap.insert(std::make_pair(opOCtumbletokneeling, std::bind(&ObjectController::_handleTumbleToKneeling, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtumbletoprone, std::bind(&ObjectController::_handleTumbleToProne, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtumbletostanding, std::bind(&ObjectController::_handleTumbleToStanding, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtakecover, std::bind(&ObjectController::_handleTakeCover, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCaim, std::bind(&ObjectController::_handleAim, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	//entertainer
	mCommandMap.insert(std::make_pair(opOCstartdance, std::bind(&ObjectController::_handlestartdance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstopdance, std::bind(&ObjectController::_handlestopdance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstartmusic, std::bind(&ObjectController::_handlestartmusic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstopmusic, std::bind(&ObjectController::_handlestopmusic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCflourish, std::bind(&ObjectController::_handleflourish, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCwatch, std::bind(&ObjectController::_handlewatch, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOClisten, std::bind(&ObjectController::_handlelisten, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstopwatching, std::bind(&ObjectController::_handlestopwatching, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstoplistening, std::bind(&ObjectController::_handlestoplistening, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCPauseMusic, std::bind(&ObjectController::_handlePauseMusic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCPauseDance, std::bind(&ObjectController::_handlePauseDance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCChangeMusic, std::bind(&ObjectController::_handleChangeMusic, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCChangeDance, std::bind(&ObjectController::_handleChangeDance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCDenyService, std::bind(&ObjectController::_handleDenyService, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCStartBand, std::bind(&ObjectController::_handleStartBand, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCStopBand, std::bind(&ObjectController::_handleStopBand, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCBandFlourish, std::bind(&ObjectController::_handleBandFlourish, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCImageDesign, std::bind(&ObjectController::_handleImageDesign, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCStatMigration, std::bind(&ObjectController::_handleStatMigration, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCRequestStatMigrationData, std::bind(&ObjectController::_handleRequestStatMigrationData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCHoloEmote, std::bind(&ObjectController::_handlePlayHoloEmote, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCDazzle, std::bind(&ObjectController::_handleDazzle, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCFireJet, std::bind(&ObjectController::_handleFireJet, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCDistract, std::bind(&ObjectController::_handleDistract, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCColorLights, std::bind(&ObjectController::_handleColorLights, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCSmokeBomb, std::bind(&ObjectController::_handleSmokeBomb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCSpotLight, std::bind(&ObjectController::_handleSpotLight, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCVentriloquism, std::bind(&ObjectController::_handleVentriloquism, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// scout
	mCommandMap.insert(std::make_pair(opOCharvestcorpse, std::bind(&ObjectController::_handleHarvestCorpse, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmaskscent, std::bind(&ObjectController::_handleMaskScent, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforage, std::bind(&ObjectController::_handleForage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCthrowtrap, std::bind(&ObjectController::_handleThrowTrap, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// medic
	mCommandMap.insert(std::make_pair(opOCdiagnose, std::bind(&ObjectController::_handleDiagnose, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealdamage, std::bind(&ObjectController::_handleHealDamage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealwound, std::bind(&ObjectController::_handleHealWound, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmedicalforage, std::bind(&ObjectController::_handleMedicalForage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtenddamage, std::bind(&ObjectController::_handleTendDamage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtendwound, std::bind(&ObjectController::_handleTendWound, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCfirstaid, std::bind(&ObjectController::_handleFirstAid, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCquickheal, std::bind(&ObjectController::_handleQuickHeal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdragincapacitatedplayer, std::bind(&ObjectController::_handleDragIncapacitatedPlayer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// bio - engineer
	mCommandMap.insert(std::make_pair(opOCsampledna, std::bind(&ObjectController::_handleSampleDNA, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// combat medic
	mCommandMap.insert(std::make_pair(opOCapplypoison, std::bind(&ObjectController::_handleApplyPoison, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCapplydisease, std::bind(&ObjectController::_handleApplyDisease, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmind, std::bind(&ObjectController::_handleHealMind, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// doctor
	mCommandMap.insert(std::make_pair(opOChealstate, std::bind(&ObjectController::_handleHealState, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCcurepoison, std::bind(&ObjectController::_handleCurePoison, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCcuredisease, std::bind(&ObjectController::_handleCureDisease, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealenhance, std::bind(&ObjectController::_handleHealEnhance, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCextinguishfire, std::bind(&ObjectController::_handleExtinguishFire, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCreviveplayer, std::bind(&ObjectController::_handleRevivePlayer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// ranger
	mCommandMap.insert(std::make_pair(opOCareatrack, std::bind(&ObjectController::_handleAreaTrack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCconceal, std::bind(&ObjectController::_handleConceal, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrescue, std::bind(&ObjectController::_handleRescue, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// smuggler
	mCommandMap.insert(std::make_pair(opOCfeigndeath, std::bind(&ObjectController::_handleFeignDeath, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// squad leader
	mCommandMap.insert(std::make_pair(opOCsysgroup, std::bind(&ObjectController::_handleSysGroup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCsteadyaim, std::bind(&ObjectController::_handleSteadyAim, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCvolleyfire, std::bind(&ObjectController::_handleVolleyFire, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCformup, std::bind(&ObjectController::_handleFormup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCboostmorale, std::bind(&ObjectController::_handleBoostMorale, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCrally, std::bind(&ObjectController::_handleRally, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCretreat, std::bind(&ObjectController::_handleRetreat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// teras kasi
	mCommandMap.insert(std::make_pair(opOCmeditate, std::bind(&ObjectController::_handleMeditate, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCpowerboost, std::bind(&ObjectController::_handlePowerBoost, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceofwill, std::bind(&ObjectController::_handleForceOfWill, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// force defense
	mCommandMap.insert(std::make_pair(opOCavoidincapacitation, std::bind(&ObjectController::_handleAvoidIncapacitation, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// force enhancement
	mCommandMap.insert(std::make_pair(opOCforceabsorb1, std::bind(&ObjectController::_handleForceAbsorb1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceabsorb2, std::bind(&ObjectController::_handleForceAbsorb2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcespeed1, std::bind(&ObjectController::_handleForceSpeed1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcespeed2, std::bind(&ObjectController::_handleForceSpeed2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcerun1, std::bind(&ObjectController::_handleForceRun1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcerun2, std::bind(&ObjectController::_handleForceRun2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcerun3, std::bind(&ObjectController::_handleForceRun3, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcefeedback1, std::bind(&ObjectController::_handleForceFeedback1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcefeedback2, std::bind(&ObjectController::_handleForceFeedback2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcearmor1, std::bind(&ObjectController::_handleForceArmor1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcearmor2, std::bind(&ObjectController::_handleForceArmor1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceresistbleeding, std::bind(&ObjectController::_handleForceResistBleeding, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceresistdisease, std::bind(&ObjectController::_handleForceResistDisease, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceresistpoison, std::bind(&ObjectController::_handleForceResistPoison, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceresiststates, std::bind(&ObjectController::_handleForceResistStates, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtransferforce, std::bind(&ObjectController::_handleTransferForce, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCchannelforce, std::bind(&ObjectController::_handleChannelForce, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdrainforce, std::bind(&ObjectController::_handleDrainForce, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceshield1, std::bind(&ObjectController::_handleForceShield1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceshield2, std::bind(&ObjectController::_handleForceShield2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcemeditate, std::bind(&ObjectController::_handleForceMeditate, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCregainconsciousness, std::bind(&ObjectController::_handleRegainConsciousness, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// force healing
	mCommandMap.insert(std::make_pair(opOChealallself1, std::bind(&ObjectController::_handleHealAllSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealallself2, std::bind(&ObjectController::_handleHealAllSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthself1, std::bind(&ObjectController::_handleHealHealthSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthself2, std::bind(&ObjectController::_handleHealHealthSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionself1, std::bind(&ObjectController::_handleHealActionSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionself2, std::bind(&ObjectController::_handleHealActionSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindself1, std::bind(&ObjectController::_handleHealMindSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindself2, std::bind(&ObjectController::_handleHealMindSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionwoundself1, std::bind(&ObjectController::_handleHealActionWoundSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionwoundself2, std::bind(&ObjectController::_handleHealActionWoundSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundself1, std::bind(&ObjectController::_handleHealHealthWoundSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundself2, std::bind(&ObjectController::_handleHealHealthWoundSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealbattlefatigueself1, std::bind(&ObjectController::_handleHealBattleFatigueSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealbattlefatigueself2, std::bind(&ObjectController::_handleHealBattleFatigueSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindwoundself1, std::bind(&ObjectController::_handleHealMindWoundSelf1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindwoundself2, std::bind(&ObjectController::_handleHealMindWoundSelf2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionwoundother1, std::bind(&ObjectController::_handleHealActionWoundOther1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealactionwoundother2, std::bind(&ObjectController::_handleHealActionWoundOther2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundother1, std::bind(&ObjectController::_handleHealHealthWoundOther1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundother2, std::bind(&ObjectController::_handleHealHealthWoundOther2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindwoundother1, std::bind(&ObjectController::_handleHealMindWoundOther1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealmindwoundother2, std::bind(&ObjectController::_handleHealMindWoundOther2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealallother1, std::bind(&ObjectController::_handleHealAllOther1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealallother2, std::bind(&ObjectController::_handleHealAllOther2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealstatesother, std::bind(&ObjectController::_handleHealStatesOther, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCstopbleeding, std::bind(&ObjectController::_handleStopBleeding, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcecuredisease, std::bind(&ObjectController::_handleForceCureDisease, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcecurepoison, std::bind(&ObjectController::_handleForceCurePoison, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOChealstatesself, std::bind(&ObjectController::_handleHealStatesSelf, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtotalhealother, std::bind(&ObjectController::_handleTotalHealOther, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCtotalhealself, std::bind(&ObjectController::_handleTotalHealSelf, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// force powers
	mCommandMap.insert(std::make_pair(opOCanimalscare, std::bind(&ObjectController::_handleAnimalScare, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcelightningsingle1, std::bind(&ObjectController::_handleForceLightningSingle1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcelightningsingle2, std::bind(&ObjectController::_handleForceLightningSingle2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcelightningcone1, std::bind(&ObjectController::_handleForceLightningCone1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcelightningcone2, std::bind(&ObjectController::_handleForceLightningCone2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmindblast1, std::bind(&ObjectController::_handleMindblast1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmindblast2, std::bind(&ObjectController::_handleMindblast2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCanimalcalm, std::bind(&ObjectController::_handleAnimalCalm, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCanimalattack, std::bind(&ObjectController::_handleAnimalAttack, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceweaken1, std::bind(&ObjectController::_handleForceWeaken1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceweaken2, std::bind(&ObjectController::_handleForceWeaken2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceintimidate1, std::bind(&ObjectController::_handleForceIntimidate1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceintimidate2, std::bind(&ObjectController::_handleForceIntimidate2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcethrow1, std::bind(&ObjectController::_handleForceThrow1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcethrow2, std::bind(&ObjectController::_handleForceThrow2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceknockdown1, std::bind(&ObjectController::_handleForceKnockdown1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceknockdown2, std::bind(&ObjectController::_handleForceKnockdown2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforceknockdown3, std::bind(&ObjectController::_handleForceKnockdown3, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCforcechoke, std::bind(&ObjectController::_handleForceChoke, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCjedimindtrick, std::bind(&ObjectController::_handleJediMindTrick, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));


	// groups
	mCommandMap.insert(std::make_pair(opOCinvite, std::bind(&ObjectController::_handleInvite, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCuninvite, std::bind(&ObjectController::_handleUninvite, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCjoin, std::bind(&ObjectController::_handleJoin, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdecline, std::bind(&ObjectController::_handleDecline, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdisband, std::bind(&ObjectController::_handleDisband, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCleavegroup, std::bind(&ObjectController::_handleLeaveGroup, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmakeleader, std::bind(&ObjectController::_handleMakeLeader, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCdismissgroupmember, std::bind(&ObjectController::_handleDismissGroupMember, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgroupchat, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	mCommandMap.insert(std::make_pair(opOCg, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgc, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgsay, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgtell, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCgroupsay, std::bind(&ObjectController::_handleGroupChat, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	mCommandMap.insert(std::make_pair(opOCgrouploot, std::bind(&ObjectController::_handleGroupLootMode, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCmakemasterlooter, std::bind(&ObjectController::_handleMakeMasterLooter, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// custom
	mCommandMap.insert(std::make_pair(opOCEndBurstRun, std::bind(&ObjectController::_endBurstRun, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	// admin profession
	mCommandMap.insert(std::make_pair(opOCAdminSysMsg, std::bind(&ObjectController::_handleAdminSysMsg, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminWarpSelf, std::bind(&ObjectController::_handleAdminWarpSelf, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcast, std::bind(&ObjectController::_handleBroadcast, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcastPlanet, std::bind(&ObjectController::_handleBroadcastPlanet, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcastGalaxy, std::bind(&ObjectController::_handleBroadcastGalaxy, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminShutdownGalaxy, std::bind(&ObjectController::_handleShutdownGalaxy, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opOCAdminCancelShutdownGalaxy, std::bind(&ObjectController::_handleCancelShutdownGalaxy, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

	//Structures
	mCommandMap.insert(std::make_pair(opOCPlaceStructure, std::bind(&ObjectController::_handleStructurePlacement, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opPermissionListModify, std::bind(&ObjectController::_handleModifyPermissionList, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opTransferStructure, std::bind(&ObjectController::_handleTransferStructure, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opNameStructure, std::bind(&ObjectController::_handleNameStructure, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opHarvesterGetResourceData, std::bind(&ObjectController::_handleHarvesterGetResourceData, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opHarvesterSelectResource, std::bind(&ObjectController::_handleHarvesterSelectResource, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opHarvesterActivate, std::bind(&ObjectController::_handleHarvesterActivate, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opHarvesterDeActivate, std::bind(&ObjectController::_handleHarvesterDeActivate, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opDiscardHopper, std::bind(&ObjectController::_handleDiscardHopper, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
  
	mCommandMap.insert(std::make_pair(opItemMoveForward, std::bind(&ObjectController::HandleItemMoveForward_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opItemMoveBack, std::bind(&ObjectController::HandleItemMoveBack_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opItemMoveUp, std::bind(&ObjectController::HandleItemMoveUp_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opItemMoveDown, std::bind(&ObjectController::HandleItemMoveDown_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	
	mCommandMap.insert(std::make_pair(opItemRotateLeft, std::bind(&ObjectController::HandleItemRotateLeft_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	mCommandMap.insert(std::make_pair(opItemRotateRight, std::bind(&ObjectController::HandleItemRotateRight_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
	
	mCommandMap.insert(std::make_pair(opRotateFurniture, std::bind(&ObjectController::HandleRotateFurniture_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
}


void ObjectControllerCommandMap::RegisterCppHooks_()
{
  command_map_.insert(std::make_pair(opMoveFurniture, std::bind(&HandleMoveFurniture, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));	
}

//======================================================================================================================

void ObjectControllerCommandMap::ScriptRegisterEvent(void* script,std::string eventFunction)
{
	mCmdScriptListener.registerScript(reinterpret_cast<Script*>(script),(int8*)eventFunction.c_str());
}

//======================================================================================================================


