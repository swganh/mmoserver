/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_SAMPLE_EVENT_H
#define ANH_ZONESERVER_SAMPLE_EVENT_H

#include "Utils/EventHandler.h"
#include "ArtisanManager.h"

struct SampleNode
{
    glm::vec3           Position;
    uint32				zone;
    CurrentResource*	resource;
    SurveyTool*			tool;
    PlayerObject*		player;

    uint64				mNextSampleTime;

    bool				mPassRadioactive;
    bool				mPendingSample;
    bool				mPendingSurvey;


    bool				mSampleEventFlag;
    bool				mSampleGambleFlag;
    bool				mSampleNodeFlag;
    bool				mSampleNodeRecovery;

};

//======================================================================================================================

class SampleEvent : public Anh_Utils::Event
{
public:

    SampleEvent(PlayerObject* player,SurveyTool* tool,CurrentResource* resource) : mPlayer(player), mTool(tool),mResource(resource) {}

    PlayerObject*		getPlayer() const {
        return mPlayer;
    }
    SurveyTool*			getTool() const {
        return mTool;
    }
    CurrentResource*	getResource() const {
        return mResource;
    }

private:
    PlayerObject*		mPlayer;
    SurveyTool*			mTool;
    CurrentResource*	mResource;
};


//======================================================================================================================

#endif



