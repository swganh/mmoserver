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

#ifndef ANH_CHATSERVER_PLAYER_H
#define ANH_CHATSERVER_PLAYER_H

#include <map>

#include "Utils/typedefs.h"
#include "Utils/bstring.h"

class DispatchClient;

//======================================================================================================================

typedef std::map<uint32,BString> ContactMap;

//======================================================================================================================

struct Bazaar
{
    uint64			ownerid;
    uint64			id;
    uint32			regionid;
    uint32			planetid;
    int8			string[128];
    int8			planet[32];
    int8			region[48];
};

struct PlayerData
{
    BString name;
    BString last_name;
};

class Player
{
public:
    Player(uint64 charId,DispatchClient* client,uint32 planetId)
        : mBazaar(nullptr)
        , mClient(client)
        , mCharId(charId)
        , mGroupId(0)
        , mKey(0)
        , mPlanetId(planetId)
        , mX(0)
        , mZ(0)
        , mGroupMemberIndex(0)
        , mAddPending(true)
    {}

    ~Player()
    {}

    uint64			getCharId() {
        return mCharId;
    }
    void			setCharId(uint64 charId) {
        mCharId = charId;
    }

    BString&		getName() {
        return mPlayerData.name;
    }
    void			setName(const BString name) {
        mPlayerData.name = name;
    }

    PlayerData*		getPlayerData() {
        return &mPlayerData;
    }
    BString&		getLastName() {
        return mPlayerData.last_name;
    }
    void			setLastName(const BString lastName) {
        mPlayerData.last_name = lastName;
    }

    DispatchClient*	getClient() {
        return mClient;
    }
    void			setClient(DispatchClient* client) {
        mClient = client;
    }

    uint32			getPlanetId() {
        return mPlanetId;
    }
    void			setPlanetId(uint32 planetId) {
        mPlanetId = planetId;
    }

    bool			getAddPending() {
        return mAddPending;
    }
    void			setAddPending(bool b) {
        mAddPending = b;
    }

    uint32			getKey() {
        return mKey;
    }
    void			setKey() {
        BString name = getName();
        name.toLower();
        mKey = name.getCrc();
    }

    ContactMap*		getFriendsList() {
        return &mFriendsList;
    }
    void			addFriend(BString name) {
        mFriendsList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
    }
    void			removeFriend(uint32 nameCrc);
    bool			checkFriend(uint32 nameCrc);

    ContactMap*		getIgnoreList() {
        return &mIgnoreList;
    }
    void			addIgnore(BString name) {
        mIgnoreList.insert(std::make_pair(name.getCrc(),name.getAnsi()));
    }
    void			removeIgnore(uint32 nameCrc);
    bool			checkIgnore(uint32 nameCrc);

    uint16			getGroupMemberIndex() {
        return mGroupMemberIndex;
    }
    void			setGroupMemberIndex(uint16 index) {
        mGroupMemberIndex = index;
    }

    uint64			getGroupId() {
        return mGroupId;
    }
    void			setGroupId(uint64 groupId) {
        mGroupId = groupId;
    }

    void			setPositionX(float position) {
        mX = position;
    }
    float			getPositionX() {
        return mX;
    }

    void			setBazaar(Bazaar* bazaar) {
        mBazaar = bazaar;
    }
    Bazaar*			getBazaar() {
        return mBazaar;
    }

    void			setPositionZ(float position) {
        mZ = position;
    }
    float			getPositionZ() {
        return mZ;
    }

private:
    ContactMap			mFriendsList,mIgnoreList;
    PlayerData  		mPlayerData;
    Bazaar*		    	mBazaar;
    DispatchClient*		mClient;
    uint64				mCharId;
    uint64				mGroupId;
    uint32				mKey;
    uint32				mPlanetId;
    float				mX;
    float				mZ;
    uint16				mGroupMemberIndex;
    bool				mAddPending;
};

#endif

