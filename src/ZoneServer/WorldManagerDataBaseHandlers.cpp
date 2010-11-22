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

#include "WorldManager.h"
#include "PlayerObject.h"
#include "CharacterLoginHandler.h"
#include "CreatureSpawnRegion.h"
#include "HarvesterFactory.h"
#include "FactoryFactory.h"
#include "GroupObject.h"
#include "GroupManager.h"
#include "HouseFactory.h"
#include "ObjectFactory.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"
#include "Heightmap.h"
#include "Common/ConfigManager.h"
#include "Common/Crc.h"

#include <cppconn/resultset.h>


//======================================================================================================================


void WorldManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    WMAsyncContainer* asyncContainer = reinterpret_cast<WMAsyncContainer*>(ref);

    switch(mState)
    {
        //
        // startup queries
        //
    case WMState_StartUp:
    {
        switch(asyncContainer->mQuery)
        {
        case WMQuery_ObjectCount:
        {
            // we got the total objectCount we need to load
            DataBinding*	binding = mDatabase->createDataBinding(1);
            binding->addField(DFT_uint32,0,4);
            result->getNextRow(binding,&mTotalObjectCount);

            LOG(INFO) << "Loading " << mTotalObjectCount << " World Manager Objects... ";

            if(mTotalObjectCount > 0)
            {
                // this loads all buildings with cells and objects they contain
                _loadBuildings();	 //NOT PlayerStructures!!!!!!!!!!!!!!!!!!!!!!!!!! they are handled seperately further down

                if(mZoneId!=41)
                {
                    // load objects in world
                    _loadAllObjects(0);

                    // load zone regions
                    int8 sql[128] ;
                    sprintf(sql, "SELECT id FROM zone_regions WHERE planet_id=%u ORDER BY id;",mZoneId);
                    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }

                        while(result_set->next())
                        {
                            gObjectFactory->requestObject(ObjType_Region, Region_Zone, 0, this, result_set->getUInt64(1));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Zone Regions";
                    });
                    
                }
                // load client effects
                int8 sql[128] ;
                sprintf(sql, "SELECT * FROM clienteffects ORDER BY id;");
                mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }
                        // tell vector how much space we need to stop unecessary allocation.
                        mvClientEffects.reserve(result_set->rowsCount());
                        while(result_set->next())
                        {
                            mvClientEffects.push_back(result_set->getString("effect"));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Client Effects";
                    });

                // load attribute keys
                sql[0] = 0 ;
                sprintf(sql, "SELECT id, name FROM attributes ORDER BY id;");
                mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }
                        
                        while(result_set->next())
                        {
                            std::string name = result_set->getString("name");
                            uint32_t crc = common::memcrc(name);
                            mObjectAttributeKeyMap.insert(std::make_pair(crc, name));
                            mObjectAttributeIDMap.insert(std::make_pair(crc, result_set->getInt("id")));
                        }
                        LOG_IF(INFO, mObjectAttributeKeyMap.size()) << "Loaded " << mObjectAttributeKeyMap.size() << " Client Effects";
                    });

                // load sounds
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Sounds),"SELECT * FROM sounds ORDER BY id;");

                // load moods
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Moods),"SELECT * FROM moods ORDER BY id;");
                
                // load npc converse animations
                
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_NpcConverseAnimations),"SELECT * FROM conversation_animations ORDER BY id;");
                
                // load npc chatter
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_NpcChatter),"SELECT * FROM npc_chatter WHERE planetId=%u OR planetId=99;",mZoneId);
                

                if(mZoneId != 41)
                {
                    // load cities
                    int8 sql[128];
                    sprintf(sql, "SELECT id FROM cities WHERE planet_id=%u ORDER BY id;",mZoneId);
                    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }
                        while(result_set->next())
                        {
                            gObjectFactory->requestObject(ObjType_Region, Region_City, 0, this, result_set->getUInt64(1));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " City Regions";
                        LOG_IF(INFO, !result_set->rowsCount()) <<"Unable to load cities with region id: " << mZoneId;
                    });

                    // load badge regions
                    sql[0] = 0;
                    sprintf(sql, "SELECT id FROM badge_regions WHERE planet_id=%u ORDER BY id;",mZoneId);
                    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }
                        while(result_set->next())
                        {
                            gObjectFactory->requestObject(ObjType_Region, Region_Badge, 0, this, result_set->getUInt64(1));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Badge Regions";
                        LOG_IF(INFO, !result_set->rowsCount()) << "Unable to load badges with region id: " << mZoneId;
                    });

                    //load spawn regions
                    sql[0] = 0;
                    sprintf(sql, "SELECT id FROM spawn_regions WHERE planet_id=%u ORDER BY id;",mZoneId);
                    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }
                        while(result_set->next())
                        {
                            gObjectFactory->requestObject(ObjType_Region, Region_Spawn, 0, this, result_set->getUInt64(1));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Spawn Regions";
                         LOG_IF(INFO, !result_set->rowsCount())  << "Unable to load spawn regions with id: " << mZoneId;
                    });

                    // load world scripts
                    mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_WorldScripts),"SELECT priority,file FROM config_zone_scripts WHERE planet_id=%u ORDER BY id;",mZoneId);
                    

                    //load creature spawn regions, and optionally heightmaps cache.
                    sql[0] = 0;
                    sprintf(sql, "SELECT id, spawn_x, spawn_z, spawn_width, spawn_length FROM spawns WHERE spawn_planet=%u ORDER BY id;",mZoneId);
                    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
                        if (! result)
                        {
                            return;
                        }

                        while(result_set->next())
                        {
                            std::shared_ptr<CreatureSpawnRegion> creatureSpawnRegion(new CreatureSpawnRegion());
                            creatureSpawnRegion->mId = result_set->getInt64(1);
                            creatureSpawnRegion->mPosX = result_set->getDouble(2);
                            creatureSpawnRegion->mPosZ = result_set->getDouble(3);
                            creatureSpawnRegion->mWidth =  result_set->getDouble(4);
                            creatureSpawnRegion->mLength  = result_set->getDouble(5);

                            mCreatureSpawnRegionMap.insert(std::make_pair<uint64_t, std::shared_ptr<CreatureSpawnRegion>>
                                (creatureSpawnRegion->mId, creatureSpawnRegion));
                        }
                        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Creature Spawn Regions";
                        LOG_IF(INFO, !result_set->rowsCount()) << "No Creature Spawn Regions Loaded with ID: " << mZoneId;
                    });
                    
                }

                // load harvesters
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Harvesters),"SELECT s.id FROM structures s INNER JOIN harvesters h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);
                

                // load factories
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Factories),"SELECT s.id FROM structures s INNER JOIN factories f ON (s.id = f.id) WHERE zone=%u ORDER BY id;",mZoneId);
                

                // load playerhouses
                mDatabase->executeSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Houses),"SELECT s.id FROM structures s INNER JOIN houses h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);
                
            }
            // no objects to load, so we are done
            else
            {
                _handleLoadComplete();
            }

            mDatabase->destroyDataBinding(binding);
        }
        break;

        //load harvesters
        case WMQuery_Harvesters:
        {
            DataBinding* harvesterBinding = mDatabase->createDataBinding(1);
            harvesterBinding->addField(DFT_int64,0,8);

            uint64 harvesterId;
            uint64 count = result->getRowCount();

            for(uint64 i = 0; i < count; i++)
            {
                result->getNextRow(harvesterBinding,&harvesterId);

                gHarvesterFactory->requestObject(this,harvesterId,0,0,asyncContainer->mClient);
            }

            LOG_IF(INFO, count) << "Loaded " << count << " Harvesters";

            mDatabase->destroyDataBinding(harvesterBinding);


        }
        break;

        case WMQuery_Houses:
        {
            DataBinding* houseBinding = mDatabase->createDataBinding(1);
            houseBinding->addField(DFT_int64,0,8);

            uint64 houseId;
            uint64 count = result->getRowCount();


            for(uint64 i = 0; i < count; i++)
            {
                result->getNextRow(houseBinding,&houseId);

                gHouseFactory->requestObject(this,houseId,0,0,asyncContainer->mClient);
            }

            LOG_IF(INFO, count) << "Loaded " << count << " Player Houses";

            mDatabase->destroyDataBinding(houseBinding);
        }
        break;

        //load harvesters
        case WMQuery_Factories:
        {
            DataBinding* factoryBinding = mDatabase->createDataBinding(1);
            factoryBinding->addField(DFT_int64,0,8);

            uint64 factoryId;
            uint64 count = result->getRowCount();

            for(uint64 i = 0; i < count; i++)
            {
                result->getNextRow(factoryBinding,&factoryId);
                gFactoryFactory->requestObject(this,factoryId,0,0,asyncContainer->mClient);
            }

            LOG_IF(INFO, count) << "Loaded " << count << " Factories";

            mDatabase->destroyDataBinding(factoryBinding);
        }
        break;


        // zone regions
        case WMQuery_ZoneRegions:
        {
            LOG(INFO) << "Entered handleDatabaseJobComplete :: WMQuery_ZoneRegions";
        }
        break;

        // planet names and according terrain file names
        case WMQuery_PlanetNamesAndFiles:
        {
            BString			tmp;
            DataBinding*	nameBinding = mDatabase->createDataBinding(1);
            nameBinding->addField(DFT_bstring,0,255,1);

            uint64 rowCount = result->getRowCount();
            mvPlanetNames.reserve((uint32)rowCount);
            for(uint64 i = 0; i < rowCount; i++)
            {
                result->getNextRow(nameBinding,&tmp);
                mvPlanetNames.push_back(BString(tmp.getAnsi()));
            }

            mDatabase->destroyDataBinding(nameBinding);

            result->resetRowIndex();

            DataBinding*	fileBinding = mDatabase->createDataBinding(1);
            fileBinding->addField(DFT_bstring,0,255,2);
            mvTrnFileNames.reserve((uint32)rowCount);
            for(uint64 i = 0; i < rowCount; i++)
            {
                result->getNextRow(fileBinding,&tmp);
                mvTrnFileNames.push_back(BString(tmp.getAnsi()));
            }

            mDatabase->destroyDataBinding(fileBinding);

            //start loading heightmap
            if(mZoneId != 41)
            {
                int16 resolution = 0;
                if (gConfig->keyExists("heightMapResolution"))
                    resolution = gConfig->read<int>("heightMapResolution");

                if (!Heightmap::Instance(resolution))
                    assert(false && "WorldManager::_handleLoadComplete Missing heightmap, look for it on the forums.");
            }
        }
        break;

        // global sounds map
        case WMQuery_Sounds:
        {
            BString			tmp;
            DataBinding*	binding = mDatabase->createDataBinding(1);
            binding->addField(DFT_bstring,0,255,1);

            uint64 effectCount = result->getRowCount();
            mvSounds.reserve((uint32)effectCount);
            for(uint64 i = 0; i < effectCount; i++)
            {
                result->getNextRow(binding,&tmp);

                mvSounds.push_back(BString(tmp.getAnsi()));
            }

            LOG_IF(INFO, effectCount) << "Loaded " << effectCount << " Sounds";

            mDatabase->destroyDataBinding(binding);
        }
        break;

        // global moods map
        case WMQuery_Moods:
        {
            BString			tmp;
            DataBinding*	binding = mDatabase->createDataBinding(1);
            binding->addField(DFT_bstring,0,255,1);

            uint64 effectCount = result->getRowCount();
            mvMoods.reserve((uint32)effectCount);
            for(uint64 i = 0; i < effectCount; i++)
            {
                result->getNextRow(binding,&tmp);

                mvMoods.push_back(BString(tmp.getAnsi()));
            }

            LOG_IF(INFO, effectCount) << "Loaded " << effectCount << " Moods";


            mDatabase->destroyDataBinding(binding);
        }
        break;

        // global npc animations map
        case WMQuery_NpcConverseAnimations:
        {
            BString			tmp;
            DataBinding*	binding = mDatabase->createDataBinding(1);
            binding->addField(DFT_bstring,0,255,1);

            uint64 animCount = result->getRowCount();
            mvNpcConverseAnimations.reserve((uint32)animCount);
            for(uint64 i = 0; i < animCount; i++)
            {
                result->getNextRow(binding,&tmp);

                mvNpcConverseAnimations.push_back(BString(tmp.getAnsi()));
            }

            LOG_IF(INFO, animCount) << "Loaded " << animCount << " NPC Converse Animations";

            mDatabase->destroyDataBinding(binding);
        }
        break;

        // random npc phrases/animations map
        case WMQuery_NpcChatter:
        {
            BString			tmp;
            DataBinding*	binding = mDatabase->createDataBinding(1);
            binding->addField(DFT_bstring,0,255,1);

            uint32			animId;
            DataBinding*	animbinding = mDatabase->createDataBinding(1);
            animbinding->addField(DFT_uint32,0,4,2);

            uint64 phraseCount = result->getRowCount();
            mvNpcChatter.reserve((uint32)phraseCount);
            for(uint64 i = 0; i < phraseCount; i++)
            {
                result->getNextRow(binding,&tmp);
                result->resetRowIndex(static_cast<int>(i));
                result->getNextRow(animbinding,&animId);

                tmp.convert(BSTRType_Unicode16);

                mvNpcChatter.push_back(std::make_pair(std::wstring(tmp.getUnicode16()),animId));
            }

            LOG_IF(INFO, phraseCount) << "Loaded " << phraseCount << " NPC Phrases";

            mDatabase->destroyDataBinding(binding);
            mDatabase->destroyDataBinding(animbinding);
        }
        break;

        // world scripts
        case WMQuery_WorldScripts:
        {
            DataBinding*	scriptBinding = mDatabase->createDataBinding(2);
            scriptBinding->addField(DFT_uint32,offsetof(Script,mPriority),4,0);
            scriptBinding->addField(DFT_string,offsetof(Script,mFile),255,1);

            uint64 scriptCount = result->getRowCount();

            for(uint64 i = 0; i < scriptCount; i++)
            {
                Script* script = gScriptEngine->createScript();

                result->getNextRow(scriptBinding,script);

                mWorldScripts.push_back(script);
            }

            LOG_IF(INFO, scriptCount) << "Loaded " << scriptCount << " World Scripts";

            mDatabase->destroyDataBinding(scriptBinding);
        }
        break;

        // buildings
        case WMQuery_All_Buildings:
        {

            uint64			buildingCount;
            uint64			buildingId;
            DataBinding*	buildingBinding = mDatabase->createDataBinding(1);
            buildingBinding->addField(DFT_int64,0,8);

            buildingCount = result->getRowCount();

            for(uint64 i = 0; i < buildingCount; i++)
            {
                result->getNextRow(buildingBinding,&buildingId);

                gObjectFactory->requestObject(ObjType_Building,0,0,this,buildingId,asyncContainer->mClient);
            }

            LOG_IF(INFO, buildingCount) << "Loaded " << buildingCount << " Buildings";

            mDatabase->destroyDataBinding(buildingBinding);
        }
        break;

        // city regions
        case WMQuery_Cities:
        {
            LOG(INFO) << "Entered handleDatabaseJobComplete :: WMQuery_Cities";
        }
        break;

        // badge regions
        case WMQuery_BadgeRegions:
        {
            LOG(INFO) << "Entered handleDatabaseJobComplete :: WMQuery_BadgeRegions";
        }
        break;

        // spawn regions
        case WMQuery_SpawnRegions:
        {
            LOG(INFO) << "Entered handleDatabaseJobComplete :: WMQuery_SpawnRegions";
        }
        break;

        // Creature spawn regions
        case WMQuery_CreatureSpawnRegions:
        {
            LOG(INFO) << "Entered handleDatabaseJobComplete :: WMQuery_CreatureSpawnRegions";
        }
        break;

        // container->child objects
        case WMQuery_AllObjectsChildObjects:
        {
            WMQueryContainer queryContainer;

            DataBinding*	binding = mDatabase->createDataBinding(2);
            binding->addField(DFT_bstring,offsetof(WMQueryContainer,mString),64,0);
            binding->addField(DFT_uint64,offsetof(WMQueryContainer,mId),8,1);

            uint64 count = result->getRowCount();


            for(uint32 i = 0; i < count; i++)
            {
                result->getNextRow(binding,&queryContainer);

                if(strcmp(queryContainer.mString.getAnsi(),"terminals") == 0)
                    gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Terminal,0,this,queryContainer.mId,asyncContainer->mClient);
                else if(strcmp(queryContainer.mString.getAnsi(),"ticket_collectors") == 0)
                    gObjectFactory->requestObject(ObjType_Tangible,TanGroup_TicketCollector,0,this,queryContainer.mId,asyncContainer->mClient);
                else if(strcmp(queryContainer.mString.getAnsi(),"shuttles") == 0)
                    gObjectFactory->requestObject(ObjType_Creature,CreoGroup_Shuttle,0,this,queryContainer.mId,asyncContainer->mClient);

             
                // now to the ugly part

                if(strcmp(queryContainer.mString.getAnsi(),"containers") == 0)
                    gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Container,0,this,queryContainer.mId,asyncContainer->mClient);
                else if(strcmp(queryContainer.mString.getAnsi(),"persistent_npcs") == 0)
                    gObjectFactory->requestObject(ObjType_NPC,CreoGroup_PersistentNpc,0,this,queryContainer.mId,asyncContainer->mClient);
                else if(strcmp(queryContainer.mString.getAnsi(),"items") == 0)
                    gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Item,0,this,queryContainer.mId,asyncContainer->mClient);
                else if(strcmp(queryContainer.mString.getAnsi(),"resource_containers") == 0)
                    gObjectFactory->requestObject(ObjType_Tangible,TanGroup_ResourceContainer,0,this,queryContainer.mId,asyncContainer->mClient);
            }

            LOG_IF(INFO, count) << "Loaded " << count << " Cell Children";

            mDatabase->destroyDataBinding(binding);
        }
        break;

        default:
            break;
        }
    }
    break;

    //
    // Queries in running state
    //
    case WMState_Running:
    {
        switch(asyncContainer->mQuery)
        {

            // TODO: make stored function for saving
        case WMQuery_SavePlayer_Position:
        {

            PlayerObject* playerObject			= dynamic_cast<PlayerObject*>(asyncContainer->mObject);
            //saving ourselves async might see us deleted before finish
            if(!playerObject)
            {
                break;
            }

            WMAsyncContainer* asyncContainer2	= new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_SavePlayer_Attributes);

            Ham* ham							= playerObject->getHam();

            if(asyncContainer->mBool)
            {
                asyncContainer2->mBool = true;
            }

            asyncContainer2->mObject		= asyncContainer->mObject;
            asyncContainer2->clContainer	= asyncContainer->clContainer;
            asyncContainer2->mLogout		= asyncContainer->mLogout;

            mDatabase->executeSqlAsync(this,asyncContainer2,"UPDATE character_attributes SET health_current=%u,action_current=%u,mind_current=%u"
                                       ",health_wounds=%u,strength_wounds=%u,constitution_wounds=%u,action_wounds=%u,quickness_wounds=%u"
                                       ",stamina_wounds=%u,mind_wounds=%u,focus_wounds=%u,willpower_wounds=%u,battlefatigue=%u,posture=%u,moodId=%u,title=\'%s\'"
                                       ",character_flags=%u,states=%"PRIu64",language=%u,new_player_exemptions=%u WHERE character_id=%"PRIu64""
                                       ,ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier(),
                                       ham->mAction.getCurrentHitPoints() - ham->mAction.getModifier(),
                                       ham->mMind.getCurrentHitPoints() - ham->mMind.getModifier()
                                       ,ham->mHealth.getWounds(),ham->mStrength.getWounds()
                                       ,ham->mConstitution.getWounds(),ham->mAction.getWounds(),ham->mQuickness.getWounds(),ham->mStamina.getWounds(),ham->mMind.getWounds()
                                       ,ham->mFocus.getWounds(),ham->mWillpower.getWounds(),ham->getBattleFatigue(),playerObject->states.getPosture(),playerObject->getMoodId(),playerObject->getTitle().getAnsi()
                                       ,playerObject->getPlayerFlags(),playerObject->states.getAction(),playerObject->getLanguage(),playerObject->getNewPlayerExemptions(),playerObject->getId());
            
        }
        break;

        case WMQuery_SavePlayer_Attributes:
        {
            if(asyncContainer->mBool)
            {
                PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);

                //saving ourselves async might see us deleted before finish
                if(!playerObject)
                {
                    SAFE_DELETE(asyncContainer->clContainer);
                    break;
                }
                //delete the old char

                //remove the player out of his group - if any
                GroupObject* group = gGroupManager->getGroupObject(playerObject->getGroupId());
                if(group)
                {
                    group->removePlayer(playerObject->getId());
                }

                destroyObject(playerObject);

                //are we to load a new character???
                if(asyncContainer->mLogout == WMLogOut_Char_Load)
                {
                    gObjectFactory->requestObject(ObjType_Player,0,0,asyncContainer->clContainer->ofCallback,asyncContainer->clContainer->mPlayerId,asyncContainer->clContainer->mClient);

                    //now destroy the asyncContainer->clContainer
                    SAFE_DELETE(asyncContainer->clContainer);

                }

            }
            if(asyncContainer->mLogout == WMLogOut_Zone_Transfer)
            {
                //update the position to the new planets position
                const glm::vec3& destination = asyncContainer->clContainer->destination;

                //in this case we retain the asynccontainer and let it be destroyed by the clientlogin handler
                mDatabase->executeSqlAsync(asyncContainer->clContainer->dbCallback,asyncContainer->clContainer,"UPDATE characters SET parent_id=0,x='%f', y='%f', z='%f', planet_id='%u' WHERE id='%"PRIu64"';", destination.x, destination.y, destination.z, asyncContainer->clContainer->planet, asyncContainer->clContainer->player->getId());
                
            }
        }
        break;

        default:
            break;
        }
    }
    break;

    //
    // queries when shutting down
    //
    case WMState_ShutDown:
    {

    }
    break;

    default:
        LOG(FATAL)<<"World Manager Database Callback: unknown state: " << mState;
        break;
    }

    mWM_DB_AsyncPool.ordered_free(asyncContainer);
}
