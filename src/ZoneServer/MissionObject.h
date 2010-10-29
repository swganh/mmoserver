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

#ifndef ANH_ZONESERVER_MISSION_OBJECT_H
#define ANH_ZONESERVER_MISSION_OBJECT_H

#include "Object.h"

class PlayerObject;
class NPCObject;
class WaypointObject;
class Terminal;
class ResourceType;
class Buff;

//=============================================================================

typedef struct
{
    glm::vec3	Coordinates;
    uint64				CellID;
    uint32				PlanetCRC;
} Location;



enum mission_types
{
    destroy	= 	0x74EF9BE3,
    deliver =	0xE5C27EC6, //crc = deliver
    recon	= 	0x34F4C2E4,
    hunting	= 	0x906999A2,
    musician = 	0x4AD93196,
    dancer	=	0x0F067B37,
    crafting =   0xE5F6DC59,
    survey	= 	0x19C9FAC1,
    bountry	= 	0x2904F372
};


class MissionObject : public Object
{
    friend class ObjectFactory;

public:

    MissionObject();
    MissionObject(PlayerObject * owner, uint64 parent_id);
    ~MissionObject();

    void				clear();
    bool				check(uint64 callTime,void*);
    void				sendAttributes(PlayerObject* playerObject);


    PlayerObject*		getOwner() {
        return mOwner;
    }
    void				setOwner(PlayerObject* owner) {
        mOwner = owner;
    }
    Terminal*			getIssuingTerminal() {
        return mIssuingTerminal;
    }
    void				setIssuingTerminal(Terminal* issuing_terminal) {
        mIssuingTerminal = issuing_terminal;
    }

    BString 				getNameFile() {
        return mNameFile;
    }
    void				setNameFile(const char* name_file) {
        mNameFile = name_file;
    }
    BString				getName() {
        return mName;
    }
    void				setName(const char* name) {
        mName = name;
    }
    BString				getTitleFile() {
        return mTitleFile;
    }
    void				setTitleFile(const char* title_file) {
        mTitleFile = title_file;
    }
    BString				getTitle() {
        return mTitle;
    }
    void				setTitle(const char* title) {
        mTitle = title;
    }

    BString				getTarget() {
        return mTarget;
    }
    void				setTarget(const char* target) {
        mTarget = target;
    }
    uint32				getTargetModel() {
        return mTargetModel;
    }
    void				setTargetModel(uint32 target_model) {
        mTargetModel = target_model;
    }
    ResourceType*		getTargetResource() {
        return mTargetResource;
    }
    void				setTargetResource(ResourceType* target) {
        mTargetResource = target;
    }

    Location 			getStart() {
        return mStart;
    }
    void				setStart(Location mission_start) {
        mStart = mission_start;
    }
    Location			getDestination() {
        return mDestination;
    }
    void				setDestination(Location mission_end) {
        mDestination = mission_end;
    }

    int					getReward() {
        return mReward;
    }
    void				setReward(int reward) {
        mReward = reward;
    }

    int					getDifficulty() {
        return mDifficulty;
    }
    void				setDifficulty(int difficulty) {
        mDifficulty = difficulty;
    }

    BString				getCreator() {
        return mCreator;
    }
    void				setCreator(const char* creator) {
        mCreator = creator;
    }

    BString				getDetailFile() {
        return mDetailFile;
    }
    void				setDetailFile(const char* detail_file) {
        mDetailFile = detail_file;
    }
    BString				getDetail()	{
        return mDetail;
    }
    void				setDetail(const char* detail) {
        mDetail = detail;
    }

    int					getRefreshCount() {
        return mRefreshCount;
    }
    int					setRefreshCount(int refresh_count) {
        mRefreshCount = refresh_count;
        return mRefreshCount;
    }

    uint32				getMissionType() {
        return mMissionType;
    }
    void				setMissionType(uint32 mission_type) {
        mMissionType = mission_type;
    }

    WaypointObject*		getWaypoint() {
        return mWaypoint;
    }
    void				setWaypoint(WaypointObject* mission_waypoint) {
        mWaypoint = mission_waypoint;
    }

    int					getNum() {
        return mNum;
    }
    void				setNum(int new_num) {
        mNum = new_num;
    }

    NPCObject*			getStartNPC() {
        return mStartNPC;
    }
    NPCObject*			getDestinationNPC()   {
        return mDestinationNPC;
    }
    void				setStartNPC(NPCObject* npc) {
        mStartNPC = npc;
    }
    void				setDestinationNPC(NPCObject* npc)   {
        mDestinationNPC = npc;
    }

    Buff*				getEntertainingTimer() {
        return mEntertainingTimer;
    }
    void				setEntertainingTimer(Buff* timer) {
        mEntertainingTimer = timer;
    }
    bool				getInProgress()  {
        return mInProgress;
    }
    void				setInProgress(bool is) {
        mInProgress = is;
    }

    uint64				getTaskId() {
        return mTaskId;
    }
    void				setTaskId(uint64 task_id) {
        mTaskId = task_id;
    }


protected:

    PlayerObject*			mOwner;					//Player Object that this mission belongs too
    Terminal*				mIssuingTerminal;       //Terminal that issued this mission

    BString					mNameFile;				//ASCII
    BString					mName;					//ASCII
    BString					mTitleFile;				//ASCII
    BString					mTitle;					//ASCII

    BString					mTarget;				//'@stf:name' format - ASCII
    uint32					mTargetModel;			//CRC of .iff
    ResourceType*			mTargetResource;		/* Type of resource we must survey for. The "Diffaculty"
														   variable is the needed efficiency.                 */
    Location				mStart;					//Start Location
    Location				mDestination;			//End Location

    int						mReward;				//Credit amount to reward upon mission complete
    int						mDifficulty;            //Diffaculty\Efficiency
    BString					mCreator;				//UNICODE

    BString					mDetailFile;			//ASCII
    BString					mDetail;				//Description - ASCII

    int						mRefreshCount;			//Counter Provided By the Terminal
    uint32					mMissionType;			//CRC Value

    bool					external;				//if true mission was not assigned an id via id_mask
    WaypointObject*			mWaypoint;

    int						mNum;					//The mission entry in the stf file ex: m3t num=3

    NPCObject*				mStartNPC;
    NPCObject*				mDestinationNPC;

    Buff*					mEntertainingTimer;		//The buff used for entertainer missions
    bool					mInProgress;			//set to true when the player starts the buff used in entertainer missions

    uint64					mTaskId;

private:

    inline uint64 getNewMissionId(uint16 * id_mask, uint64 player_id) {
        for(int i=0; i<16; ++i)
            if(!(*id_mask & (1 << i))) {
                *id_mask |= (1 << i);
                return (player_id + 700) + (i << 1);
            }
        return 0;
    }
    inline void   freeMissionId(uint16 * id_mask, uint64 player_id) {
        *id_mask &= ~(1<<((this->mId-(player_id + 700))>>1));
    }


};

//=============================================================================

#endif

