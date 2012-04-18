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
#pragma once
#ifndef ANH_ZONESERVER_ARTISAN_MANAGER_H
#define ANH_ZONESERVER_ARTISAN_MANAGER_H

#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/EventHandler.h"
#include "Utils/Singleton.h"
#include "ObjectFactoryCallback.h"
#include "HeightMapCallback.h"
#include "UICallback.h"
#include "glm/glm.hpp"
#include "Common/Event.h"

class ArtisanManager;
#define gArtisanManager ::utils::Singleton<ArtisanManager>::Instance()
//======================================================================================================================
class Message;
class MessageDispatch;
class Object;
class PlayerObject;
class ObjectControllerCmdProperties;
class Database;
class ArtisanHeightmapAsyncContainer;
class CurrentResource;
class SurveyTool;
class UIWindow;

typedef struct tagResourceLocation
{
    glm::vec3		position;
    float			ratio;
} ResourceLocation;

class ArtisanManager : public ObjectFactoryCallback, public HeightMapCallBack, public UICallback
{
public:
    ArtisanManager();
    ~ArtisanManager();

    // inherited callbacks
    virtual void			heightMapCallback(HeightmapAsyncContainer *ref) {
        HeightmapArtisanHandler(ref);
    }
    void					HeightmapArtisanHandler(HeightmapAsyncContainer* ref);

    bool					handleRequestSurvey(Object* player,Object* target, Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool					handleRequestCoreSample(Object* player,Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool					handleSurvey(Object* player, Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);
    bool					handleSample(Object* player, Object* target,Message* message,ObjectControllerCmdProperties* cmdProperties);

    /**
    * Sets the proper flags for sample node recovery
    *
    * a distance check is done to see if we should recover the node
    *
    * @param PlayerObject* player
    *   player object that contains the sampling data
    * @return bool recoverFlag
    *   This returns true if the sample node will be recovered
    */
    bool					setupForNodeSampleRecovery(PlayerObject* player);
    /**
    * Sets up the sample UI Event
    *
    * The UI Event is handled internally
    *
    * @param PlayerObject* player
    *   player object that contains the sampling data
    * @param CurrentResource* resource
    *   resource that we are sampling
    * @param SurveyTool* tool
    *	current survey tool that we are using to sample
    * @return bool flag
    *   This returns true if the UI Event is setup properly
    */
    bool					setupSampleEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool);
    /**
    * gets the radioactive sample for the player
    *
    * We check the resource type to see if it's radioactive, then we will set the UI Event
    * handled internally
    *
    * @param PlayerObject* player
    *   player object that contains the sampling data
    * @param CurrentResource* resource
    *   resource that we are sampling
    * @param SurveyTool* tool
    *	current survey tool that we are using to sample
    * @return bool radioactiveFlag
    *   This returns true if the player is recovering radioactives
    */
    bool					getRadioactiveSample(PlayerObject* player, CurrentResource* resource, SurveyTool* tool);
    /**
    * Checks if the player should stop sampling or not.
    *
    * the player is updated if sampling is stopped
    *
    * @param PlayerObject* player
    *   player object that contains the sampling data
    * @param CurrentResource* resource
    *   resource that we are sampling
    * @param SurveyTool* tool
    *	current survey tool that we are using to sample
    * @return bool flag
    *   This returns true if sampling should stop
    */
    bool					stopSampling(PlayerObject* player, CurrentResource* resource, SurveyTool* tool);
    /**
    * Finishes sampling
    *
    * handles setting the xp and putting the resource in the inventory
    *
    * @param PlayerObject* player
    *   player object that contains the sampling data
    * @param CurrentResource* resource
    *   resource that we are sampling
    * @param SurveyTool* tool
    *	current survey tool that we are using to sample
    * @param uint32 sampleAmt
    *	the sample amount from sampling
    */
    void					finishSampling(PlayerObject* player, CurrentResource* resource, SurveyTool* tool, uint32 sampleAmt);

    //events
    void					sampleEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool);
    void					surveyEvent(PlayerObject* player, CurrentResource* resource, SurveyTool* tool);

private:
    ObjectFactoryCallback*					mObjectFactoryCallback;
    HeightMapCallBack*						mHeightMapCallback;
    UICallback*								mUICallback;
    void									handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);
    uint32									mSampleActionCost;
    uint32									mSurveyMindCost;
};

class ArtisanEvents : public ::common::BaseEvent
{
public:
    explicit ArtisanEvents(::common::ByteBuffer& in);
    explicit ArtisanEvents(uint64_t subject_id = 0, uint64_t delay_ms = 0);
    ArtisanEvents(uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback);

    ~ArtisanEvents();

    const ::common::EventType& event_type() const;

private:
    void onSerialize(::common::ByteBuffer& out) const;
    void onDeserialize(::common::ByteBuffer& in);

    bool onConsume(bool handled) const;

    static const ::common::EventType event_type_;

};
#endif