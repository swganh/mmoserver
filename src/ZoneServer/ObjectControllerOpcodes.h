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

#ifndef ANH_ZONESEVER_OBJECTCONTROLLEROPCODES_H
#define ANH_ZONESEVER_OBJECTCONTROLLEROPCODES_H


enum object_controller_opcodes
{
    opDataTransform						= 0x00000071,
    opOC0x83    						= 0x00000083,
    opCombatAction						= 0x000000CC,
    opStartNpcConversation				= 0x000000DD,
    opStopNpcConversation				= 0x000000DE,
    opNpcConversationMessage			= 0x000000DF,
    opNpcConversationOptions			= 0x000000E0,
    opHarvesterResourceData				= 0x000000EA,
    opResourceEmptyHopper				= 0x000000ED,
    opResourceEmptyHopperResponse		= 0x000000EE,
    opDataTransformWithParent			= 0x000000F1,
    opSendAnimation						= 0x000000F2,
    opSpatialChat						= 0x000000F4,
    opMissionTerminalOpen               = 0x000000F5,
    opMissionDetailsRequest             = 0x000000F7,
    opGenericMissionRequest				= 0x000000F9,
    opMissionGenericResponse			= 0x000000FA,
    opMissionCreateRequest              = 0x000000FF,
    opDraftSchematics					= 0x00000102,
    opManufactureSlots					= 0x00000103,
    opCraftExperiment					= 0x00000106,
    opCraftFillSlot						= 0x00000107,
    opCraftEmptySlot					= 0x00000108,
    opCreatePrototypeResponse			= 0x0000010A,
    opCraftAcknowledge					= 0x0000010C,
    opCraftCancelResponse				= 0x0000010F,
    opCraftExperimentResponse			= 0x00000113,
    opOCSecureTrade						= 0x00000115,
    opCommandQueueEnqueue				= 0x00000116,
    opCommandQueueRemove				= 0x00000117,
    opOCCurrentTarget					= 0x00000126,
    opSpatialEmote						= 0x0000012E,
    opPosture							= 0x00000131,
    opCombatSpam						= 0x00000134,
    opSitOnObject						= 0x0000013B,
    opTeleportAck						= 0x0000013f,
    opMissionAbort						= 0x00000142,
    opObjectMenuRequest					= 0x00000146,
    opObjectMenuResponse				= 0x00000147,
    opCraftCustomization				= 0x0000015A,
    opEntertainerFlourish				= 0x00000166,
    opShowFlyText						= 0x000001BD,
    opGenericIntResponse				= 0x000001BE,
    opDraftSlotsQueryResponse			= 0x000001BF,
    opSharedNetworkMessage				= 0x000001C2,
    opBiographyUpdate					= 0x000001DB,
    opPlayersNearYou					= 0x000001E7,
    opStartingLocationList				= 0x000001FC,
    opResourceWeights					= 0x00000207,
    opImageDesignStartMessage			= 0x0000023a,
    opImageDesignChangeMessage			= 0x00000238,
    opImageDesignStopMessage			= 0x00000239,
    opApplyBuff							= 0x00000229,
    opRemoveBuff						= 0x0000022A,
    opMissionComplete					= 0x000004C5 //Actual name unknown

};

//=======================================================================

enum object_controller_brawler_functions
{
    opOCberserk1						= 0xE212D37,
    opOCunarmedlunge1					= 0x38bddd4,
    opOCcenterofbeing					= 0x5997ac66,
    opOCintimidate1						= 0x5619e044,
    opOCmelee1hlunge1					= 0x4dc1571,
    opOCmelee2hlunge1					= 0xe83e702f,
    opOCpolearmlunge1					= 0x7fd6584d,
    opOCtaunt							= 0x52342d60,
    opOCwarcry1							= 0x6a56ecc3,
    opOCunarmedhit1						= 0x3124e0dd,
    opOCunarmedstun1					= 0xcf15393d,
    opOCunarmedblind1					= 0xca8b008b,
    opOCunarmedspinattack1				= 0xdb447550,
    opOCunarmedlunge2					= 0xec8fb0d,
    opOCmelee1hhit1						= 0x584e5d11,
    opOCmelee1hbodyhit1					= 0xce565bff,
    opOCmelee1hdizzyhit1				= 0x64e2d561,
    opOCmelee1hspinattack1				= 0xbb88156a,
    opOCmelee2hhit1						= 0x75364a99,
    opOCmelee2hheadhit1					= 0x1934bc13,
    opOCmelee2hsweep1					= 0x49bd34a6,
    opOCmelee2hspinattack1				= 0x86a5f2d2,
    opOCpolearmhit1						= 0x844e3aec,
    opOCpolearmleghit1					= 0xebb64bf6,
    opOCpolearmstun1					= 0x0f34ee28,
    opOCpolearmspinattack1				= 0x63319278,
    opOCberserk2						= 0x03620bee,
    opOCintimidate2						= 0x5b5ac69d,
    opOCmelee1hlunge2					= 0x099f33a8,
    opOCpolearmlunge2					= 0x72957e94,
    opOCmelee2hlunge2					= 0xe57d56f6,
    opOCwarcry2							= 0x6715ca1a
};

//=======================================================================

enum object_controller_artisan_functions
{
    opOCsample							= 0x8d0c1504,
    opOCrequestcoresample				= 0x9223c634,
    opOCsurvey							= 0x19c9fac1,
    opOCrequestsurvey					= 0xc00cfa18


};

//=======================================================================

enum object_controller_entertainer_functions
{
    opOCstartmusic						= 0xddd1e8f1,
    opOCstopmusic						= 0x4A0D52DD,
    opOCstartdance						= 0x7B1DCBE0,
    opOCstopdance						= 0xECC171CC,
    opOCflourish						= 0xC8998CE9,
    opOCwatch							= 0xEC93CA43,
    opOClisten							= 0x5855BB1B,
    opOCstopwatching					= 0x6651AD9A,
    opOCstoplistening					= 0xC2E4D4D0,
    opOCPauseMusic						= 0x52054D1F,
    opOCPauseDance						= 0xF4C96E0E,
    opOCChangeDance						= 0x13EE2D35,
    opOCChangeMusic						= 0xB5220E24,
    opOCDenyService						= 0x180AB134,
    opOCStartBand						= 0xFF455C54,
    opOCStopBand						= 0x69ED1B98,
    opOCBandFlourish					= 0xF4C60EC3,
    opOCImageDesign						= 0xdfc959ba,
    opOCStatMigration					= 0x7afca539,
    opOCRequestStatMigrationData		= 0xE7407732,
    opOCHoloEmote						= 0xE8C45452,
    opOCDazzle							= 0x9c7713a5,
    opOCFireJet							= 0x35ed32be,
    opOCDistract						= 0x2434ac3a,
    opOCColorLights						= 0xB008CBFA,
    opOCSmokeBomb						= 0xD536B419,
    opOCSpotLight						= 0xed4aa746,
    opOCVentriloquism					= 0x6cb6978f
};

//=======================================================================

enum object_controller_scout_functions
{
    opOCharvestcorpse					= 0x029D0CC5,
    opOCmaskscent						= 0xb0f4c4b6,
    opOCforage							= 0x494f9f80,
    opOCthrowtrap						= 0x0549BE67

};

//=======================================================================

enum object_controller_marksman_functions
{
    opOCoverchargeshot1					= 0x795d8310,
    opOCoverchargeshot2					= 0x741ea5c9,
    opOCpointblankarea1					= 0xfdcc0480,
    opOCpointblanksingle1				= 0x5842294e,
    opOCheadshot1						= 0x80f6ab48,
    opOCheadshot2						= 0x8db58d91,
    opOCmindshot1						= 0x922143e8,
    opOCbodyshot1						= 0xe3f9c921,
    opOCbodyshot2						= 0xeebaeff8,
    opOChealthshot1						= 0x6c4bae17,
    opOClegshot1						= 0x27308ffd,
    opOClegshot2						= 0x2a73a924,
    opOCfullautosingle1					= 0xbae4f3db,
    opOCactionshot1						= 0x484536a1,
    opOCthreatenshot					= 0xee14ca1e,
    opOCwarningshot						= 0xf44972f6,
    opOCsuppressionfire1				= 0xf3f3d495,
    opOCtumbletokneeling				= 0x49346510,
    opOCtumbletoprone					= 0x0b30c9b5,
    opOCtumbletostanding				= 0x54314196,
    opOCtakecover						= 0x3903080b,
    opOCdiveshot						= 0x857fcdd0,
    opOCkipupshot						= 0x5394d72d,
    opOCrollshot						= 0x03250b4d,
    opOCaim								= 0xa0a107f8
};

//=======================================================================

enum object_controller_medic_functions
{
    opOCdiagnose						= 0xDC7CF134,
    opOChealdamage						= 0x0A9F00A0,
    opOChealwound						= 0x2087CE04,
    opOCmedicalforage					= 0xC6132B18,
    opOCtenddamage						= 0x18CD8967,
    opOCtendwound						= 0x31DD4C4B,
    opOCfirstaid						= 0xD5F85133,
    opOCquickheal						= 0x7AB1850D,
    opOCdragincapacitatedplayer			= 0x273A06DA
};

//=======================================================================

enum object_controller_admin_functions
{
    opOCAdminSysMsg						= 0xa0cb8ed,
    opOCAdminWarpSelf					= 0x523daa6c,
    opOCAdminBroadcast					= 0x99bd8693,
    opOCAdminBroadcastPlanet			= 0xEB0B6D6B,
    opOCAdminBroadcastGalaxy			= 0x5AD7B5CE,
    opOCAdminShutdownGalaxy				= 0xFC96BB5C,
    opOCAdminCancelShutdownGalaxy		= 0x72790D12
};

//=======================================================================

enum object_controller_group_functions
{
    opOCinvite							= 0x88505D58,
    opOCuninvite						= 0x2E26A47F,
    opOCjoin							= 0xA99E6807,
    opOCdecline							= 0x43E1F84F,
    opOCdisband							= 0x46D22D3A,
    opOCleavegroup						= 0x5061D654,
    opOCdismissgroupmember				= 0x2F50053B,
    opOCmakeleader						= 0x939AD584,

    // GroupChat alias
    opOCg								= 0x315D6D9,
    opOCgc								= 0x8845FDC,
    opOCgsay							= 0x3CDFF0CC,
    opOCgtell							= 0x8A1C105D,
    opOCgroupsay						= 0xF5D6AF9C,
    opOCgroupchat						= 0x68851C4F,

    opOCgrouploot						= 0xe778c14f,
    opOCmakemasterlooter				= 0x9707b934
};

//=======================================================================

enum object_controller_generic_functions
{
    opOCtarget							= 0xD6A8B702,
    opOCattack							= 0xA8FEF90A,
    opOCpeace							= 0x4178FD6A,
    opOCdeathblow						= 0xE5F3B39B,
    opOCloot							= 0xEF3CBEDB,

    opOCLogout							= 0x03B65950,
    opOCLogoutClient					= 0xCBC43B59,
    opOCteach							= 0x5041F83A,
    opOCmatch							= 0xD63286B8,

    opOCspatialchatinternal				= 0x7C8D63D4,
    opOCsocialinternal					= 0x32CF1BEE,
    opOCsetmoodinternal					= 0x7759F35E,
    opOCNPCConversationStart			= 0x04cdafce,
    opOCNPCConversationStop				= 0x97d734fe,
    opOCNPCConversationSelect			= 0x305ede19,
    opOCPurchaseTicket					= 0xbff5be51,
    opOCServerDestroyObject				= 0xe7aec4fb,

    opOCrequestquestimersandcounters	= 0x35C28E6F,

    opOCresourcecontainersplit			= 0x74952854,
    opOCresourcecontainertransfer		= 0xf7262a75,
    opOClfg								= 0x3ad396a5,
    opOCnewbiehelper					= 0x441f4a3e,
    opOCroleplay						= 0x32871193,
    opOCtoggleAwayFromKeyboard			= 0x9b9fe4a8,
    opOCtoggleDisplayingFactionRank		= 0x665c7c03,
    opOCanon							= 0xd40d5142,
    opOCrequestbadges					= 0xca604b86,
    opOCsetcurrentskilltitle			= 0xdb555329,
    opOCsetspokenlanguage				= 0xae2907e4,
    opOCsitserver						= 0xB719FA26,
    opOCstand							= 0xA8A25C79,
    opOCprone							= 0xBD8D02AF,
    opOCkneel							= 0x01B48B26,
    opOCburstrun						= 0xFC3D1CB2,
    opOCTarget							= 0xd6a8b702,
    opOCduel							= 0x468b3802,
    opOCendduel							= 0x34c0add2,

    opOCopencontainer					= 0x70177586,
    opOCclosecontainer					= 0x310A90F6,
    opOCtransferitemarmor				= 0x18726ca1,
    opOCtransferitem					= 0x3cfb449d,
    opOCtransferitemmisc				= 0x82f75977,
    opOCtransferitemweapon				= 0x335676c7,

    opOCgetattributesbatch				= 0x164550EF,
    opOCrequestDraftslotsBatch			= 0x5fd21eb0,
    opOCrequestResourceWeightsBatch		= 0x9a8b385c,
    opOCSynchronizedUIListen			= 0xf9996ab6,
    opOCSynchronizedUIStopListening		= 0x7c158efd,
    opOCRequestCraftingSession			= 0x94ac516,
    opOCCancelCraftingSession			= 0x83250E2A,
    opOCrequestcharactersheetinfo		= 0x887B5461,
    opOCrequestbiography				= 0x1BAD8FFC,
    opOCsetbiography					= 0xfbe911e4,
    opOCeditbiography					= 0xe019d461,
    opOCsurrenderskill					= 0x006a99f1,
    opOCclientqualifiedforskill			= 0x7a48c5f6,
    opOCrequestcharactermatch			= 0x8a19d7e1,
    opOCBoardTransport					= 0x5dcd41a2,
    opOCNewbieSelectStartingLocation	= 0xCF2D30F4,

    opOCExtractObject					= 0xF62E35BA,
    opOCFactoryCrateSplit				= 0x8BBB0B89,

    //structures
    opOCPlaceStructureMode				= 0x07625BAA,
    opOCPlaceStructure					= 0x7AF26B0B,
    opPermissionListModify				= 0x896713f2,
    opTransferStructure					= 0x116BDA06,
    opNameStructure						= 0xc367b461,
    opHarvesterGetResourceData			= 0xF096C059,
    opHarvesterSelectResource			= 0xff549d14,
    opHarvesterActivate					= 0xCE645C94,
    opHarvesterDeActivate				= 0x2C61CB03,
    opDiscardHopper						= 0xC89B9E26,

    opItemMoveUp      = 0xf6a83a09,
    opItemMoveForward	= 0xb8effe04,
    opItemMoveBack		= 0x67ac3903,
    opItemMoveDown		= 0xefe50324,
    opItemRotateRight	= 0x8dfc28ab,
    opItemRotateLeft	= 0x3816ae47,

    opMoveFurniture   = 0x4b74a403,
    opRotateFurniture = 0xa8eb1d48,


    //social
    opOCtip								= 0xc64d8cb0,
    opOCaddfriend						= 0x2a2357ed,
    opOCremovefriend					= 0x8e9091d7,
    opOCaddignore						= 0x929ad345,
    opOCremoveignore					= 0x3629157f,
    opOCfiendfriend						= 0x30be6ee9,

    //crafting
    opOCSelectDraftSchematic			= 0x89242e02,
    opOCnextcraftingstage				= 0x6ad8ed4d,
    opOCcreateprototype					= 0xd61ff415,
    opOCcreatemanfschematic				= 0xf4b66795,

    opOCrequestwaypointatposition		= 0x4982E17B,
    opOCsetwaypointactivestatus			= 0xC3EDA6B6,
    opOCwaypoint						= 0x640543FE,
    opOCsetwaypointname					= 0x398f891a,

    // custom
    opOCEndBurstRun						= 0xc79ecec6,

    opOCmount							= 0xE007BF31,
    opOCdismount						= 0x06F978ED
};

//=======================================================================

enum object_controller_bioengineer_functions
{
    opOCsampledna						= 0xD04917BE
};

//=======================================================================

enum object_controller_bountyhunter_functions
{
    opOCassigndroid						= 0x6B5C8AE3
};

//=======================================================================

enum object_controller_combatmedic_functions
{
    opOCapplypoison						= 0xE08596DB,
    opOCapplydisease					= 0xAF5E4D90,
    opOChealmind						= 0xDFAC57EE
};

//=======================================================================

enum object_controller_carbineer_functions
{
    opOCactionshot2						= 0x45061078,
    opOCfullautosingle2					= 0xB7A7D502,
    opOCscattershot1					= 0xAD866D55,
    opOCwildshot1						= 0x7A48C259,
    opOCscattershot2					= 0xA0C54B8C,
    opOCwildshot2						= 0x770BE480,
    opOClegshot3						= 0x2EB2B493,
    opOCcripplingshot					= 0xE68A8B94,
    opOCburstshot1						= 0x3881333D,
    opOCburstshot2						= 0x35C215E4,
    opOCsuppressionfire2				= 0xFEB0F24C,
    opOCfullautoarea1					= 0x8D1B984E,
    opOCchargeshot1						= 0x109F8ACF,
    opOCfullautoarea2					= 0x8058BE97,
    opOCchargeshot2						= 0x1DDCAC16
};

//=======================================================================

enum object_controller_creaturehandler_functions
{

};

//=======================================================================

enum object_controller_commando_functions
{
    opOCflamesingle1					= 0xE3EF3074,
    opOCflamecone1						= 0xD5ACD94F,
    opOCflamesingle2					= 0xEEAC16AD,
    opOCfireacidsingle1					= 0xEC009F1F,
    opOCfireacidcone1					= 0x3A5427FE,
    opOCfireacidsingle2					= 0xE143B9C6,
    opOCfireacidcone2					= 0x37170127,
    opOCflamecone2						= 0xD8EFFF96
};

//=======================================================================

enum object_controller_doctor_functions
{
    opOChealstate						= 0x4A386BD5,
    opOCcurepoison						= 0x1754A3E5,
    opOCextinguishfire					= 0xDF49EA58,
    opOCcuredisease						= 0xE994DE9C,
    opOChealenhance						= 0xEEE029CF,
    opOCreviveplayer					= 0xC9759876
};

//=======================================================================

enum object_controller_fencer_functions
{
    opOCmelee1hhit2						= 0x550D7BC8,
    opOCmelee1hscatterhit1				= 0x5BB4F3A0,
    opOCmelee1hscatterhit2				= 0x56F7D579,
    opOCmelee1hdizzyhit2				= 0x69A1F3B8,
    opOCmelee1hhealthhit1				= 0x0EEFD0F6,
    opOCmelee1hspinattack2				= 0xB6CB33B3,
    opOCmelee1hhealthhit2				= 0x03ACF62F,
    opOCmelee1hbodyhit2					= 0xC3157D26,
    opOCmelee1hbodyhit3					= 0xC7D46091,
    opOCmelee1hblindhit1				= 0x362CFC22,
    opOCmelee1hblindhit2				= 0x3B6FDAFB,
    opOCmelee1hhit3						= 0x51CC667F
};

//=======================================================================

enum object_controller_pikeman_functions
{
    opOCpolearmhit2						= 0x890D1C35,
    opOCpolearmstun2					= 0x0277C8F1,
    opOCpolearmspinattack2				= 0x6E72B4A1,
    opOCpolearmleghit2					= 0xE6F56D2F,
    opOCpolearmleghit3					= 0xE2347098,
    opOCpolearmarea1					= 0x7E1EBEBA,
    opOCpolearmarea2					= 0x735D9863,
    opOCpolearmsweep1					= 0xDE551CC4,
    opOCpolearmsweep2					= 0xD3163A1D,
    opOCpolearmactionhit1				= 0x43162C1F,
    opOCpolearmactionhit2				= 0x4E550AC6,
    opOCpolearmhit3						= 0x8DCC0182
};

//=======================================================================

enum object_controller_pistoleer_functions
{
    opOChealthshot2						= 0x610888CE,
    opOCpointblanksingle2				= 0x55010F97,
    opOCbodyshot3						= 0xEA7BF24F,
    opOCpointblankarea2					= 0xF08F2259,
    opOCdisarmingshot1					= 0xD4DA2068,
    opOCdisarmingshot2					= 0xD99906B1,
    opOCdoubletap						= 0xEB830578,
    opOCstoppingshot					= 0x1673720C,
    opOCfanshot							= 0x10921EDB,
    opOCpistolmeleedefense1				= 0x6E76C41F,
    opOCpistolmeleedefense2				= 0x6335E2C6,
    opOCmultitargetpistolshot			= 0x4000FC87
};

//=======================================================================

enum object_controller_ranger_functions
{
    opOCareatrack						= 0xAA4E8A8C,
    opOCconceal							= 0x1F0512D5,
    opOCrescue							= 0x2F226EEE
};

//=======================================================================

enum object_controller_rifleman_functions
{
    opOCstrafeshot1						= 0x85B33F08,
    opOCstrafeshot2						= 0x88F019D1,
    opOCmindshot2						= 0x9F626531,
    opOCheadshot3						= 0x89749026,
    opOCsurpriseshot					= 0x61DC9EBA,
    opOCsnipershot						= 0x13CC58E1,
    opOCconcealshot						= 0x5A70C24B,
    opOCflushingshot1					= 0x60CE31D9,
    opOCflushingshot2					= 0x6D8D1700,
    opOCstartleshot1					= 0x04B23F7E,
    opOCstartleshot2					= 0x09F119A7,
    opOCflurryshot1						= 0x471EAC17,
    opOCflurryshot2						= 0x4A5D8ACE
};

//=======================================================================

enum object_controller_smuggler_functions
{
    opOCfeigndeath						= 0x4906C303,
    opOCpanicshot						= 0x6AC22291,
    opOClowblow							= 0xD4DBBC99,
    opOClastditch						= 0x29D6FF40
};

//=======================================================================

enum object_controller_squadleader_functions
{
    opOCsysgroup						= 0x5D6A8C24,
    opOCsteadyaim						= 0xA5BDBBA6,
    opOCvolleyfire						= 0x7EF26D6A,
    opOCformup							= 0xAB9FFCA3,
    opOCboostmorale						= 0xD29DEA7E,
    opOCrally							= 0x637279EA,
    opOCretreat							= 0x4871BBF4
};

//=======================================================================

enum object_controller_swordsman_functions
{
    opOCmelee2hhit2						= 0x78756C40,
    opOCmelee2hhit3						= 0x7CB471F7,
    opOCmelee2harea1					= 0x8B172B04,
    opOCmelee2harea2					= 0x86540DDD,
    opOCmelee2harea3					= 0x8295106A,
    opOCmelee2hheadhit2					= 0x14779ACA,
    opOCmelee2hheadhit3					= 0x10B6877D,
    opOCmelee2hspinattack2				= 0x8BE6D40B,
    opOCmelee2hsweep2					= 0x44FE127F,
    opOCmelee2hmindhit1					= 0xFAEE81FB,
    opOCmelee2hmindhit2					= 0xF7ADA722
};

//=======================================================================

enum object_controller_teraskasi_functions
{
    opOCunarmedhit2						= 0x3C67C604,
    opOCunarmedhit3						= 0x38A6DBB3,
    opOCmeditate						= 0x124629F2,
    opOCpowerboost						= 0x8C2221CB,
    opOCforceofwill						= 0x2EF7EE38,
    opOCunarmedknockdown1				= 0x26B9DB1B,
    opOCunarmedknockdown2				= 0x2BFAFDC2,
    opOCunarmeddizzy1					= 0x792D1600,
    opOCunarmedcombo					= 0x874523C4,
    opOCunarmedcombo2					= 0x8A06051D,
    opOCunarmedspinattack2				= 0xD6075389,
    opOCunarmedbodyhit1					= 0x57ECDE87,
    opOCunarmedleghit1					= 0x638B5475,
    opOCunarmedheadhit1					= 0x05FEC475
};

//=======================================================================

enum object_controller_force_defense_functions
{
    opOCavoidincapacitation					= 0x000E0422
};

//=======================================================================

enum object_controller_force_enhancement_functions
{
    opOCforceabsorb1						= 0x12F561A0,
    opOCforceabsorb2						= 0x1FB64779,
    opOCforcespeed1							= 0x9DD18562,
    opOCforcespeed2							= 0x9092A3BB,
    opOCforcerun1							= 0x97640B97,
    opOCforcerun2							= 0x9A272D4E,
    opOCforcerun3							= 0x9EE630F9,
    opOCforcefeedback1						= 0x966BB324,
    opOCforcefeedback2						= 0x9B2895FD,
    opOCforcearmor1							= 0x8EC31B36,
    opOCforcearmor2							= 0x83803DEF,
    opOCforceresistbleeding					= 0xBE92679A,
    opOCforceresistdisease					= 0xCBC4FC32,
    opOCforceresistpoison					= 0x544F3691,
    opOCforceresiststates					= 0xCB9CDFE5,
    opOCtransferforce						= 0xB371DE87,
    opOCchannelforce						= 0x2D8F25D8,
    opOCdrainforce							= 0x5DFF0378,
    opOCforceshield1						= 0x62A2B941,
    opOCforceshield2						= 0x6FE19F98,
    opOCforcemeditate						= 0xBD40A262,
    opOCregainconsciousness					= 0xF279EA94
};

//=======================================================================

enum object_controller_force_healing_functions
{
    opOChealallself1						= 0x1132654E,
    opOChealallself2						= 0x1C714397,
    opOChealhealthself1						= 0xAFC151D3,
    opOChealhealthself2						= 0xA282770A,
    opOChealactionself1						= 0x8BCFC965,
    opOChealactionself2						= 0x868CEFBC,
    opOChealmindself1						= 0x77754ACC,
    opOChealmindself2						= 0x7A366C15,
    opOChealactionwoundself1				= 0x2D33AC6C,
    opOChealactionwoundself2				= 0x20708AB5,
    opOChealhealthwoundself1				= 0x9F16F755,
    opOChealhealthwoundself2				= 0x9255D18C,
    opOChealbattlefatigueself1				= 0xC3074419,
    opOChealbattlefatigueself2				= 0xCE4462C0,
    opOChealmindwoundself1					= 0x2D6D1E0D,
    opOChealmindwoundself2					= 0x202E38D4,
    opOChealactionwoundother1				= 0x7F091BDA,
    opOChealactionwoundother2				= 0x724A3D03,
    opOChealhealthwoundother1				= 0xEEEE94CA,
    opOChealhealthwoundother2				= 0xE3ADB213,
    opOChealmindwoundother1					= 0x21BB7ADA,
    opOChealmindwoundother2					= 0x2CF85C03,
    opOChealallother1						= 0x9FFECF2E,
    opOChealallother2						= 0x92BDE9F7,
    opOChealstatesother						= 0x6831F67A,
    opOCstopbleeding						= 0x650449D9,
    opOCforcecuredisease					= 0x0288BA78,
    opOCforcecurepoison						= 0x99EA3DC1,
    opOChealstatesself						= 0x2E286256,
    opOCtotalhealother						= 0x6B370958,
    opOCtotalhealself						= 0x18E5D089
};

//=======================================================================

enum object_controller_force_power_functions
{
    opOCanimalscare							= 0xE001C174,
    opOCforcelightningsingle1				= 0x583B6776,
    opOCforcelightningsingle2				= 0x557841AF,
    opOCforcelightningcone1					= 0xC1A411E6,
    opOCforcelightningcone2					= 0xCCE7373F,
    opOCmindblast1							= 0x0B30AB2A,
    opOCmindblast2							= 0x06738DF3,
    opOCanimalcalm							= 0x22E1565B,
    opOCanimalattack						= 0xA7E89355,
    opOCforceweaken1						= 0xA8BDB9D8,
    opOCforceweaken2						= 0xA5FE9F01,
    opOCforceintimidate1					= 0xBD0584A0,
    opOCforceintimidate2					= 0xB046A279,
    opOCforcethrow1							= 0x9A6A843E,
    opOCforcethrow2							= 0x9729A2E7,
    opOCforceknockdown1						= 0x68E60EE9,
    opOCforceknockdown2						= 0x65A52830,
    opOCforceknockdown3						= 0x61643587,
    opOCforcechoke							= 0x4B0A9D33,
    opOCjedimindtrick						= 0xED1E2488
};

//=======================================================================

enum object_controller_force_lightsaber_functions
{
    opOCsaber1hhit1							= 0x9B9757C2,
    opOCsaber1hhit2							= 0x96D4711B,
    opOCsaber1hhit3							= 0x92156CAC,
    opOCsaber2hhit1							= 0xB6EF404A,
    opOCsaber2hhit2							= 0xBBAC6693,
    opOCsaber2hhit3							= 0x92156CAC,
    opOCsaberpolearmhit1					= 0x471423BA,
    opOCsaberpolearmhit2					= 0x4A570563,
    opOCsaberpolearmhit3					= 0x4E9618D4,
    opOCsaber1hcombohit1					= 0xE7671D59,
    opOCsaber1hcombohit2					= 0xEA243B80,
    opOCsaber1hcombohit3					= 0xEEE52637,
    opOCsaber1hheadhit1						= 0x354E8CC9,
    opOCsaber1hheadhit2						= 0x380DAA10,
    opOCsaber1hheadhit3						= 0x3CCCB7A7,
    opOCsaber1hflurry						= 0xA8F3F3AD,
    opOCsaber1hflurry2						= 0x48930003,
    opOCsaber2hbodyhit1						= 0xE22C6B25,
    opOCsaber2hbodyhit2						= 0xEF6F4DFC,
    opOCsaber2hbodyhit3						= 0xEBAE504B,
    opOCsaber2hsweep1						= 0xC11D6447,
    opOCsaber2hsweep2						= 0xCC5E429E,
    opOCsaber2hsweep3						= 0xC89F5F29,
    opOCsaber2hfrenzy						= 0x5DB5A3C5,
    opOCsaberpolearmleghit1					= 0x30D7DD42,
    opOCsaberpolearmleghit2					= 0x3D94FB9B,
    opOCsaberpolearmleghit3					= 0x3955E62C,
    opOCsaberpolearmspinattack1				= 0xEF9A22DF,
    opOCsaberpolearmspinattack2				= 0xE2D90406,
    opOCsaberpolearmspinattack3				= 0xE61819B1,
    opOCsaberpolearmdervish					= 0x0A45B8A1,
    opOCsaberpolearmdervish2				= 0x06756163,
    opOCsaberslash1							= 0xB7F2D310,
    opOCsaberslash2							= 0xBAB1F5C9,
    opOCsaberthrow1							= 0xF8465559,
    opOCsaberthrow2							= 0xF5057380,
    opOCsaberthrow3							= 0xF1C46E37,
    opOCsaber2hphantom						= 0x1712E1DD
};

//=======================================================================


#endif


