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
            query_stream << "SELECT id FROM "<<mDatabase->galaxy()<<".zone_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Zone, 0, this, result_set->getUInt64(1));
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Zone Regions";
				}
            });

        }
        // load client effects
        stringstream query_stream;
        query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".clienteffects ORDER BY id;";
                    
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

			if (mvClientEffects.size())
			{
				LOG(INFO) << "Loaded " << mvClientEffects.size() << " Client Effects";
			}
        });

        // load attribute keys
        query_stream.str(std::string());
        query_stream << "SELECT id, name FROM "<<mDatabase->galaxy()<<".attributes ORDER BY id;";
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

			if (mObjectAttributeKeyMap.size())
			{
				LOG(INFO) << "Loaded " << mObjectAttributeKeyMap.size() << " Attributes";
			}
        });

        // load sounds
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".sounds ORDER BY id;";
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
            while(result_set->next())
            {
                mvSounds.push_back(result_set->getString("name"));
            }

			if (mvSounds.size())
			{
				LOG(INFO) << "Loaded " << mvSounds.size() << " Sounds";
			}
        });

        // load moods
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".moods ORDER BY id;";
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvMoods.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvMoods.push_back(result_set->getString("name"));
            }

			if (mvMoods.size())
			{
				LOG(INFO) << "Loaded " << mvMoods.size() << " Moods";
			}
        });

        // load npc converse animations
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".conversation_animations ORDER BY id;";
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
            if (! result) {
                return;
            }

            std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

            mvNpcConverseAnimations.reserve(result_set->rowsCount());
            while(result_set->next())
            {
                mvNpcConverseAnimations.push_back(result_set->getString("name"));
            }

			if (mvNpcConverseAnimations.size())
			{
				LOG(INFO) << "Loaded " << mvNpcConverseAnimations.size() << " NPC Converse Animations";
			}
        });
        // load npc chatter
        query_stream.str(std::string());
        query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".npc_chatter WHERE planetId=" << mZoneId
                     << " OR planetId=99";
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

			if (mvNpcChatter.size())
			{
				LOG(INFO) << "Loaded " << mvNpcChatter.size() << " NPC Chatter Phrases";
			}
        });

        if(mZoneId != 41)
        {
            // load cities
            stringstream query_stream;
            query_stream << "SELECT id FROM "<<mDatabase->galaxy()<<".cities WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_City, 0, this, result_set->getUInt64(1));
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " City Regions";
				}
				else
				{
					LOG(INFO) << "Unable to load cities with region id: " << mZoneId;
				}
            });

            // load badge regions
            query_stream.str(std::string());
            query_stream << "SELECT id FROM "<<mDatabase->galaxy()<<".badge_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Badge, 0, this, result_set->getUInt64(1));
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Badge Regions";
				}
				else
				{
					LOG(INFO) << "Unable to load badges with region id: " << mZoneId;
				}
            });

            //load spawn regions
            query_stream.str(std::string());
            query_stream << "SELECT id FROM "<<mDatabase->galaxy()<<".spawn_regions WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gObjectFactory->requestObject(ObjType_Region, Region_Spawn, 0, this, result_set->getUInt64(1));
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Spawn Regions";
				}
				else
				{
					LOG(INFO) << "Unable to load spawn regions with id: " << mZoneId;
				}
            });

            // load world scripts
            query_stream.str(std::string());
            query_stream << "SELECT priority,file FROM "<<mDatabase->galaxy()<<".config_zone_scripts WHERE planet_id=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

				if (mWorldScripts.size())
				{
					LOG(INFO) << "Loaded " << mWorldScripts.size() << " World Scripts";
				}
            });

            //load creature spawn regions, and optionally heightmaps cache.
            query_stream.str(std::string());
            query_stream << "SELECT id, spawn_x, spawn_z, spawn_width, spawn_length FROM "<<mDatabase->galaxy()<<".spawns WHERE spawn_planet=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Creature Spawn Regions";
				}
				else
				{
					LOG(INFO) << "No Creature Spawn Regions Loaded with ID: " << mZoneId;
				}
            });

            // load harvesters
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<mDatabase->galaxy()<<".structures s INNER JOIN "<<mDatabase->galaxy()<<".harvesters h ON (s.id = h.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }
                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gHarvesterFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Player Harvesters";
				}
				else
				{
					LOG(INFO) << "No Harvesters to Load in Zone: " << mZoneId;
				}
            });

            // load factories
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<mDatabase->galaxy()<<".structures s INNER JOIN "<<mDatabase->galaxy()<<".factories f ON (s.id = f.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gFactoryFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Player Factories";
				}
				else
				{
					LOG(INFO) << "No Player Factories to Load in Zone: " << mZoneId;
				}
            });

            // load playerhouses
            query_stream.str(std::string());
            query_stream << "SELECT s.id FROM "<<mDatabase->galaxy()<<".structures s INNER JOIN "<<mDatabase->galaxy()<<".houses h ON (s.id = h.id) WHERE zone=" << mZoneId
                         << " ORDER BY id;";
            mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
                if (! result) {
                    return;
                }

                std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

                while(result_set->next())
                {
                    gHouseFactory->requestObject(this,result_set->getUInt64(1),0,0,0);
                }

				if (result_set->rowsCount())
				{
					LOG(INFO) << "Loaded " << result_set->rowsCount() << " Player Houses";
				}
				else
				{
					LOG(INFO) << "No Player Houses to Load in Zone: " << mZoneId;
				}
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
    query_stream << "SELECT id FROM "<<mDatabase->galaxy()<<".buildings WHERE planet_id = " << mZoneId;
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
        if (! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();

        while(result_set->next())
        {
            gObjectFactory->requestObject(ObjType_Building,0,0,this,result_set->getUInt64(1));
        }

		if (result_set->rowsCount())
		{
			LOG(INFO) << "Loaded " << result_set->rowsCount() << " Buildings";
		}
    });
}

void WorldManager::_loadAllObjects(uint64 parentId)
{
    stringstream query_stream;
    query_stream << "(SELECT \'terminals\',terminals.id FROM "<<mDatabase->galaxy()<<".terminals INNER JOIN "<<mDatabase->galaxy()<<".terminal_types ON (terminals.terminal_type = terminal_types.id)"
                 << " WHERE (terminal_types.name NOT LIKE 'unknown') AND (terminals.parent_id = " << parentId << ") AND (terminals.planet_id = "<< mZoneId << " ))"
                 << " UNION (SELECT \'containers\',containers.id FROM "<<mDatabase->galaxy()<<".containers INNER JOIN "<<mDatabase->galaxy()<<".container_types ON (containers.container_type = container_types.id)"
                 << " WHERE (container_types.name NOT LIKE 'unknown') AND (containers.parent_id = " << parentId << ") AND (containers.planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'ticket_collectors\',ticket_collectors.id FROM "<<mDatabase->galaxy()<<".ticket_collectors WHERE (parent_id=" << parentId << ") AND (planet_id="<< mZoneId << "))"
                 << " UNION (SELECT \'persistent_npcs\',persistent_npcs.id FROM "<<mDatabase->galaxy()<<".persistent_npcs WHERE (parentId=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'shuttles\',shuttles.id FROM "<<mDatabase->galaxy()<<".shuttles WHERE (parentId=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'items\',items.id FROM "<<mDatabase->galaxy()<<".items WHERE (parent_id=" << parentId << ") AND (planet_id = "<< mZoneId << "))"
                 << " UNION (SELECT \'resource_containers\',resource_containers.id FROM "<<mDatabase->galaxy()<<".resource_containers WHERE (parent_id=" << parentId << ") AND (planet_id = "<< mZoneId <<"))";
    
        mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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

		if (result_set->rowsCount())
		{
			LOG(INFO) << "Loaded " << result_set->rowsCount() << " Objects";
		}
    });
}
void    WorldManager::_loadPlanetNamesAndFiles()
{
    stringstream query_stream;
    query_stream << "SELECT * FROM "<<mDatabase->galaxy()<<".planet ORDER BY planet_id;";
    mDatabase->executeAsyncSql(query_stream, [=] (DatabaseResult* result) {
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
            if (mHeightmapResolution)
                resolution =mHeightmapResolution;

            if (!Heightmap::Instance(mHeightmapResolution))
                assert(false && "WorldManager::_handleLoadComplete Missing heightmap, look for it on the forums.");
        }
    } ) ;
}
void WorldManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{}
