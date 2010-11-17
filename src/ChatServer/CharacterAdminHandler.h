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

#ifndef SRC_CHATSERVER_CHARACTERADMINHANDLER_H_
#define SRC_CHATSERVER_CHARACTERADMINHANDLER_H_

#include "Utils/typedefs.h"
#include "Utils/bstring.h"


class Message;
class Database;
class DatabaseResult;
class MessageDispatch;
class DispatchClient;


struct CharacterCreateInfo {
    uint32        mAccountId;
    uint64        mCharacterId;
    BString        mFirstName;
    BString        mLastName;
    BString        mBaseModel;
    BString        mProfession;
    BString        mStartCity;
    float         mScale;
    BString        mBiography;
    uint16        mAppearance[0x255];
    uint16        mHairCustomization[0x71];
    BString        mHairModel;
    BString        mHairColor;
};


class CharacterAdminHandler {
public:

    CharacterAdminHandler(Database* database, MessageDispatch* dispatch);
    ~CharacterAdminHandler();

    void Process();

private:

    void _processRandomNameRequest(Message* message, DispatchClient* client);
    void _processCreateCharacter(Message* message, DispatchClient* client);
    void _parseAppearanceData(Message* message, CharacterCreateInfo* info);
         
    void _parseHairData(Message* message, CharacterCreateInfo* info);
         
    void _sendCreateCharacterSuccess(uint64 characterId, DispatchClient* client);
    void _sendCreateCharacterFailed(uint32 errorCode, DispatchClient* client);

    Database* database_;
    MessageDispatch* message_dispatch_;
};


#endif // SRC_CHATSERVER_CHARACTERADMINHANDLER_H_

