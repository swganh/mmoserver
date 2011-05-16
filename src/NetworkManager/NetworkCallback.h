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

#ifndef ANH_NETWORKMANAGER_NETWORKCALLBACK_H
#define ANH_NETWORKMANAGER_NETWORKCALLBACK_H

//======================================================================================================================

class Session;
class Service;
class NetworkClient;
class Message;

//======================================================================================================================

class NetworkCallback
{
public:

    virtual NetworkClient*  handleSessionConnect(Session* session, Service* service)            {
        return (NetworkClient*)-1;
    };
    virtual void            handleSessionDisconnect(NetworkClient* client)                      {};
    virtual void            handleSessionMessage(NetworkClient* client, Message* message)       {};

private:
};

//======================================================================================================================

#endif //ANH_NETWORKMANAGER_NETWORKCALLBACK_H



