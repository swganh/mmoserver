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

#ifndef ANH_ZONESERVER_MISSIONMANAGER_H
#define ANH_ZONESERVER_MISSIONMANAGER_H

#define gMissionManager MissionManager::getSingletonPtr()

#include <map>
#include "Utils/bstring.h"
#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"

//======================================================================================================================

class CurrentResource;
class Database;
class DatabaseCallback;
class DatabaseResult;
class DispatchClient;
class MissionObject;
class NPCObject;
class PlayerObject;
class Buff;

typedef struct tagResourceLocation ResourceLocation;


//======================================================================================================================

/// @class MissionManagerAsyncContainer
/// @brief Transport container for database callback
///
/// This class is used to transport context-specific information into the database
/// callback when an asynchronous database access completes.

enum MissionQueryType
{
    MissionQuery_NULL					=	0,
    MissionQuery_Load_Types			    =	1,
    MissionQuery_Load_Terminal_Type		=	2,
    MissionQuery_Load_Names			    =	3,
    MissionQuery_Load_Names_File	    =	4


};

enum MissionDifficultyEnum
{
    MissionDifficulty_Easy					=	0,
    MissionDifficulty_Medium				=	1,
    MissionDifficulty_Hard					=	2

};

enum MissionTypeEnum
{
    MissionTypeDestroy				=	0,
    MissionTypeDeliver				=	1,
    MissionTypeDancer				=	2,
    MissionTypeMusician				=	3,
    MissionTypeCraft				=	4,
    MissionTypeBounty				=	5,
    MissionTypeEscort				=	6,
    MissionTypeEscorttoCreator		=	7,
    MissionTypeHunting				=	8,
    MissionTypeRecon				=	9,
    MissionTypeSurvey				=	10

};

enum MissionFactionEnum
{
    MissionFaction_Neutral			=	0,
    MissionFaction_Imperial			=	1,
    MissionFaction_Rebel			=	2

};

enum MissionTarget
{
    MissionTarget_NPC				=	0,
    MissionTarget_Creature			=	1

};

enum MissionGiver
{
    MissionGiver_NPC				=	0,
    MissionGiver_NameProvided		=	1


};

struct Mission_Names
{
    uint32  id;
    uint32  type;
    BString  mission_name;
    BString  name;
};

typedef std::map<uint32,Mission_Names*>		NameMap;

struct Mission_Types
{
    uint64	id;
    BString	stf;
    uint32	content;
    uint32	nameprovided;
    NameMap names;
};

struct Terminal_Mission_Link
{
    uint32					id;
    uint64					terminal;
    uint32					mission_type;
    uint32					content;
    uint32					name;
    MissionDifficultyEnum	difficulty;
    MissionTypeEnum			type;
    MissionFactionEnum		faction;
    MissionGiver			giver;
    MissionTarget			target;
    Mission_Types*			missiontype;

};

typedef std::vector<Terminal_Mission_Link*>		MissionLinkList;

struct Terminal_Type
{
    uint64				id;
    MissionLinkList		list;

};

typedef std::map<uint32,Mission_Types*>		MissionMap;
typedef std::map<uint64,Terminal_Type*>		TerminalMap;


class MissionManagerAsyncContainer
{
public:
    MissionManagerAsyncContainer(MissionQueryType qt,DispatchClient* client) {
        mQueryType = qt;
        mClient = client;
    }
    ~MissionManagerAsyncContainer() {}

    MissionQueryType	mQueryType;
    DispatchClient*		mClient;

};

//======================================================================================================================

/// @class MissionManager
/// @brief Player mission management
///
/// This class processes Player mission actions.

class MissionManager : public DatabaseCallback
{
public:     // methods
    MissionManager(Database* database, uint32 zone);
    ~MissionManager();

    static MissionManager*  Init(Database* database, uint32 zone);
    static MissionManager*  getSingletonPtr()  {
        return mSingleton;
    }

    virtual void handleDatabaseJobComplete(void* ref,DatabaseResult* result);

    void    createRequest(PlayerObject* player);
    ///  @short Mission list request
    ///
    ///  The player has opened a mission terminal, and the terminal now needs to
    ///  be populated with summary entries for that terminal type.
    ///
    ///  @param[in] player The player who is requesting the action
    ///  @param[in] terminal_id The identifier of the specific terminal that was opened
    void    listRequest(PlayerObject* player,uint64 terminal_id,uint8 refresh_count);

    ///  @short Mission details request
    ///
    ///  The player has selected a specific mission on the terminal, and we
    ///  must now provide the details of that mission for display.
    ///
    ///  @param[in] player The player who is requesting the action
    void    detailsRequest(PlayerObject* player);

    ///  @short Mission create request
    ///
    ///  The player has elected to accept a specific mission.  We must activate
    ///  that mission for the user, and spawn the mission assets.
    ///
    ///  @param[in] player The player who is requesting the action
    ///  @param[in] mission_id The id of the mission accepted
    void	missionRequest(PlayerObject* player, uint64 mission_id);
    void    missionComplete(PlayerObject* player, MissionObject* mission);
    void	missionCompleteEntertainer(PlayerObject* player,Buff* timer);
    void	missionAbort(PlayerObject* player, uint64 mission_id);
    void	missionFailed(PlayerObject* player, MissionObject* mission);
    void	missionFailedEntertainer(PlayerObject* player);

    //Mission
    bool	checkDeliverMission(PlayerObject* player,NPCObject* npc);
    void	checkMusicianMission(PlayerObject* player);
    void    checkDancerMission(PlayerObject* player);
    void    checkSurveyMission(PlayerObject* player,CurrentResource* resource,ResourceLocation highestDist);
    bool	checkCraftingMission(PlayerObject* player,NPCObject* npc);
    bool	checkReconMission(MissionObject* mission);



    //Mission Generation Functions
    MissionObject* generateDestroyMission(MissionObject* mission, uint64 terminal);
    MissionObject* generateDeliverMission(MissionObject* mission);
    MissionObject* generateEntertainerMission(MissionObject* mission,int count);
    MissionObject* generateSurveyMission(MissionObject* mission);
    MissionObject* generateCraftingMission(MissionObject* mission);
    MissionObject* generateReconMission(MissionObject* mission);

private:
    static bool             mInsFlag;
    static MissionManager*  mSingleton;
    Database*               mDatabase;

    MissionMap				mMissionMap;
    TerminalMap				mTerminalMap;
    NameMap					mNameMap;
};

#endif
