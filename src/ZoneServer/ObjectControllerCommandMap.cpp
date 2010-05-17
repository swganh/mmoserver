/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

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



//======================================================================================================================

bool						ObjectControllerCommandMap::mInsFlag = false;
ObjectControllerCommandMap* ObjectControllerCommandMap::mSingleton = NULL;

//======================================================================================================================

ObjectControllerCommandMap::ObjectControllerCommandMap(Database* database) :
mDatabase(database)
{
	// setup cpp hooks
	_registerCppHooks();

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

//======================================================================================================================
//
// setup cpp hooks
//

void ObjectControllerCommandMap::_registerCppHooks()
{
	mCommandMap.insert(std::make_pair(opOCspatialchatinternal,&ObjectController::_handleSpatialChatInternal));
	mCommandMap.insert(std::make_pair(opOCsocialinternal,&ObjectController::_handleSocialInternal));
	mCommandMap.insert(std::make_pair(opOCsetmoodinternal,&ObjectController::_handleSetMoodInternal));
	mCommandMap.insert(std::make_pair(opOCopencontainer,&ObjectController::_handleOpenContainer));
	mCommandMap.insert(std::make_pair(opOCclosecontainer,&ObjectController::_handleCloseContainer));
	mCommandMap.insert(std::make_pair(opOCtransferitem,&ObjectController::_handleTransferItem));
	mCommandMap.insert(std::make_pair(opOCtransferitemarmor,&ObjectController::_handleTransferItem));
	mCommandMap.insert(std::make_pair(opOCtransferitemmisc,&ObjectController::_handleTransferItemMisc));
	mCommandMap.insert(std::make_pair(opOCtransferitemweapon,&ObjectController::_handleTransferItem));
	mCommandMap.insert(std::make_pair(opOCsitserver,&ObjectController::_handleSitServer));
	mCommandMap.insert(std::make_pair(opOCstand,&ObjectController::_handleStand));
	mCommandMap.insert(std::make_pair(opOCprone,&ObjectController::_handleProne));
	mCommandMap.insert(std::make_pair(opOCkneel,&ObjectController::_handleKneel));
	mCommandMap.insert(std::make_pair(opOCrequestquestimersandcounters,&ObjectController::_handleRequestQuestTimersAndCounters));
	mCommandMap.insert(std::make_pair(opOCNPCConversationStart,&ObjectController::_handleNPCConversationStart));
	mCommandMap.insert(std::make_pair(opOCNPCConversationStop,&ObjectController::_handleNPCConversationStop));
	mCommandMap.insert(std::make_pair(opOCNPCConversationSelect,&ObjectController::_handleNPCConversationSelect));
	mCommandMap.insert(std::make_pair(opOCPurchaseTicket,&ObjectController::_handlePurchaseTicket));
	mCommandMap.insert(std::make_pair(opOCgetattributesbatch,&ObjectController::_handleGetAttributesBatch));
	mCommandMap.insert(std::make_pair(opOCServerDestroyObject,&ObjectController::_handleServerDestroyObject));
	mCommandMap.insert(std::make_pair(opOCTarget,&ObjectController::_handleTarget));
	mCommandMap.insert(std::make_pair(opOCrequestDraftslotsBatch,&ObjectController::_handleRequestDraftslotsBatch));
	mCommandMap.insert(std::make_pair(opOCrequestResourceWeightsBatch,&ObjectController::_handleRequestResourceWeightsBatch));
	mCommandMap.insert(std::make_pair(opOCSynchronizedUIListen,&ObjectController::_handleSynchronizedUIListen));
	mCommandMap.insert(std::make_pair(opOCsetcurrentskilltitle,&ObjectController::_handleSetCurrentSkillTitle));
	mCommandMap.insert(std::make_pair(opOCrequestbadges,&ObjectController::_handleRequestBadges));
	mCommandMap.insert(std::make_pair(opOCsetspokenlanguage, &ObjectController::_handleSetSpokenLanguage));
	mCommandMap.insert(std::make_pair(opOClfg,&ObjectController::_handleLfg));
	mCommandMap.insert(std::make_pair(opOCnewbiehelper,&ObjectController::_handleNewbieHelper));
	mCommandMap.insert(std::make_pair(opOCroleplay,&ObjectController::_handleRolePlay));
	mCommandMap.insert(std::make_pair(opOCtoggleAwayFromKeyboard,&ObjectController::_handleToggleAFK));
	mCommandMap.insert(std::make_pair(opOCtoggleDisplayingFactionRank,&ObjectController::_handleToggleDisplayFactionRank));
	mCommandMap.insert(std::make_pair(opOCanon,&ObjectController::_handleAnon));
	mCommandMap.insert(std::make_pair(opOCrequestsurvey,&ObjectController::_handleRequestSurvey));
	mCommandMap.insert(std::make_pair(opOCsurvey,&ObjectController::_handleSurvey));
	mCommandMap.insert(std::make_pair(opOCrequestcoresample,&ObjectController::_handleRequestCoreSample));
	mCommandMap.insert(std::make_pair(opOCsample,&ObjectController::_handleSample));

	mCommandMap.insert(std::make_pair(opOCrequestbadges,&ObjectController::_handleRequestBadges));
	mCommandMap.insert(std::make_pair(opOCrequestwaypointatposition, &ObjectController::_handleRequestWaypointAtPosition));
	mCommandMap.insert(std::make_pair(opOCsetwaypointactivestatus, &ObjectController::_handleSetWaypointActiveStatus));
	mCommandMap.insert(std::make_pair(opOCwaypoint,&ObjectController::_handleWaypoint));
	mCommandMap.insert(std::make_pair(opOCsetwaypointname, &ObjectController::_handleSetWaypointName));
	mCommandMap.insert(std::make_pair(opOCrequestcharactersheetinfo,&ObjectController::_handleRequestCharacterSheetInfo));
	mCommandMap.insert(std::make_pair(opOCrequestbiography,&ObjectController::_handleRequestBiography));
	mCommandMap.insert(std::make_pair(opOCsetbiography,&ObjectController::_handleSetBiography));
	mCommandMap.insert(std::make_pair(opOCeditbiography,&ObjectController::_handleEditBiography));
	mCommandMap.insert(std::make_pair(opOCsurrenderskill,&ObjectController::_handleSurrenderSkill));
	mCommandMap.insert(std::make_pair(opOCclientqualifiedforskill,&ObjectController::_handleClientQualifiedForSkill));
	mCommandMap.insert(std::make_pair(opOCteach,&ObjectController::_handleTeach));
	mCommandMap.insert(std::make_pair(opOCBoardTransport,&ObjectController::_handleBoardTransport));
	mCommandMap.insert(std::make_pair(opOCNewbieSelectStartingLocation,&ObjectController::_handleNewbieSelectStartingLocation));

	mCommandMap.insert(std::make_pair(opOCLogoutClient,&ObjectController::_handleClientLogout));
	mCommandMap.insert(std::make_pair(opOCburstrun,&ObjectController::_BurstRun));

	mCommandMap.insert(std::make_pair(opOCFactoryCrateSplit,&ObjectController::_handleFactoryCrateSplit));
	mCommandMap.insert(std::make_pair(opOCExtractObject,&ObjectController::_ExtractObject));
	mCommandMap.insert(std::make_pair(opOCresourcecontainertransfer,&ObjectController::_handleResourceContainerTransfer));
	mCommandMap.insert(std::make_pair(opOCresourcecontainersplit, &ObjectController::_handleResourceContainerSplit));
	


	

	//pets,mounts
	mCommandMap.insert(std::make_pair(opOCmount,&ObjectController::_handleMount));
	mCommandMap.insert(std::make_pair(opOCdismount,&ObjectController::_handleDismount));

	//social
	mCommandMap.insert(std::make_pair(opOCrequestcharactermatch,&ObjectController::_handleRequestCharacterMatch));
	mCommandMap.insert(std::make_pair(opOCtip,&ObjectController::_handleTip));
	mCommandMap.insert(std::make_pair(opOCaddfriend,&ObjectController::_handleAddFriend));
	mCommandMap.insert(std::make_pair(opOCremovefriend,&ObjectController::_handleRemoveFriend));
	mCommandMap.insert(std::make_pair(opOCaddignore,&ObjectController::_handleAddIgnore));
	mCommandMap.insert(std::make_pair(opOCremoveignore,&ObjectController::_handleRemoveIgnore));
	mCommandMap.insert(std::make_pair(opOCmatch,&ObjectController::_handleMatch));
	mCommandMap.insert(std::make_pair(opOCfiendfriend,&ObjectController::_handlefindfriend));

	//crafting
	mCommandMap.insert(std::make_pair(opOCRequestCraftingSession,&ObjectController::_handleRequestCraftingSession));
	mCommandMap.insert(std::make_pair(opOCCancelCraftingSession,&ObjectController::_handleCancelCraftingSession));
	mCommandMap.insert(std::make_pair(opOCSelectDraftSchematic,&ObjectController::_handleSelectDraftSchematic));
	mCommandMap.insert(std::make_pair(opOCnextcraftingstage,&ObjectController::_handleNextCraftingStage));
	mCommandMap.insert(std::make_pair(opOCcreateprototype,&ObjectController::_handleCreatePrototype));
	mCommandMap.insert(std::make_pair(opOCcreatemanfschematic,&ObjectController::_handleCreateManufactureSchematic));

	// combat
	mCommandMap.insert(std::make_pair(opOCduel,&ObjectController::_handleDuel));
	mCommandMap.insert(std::make_pair(opOCendduel,&ObjectController::_handleEndDuel));
	mCommandMap.insert(std::make_pair(opOCpeace,&ObjectController::_handlePeace));
	mCommandMap.insert(std::make_pair(opOCdeathblow,&ObjectController::_handleDeathBlow));
	mCommandMap.insert(std::make_pair(opOCloot,&ObjectController::_handleLoot));

	//attackhandler are NOT par of the commandMap!!!


	// brawler
	mCommandMap.insert(std::make_pair(opOCberserk1,&ObjectController::_handleBerserk1));
	mCommandMap.insert(std::make_pair(opOCcenterofbeing,&ObjectController::_handleCenterOfBeing));
	mCommandMap.insert(std::make_pair(opOCintimidate1,&ObjectController::_handleIntimidate1));
	mCommandMap.insert(std::make_pair(opOCtaunt,&ObjectController::_handleTaunt));
	mCommandMap.insert(std::make_pair(opOCwarcry1,&ObjectController::_handleWarcry1));
	mCommandMap.insert(std::make_pair(opOCberserk2,&ObjectController::_handleBerserk2));
	mCommandMap.insert(std::make_pair(opOCintimidate2,&ObjectController::_handleIntimidate2));
	mCommandMap.insert(std::make_pair(opOCwarcry2,&ObjectController::_handleWarcry2));

	// marksman
	mCommandMap.insert(std::make_pair(opOCtumbletokneeling,&ObjectController::_handleTumbleToKneeling));
	mCommandMap.insert(std::make_pair(opOCtumbletoprone,&ObjectController::_handleTumbleToProne));
	mCommandMap.insert(std::make_pair(opOCtumbletostanding,&ObjectController::_handleTumbleToStanding));
	mCommandMap.insert(std::make_pair(opOCtakecover,&ObjectController::_handleTakeCover));
	mCommandMap.insert(std::make_pair(opOCaim,&ObjectController::_handleAim));

	//entertainer
	mCommandMap.insert(std::make_pair(opOCstartdance,&ObjectController::_handlestartdance));
	mCommandMap.insert(std::make_pair(opOCstopdance,&ObjectController::_handlestopdance));
	mCommandMap.insert(std::make_pair(opOCstartmusic,&ObjectController::_handlestartmusic));
	mCommandMap.insert(std::make_pair(opOCstopmusic,&ObjectController::_handlestopmusic));
	mCommandMap.insert(std::make_pair(opOCflourish,&ObjectController::_handleflourish));
	mCommandMap.insert(std::make_pair(opOCwatch,&ObjectController::_handlewatch));
	mCommandMap.insert(std::make_pair(opOClisten,&ObjectController::_handlelisten));
	mCommandMap.insert(std::make_pair(opOCstopwatching,&ObjectController::_handlestopwatching));
	mCommandMap.insert(std::make_pair(opOCstopwatching,&ObjectController::_handlestoplistening));
	mCommandMap.insert(std::make_pair(opOCPauseMusic,&ObjectController::_handlePauseMusic));
	mCommandMap.insert(std::make_pair(opOCPauseDance,&ObjectController::_handlePauseDance));
	mCommandMap.insert(std::make_pair(opOCChangeMusic,&ObjectController::_handleChangeMusic));
	mCommandMap.insert(std::make_pair(opOCChangeDance,&ObjectController::_handleChangeDance));
	mCommandMap.insert(std::make_pair(opOCDenyService,&ObjectController::_handleDenyService));
	mCommandMap.insert(std::make_pair(opOCStartBand,&ObjectController::_handleStartBand));
	mCommandMap.insert(std::make_pair(opOCStopBand,&ObjectController::_handleStopBand));
	mCommandMap.insert(std::make_pair(opOCBandFlourish,&ObjectController::_handleBandFlourish));
	mCommandMap.insert(std::make_pair(opOCImageDesign,&ObjectController::_handleImageDesign));
	mCommandMap.insert(std::make_pair(opOCStatMigration,&ObjectController::_handleStatMigration));
	mCommandMap.insert(std::make_pair(opOCRequestStatMigrationData,&ObjectController::_handleRequestStatMigrationData));
	mCommandMap.insert(std::make_pair(opOCHoloEmote,&ObjectController::_handlePlayHoloEmote));
	mCommandMap.insert(std::make_pair(opOCDazzle,&ObjectController::_handleDazzle));
	mCommandMap.insert(std::make_pair(opOCFireJet,&ObjectController::_handleFireJet));
	mCommandMap.insert(std::make_pair(opOCDistract,&ObjectController::_handleDistract));
	mCommandMap.insert(std::make_pair(opOCColorLights,&ObjectController::_handleColorLights));
	mCommandMap.insert(std::make_pair(opOCSmokeBomb,&ObjectController::_handleSmokeBomb));
	mCommandMap.insert(std::make_pair(opOCSpotLight,&ObjectController::_handleSpotLight));
	mCommandMap.insert(std::make_pair(opOCVentriloquism,&ObjectController::_handleVentriloquism));

	// scout
	mCommandMap.insert(std::make_pair(opOCharvestcorpse,&ObjectController::_handleHarvestCorpse));
	mCommandMap.insert(std::make_pair(opOCmaskscent,&ObjectController::_handleMaskScent));
	mCommandMap.insert(std::make_pair(opOCforage,&ObjectController::_handleForage));
	mCommandMap.insert(std::make_pair(opOCthrowtrap, &ObjectController::_handleThrowTrap));

	// medic
	mCommandMap.insert(std::make_pair(opOCdiagnose,&ObjectController::_handleDiagnose));
	mCommandMap.insert(std::make_pair(opOChealdamage,&ObjectController::_handleHealDamage));
	mCommandMap.insert(std::make_pair(opOChealwound,&ObjectController::_handleHealWound));
	mCommandMap.insert(std::make_pair(opOCmedicalforage,&ObjectController::_handleMedicalForage));
	mCommandMap.insert(std::make_pair(opOCtenddamage,&ObjectController::_handleTendDamage));
	mCommandMap.insert(std::make_pair(opOCtendwound,&ObjectController::_handleTendWound));
	mCommandMap.insert(std::make_pair(opOCfirstaid,&ObjectController::_handleFirstAid));
	mCommandMap.insert(std::make_pair(opOCquickheal,&ObjectController::_handleQuickHeal));
	mCommandMap.insert(std::make_pair(opOCdragincapacitatedplayer,&ObjectController::_handleDragIncapacitatedPlayer));

	// bio - engineer
	mCommandMap.insert(std::make_pair(opOCsampledna,&ObjectController::_handleSampleDNA));

	// combat medic
	mCommandMap.insert(std::make_pair(opOCapplypoison,&ObjectController::_handleApplyPoison));
	mCommandMap.insert(std::make_pair(opOCapplydisease,&ObjectController::_handleApplyDisease));
	mCommandMap.insert(std::make_pair(opOChealmind,&ObjectController::_handleHealMind));

	// doctor
	mCommandMap.insert(std::make_pair(opOChealstate,&ObjectController::_handleHealState));
	mCommandMap.insert(std::make_pair(opOCcurepoison,&ObjectController::_handleCurePoison));
	mCommandMap.insert(std::make_pair(opOCcuredisease,&ObjectController::_handleCureDisease));
	mCommandMap.insert(std::make_pair(opOChealenhance,&ObjectController::_handleHealEnhance));
	mCommandMap.insert(std::make_pair(opOCextinguishfire,&ObjectController::_handleExtinguishFire));
	mCommandMap.insert(std::make_pair(opOCreviveplayer,&ObjectController::_handleRevivePlayer));

	// ranger
	mCommandMap.insert(std::make_pair(opOCareatrack,&ObjectController::_handleAreaTrack));
	mCommandMap.insert(std::make_pair(opOCconceal,&ObjectController::_handleConceal));
	mCommandMap.insert(std::make_pair(opOCrescue,&ObjectController::_handleRescue));

	// smuggler
	mCommandMap.insert(std::make_pair(opOCfeigndeath,&ObjectController::_handleFeignDeath));

	// squad leader
	mCommandMap.insert(std::make_pair(opOCsysgroup,&ObjectController::_handleSysGroup));
	mCommandMap.insert(std::make_pair(opOCsteadyaim,&ObjectController::_handleSteadyAim));
	mCommandMap.insert(std::make_pair(opOCvolleyfire,&ObjectController::_handleVolleyFire));
	mCommandMap.insert(std::make_pair(opOCformup,&ObjectController::_handleFormup));
	mCommandMap.insert(std::make_pair(opOCboostmorale,&ObjectController::_handleBoostMorale));
	mCommandMap.insert(std::make_pair(opOCrally,&ObjectController::_handleRally));
	mCommandMap.insert(std::make_pair(opOCretreat,&ObjectController::_handleRetreat));

	// teras kasi
	mCommandMap.insert(std::make_pair(opOCmeditate,&ObjectController::_handleMeditate));
	mCommandMap.insert(std::make_pair(opOCpowerboost,&ObjectController::_handlePowerBoost));
	mCommandMap.insert(std::make_pair(opOCforceofwill,&ObjectController::_handleForceOfWill));

	// force defense
	mCommandMap.insert(std::make_pair(opOCavoidincapacitation,&ObjectController::_handleAvoidIncapacitation));

	// force enhancement
	mCommandMap.insert(std::make_pair(opOCforceabsorb1,&ObjectController::_handleForceAbsorb1));
	mCommandMap.insert(std::make_pair(opOCforceabsorb2,&ObjectController::_handleForceAbsorb2));
	mCommandMap.insert(std::make_pair(opOCforcespeed1,&ObjectController::_handleForceSpeed1));
	mCommandMap.insert(std::make_pair(opOCforcespeed2,&ObjectController::_handleForceSpeed2));
	mCommandMap.insert(std::make_pair(opOCforcerun1,&ObjectController::_handleForceRun1));
	mCommandMap.insert(std::make_pair(opOCforcerun2,&ObjectController::_handleForceRun2));
	mCommandMap.insert(std::make_pair(opOCforcerun3,&ObjectController::_handleForceRun3));
	mCommandMap.insert(std::make_pair(opOCforcefeedback1,&ObjectController::_handleForceFeedback1));
	mCommandMap.insert(std::make_pair(opOCforcefeedback2,&ObjectController::_handleForceFeedback2));
	mCommandMap.insert(std::make_pair(opOCforcearmor1,&ObjectController::_handleForceArmor1));
	mCommandMap.insert(std::make_pair(opOCforcearmor2,&ObjectController::_handleForceArmor1));
	mCommandMap.insert(std::make_pair(opOCforceresistbleeding,&ObjectController::_handleForceResistBleeding));
	mCommandMap.insert(std::make_pair(opOCforceresistdisease,&ObjectController::_handleForceResistDisease));
	mCommandMap.insert(std::make_pair(opOCforceresistpoison,&ObjectController::_handleForceResistPoison));
	mCommandMap.insert(std::make_pair(opOCforceresiststates,&ObjectController::_handleForceResistStates));
	mCommandMap.insert(std::make_pair(opOCtransferforce,&ObjectController::_handleTransferForce));
	mCommandMap.insert(std::make_pair(opOCchannelforce,&ObjectController::_handleChannelForce));
	mCommandMap.insert(std::make_pair(opOCdrainforce,&ObjectController::_handleDrainForce));
	mCommandMap.insert(std::make_pair(opOCforceshield1,&ObjectController::_handleForceShield1));
	mCommandMap.insert(std::make_pair(opOCforceshield2,&ObjectController::_handleForceShield2));
	mCommandMap.insert(std::make_pair(opOCforcemeditate,&ObjectController::_handleForceMeditate));
	mCommandMap.insert(std::make_pair(opOCregainconsciousness,&ObjectController::_handleRegainConsciousness));

	// force healing
	mCommandMap.insert(std::make_pair(opOChealallself1,&ObjectController::_handleHealAllSelf1));
	mCommandMap.insert(std::make_pair(opOChealallself2,&ObjectController::_handleHealAllSelf2));
	mCommandMap.insert(std::make_pair(opOChealhealthself1,&ObjectController::_handleHealHealthSelf1));
	mCommandMap.insert(std::make_pair(opOChealhealthself2,&ObjectController::_handleHealHealthSelf2));
	mCommandMap.insert(std::make_pair(opOChealactionself1,&ObjectController::_handleHealActionSelf1));
	mCommandMap.insert(std::make_pair(opOChealactionself2,&ObjectController::_handleHealActionSelf2));
	mCommandMap.insert(std::make_pair(opOChealmindself1,&ObjectController::_handleHealMindSelf1));
	mCommandMap.insert(std::make_pair(opOChealmindself2,&ObjectController::_handleHealMindSelf2));
	mCommandMap.insert(std::make_pair(opOChealactionwoundself1,&ObjectController::_handleHealActionWoundSelf1));
	mCommandMap.insert(std::make_pair(opOChealactionwoundself2,&ObjectController::_handleHealActionWoundSelf2));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundself1,&ObjectController::_handleHealHealthWoundSelf1));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundself2,&ObjectController::_handleHealHealthWoundSelf2));
	mCommandMap.insert(std::make_pair(opOChealbattlefatigueself1,&ObjectController::_handleHealBattleFatigueSelf1));
	mCommandMap.insert(std::make_pair(opOChealbattlefatigueself2,&ObjectController::_handleHealBattleFatigueSelf2));
	mCommandMap.insert(std::make_pair(opOChealmindwoundself1,&ObjectController::_handleHealMindWoundSelf1));
	mCommandMap.insert(std::make_pair(opOChealmindwoundself2,&ObjectController::_handleHealMindWoundSelf2));
	mCommandMap.insert(std::make_pair(opOChealactionwoundother1,&ObjectController::_handleHealActionWoundOther1));
	mCommandMap.insert(std::make_pair(opOChealactionwoundother2,&ObjectController::_handleHealActionWoundOther2));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundother1,&ObjectController::_handleHealHealthWoundOther1));
	mCommandMap.insert(std::make_pair(opOChealhealthwoundother2,&ObjectController::_handleHealHealthWoundOther2));
	mCommandMap.insert(std::make_pair(opOChealmindwoundother1,&ObjectController::_handleHealMindWoundOther1));
	mCommandMap.insert(std::make_pair(opOChealmindwoundother2,&ObjectController::_handleHealMindWoundOther2));
	mCommandMap.insert(std::make_pair(opOChealallother1,&ObjectController::_handleHealAllOther1));
	mCommandMap.insert(std::make_pair(opOChealallother2,&ObjectController::_handleHealAllOther2));
	mCommandMap.insert(std::make_pair(opOChealstatesother,&ObjectController::_handleHealStatesOther));
	mCommandMap.insert(std::make_pair(opOCstopbleeding,&ObjectController::_handleStopBleeding));
	mCommandMap.insert(std::make_pair(opOCforcecuredisease,&ObjectController::_handleForceCureDisease));
	mCommandMap.insert(std::make_pair(opOCforcecurepoison,&ObjectController::_handleForceCurePoison));
	mCommandMap.insert(std::make_pair(opOChealstatesself,&ObjectController::_handleHealStatesSelf));
	mCommandMap.insert(std::make_pair(opOCtotalhealother,&ObjectController::_handleTotalHealOther));
	mCommandMap.insert(std::make_pair(opOCtotalhealself,&ObjectController::_handleTotalHealSelf));

	// force powers
	mCommandMap.insert(std::make_pair(opOCanimalscare,&ObjectController::_handleAnimalScare));
	mCommandMap.insert(std::make_pair(opOCforcelightningsingle1,&ObjectController::_handleForceLightningSingle1));
	mCommandMap.insert(std::make_pair(opOCforcelightningsingle2,&ObjectController::_handleForceLightningSingle2));
	mCommandMap.insert(std::make_pair(opOCforcelightningcone1,&ObjectController::_handleForceLightningCone1));
	mCommandMap.insert(std::make_pair(opOCforcelightningcone2,&ObjectController::_handleForceLightningCone2));
	mCommandMap.insert(std::make_pair(opOCmindblast1,&ObjectController::_handleMindblast1));
	mCommandMap.insert(std::make_pair(opOCmindblast2,&ObjectController::_handleMindblast2));
	mCommandMap.insert(std::make_pair(opOCanimalcalm,&ObjectController::_handleAnimalCalm));
	mCommandMap.insert(std::make_pair(opOCanimalattack,&ObjectController::_handleAnimalAttack));
	mCommandMap.insert(std::make_pair(opOCforceweaken1,&ObjectController::_handleForceWeaken1));
	mCommandMap.insert(std::make_pair(opOCforceweaken2,&ObjectController::_handleForceWeaken2));
	mCommandMap.insert(std::make_pair(opOCforceintimidate1,&ObjectController::_handleForceIntimidate1));
	mCommandMap.insert(std::make_pair(opOCforceintimidate2,&ObjectController::_handleForceIntimidate2));
	mCommandMap.insert(std::make_pair(opOCforcethrow1,&ObjectController::_handleForceThrow1));
	mCommandMap.insert(std::make_pair(opOCforcethrow2,&ObjectController::_handleForceThrow2));
	mCommandMap.insert(std::make_pair(opOCforceknockdown1,&ObjectController::_handleForceKnockdown1));
	mCommandMap.insert(std::make_pair(opOCforceknockdown2,&ObjectController::_handleForceKnockdown2));
	mCommandMap.insert(std::make_pair(opOCforceknockdown3,&ObjectController::_handleForceKnockdown3));
	mCommandMap.insert(std::make_pair(opOCforcechoke,&ObjectController::_handleForceChoke));
	mCommandMap.insert(std::make_pair(opOCjedimindtrick,&ObjectController::_handleJediMindTrick));


	// groups
	mCommandMap.insert(std::make_pair(opOCinvite,&ObjectController::_handleInvite));
	mCommandMap.insert(std::make_pair(opOCuninvite,&ObjectController::_handleUninvite));
	mCommandMap.insert(std::make_pair(opOCjoin,&ObjectController::_handleJoin));
	mCommandMap.insert(std::make_pair(opOCdecline,&ObjectController::_handleDecline));
	mCommandMap.insert(std::make_pair(opOCdisband,&ObjectController::_handleDisband));
	mCommandMap.insert(std::make_pair(opOCleavegroup,&ObjectController::_handleLeaveGroup));
	mCommandMap.insert(std::make_pair(opOCmakeleader,&ObjectController::_handleMakeLeader));
	mCommandMap.insert(std::make_pair(opOCdismissgroupmember,&ObjectController::_handleDismissGroupMember));
	mCommandMap.insert(std::make_pair(opOCgroupchat,&ObjectController::_handleGroupChat));

	mCommandMap.insert(std::make_pair(opOCg,&ObjectController::_handleGroupChat));
	mCommandMap.insert(std::make_pair(opOCgc,&ObjectController::_handleGroupChat));
	mCommandMap.insert(std::make_pair(opOCgsay,&ObjectController::_handleGroupChat));
	mCommandMap.insert(std::make_pair(opOCgtell,&ObjectController::_handleGroupChat));
	mCommandMap.insert(std::make_pair(opOCgroupsay,&ObjectController::_handleGroupChat));

	mCommandMap.insert(std::make_pair(opOCgrouploot,&ObjectController::_handleGroupLootMode));
	mCommandMap.insert(std::make_pair(opOCmakemasterlooter,&ObjectController::_handleMakeMasterLooter));

	// custom
	mCommandMap.insert(std::make_pair(opOCEndBurstRun,&ObjectController::_endBurstRun));

	// admin profession
	mCommandMap.insert(std::make_pair(opOCAdminSysMsg,&ObjectController::_handleAdminSysMsg));
	mCommandMap.insert(std::make_pair(opOCAdminWarpSelf,&ObjectController::_handleAdminWarpSelf));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcast,&ObjectController::_handleBroadcast));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcastPlanet,&ObjectController::_handleBroadcastPlanet));
	mCommandMap.insert(std::make_pair(opOCAdminBroadcastGalaxy,&ObjectController::_handleBroadcastGalaxy));
	mCommandMap.insert(std::make_pair(opOCAdminShutdownGalaxy,&ObjectController::_handleShutdownGalaxy));
	mCommandMap.insert(std::make_pair(opOCAdminCancelShutdownGalaxy,&ObjectController::_handleCancelShutdownGalaxy));

	//Structures
	mCommandMap.insert(std::make_pair(opOCPlaceStructure,&ObjectController::_handleStructurePlacement));
	mCommandMap.insert(std::make_pair(opPermissionListModify,&ObjectController::_handleModifyPermissionList));
	mCommandMap.insert(std::make_pair(opTransferStructure,&ObjectController::_handleTransferStructure));
	mCommandMap.insert(std::make_pair(opNameStructure,&ObjectController::_handleNameStructure));
	mCommandMap.insert(std::make_pair(opHarvesterGetResourceData,&ObjectController::_handleHarvesterGetResourceData));
	mCommandMap.insert(std::make_pair(opHarvesterSelectResource,&ObjectController::_handleHarvesterSelectResource));
	mCommandMap.insert(std::make_pair(opHarvesterActivate,&ObjectController::_handleHarvesterActivate));
	mCommandMap.insert(std::make_pair(opHarvesterDeActivate,&ObjectController::_handleHarvesterDeActivate));
	mCommandMap.insert(std::make_pair(opDiscardHopper,&ObjectController::_handleDiscardHopper));

	mCommandMap.insert(std::make_pair(opItemRotation,&ObjectController::_handleItemRotation));

	mCommandMap.insert(std::make_pair(opItemRotationRight90,&ObjectController::_handleItemRotationRight90));
	mCommandMap.insert(std::make_pair(opItemRotationLeft90,&ObjectController::_handleItemRotationLeft90));

	
	mCommandMap.insert(std::make_pair(opItemMoveUp,&ObjectController::_handleItemMoveUp));

	mCommandMap.insert(std::make_pair(opItemMoveForward,&ObjectController::_handleItemMoveForward));
	mCommandMap.insert(std::make_pair(opItemMoveBack,&ObjectController::_handleItemMoveBack));
	mCommandMap.insert(std::make_pair(opItemMoveDown,&ObjectController::_handleItemMoveDown));
	
	
	

	
}

//======================================================================================================================

void ObjectControllerCommandMap::ScriptRegisterEvent(void* script,std::string eventFunction)
{
	mCmdScriptListener.registerScript(reinterpret_cast<Script*>(script),(int8*)eventFunction.c_str());
}

//======================================================================================================================


