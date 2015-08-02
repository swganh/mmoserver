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

#ifndef ANH_CHATSERVER_CHANNEL_H
#define ANH_CHATSERVER_CHANNEL_H

#include <map>
#include <vector>

#include "Utils/typedefs.h"
#include "Utils/bstring.h"

class BString;
class ChatAvatarId;
class Player;

//======================================================================================================================

typedef std::map<uint32,ChatAvatarId*>  AvatarNameMap;
typedef std::vector<ChatAvatarId*>      ChatAvatarIdList;
typedef std::pair<uint32, BString>     CrcStringPair;
typedef std::map<uint32, BString>      NameByCrcMap;

//======================================================================================================================

struct ChannelData
{
    ChannelData()
        : is_private(0)
        , is_moderated(0)
    {}

    BString	name;
    BString	title;
    uint32	id;
    uint8		is_private;
    uint8		is_moderated;
};

//======================================================================================================================

class Channel
{
public:
    Channel();
    ~Channel();

    ChannelData* getChannelData();

    uint32 getId() const;
    void setId(uint32 id);

    BString getName() const;
    void setName(const BString name);

    BString getFullPath() const;

    BString getGalaxy() const;
    void setGalaxy(const BString galaxy);

    BString getTitle() const;
    void setTitle(const BString title);

    ChatAvatarId* getOwner() const;
    void setOwner(ChatAvatarId* owner);

    ChatAvatarId* getCreator() const;
    void setCreator(ChatAvatarId* creator);

    uint8 isPrivate() const;
    void setPrivate(uint8 priv);

    uint8 isModerated() const;
    void setModerated(uint8 moderated);

    void addUser(ChatAvatarId* avatar);
    void removeUser(Player* player);

    ChatAvatarId*	removeUser(BString name);
    ChatAvatarId*	findUser(BString name);
    ChatAvatarId*	findUser(Player* player);

    void addInvitedUser(BString name);

    BString removeInvitedUser(BString name);
    bool isInvited(BString name) const;

    void addModerator(BString name);
    BString removeModerator(BString name);
    bool isModerator(BString name) const;
    bool isOwner(BString name) const;

    void banUser(BString name);
    BString unBanUser(BString name);
    bool isBanned(BString name) const;

    void clearChannel();

    ChatAvatarIdList* getUserList();
    NameByCrcMap* getModeratorList();
    NameByCrcMap* getBanned();
    NameByCrcMap* getInvited();

private:
    ChannelData				mChannelData;
    ChatAvatarId*			mOwner;
    ChatAvatarId*			mCreator;
    BString						mGalaxy;
    ChatAvatarIdList	mUsers;
    AvatarNameMap			mUserMap;
    NameByCrcMap			mModerators;
    NameByCrcMap			mBanned;
    NameByCrcMap			mInvited;
};

#endif

