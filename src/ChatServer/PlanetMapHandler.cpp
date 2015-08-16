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

#include "PlanetMapHandler.h"
#include "ChatOpcodes.h"



#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

//#include <stdio.h>
#include <assert.h>

//======================================================================================================================

PlanetMapHandler::PlanetMapHandler(Database* database, MessageDispatch* dispatch)
{
    mDatabase = database;
    mMessageDispatch = dispatch;

    mMessageDispatch->RegisterMessageCallback(opGetMapLocationsMessage,std::bind(&PlanetMapHandler::_processMapLocationsRequest, this, std::placeholders::_1, std::placeholders::_2));


    // We're going to build our databinding here.
    mDataBinding = mDatabase->createDataBinding(7);
    mDataBinding->addField(DFT_int64, offsetof(MapLocation, mId), 8);
    mDataBinding->addField(DFT_string, offsetof(MapLocation, mName), 64);
    mDataBinding->addField(DFT_float, offsetof(MapLocation, mX), 4);
    mDataBinding->addField(DFT_float, offsetof(MapLocation, mZ), 4);
    mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mCategory), 1);
    mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mSubCategory), 1);
    mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mListIcon), 1);
}


//======================================================================================================================

PlanetMapHandler::~PlanetMapHandler()
{
    // Destroy our DataBinding object
    mDatabase->destroyDataBinding(mDataBinding);

    // Unregister our message callbacks
    mMessageDispatch->UnregisterMessageCallback(opGetMapLocationsMessage);
}

//======================================================================================================================

void PlanetMapHandler::Process()
{

}

//======================================================================================================================
void PlanetMapHandler::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
    Message*	newMessage;
    uint64    locationsCount;

    MapLocation location;

    // Get our container back
    PlanetMapHandlerAsyncContainer* container = reinterpret_cast<PlanetMapHandlerAsyncContainer*>(ref);
    DispatchClient* client = reinterpret_cast<DispatchClient*>(container->mClient);

    if(!client)
    {
        delete container;
        return;
    }

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opHeartBeat);
    newMessage = gMessageFactory->EndMessage();

    client->SendChannelAUnreliable(newMessage, client->getAccountId(), CR_Client, 1);

    locationsCount = result->getRowCount();

    // build our reply
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opGetMapLocationsResponseMessage);
    gMessageFactory->addString(container->mPlanetName);

    gMessageFactory->addUint32((uint32)locationsCount);

    for (uint32 i = 0; i < locationsCount; i++)
    {
        result->getNextRow(mDataBinding, (void*)&location);

        gMessageFactory->addUint64(location.mId);

        BString tempString;
        tempString = location.mName;
        tempString.convert(BSTRType_Unicode16);
        gMessageFactory->addString(tempString);

        gMessageFactory->addFloat(location.mX);
        gMessageFactory->addFloat(location.mZ);
        gMessageFactory->addUint8(location.mCategory);
        gMessageFactory->addUint8(location.mSubCategory);
        gMessageFactory->addUint8(location.mListIcon);
    }

    gMessageFactory->addUint64(0);  // next location id , must be 0 for last item

    // listing footer
    gMessageFactory->addUint64(0); // unknown
    gMessageFactory->addUint32(0); // unknown
    gMessageFactory->addUint8(0);  // unknown

    newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 8);

    // Destroy our DB objects
    delete (container);
}


//======================================================================================================================
void PlanetMapHandler::_processMapLocationsRequest(Message* message, DispatchClient* client)
{
    PlanetMapHandlerAsyncContainer* container = new PlanetMapHandlerAsyncContainer();

    // Set our client object
    container->mClient = client;

    // get the requested planet
    message->getStringAnsi(container->mPlanetName);

    // Send our job in.
    mDatabase->executeProcedureAsync(this, (void*)container, "CALL %s.sp_PlanetaryMapLocations('%s')", mDatabase->galaxy(),  container->mPlanetName.getAnsi());
    
}

//======================================================================================================================


