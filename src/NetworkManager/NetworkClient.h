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

#ifndef ANH_NETWORKMANAGER_CLIENT_H
#define ANH_NETWORKMANAGER_CLIENT_H

#include "Utils/typedefs.h"

//======================================================================================================================

class Session;
class Message;

// Defines for common cluster routes.  Can't enum since instances will have arbitrary Ids.
#define CR_Client            0
#define CR_Connection        1
#define CR_Unused1           2
#define CR_Unused2           3
#define CR_Unused3           4
#define CR_Admin             5
#define CR_Chat              6
#define CR_Unused4           7
#define CR_Corellia          8
#define CR_Dantooine         9
#define CR_Dathomir         10
#define CR_Endor            11
#define CR_Lok              12
#define CR_Naboo            13
#define CR_Rori             14
#define CR_Talus            15
#define CR_Tatooine         16
#define CR_Yavin4           17

//======================================================================================================================

class NetworkClient
{
public:

    NetworkClient(void) : mSession(0) {}

    virtual ~NetworkClient(void) {};

    virtual void	SendChannelA(Message* message, uint8 priority, bool fastpath);
    virtual void	SendChannelAUnreliable(Message* message, uint8 priority);
    virtual void	Disconnect(uint8 reason);

    Session*		getSession(void) {
        return mSession;
    }
    void			setSession(Session* session) {
        mSession = session;
    }

protected:

    Session*	mSession;
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_CLIENT_H



