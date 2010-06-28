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
#include "ConfigManager/ConfigManager.h"


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
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_uint32,0,4);
					result->GetNextRow(binding,&mTotalObjectCount);

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loading objects...");

					if(mTotalObjectCount > 0)
					{
						// this loads all buildings with cells and objects they contain
						_loadBuildings();	 //NOT PlayerStructures!!!!!!!!!!!!!!!!!!!!!!!!!! they are handled seperately further down
						
						if(mZoneId!=41)
						{
							// load objects in world
							_loadAllObjects(0);

							// load zone regions
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_ZoneRegions),"SELECT id FROM zone_regions WHERE planet_id=%u ORDER BY id;",mZoneId);
						}
						// load client effects
						if(!mDebug)
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_ClientEffects),"SELECT * FROM clienteffects ORDER BY id;");

						

						// load attribute keys
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_AttributeKeys),"SELECT id, name FROM attributes ORDER BY id;");

						// load sounds
						if(!mDebug)
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Sounds),"SELECT * FROM sounds ORDER BY id;");

						// load moods
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Moods),"SELECT * FROM moods ORDER BY id;");

						// load npc converse animations
						if(!mDebug)
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_NpcConverseAnimations),"SELECT * FROM conversation_animations ORDER BY id;");

						// load npc chatter
						if(!mDebug)
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_NpcChatter),"SELECT * FROM npc_chatter WHERE planetId=%u OR planetId=99;",mZoneId);

						if(mZoneId != 41)
						{
							// load cities
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Cities),"SELECT id FROM cities WHERE planet_id=%u ORDER BY id;",mZoneId);

							// load badge regions
							if(!mDebug)
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_BadgeRegions),"SELECT id FROM badge_regions WHERE planet_id=%u ORDER BY id;",mZoneId);

							//load spawn regions
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_SpawnRegions),"SELECT id FROM spawn_regions WHERE planet_id=%u ORDER BY id;",mZoneId);

							// load world scripts
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_WorldScripts),"SELECT priority,file FROM config_zone_scripts WHERE planet_id=%u ORDER BY id;",mZoneId);

							//load creature spawn regions, and optionally heightmaps cache.
							mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_CreatureSpawnRegions),"SELECT id, spawn_x, spawn_z, spawn_width, spawn_length FROM spawns WHERE spawn_planet=%u ORDER BY id;",mZoneId);
						}

						// load harvesters
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Harvesters),"SELECT s.id FROM structures s INNER JOIN harvesters h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);

						// load factories
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Factories),"SELECT s.id FROM structures s INNER JOIN factories f ON (s.id = f.id) WHERE zone=%u ORDER BY id;",mZoneId);

						// load playerhouses
						mDatabase->ExecuteSqlAsync(this,new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_Houses),"SELECT s.id FROM structures s INNER JOIN houses h ON (s.id = h.id) WHERE zone=%u ORDER BY id;",mZoneId);
										

					}
					// no objects to load, so we are done
					else
					{
						_handleLoadComplete();
					}

					mDatabase->DestroyDataBinding(binding);
				}
				break;

				//load harvesters
				case WMQuery_Harvesters:
				{
					DataBinding* harvesterBinding = mDatabase->CreateDataBinding(1);
					harvesterBinding->addField(DFT_int64,0,8);

					uint64 harvesterId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(harvesterBinding,&harvesterId);

						gHarvesterFactory->requestObject(this,harvesterId,0,0,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded harvesters.");

					mDatabase->DestroyDataBinding(harvesterBinding);


				}
				break;

				case WMQuery_Houses:
				{
					DataBinding* houseBinding = mDatabase->CreateDataBinding(1);
					houseBinding->addField(DFT_int64,0,8);

					uint64 houseId;
					uint64 count = result->getRowCount();

						
					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(houseBinding,&houseId);

						gHouseFactory->requestObject(this,houseId,0,0,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded playerhouses.");

					mDatabase->DestroyDataBinding(houseBinding);
				}
				break;

				//load harvesters
				case WMQuery_Factories:
				{
					DataBinding* factoryBinding = mDatabase->CreateDataBinding(1);
					factoryBinding->addField(DFT_int64,0,8);

					uint64 factoryId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(factoryBinding,&factoryId);

						gFactoryFactory->requestObject(this,factoryId,0,0,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded factories.");

					mDatabase->DestroyDataBinding(factoryBinding);
				}
				break;


				// zone regions
				case WMQuery_ZoneRegions:
				{

					DataBinding* regionBinding = mDatabase->CreateDataBinding(1);
					regionBinding->addField(DFT_int64,0,8);

					uint64 regionId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(regionBinding,&regionId);

						gObjectFactory->requestObject(ObjType_Region,Region_Zone,0,this,regionId,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded zone regions.");

					mDatabase->DestroyDataBinding(regionBinding);
				}
				break;

				// planet names and according terrain file names
				case WMQuery_PlanetNamesAndFiles:
				{
					BString			tmp;
					DataBinding*	nameBinding = mDatabase->CreateDataBinding(1);
					nameBinding->addField(DFT_bstring,0,255,1);

					uint64 rowCount = result->getRowCount();
					mvPlanetNames.reserve((uint32)rowCount);
					for(uint64 i = 0;i < rowCount;i++)
					{
						result->GetNextRow(nameBinding,&tmp);
						mvPlanetNames.push_back(BString(tmp.getAnsi()));
					}

					mDatabase->DestroyDataBinding(nameBinding);

					result->ResetRowIndex();

					DataBinding*	fileBinding = mDatabase->CreateDataBinding(1);
					fileBinding->addField(DFT_bstring,0,255,2);
					mvTrnFileNames.reserve((uint32)rowCount);
					for(uint64 i = 0;i < rowCount;i++)
					{
						result->GetNextRow(fileBinding,&tmp);
						mvTrnFileNames.push_back(BString(tmp.getAnsi()));
					}

					mDatabase->DestroyDataBinding(fileBinding);

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

				// global attribute lookup map
				case WMQuery_AttributeKeys:
				{
					struct loadstruct
					{
						uint32 id;
						BString attribute;
					};

					loadstruct	tmp;
					DataBinding* binding = mDatabase->CreateDataBinding(2);
					binding->addField(DFT_uint32,offsetof(loadstruct,id),4,0);
					binding->addField(DFT_bstring,offsetof(loadstruct,attribute),128,1);

					uint64 attributeCount = result->getRowCount();

					for(uint64 i = 0;i < attributeCount;i++)
					{
						result->GetNextRow(binding,&tmp);

						mObjectAttributeKeyMap.insert(std::make_pair(tmp.attribute.getCrc(),BString(tmp.attribute.getAnsi())));
						mObjectAttributeIDMap.insert(std::make_pair(tmp.attribute.getCrc(), tmp.id));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded attribute keys.");

					mDatabase->DestroyDataBinding(binding);
				}
				break;

				// global client effects map
				case WMQuery_ClientEffects:
				{
					BString			tmp;
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_bstring,0,255,1);

					uint64 effectCount = result->getRowCount();
					mvClientEffects.reserve((uint32)effectCount);
					for(uint64 i = 0;i < effectCount;i++)
					{
						result->GetNextRow(binding,&tmp);

						mvClientEffects.push_back(BString(tmp.getAnsi()));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded Client Effects.");


					mDatabase->DestroyDataBinding(binding);
				}
				break;

				// global sounds map
				case WMQuery_Sounds:
				{
					BString			tmp;
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_bstring,0,255,1);

					uint64 effectCount = result->getRowCount();
					mvSounds.reserve((uint32)effectCount);
					for(uint64 i = 0;i < effectCount;i++)
					{
						result->GetNextRow(binding,&tmp);

						mvSounds.push_back(BString(tmp.getAnsi()));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded sound effects.");

					mDatabase->DestroyDataBinding(binding);
				}
				break;

				// global moods map
				case WMQuery_Moods:
				{
					BString			tmp;
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_bstring,0,255,1);

					uint64 effectCount = result->getRowCount();
					mvMoods.reserve((uint32)effectCount);
					for(uint64 i = 0;i < effectCount;i++)
					{
						result->GetNextRow(binding,&tmp);

						mvMoods.push_back(BString(tmp.getAnsi()));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded moods.");


					mDatabase->DestroyDataBinding(binding);
				}
				break;

				// global npc animations map
				case WMQuery_NpcConverseAnimations:
				{
					BString			tmp;
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_bstring,0,255,1);

					uint64 animCount = result->getRowCount();
					mvNpcConverseAnimations.reserve((uint32)animCount);
					for(uint64 i = 0;i < animCount;i++)
					{
						result->GetNextRow(binding,&tmp);

						mvNpcConverseAnimations.push_back(BString(tmp.getAnsi()));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded NPC conversational animations.");

					mDatabase->DestroyDataBinding(binding);
				}
				break;

				// random npc phrases/animations map
				case WMQuery_NpcChatter:
				{
					BString			tmp;
					DataBinding*	binding = mDatabase->CreateDataBinding(1);
					binding->addField(DFT_bstring,0,255,1);

					uint32			animId;
					DataBinding*	animbinding = mDatabase->CreateDataBinding(1);
					animbinding->addField(DFT_uint32,0,4,2);

					uint64 phraseCount = result->getRowCount();
					mvNpcChatter.reserve((uint32)phraseCount);
					for(uint64 i = 0;i < phraseCount;i++)
					{
						result->GetNextRow(binding,&tmp);
						result->ResetRowIndex(static_cast<int>(i));
						result->GetNextRow(animbinding,&animId);

						tmp.convert(BSTRType_Unicode16);

						mvNpcChatter.push_back(std::make_pair(std::wstring(tmp.getUnicode16()),animId));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded NPC phrases.");

					mDatabase->DestroyDataBinding(binding);
					mDatabase->DestroyDataBinding(animbinding);
				}
				break;

				// world scripts
				case WMQuery_WorldScripts:
				{
					DataBinding*	scriptBinding = mDatabase->CreateDataBinding(2);
					scriptBinding->addField(DFT_uint32,offsetof(Script,mPriority),4,0);
					scriptBinding->addField(DFT_string,offsetof(Script,mFile),255,1);

					uint64 scriptCount = result->getRowCount();
					
					for(uint64 i = 0;i < scriptCount;i++)
					{
						Script* script = gScriptEngine->createScript();

						result->GetNextRow(scriptBinding,script);

						mWorldScripts.push_back(script);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded world scripts.");

					mDatabase->DestroyDataBinding(scriptBinding);
				}
				break;

				// buildings
				case WMQuery_All_Buildings:
				{

					uint64			buildingCount;
					uint64			buildingId;
					DataBinding*	buildingBinding = mDatabase->CreateDataBinding(1);
					buildingBinding->addField(DFT_int64,0,8);

					buildingCount = result->getRowCount();

					for(uint64 i = 0;i < buildingCount;i++)
					{
						result->GetNextRow(buildingBinding,&buildingId);

						gObjectFactory->requestObject(ObjType_Building,0,0,this,buildingId,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded buildings");

					mDatabase->DestroyDataBinding(buildingBinding);
				}
				break;

				// city regions
				case WMQuery_Cities:
				{
					DataBinding*	cityBinding = mDatabase->CreateDataBinding(1);
					cityBinding->addField(DFT_int64,0,8);

					uint64 cityId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(cityBinding,&cityId);

						gObjectFactory->requestObject(ObjType_Region,Region_City,0,this,cityId,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded city regions.");

					

					mDatabase->DestroyDataBinding(cityBinding);
				}
				break;

				// badge regions
				case WMQuery_BadgeRegions:
				{
					DataBinding*	badgeBinding = mDatabase->CreateDataBinding(1);
					badgeBinding->addField(DFT_int64,0,8);

					uint64 badgeId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(badgeBinding,&badgeId);

						gObjectFactory->requestObject(ObjType_Region,Region_Badge,0,this,badgeId,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded badge regions.");
					

					mDatabase->DestroyDataBinding(badgeBinding);
				}
				break;

				// spawn regions
				case WMQuery_SpawnRegions:
				{
					DataBinding*	spawnBinding = mDatabase->CreateDataBinding(1);
					spawnBinding->addField(DFT_int64,0,8);

					uint64 regionId;
					uint64 count = result->getRowCount();

					for(uint64 i = 0;i < count;i++)
					{
						result->GetNextRow(spawnBinding,&regionId);

						gObjectFactory->requestObject(ObjType_Region,Region_Spawn,0,this,regionId,asyncContainer->mClient);
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded spawn regions.");
					

					mDatabase->DestroyDataBinding(spawnBinding);
				}
				break;

				// Creature spawn regions
				case WMQuery_CreatureSpawnRegions:
				{
					DataBinding*	creatureSpawnBinding = mDatabase->CreateDataBinding(5);
					creatureSpawnBinding->addField(DFT_int64,offsetof(CreatureSpawnRegion,mId),8,0);
					creatureSpawnBinding->addField(DFT_float,offsetof(CreatureSpawnRegion,mPosX),4,1);
					creatureSpawnBinding->addField(DFT_float,offsetof(CreatureSpawnRegion,mPosZ),4,2);
					creatureSpawnBinding->addField(DFT_float,offsetof(CreatureSpawnRegion,mWidth),4,3);
					creatureSpawnBinding->addField(DFT_float,offsetof(CreatureSpawnRegion,mLength),4,4);

					uint64 count = result->getRowCount();
					
					for(uint64 i = 0;i < count;i++)
					{
						CreatureSpawnRegion *creatureSpawnRegion = new CreatureSpawnRegion();
						result->GetNextRow(creatureSpawnBinding,creatureSpawnRegion);
						mCreatureSpawnRegionMap.insert(std::make_pair(creatureSpawnRegion->mId,creatureSpawnRegion));
					}

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded creature spawn regions.");


					mDatabase->DestroyDataBinding(creatureSpawnBinding);
				}
				break;

				// container->child objects
				case WMQuery_AllObjectsChildObjects:
				{
					WMQueryContainer queryContainer;

					DataBinding*	binding = mDatabase->CreateDataBinding(2);
					binding->addField(DFT_bstring,offsetof(WMQueryContainer,mString),64,0);
					binding->addField(DFT_uint64,offsetof(WMQueryContainer,mId),8,1);

					uint64 count = result->getRowCount();

					
					for(uint32 i = 0;i < count;i++)
					{
						result->GetNextRow(binding,&queryContainer);

						if(strcmp(queryContainer.mString.getAnsi(),"terminals") == 0)
							gObjectFactory->requestObject(ObjType_Tangible,TanGroup_Terminal,0,this,queryContainer.mId,asyncContainer->mClient);
						else if(strcmp(queryContainer.mString.getAnsi(),"ticket_collectors") == 0)
							gObjectFactory->requestObject(ObjType_Tangible,TanGroup_TicketCollector,0,this,queryContainer.mId,asyncContainer->mClient);
						else if(strcmp(queryContainer.mString.getAnsi(),"shuttles") == 0)
							gObjectFactory->requestObject(ObjType_Creature,CreoGroup_Shuttle,0,this,queryContainer.mId,asyncContainer->mClient);

						if(mDebug)
						{
							mTotalObjectCount--;
							continue;
						}
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

					if(result->getRowCount())
						gLogger->log(LogManager::NOTICE,"Loaded cell children...");

					mDatabase->DestroyDataBinding(binding);
				}
				break;

				default: break;
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
					WMAsyncContainer* asyncContainer2	= new(mWM_DB_AsyncPool.ordered_malloc()) WMAsyncContainer(WMQuery_SavePlayer_Attributes);
					PlayerObject* playerObject			= dynamic_cast<PlayerObject*>(asyncContainer->mObject);
					Ham* ham							= playerObject->getHam();

					if(asyncContainer->mBool)
					{
						asyncContainer2->mBool = true;
					}

					asyncContainer2->mObject		= asyncContainer->mObject;
					asyncContainer2->clContainer	= asyncContainer->clContainer;
					asyncContainer2->mLogout		= asyncContainer->mLogout;

					mDatabase->ExecuteSqlAsync(this,asyncContainer2,"UPDATE character_attributes SET health_current=%u,action_current=%u,mind_current=%u"
						",health_wounds=%u,strength_wounds=%u,constitution_wounds=%u,action_wounds=%u,quickness_wounds=%u"
						",stamina_wounds=%u,mind_wounds=%u,focus_wounds=%u,willpower_wounds=%u,battlefatigue=%u,posture=%u,moodId=%u,title=\'%s\'"
						",character_flags=%u,states=%"PRIu64",language=%u,new_player_exemptions=%u WHERE character_id=%"PRIu64""
						,ham->mHealth.getCurrentHitPoints() - ham->mHealth.getModifier(),
						ham->mAction.getCurrentHitPoints() - ham->mAction.getModifier(),
						ham->mMind.getCurrentHitPoints() - ham->mMind.getModifier()
						,ham->mHealth.getWounds(),ham->mStrength.getWounds()
						,ham->mConstitution.getWounds(),ham->mAction.getWounds(),ham->mQuickness.getWounds(),ham->mStamina.getWounds(),ham->mMind.getWounds()
						,ham->mFocus.getWounds(),ham->mWillpower.getWounds(),ham->getBattleFatigue(),playerObject->getPosture(),playerObject->getMoodId(),playerObject->getTitle().getAnsi()
						,playerObject->getPlayerFlags(),playerObject->getState(),playerObject->getLanguage(),playerObject->getNewPlayerExemptions(),playerObject->getId());
				}
				break;

				case WMQuery_SavePlayer_Attributes:
				{
					if(asyncContainer->mBool)
					{
						PlayerObject* playerObject = dynamic_cast<PlayerObject*>(asyncContainer->mObject);
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
						mDatabase->ExecuteSqlAsync(asyncContainer->clContainer->dbCallback,asyncContainer->clContainer,"UPDATE characters SET parent_id=0,x='%f', y='%f', z='%f', planet_id='%u' WHERE id='%I64u';", destination.x, destination.y, destination.z, asyncContainer->clContainer->planet, asyncContainer->clContainer->player->getId());
					}
				}
				break;

				default:break;
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
			gLogger->log(LogManager::EMERGENCY,"World Manager Database Callback: unknown state: %i",mState);
		break;
	}

	mWM_DB_AsyncPool.ordered_free(asyncContainer);
}
