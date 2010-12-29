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

#ifndef ANH_ZONESERVER_CHARACTERLOGINHANDLER_H
#define ANH_ZONESERVER_CHARACTERLOGINHANDLER_H

#include "ObjectFactoryCallback.h"
#include "DatabaseManager/DatabaseCallback.h"

#include <glm/glm.hpp>
#include <set>

//======================================================================================================================

class Message;
class Database;
class MessageDispatch;
class PlayerObject;

//======================================================================================================================


typedef std::set<uint64>				ObjectIDSet;

enum CLHCallBack
{
	CLHCallBack_None				= 0,
	CLHCallBack_Transfer_Ticket		= 1,
	CLHCallBack_Transfer_Position	= 2
};

class CharacterLoadingContainer
{
public:

	DispatchClient*				mClient;
	ObjectFactoryCallback*		ofCallback;
	DatabaseCallback*			dbCallback;
	uint64						mPlayerId;
	glm::vec3			        destination;
	uint16						planet;
	PlayerObject*				player;
	CLHCallBack					callBack;
};

class CharacterLoginHandler : public ObjectFactoryCallback, public DatabaseCallback
{
	public:

        CharacterLoginHandler(Database* database, MessageDispatch* dispatch);
        ~CharacterLoginHandler(void);

		// DatabaseCallback
		virtual void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		  // ObjectFactoryCallback
		virtual void	handleObjectReady(Object* object,DispatchClient* client);

	private:
        void    _processCmdSceneReady(Message* message, DispatchClient* client);
        void	_processSelectCharacter(Message* message, DispatchClient* client);
        void	_processNewbieTutorialResponse(Message* message, DispatchClient* client);
		void    _processClusterClientDisconnect(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferApprovedByTicket(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferApprovedByPosition(Message* message, DispatchClient* client);
		void    _processClusterZoneTransferDenied(Message* message, DispatchClient* client);

		Database*					mDatabase;
		MessageDispatch*			mMessageDispatch;

		uint32						mZoneId;
        
		ObjectIDSet					playerZoneList;
};




#endif // ANH_ZONESERVER_CHARACTERLOGINHANDLER_H


