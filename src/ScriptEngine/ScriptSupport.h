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

#ifndef ANH_SCRIPT_SUPPORT_H
#define ANH_SCRIPT_SUPPORT_H

#include "Utils/typedefs.h"
#include "ZoneServer/ObjectFactoryCallback.h"
#include <map>
#include <string>
class Object;
class NPCObject;

typedef std::map<uint64,Object*>	ScriptSupportObjectMap;


class ScriptSupport : public ObjectFactoryCallback
{
public:
    // static inline ScriptSupport* Instance()
    static ScriptSupport* Instance();
    /*
    {
    	if (!mInstance)
    	{
    		mInstance = new ScriptSupport();
    	}
    	return mInstance;
    }
    */
    static void	destroyInstance(void);

protected:
    ScriptSupport();
    ~ScriptSupport();

public:

    // Enabled for scripting

    // NPC's
    uint64				npcCreate(uint64 npcId);
    NPCObject*			npcGetObject(uint64 id);

    void				npcSpawn(NPCObject* npc,
                                 uint64 npcId,
                                 uint64 cellForSpawn,
                                 std::string firstname,
                                 std::string lastname,
                                 float dirY,
                                 float dirW,
                                 float posX,
                                 float posY,
                                 float posZ);

    void				npcSpawnPrivate(NPCObject* npc,
                                        uint64 npcId,
                                        uint64 npcPrivateOwnerId,
                                        uint64 cellForSpawn,
                                        std::string firstname,
                                        std::string lastname,
                                        float dirY,
                                        float dirW,
                                        float posX,
                                        float posY,
                                        float posZ);

    void				npcSpawnPersistent(NPCObject* npc,
                                           uint64 npcId,
                                           uint64 cellForSpawn,
                                           std::string firstname,
                                           std::string lastname,
                                           float dirY,
                                           float dirW,
                                           float posX,
                                           float posY,
                                           float posZ,
                                           uint64 respawnPeriod,
                                           uint64 templateId);

    void				lairSpawn(uint64 lairTypeId);

    void				npcDirection(NPCObject* npc, float deltaX, float deltaZ);
    void				npcMove(NPCObject* npc, float posX, float posY, float posZ);
    void				npcMoveToZone(NPCObject* npc, uint64 zoneId, float posX, float posY, float posZ);

    void				npcFormationPosition(NPCObject* npcMember, float xOffset, float zOffset);
    void				npcClonePosition(NPCObject* npcDest, NPCObject* npcSrc);
    void				npcFormationMove(NPCObject* npc, float posX, float posY, float posZ);
    void				npcFormationMoveEx(NPCObject* npc, float posX, float posY, float posZ , float xOffset, float zOffset);

    // Containers
    // uint64				containerCreate(uint64 containerId);
    // Container*			containerGetObject(uint64 id);

    //void				containerSpawn(Container* container,
    //						uint64 containerId,
    //						uint64 playerId,
    //						uint64 cellForSpawn,
    //						std::string customName,
    //						float dirY,
    //						float dirW,
    //						float posX,
    //						float posY,
    //						float posZ);

    // Items
    uint64				itemCreate(uint64 itemTypesId);
    void				itemPopulateInventory(uint64 itemId, uint64 npcId, uint64 playerId);

    // Item*				itemGetObject(uint64 id);
    // void				itemSpawn(Item* item,
    //							uint64 itemId,
    //							uint64 playerId,
    //							uint64 parentId,
    //							std::string customName,
    //							float dirY,		// In case of spawning in a cell or the world.
    //							float dirW,
    //							float posX,
    //							float posY,
    //							float posZ);

    // General
    bool				objectIsReady(uint64 npcId);
    Object*				getObject(uint64 id);
    void				eraseObject(uint64 id);
    uint32				getZoneId();
    void				enableInstance();
    bool				isInstance();
    // void				scriptPlayMusic(uint32 soundId, Object* creatureObject);
    void				scriptPlayMusic(uint32 soundId, NPCObject* creatureObject);
    uint64				getTarget(uint64 playerId);
    uint64				getParentOfTarget(uint64 playerId);
    void				sendFlyText(uint64 targetId,
                                    uint64 playerId,
                                    std::string stfFile,
                                    std::string stfVar,
                                    uint8 red,
                                    uint8 green,
                                    uint8 blue,
                                    uint8 display);
    void				scriptSystemMessage(uint64 playerId, uint64 targetId, std::string message);
    void				setPlayerPosition(uint64 playerId, uint64 cellId, float posX, float posY, float posZ);
    bool				npcInCombat(uint64 npcId);
    bool				npcIsDead(uint64 npcId);
    void				npcKill(uint64 npcId);

    void				npcTestDir(NPCObject* npc, float dirX, float dirZ);

    // Not for scripting.
    void				handleObjectReady(Object* object);
    uint64				getObjectOwnedBy(uint64 theOwner);

private:
    void				npcSpawnGeneral(uint64 npcId,
                                        uint64 npcPrivateOwnerId,
                                        uint64 cellForSpawn,
                                        std::string firstname,
                                        std::string lastname,
                                        float dirY,
                                        float dirW,
                                        float posX,
                                        float posY,
                                        float posZ,
                                        uint64 respawnPeriod);


    static ScriptSupport*	mInstance;
    ScriptSupportObjectMap	mObjectMap;

};
//=============================================================================




#endif
