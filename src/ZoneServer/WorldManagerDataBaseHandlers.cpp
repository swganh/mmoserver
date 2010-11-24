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

#include <cppconn/resultset.h>

#include "Common/ConfigManager.h"
#include "Common/Crc.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "ScriptEngine/ScriptEngine.h"
#include "ScriptEngine/ScriptSupport.h"

#include "CharacterLoginHandler.h"
#include "CreatureSpawnRegion.h"
#include "FactoryFactory.h"
#include "GroupObject.h"
#include "GroupManager.h"
#include "HarvesterFactory.h"
#include "Heightmap.h"
#include "HouseFactory.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"

//======================================================================================================================

void WorldManager::_loadWorldObjects()
{
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
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

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
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            // tell vector how much space we need to stop unecessary allocation.
            mvClientEffects.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvClientEffects.push_back(result_set->getString("effect"));
            }
            LOG_IF(INFO, mvClientEffects.size()) << "Loaded " << mvClientEffects.size() << " Client Effects";
        });

        // load attribute keys
        sql[0] = 0 ;
        sprintf(sql, "SELECT id, name FROM attributes ORDER BY id;");
        mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            while(result_set->next())
            {
                BString name = result_set->getString("name").c_str();
                mObjectAttributeKeyMap.insert(std::make_pair(name.getCrc(), name));
                mObjectAttributeIDMap.insert(std::make_pair(name.getCrc(), result_set->getInt("id")));
            }
            LOG_IF(INFO, mObjectAttributeKeyMap.size()) << "Loaded " << mObjectAttributeKeyMap.size() << " Attributes";
        });

        // load sounds
        sql[0] = 0 ;
        sprintf(sql, "SELECT * FROM sounds ORDER BY id;");
        mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            while(result_set->next())
            {
                mvSounds.push_back(result_set->getString("name"));
            }

            LOG_IF(INFO, mvSounds.size()) << "Loaded " << mvSounds.size() << " Sounds";
        });

        // load moods
        sql[0] = 0 ;
        sprintf(sql, "SELECT * FROM moods ORDER BY id;");
        mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvMoods.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvMoods.push_back(result_set->getString("name"));
            }
            LOG_IF(INFO, mvMoods.size()) << "Loaded " << mvMoods.size() << " Moods";
        });

        // load npc converse animations
        sql[0] = 0 ;
        sprintf(sql, "SELECT * FROM conversation_animations ORDER BY id;");
        mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvNpcConverseAnimations.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvNpcConverseAnimations.push_back(result_set->getString("name"));
            }
            LOG_IF(INFO, mvNpcConverseAnimations.size()) << "Loaded " << mvNpcConverseAnimations.size() << " NPC Converse Animations";
        });
        // load npc chatter
        sql[0] = 0 ;
        sprintf(sql, "SELECT * FROM npc_chatter WHERE planetId=%u OR planetId=99;", mZoneId);
        mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvNpcChatter.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                std::string phrase(result_set->getString("phrase"));
                uint32 anim = result_set->getUInt("animation");
                // convert from std::string to wstring
                std::wstring ws;
                ws.assign(phrase.begin(), phrase.end());

                mvNpcChatter.push_back(std::make_pair(ws, anim));
            }
            LOG_IF(INFO, mvNpcChatter.size()) << "Loaded " << mvNpcChatter.size()<< " NPC Chatter Phrases";
        });

        if(mZoneId != 41)
        {
            // load cities
            int8 sql[512];
            sprintf(sql, "SELECT id FROM cities WHERE planet_id=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

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
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

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
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Spawn, 0, this, result_set->getUInt64(1));
                }
                LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Spawn Regions";
                LOG_IF(INFO, !result_set->rowsCount())  << "Unable to load spawn regions with id: " << mZoneId;
            });

            // load world scripts
            sql[0] = 0;
            sprintf(sql, "SELECT priority,file FROM config_zone_scripts WHERE planet_id=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    Script* script = gScriptEngine->createScript();
                    script->setPriority(result_set->getUInt("priority"));
                    script->setFileName(result_set->getString("file").c_str());
                    mWorldScripts.push_back(script);
                }
                LOG_IF(INFO, mWorldScripts.size()) << "Loaded " << mWorldScripts.size() << " World Scripts";
            });

            //load creature spawn regions, and optionally heightmaps cache.
            sql[0] = 0;
            sprintf(sql, "SELECT id, spawn_x, spawn_z, spawn_width, spawn_length FROM spawns WHERE spawn_planet=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    std::shared_ptr<CreatureSpawnRegion> creatureSpawnRegion = std::make_shared<CreatureSpawnRegion>();
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

            // load harvesters
            sql[0] = 0;
            sprintf(sql, "SELECT s.id FROM structures s INNER JOIN harvesters h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gHarvesterFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Player Harvesters";
                LOG_IF(INFO, !result_set->rowsCount()) << "No Harvesters to Load in Zone: " << mZoneId;
            });

            // load factories
            sql[0] = 0;
            sprintf(sql, "SELECT s.id FROM structures s INNER JOIN factories f ON (s.id = f.id) WHERE zone=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gFactoryFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Player Factories";
                LOG_IF(INFO, !result_set->rowsCount()) << "No Player Factories to Load in Zone: " << mZoneId;
            });

            // load playerhouses
            sql[0] = 0;
            sprintf(sql, "SELECT s.id FROM structures s INNER JOIN houses h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);
            mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gFactoryFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Player Houses";
                LOG_IF(INFO, !result_set->rowsCount()) << "No Player Houses to Load in Zone: " << mZoneId;
            });
        }
    }
    // no objects to load, so we are done
    else
    {
        _handleLoadComplete();
    }
}
void WorldManager::_loadBuildings()
{
    int8 sql[128] ;
    sprintf(sql, "SELECT id FROM buildings WHERE planet_id = %u;",mZoneId);
    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while(result_set->next())
        {
            gObjectFactory->requestObject(ObjType_Building,0,0,this,result_set->getUInt64(1));
        }
        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Buildings";
    });
}

void WorldManager::_loadAllObjects(uint64 parentId)
{
    int8	sql[2048];
    sprintf(sql,"(SELECT \'terminals\',terminals.id FROM terminals INNER JOIN terminal_types ON (terminals.terminal_type = terminal_types.id)"
            " WHERE (terminal_types.name NOT LIKE 'unknown') AND (terminals.parent_id = %"PRIu64") AND (terminals.planet_id = %"PRIu32"))"
            " UNION (SELECT \'containers\',containers.id FROM containers INNER JOIN container_types ON (containers.container_type = container_types.id)"
            " WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = %"PRIu64") AND (containers.planet_id = %u))"
            " UNION (SELECT \'ticket_collectors\',ticket_collectors.id FROM ticket_collectors WHERE (parent_id=%"PRIu64") AND (planet_id=%u))"
            " UNION (SELECT \'persistent_npcs\',persistent_npcs.id FROM persistent_npcs WHERE (parentId=%"PRIu64") AND (planet_id = %"PRIu32"))"
            " UNION (SELECT \'shuttles\',shuttles.id FROM shuttles WHERE (parentId=%"PRIu64") AND (planet_id = %"PRIu32"))"
            " UNION (SELECT \'items\',items.id FROM items WHERE (parent_id=%"PRIu64") AND (planet_id = %"PRIu32"))"
            " UNION (SELECT \'resource_containers\',resource_containers.id FROM resource_containers WHERE (parent_id=%"PRIu64") AND (planet_id = %"PRIu32"))",
            parentId,mZoneId,parentId,mZoneId,parentId,mZoneId,parentId,mZoneId,parentId
            ,mZoneId,parentId,mZoneId,parentId,mZoneId);

    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while(result_set->next())
        {
            std::string str = result_set->getString(1);
            uint64_t id = result_set->getUInt64(2);
            if(strcmp(str.c_str(),"terminals") == 0)
                gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Terminal,0,this,id);
            else if(strcmp(str.c_str(),"ticket_collectors") == 0)
                gObjectFactory->requestObject(ObjType_Tangible,TanGroup_TicketCollector,0,this,id);
            else if(strcmp(str.c_str(),"shuttles") == 0)
                gObjectFactory->requestObject(ObjType_Creature,CreoGroup_Shuttle,0,this,id);

            if(strcmp(str.c_str(),"containers") == 0)
                gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Container,0,this,id);
            else if(strcmp(str.c_str(),"persistent_npcs") == 0)
                gObjectFactory->requestObject(ObjType_NPC,CreoGroup_PersistentNpc,0,this,id);
            else if(strcmp(str.c_str(),"items") == 0)
                gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Item,0,this,id);
            else if(strcmp(str.c_str(),"resource_containers") == 0)
                gObjectFactory->requestObject(ObjType_Tangible,TanGroup_ResourceContainer,0,this,id);
        }
        LOG_IF(INFO, result_set->rowsCount()) << "Loaded " << result_set->rowsCount() << " Buildings";
    });
}
void    WorldManager::_updatePlayerAttributesToDB(uint32 accId)
{
    PlayerObject* playerObject			= getPlayerByAccId(accId);
    if(!playerObject) {
        DLOG(INFO) << "Could not find player with AccId:" <<accId<<", save aborted.";
        return;
    }
    Ham* ham = playerObject->getHam();
    if (!ham)
    {
        DLOG(INFO) << "Could not get Ham object from player id: " << playerObject->getId();
    }
    int8 sql[2048];
    sprintf(sql, "UPDATE character_attributes SET health_current=%u,action_current=%u,mind_current=%u"
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

    mDatabase->executeAsyncSql(sql, 0);
}

void    WorldManager::_updatePlayerPositionToDB(uint32 accId)
{
    PlayerObject* playerObject			= getPlayerByAccId(accId);
    if(!playerObject) {
        DLOG(INFO) << "Could not find player with AccId:" <<accId<<", save aborted.";
        return;
    }

    int8 sql[512];
    sprintf(sql, "UPDATE characters SET parent_id=%"PRIu64",oX=%f,oY=%f,oZ=%f,oW=%f,x=%f,y=%f,z=%f,planet_id=%u,jedistate=%u WHERE id=%"PRIu64"",playerObject->getParentId()
            ,playerObject->mDirection.x,playerObject->mDirection.y,playerObject->mDirection.z,playerObject->mDirection.w
            ,playerObject->mPosition.x,playerObject->mPosition.y,playerObject->mPosition.z
            ,mZoneId,playerObject->getJediState(),playerObject->getId());
    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {

        _updatePlayerAttributesToDB(accId);
    } ) ;
}

void    WorldManager::_updatePlayerBuffsToDB(uint32 accId)
{
    // TO BE IMPLEMENTED
    // BUFF Manager is currently in control of this.
}

void    WorldManager::_updatePlayerLogoutToDB(uint32 accId, CharacterLoadingContainer* clContainer)
{
    PlayerObject* playerObject			= getPlayerByAccId(accId);
    if(!playerObject) {
        DLOG(INFO) << "Could not find player with AccId:" <<accId<<", save aborted.";
        return;
    }
    //remove the player out of his group - if any
    GroupObject* group = gGroupManager->getGroupObject(playerObject->getGroupId());
    if(group)
    {
        group->removePlayer(playerObject->getId());
    }

    destroyObject(playerObject);
}

void    WorldManager::_loadPlanetNamesAndFiles()
{
    int8 sql[512];
    sprintf(sql, "SELECT * FROM planet ORDER BY planet_id;");
    mDatabase->executeAsyncSql(sql, [=] (DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        uint32_t count = result_set->rowsCount();
        mvPlanetNames.reserve(count);
        mvTrnFileNames.reserve(count);
        while(result_set->next())
        {
            mvPlanetNames.push_back(BString(result_set->getString("name").c_str()));
            mvTrnFileNames.push_back(BString(result_set->getString("terrain_file").c_str()));
        }

        //start loading heightmap
        if(mZoneId != 41)
        {
            int16 resolution = 0;
            if (gConfig->keyExists("heightMapResolution"))
                resolution = gConfig->read<int>("heightMapResolution");

            if (!Heightmap::Instance(resolution))
                assert(false && "WorldManager::_handleLoadComplete Missing heightmap, look for it on the forums.");
        }
    } ) ;
}
void WorldManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{}
