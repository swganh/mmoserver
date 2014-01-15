// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <string>
#include "anh/byte_buffer.h"

#include "MessageLib/messages/obj_controller_message.h"

namespace swganh
{
namespace messages
{
namespace controllers
{


enum RadialIdentifier :
uint8_t
{
    Unknown					                        = 0,
    combatTarget			                        = 1,
    combatUntarget			                        = 2,
    combatAttack			                        = 3,
    combatPeace				                        = 4,
    combatDuel				                        = 5,
    combatDeathBlow			                        = 6,
    examine					                        = 7,
    tradeStart				                        = 8,
    tradeAccept				                        = 9,
    itemPickup				                        = 10,
    itemEquip				                        = 11,
    itemUnequip				                        = 12,
    itemDrop				                        = 13,
    itemDestroy				                        = 14,
    itemToken				                        = 15,
    itemOpen				                        = 16,
    itemOpenNewWindow		                        = 17,
    itemActivate			                        = 18,
    itemDeactivate			                        = 19,
    itemUse					                        = 20,
    itemUseSelf				                        = 21,
    itemUseOther			                        = 22,
    itemSit					                        = 23,
    itemMail				                        = 24,
    converseStart			                        = 25,
    converseRespond			                        = 26,
    converseResponse		                        = 27,
    converseStop			                        = 28,
    craftOptions			                        = 29,
    craftStart				                        = 30,
    craftHopperInput		                        = 31,
    craftHopperOutput		                        = 32,
    missionTerminalList		                        = 33,
    missionDetails			                        = 34,
    loot					                        = 35,
    lootAll					                        = 36,
    groupInvite				                        = 37,
    groupJoin				                        = 38,
    groupLeave				                        = 39,
    groupKick				                        = 40,
    groupDisband			                        = 41,
    groupDecline			                        = 42,
    extractObject			                        = 43,
    petCall					                        = 44,
    terminalAuctionUse		                        = 45,
    creatureFollow			                        = 46,
    creatureStopFollow		                        = 47,
    split					                        = 48,
    imagedesign				                        = 49,
    setName					                        = 50,
    itemRotate				                        = 51,
    itemRotateRight			                        = 52,
    itemRotateLeft			                        = 53,
    itemMove				                        = 54,
    itemMoveForward			                        = 55,
    ItemMoveBack			                        = 56,
    itemMoveUp				                        = 57,
    itemMoveDown			                        = 58,
    petStore				                        = 59,
    vehicleGenerate			                        = 60,
    vehicleStore			                        = 61,
    missionAbort			                        = 62,
    missionEndDuty			                        = 63,
    shipManageComponents	                        = 64,
    waypointAutopilot		                        = 65,
    programDroid			                        = 66,
    serverDivider			                        = 67,
    serverMenu1				                        = 68,
    serverMenu2				                        = 69,
    serverMenu3				                        = 70,
    serverMenu4				                        = 71,
    serverMenu5				                        = 72,
    serverMenu6				                        = 73,
    serverMenu7				                        = 74,
    serverMenu8				                        = 75,
    serverMenu9				                        = 76,
    serverMenu10			                        = 77,
    serverHarvesterManage	                        = 78,
    serverHouseManage		                        = 79,
    serverFactionHallManage	                        = 80,
    serverHue				                        = 81,
    serverObserve			                        = 82,
    serverStopObserving		                        = 83,
    serverTravelOptions		                        = 84,
    serverBazaarOptions		                        = 85,
    serverShippingOptions	                        = 86,
    serverHealWound			                        = 87,
    serverHealWoundHealth	                        = 88,
    serverHealWoundAction	                        = 89,
    serverHealWoundStrength	                        = 90,
    serverHealWoundConst	                        = 91,
    serverHealWoundQuickness                        = 92,
    serverHealWoundStamina	                        = 93,
    serverHealDamage		                        = 94,
    serverHealState			                        = 95,
    serverHealStateStunned	                        = 96,
    serverHealStateBlinded	                        = 97,
    serverHealStateDizzy	                        = 98,
    serverHealStateIntim	                        = 99,
    serverHealEnhance		                        = 100,
    serverHealEnhanceHealth	                        = 101,
    serverHealEnhanceAction	                        = 102,
    serverHealEnhanceStrangth                       = 103,
    serverHealEnhanceConst	                        = 104,
    serverHealEnhanceQuickness                      = 105,
    serverHealEnhanceStamina	                    = 106,
    serverHealFirstAid		                        = 107,
    serverHealCurePoison		                    = 108,
    serverHealCureDisease		                    = 109,
    serverHealApplyPoison		                    = 110,
    serverHealApplyDisease	                        = 111,
    serverHarvestCorpse		                        = 112,
    serverPerformanceListen	                        = 113,
    serverPerformanceWatch	                        = 114,
    serverPerformanceListenStop	                    = 115,
    serverPerformanceWatchStop	                    = 116,
    serverTerminalPermissions	                    = 117,
    serverTerminalManagement	                    = 118,
    serverTerminalPermissionsEnter	                = 119,
    serverTerminalPermissionsBanned	                = 120,
    serverTerminalPermissionsAdmin	                = 121,
    serverTerminalPermissionVendor	                = 122,
    serverTerminalPermissionsHopper	                = 123,
    serverTerminalManagementStatus	                = 124,
    serverTerminalManagementPrivacy	                = 125,
    serverTerminalManagementTransfer	            = 126,
    serverTerminalManagementResidence	            = 127,
    serverTerminalManagementDestroy	                = 128,
    serverTerminalManagementPay		                = 129,
    serverTerminalCreateVendor		                = 130,
    serverGiveVendorMaintenance		                = 131,
    serverItemOptions					            = 132,
    serverSurveyToolRange				            = 133,
    serverSurveyToolResolution		                = 134,
    serverSurveyToolClass				            = 135,
    serverProbeDroidTrackTarget		                = 136,
    serverProbeDroidFindTarget		                = 137,
    serverProbeDroidActivate		                = 138,
    serverProbeDroidBuy				                = 139,
    serverTeach						                = 140,
    petCommand						                = 141,
    petFollow						                = 142,
    petStay							                = 143,
    petGuard							            = 144,
    petFriend							            = 145,
    petAttack							            = 146,
    petPatrol							            = 147,
    petGetPatrolPoint					            = 148,
    petClearPatrolPoint		 		                = 149,
    petAssumeFormation1				                = 150,
    petAssumeFormation2				                = 151,
    petTransfer						                = 152,
    petRelease						                = 153,
    petTrick1							            = 154,
    petTrick2							            = 155,
    petTrick3							            = 156,
    petTrick4							            = 157,
    petGroup							            = 158,
    petTame							                = 159,
    petFeed							                = 160,
    petSpecialAttackOne				                = 161,
    petSpecialAttackTwo				                = 162,
    petRangedAttack					                = 163,
    diceRoll							            = 164, //Reused for HarvestMeat
    diceTwoFace						                = 165, //Reused for HarvestHide
    diceThreeFace						            = 166, //Reused for HarvestBone
    diceFourFace						            = 167, //Reused for Milk Me
    diceFiveFace						            = 168,
    diceSixFace						                = 169,
    diceSevenFace						            = 170,
    diceEightFace						            = 171,
    diceCountOne						            = 172,
    diceCountTwo						            = 173,
    diceCountThree					                = 174,
    diceCountFour						            = 175,
    createBallot						            = 176,
    vote								            = 177,
    bombingRun						                = 178,
    selfDestruct						            = 179,
    thirtySec							            = 180,
    fifteenSec						                = 181,
    serverCampDisband				                = 182,
    serverCampAssumeOwnership		                = 183,
    serverProbeDroidProram			                = 184,
    serverGuildCreate				                = 185,
    serverGuildInfo					                = 186,
    serverGuildMembers				                = 187,
    serverGuildSponsored			                = 188,
    serverGuildEnemies				                = 189,
    serverGuildSponsor				                = 190,
    serverGuildDisband				                = 191,
    serverGuildNameChange			                = 192,
    serverGuildGuildManagement		                = 193,
    serverGuildMemberManagement		                = 194,
    serverManfHopperInput			                = 195,
    serverManfHopperOutput			                = 196,
    serverManfStationSchematic		                = 197,
    elevatorUp						                = 198,
    elevatorDown					                = 199,
    serverPetOpen					                = 200,
    serverPetDpad					                = 201,
    serverMedToolDiagnose			                = 202,
    serverMedToolTendWound			                = 203,
    serverMedToolTendDamage			                = 204,
    serverPetMount					                = 205,
    serverPetDismount				                = 206,
    serverPetTrainMount				                = 207,
    serverVehicleEnter				                = 208,
    serverVehicleExit				                = 209,
    openNaviCompDpad				                = 210,
    initNavicompDpad				                = 211,
    cityStatus						                = 212,
    cityCitizens					                = 213,
    cityStructures					                = 214,
    cityTreasury					                = 215,
    cityManagement					                = 216,
    cityName						                = 217,
    cityMilitia						                = 218,
    cityTaxes						                = 219,
    cityTreasuryDeposit				                = 220,
    cityTreasuryWithdraw			                = 221,
    cityRegister					                = 222,
    cityRank						                = 223,
    cityAdmin1						                = 224,
    cityAdmin2						                = 225,
    cityAdmin3						                = 226,
    cityAdmin4						                = 227,
    cityAdmin5						                = 228,
    cityAdmin6						                = 229,
    memoryChipProgram				                = 230,
    memoryChipTransfer				                = 231,
    memoryChipAnalyze				                = 232,
    equipDroidOnChip				                = 233,
    bankJoin						                = 234,
    bankQuit						                = 235,
    bankDepositAll					                = 236,
    bankWithdrawAll					                = 237,
    bankTransfer					                = 238,
    bankItems						                = 239,
    fireworkshowAdd					                = 240,
    fireworkshowRemove				                = 241,
    fireworkshowModify				                = 242,
    fireworkshowReorder				                = 243,
    fireworkshowData				                = 244,
    operateHarvester				                = 245,
    payMaintenance					                = 246,
    depositPower					                = 247,
    StructureStatus					                = 248,
    StructureOptions				                = 249,
    StartManufacture				                = 250,
    ListIngredients					                = 251,
    StopManufacture					                = 252
};

struct RadialOptions
{
    RadialOptions()
        : parent_item(0)
        , identifier(Unknown)
        , action(1)
        , custom_description(L"")
    {}
    RadialOptions(uint8_t parent, uint8_t identifier_, uint8_t action_, std::wstring description)
        : parent_item(parent)
        , identifier((RadialIdentifier)identifier_)
        , action(action_)
        , custom_description(description) {}

    bool operator==(const RadialOptions& other)
    {
        return identifier == other.identifier;
    }
    bool operator<(const RadialOptions& other)
    {
        return identifier < other.identifier;
    }

    uint8_t parent_item;
    RadialIdentifier identifier;
    uint8_t action;
    std::wstring custom_description; // only used if action is 3
};


}
}
}  // namespace swganh::messages::controllers
