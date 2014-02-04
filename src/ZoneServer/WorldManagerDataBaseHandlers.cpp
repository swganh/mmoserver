/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "ZoneServer/WorldManager.h"

#include <cppconn/resultset.h>

#include "Common/Crc.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

//#include "ScriptEngine/ScriptEngine.h"
//#include "ScriptEngine/ScriptSupport.h"

#include "ZoneServer/GameSystemManagers/CharacterLoginHandler.h"
#include "ZoneServer/GameSystemManagers/Spawn Manager/CreatureSpawnRegion.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/FactoryFactory.h"
#include "ZoneServer/GameSystemManagers/Group Manager/GroupObject.h"
#include "ZoneServer/GameSystemManagers/Group Manager/GroupManager.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HarvesterFactory.h"
#include "ZoneServer/GameSystemManagers/Structure Manager/HouseFactory.h"
#include "ZoneServer/Objects/ObjectFactory.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"

#include <anh\app\swganh_kernel.h>

using std::stringstream;

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
            stringstream query_stream;
			query_stream << "SELECT id FROM "<< getKernel()->GetDatabase()->galaxy()<<".zone_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Zone, 0, this, result_set->getUInt64(1));
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Zone Regions";
            });

        }
        
		
		// load client effects
        stringstream query_stream;
        query_stream << "SELECT * FROM "<< getKernel()->GetDatabase()->galaxy()<<".clienteffects ORDER BY id;";            
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
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
            LOG(info) << "Loaded " << mvClientEffects.size() << " Client Effects";
        });

        // load attribute keys
        query_stream.str(std::string());
        query_stream << "SELECT id, name FROM " << getKernel()->GetDatabase()->galaxy()<<".attributes ORDER BY id;";
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            while(result_set->next())
            {
                std::string name = result_set->getString("name");
				mObjectAttributeKeyMap.insert(std::make_pair(common::memcrc(name), name));
                mObjectAttributeIDMap.insert(std::make_pair(common::memcrc(name), result_set->getInt("id")));
            }
            LOG(info) << "Loaded " << mObjectAttributeKeyMap.size() << " Attributes";
        });

        // load sounds
        query_stream.str(std::string());
        query_stream << "SELECT * FROM " << getKernel()->GetDatabase()->galaxy()<<".sounds ORDER BY id;";
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
            while(result_set->next())
            {
                mvSounds.push_back(result_set->getString("name"));
            }

            LOG(info) << "Loaded " << mvSounds.size() << " Sounds";
        });

        // load moods
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<getKernel()->GetDatabase()->galaxy()<<".moods ORDER BY id;";
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvMoods.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvMoods.push_back(result_set->getString("name"));
            }
            LOG(info) << "Loaded " << mvMoods.size() << " Moods";
        });

        // load npc converse animations
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<getKernel()->GetDatabase()->galaxy()<<".conversation_animations ORDER BY id;";
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvNpcConverseAnimations.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvNpcConverseAnimations.push_back(result_set->getString("name"));
            }
            LOG(info) << "Loaded " << mvNpcConverseAnimations.size() << " NPC Converse Animations";
        });
        // load npc chatter
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<getKernel()->GetDatabase()->galaxy()<<".npc_chatter WHERE planetId=" << mZoneId
                     << " OR planetId=99";
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
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
            LOG(info) << "Loaded " << mvNpcChatter.size()<< " NPC Chatter Phrases";
        });

        if(mZoneId != 41)
        {
            // load cities
            stringstream query_stream;
            query_stream << "SELECT id FROM "<<getKernel()->GetDatabase()->galaxy()<<".cities WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_City, 0, this, result_set->getUInt64(1));
                }
				if (result_set->rowsCount())	{
					LOG(info)  << "Loaded " << result_set->rowsCount() << " City Regions";
				}
				else
					LOG(info) <<"Unable to load cities with region id: " << mZoneId;
            });

            // load badge regions
            query_stream.str(std::string());
            query_stream << "SELECT id FROM "<<getKernel()->GetDatabase()->galaxy()<<".badge_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Badge, 0, this, result_set->getUInt64(1));
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Badge Regions";
                
            });

            //load spawn regions
            query_stream.str(std::string());
            query_stream << "SELECT id FROM "<<getKernel()->GetDatabase()->galaxy()<<".spawn_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Spawn, 0, this, result_set->getUInt64(1));
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Spawn Regions";
               
            });

            // load world scripts
            query_stream.str(std::string());
            query_stream << "SELECT priority,file FROM "<<getKernel()->GetDatabase()->galaxy()<<".config_zone_scripts WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    //Script* script = gScriptEngine->createScript();
                    //script->setPriority(result_set->getUInt("priority"));
                    //script->setFileName(result_set->getString("file").c_str());
                    //mWorldScripts.push_back(script);
                }
                //LOG(info) << "Loaded " << mWorldScripts.size() << " World Scripts";
            });

            //load creature spawn regions, and optionally heightmaps cache.
            query_stream.str(std::string());
            query_stream << "SELECT id, spawn_x, spawn_z, spawn_width, spawn_length FROM "<<getKernel()->GetDatabase()->galaxy()<<".spawns WHERE spawn_planet=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
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

                    mCreatureSpawnRegionMap.insert(std::make_pair(creatureSpawnRegion->mId, creatureSpawnRegion));
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Creature Spawn Regions";
                
            });

            // load harvesters
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".structures s INNER JOIN "<<getKernel()->GetDatabase()->galaxy()<<".harvesters h ON (s.id = h.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }
                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gHarvesterFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Player Harvesters";
                
            });

            // load factories
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".structures s INNER JOIN "<<getKernel()->GetDatabase()->galaxy()<<".factories f ON (s.id = f.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gFactoryFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Player Factories";
                
            });

            // load playerhouses
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".structures s INNER JOIN "<<getKernel()->GetDatabase()->galaxy()<<".houses h ON (s.id = h.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gHouseFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }
                LOG(info) << "Loaded " << result_set->rowsCount() << " Player Houses";
                
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
    stringstream query_stream;
    query_stream << "SELECT id FROM "<<getKernel()->GetDatabase()->galaxy()<<".buildings WHERE planet_id = " << mZoneId;
    getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while(result_set->next())
        {
            gObjectFactory->requestObject(ObjType_Building,0,0,this,result_set->getUInt64(1));
        }
        LOG(info) << "Loaded " << result_set->rowsCount() << " Buildings";
    });
}

void WorldManager::_loadAllObjects(uint64 parentId)
{
    stringstream query_stream;
    query_stream << "(SELECT \'terminals\',terminals.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".terminals INNER JOIN "<<getKernel()->GetDatabase()->galaxy()<<".terminal_types ON (terminals.terminal_type = terminal_types.id)"
                 << " WHERE (terminal_types.name NOT LIKE 'unknown') AND (terminals.parent_id = " << parentId << ") AND (terminals.planet_id = "<< mZoneId << " ))"
                 << " UNION (SELECT \'containers\',containers.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".containers INNER JOIN "<<getKernel()->GetDatabase()->galaxy()<<".container_types ON (containers.container_type = container_types.id)"
                 << " WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = " << parentId << ") AND (containers.planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'ticket_collectors\',ticket_collectors.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".ticket_collectors WHERE (parent_id=" << parentId << ") AND (planet_id="<< mZoneId << "))"
                 << " UNION (SELECT \'persistent_npcs\',persistent_npcs.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".persistent_npcs WHERE (parentId=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'shuttles\',shuttles.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".shuttles WHERE (parentId=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'items\',items.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".items WHERE (parent_id=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'resource_containers\',resource_containers.id FROM "<<getKernel()->GetDatabase()->galaxy()<<".resource_containers WHERE (parent_id=" << parentId << ") AND (planet_id = "<< mZoneId <<"))";
    
        getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
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
        LOG(info) << "Loaded " << result_set->rowsCount() << " Objects";
    });
}

void    WorldManager::_loadPlanetNamesAndFiles()
{
    stringstream query_stream;
    query_stream << "SELECT * FROM "<< getKernel()->GetDatabase()->galaxy()<<".planet ORDER BY planet_id;";
	LOG (info) << query_stream.str();
    getKernel()->GetDatabase()->executeAsyncSql(query_stream, [=] (swganh::database::DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        uint32_t count = result_set->rowsCount();
        mvPlanetNames.reserve(count);
        mvTrnFileNames.reserve(count);
        while(result_set->next())
        {
            mvPlanetNames.push_back(result_set->getString("name"));
            mvTrnFileNames.push_back(result_set->getString("terrain_file"));
        }

        
    } ) ;
}
void WorldManager::handleDatabaseJobComplete(void* ref,swganh::database::DatabaseResult* result)
{}
