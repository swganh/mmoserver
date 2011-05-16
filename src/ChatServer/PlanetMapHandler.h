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

#ifndef ANH_ADMINSERVER_PLANETMAPHANDLER_H
#define ANH_ADMINSERVER_PLANETMAPHANDLER_H

#include "DatabaseManager/DatabaseCallback.h"
#include "Utils/typedefs.h"
#include "Utils/bstring.h"


//======================================================================================================================
class Message;
class Database;
class DataBinding;
class MessageDispatch;
class DispatchClient;

class MapLocation
{
public:
    uint64		mId;
    int8  		mName[64];
    uint8		  mCategory;
    uint8		  mSubCategory;
    float		  mX;
    float		  mZ;
    uint8		  mListIcon;
};

class PlanetMapHandlerAsyncContainer
{
public:
    DispatchClient*     mClient;
    BString              mPlanetName;
};


//======================================================================================================================
class PlanetMapHandler : public DatabaseCallback
{
public:

    PlanetMapHandler(Database* database, MessageDispatch* dispatch);
    ~PlanetMapHandler();

    void			                    Process();

    // Inherited from DatabaseCallback
    virtual void                   handleDatabaseJobComplete(void* ref, DatabaseResult* result);

private:

    void                          _processMapLocationsRequest(Message* message, DispatchClient* client);


    Database*                     mDatabase;
    DataBinding*                  mDataBinding;
    MessageDispatch*              mMessageDispatch;
};


#endif


