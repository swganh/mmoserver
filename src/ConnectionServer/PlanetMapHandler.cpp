/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PlanetMapHandler.h"
#include "ConnectionDispatch.h"
#include "ConnectionClient.h"
#include "ConnectionServerOpcodes.h"

#include "LogManager/LogManager.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"

//#include <stdio.h>
#include <assert.h>

//======================================================================================================================

PlanetMapHandler::PlanetMapHandler()
{

}


//======================================================================================================================

PlanetMapHandler::~PlanetMapHandler()
{

}


//======================================================================================================================

void PlanetMapHandler::Startup(Database* database, ConnectionDispatch* dispatch)
{
	mDatabase = database;
	mConnectionDispatch = dispatch;

	mConnectionDispatch->RegisterMessageCallback(opGetMapLocationsMessage,this);


  // We're going to build our databinding here.
  mDataBinding = mDatabase->CreateDataBinding(7);
  mDataBinding->addField(DFT_int64, offsetof(MapLocation, mId), 8);
  mDataBinding->addField(DFT_string, offsetof(MapLocation, mName), 64);
  mDataBinding->addField(DFT_float, offsetof(MapLocation, mX), 4);
  mDataBinding->addField(DFT_float, offsetof(MapLocation, mZ), 4);
  mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mCategory), 1);
  mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mSubCategory), 1);
  mDataBinding->addField(DFT_uint8, offsetof(MapLocation, mListIcon), 1);
}


//======================================================================================================================

void PlanetMapHandler::Shutdown()
{
  // Destroy our DataBinding object
  mDatabase->DestroyDataBinding(mDataBinding);

  // Unregister our message callbacks
	mConnectionDispatch->UnregisterMessageCallback(opGetMapLocationsMessage);
}


//======================================================================================================================

void PlanetMapHandler::Process()
{

}


//======================================================================================================================

void PlanetMapHandler::handleDispatchMessage(uint32 opcode, Message* message, ConnectionClient* client)
{
	switch(opcode)
	{
		case opGetMapLocationsMessage:
			_processMapLocationsRequest(message, client);
			break;

		default:
		{
			// Unhandled opcode
		//	int jack = opcode;
		//	int i = 0;
		}
		break;
	} //end switch(opcode)
}


//======================================================================================================================
void PlanetMapHandler::handleDatabaseJobComplete(void* ref, DatabaseResult* result)
{
	Message*	newMessage;
	uint64    locationsCount;

	MapLocation location;

	// Get our container back
	PlanetMapHandlerAsyncContainer* container = reinterpret_cast<PlanetMapHandlerAsyncContainer*>(ref);
	ConnectionClient* client = reinterpret_cast<ConnectionClient*>(container->mClient);

	if(!client)
	{
		delete container;
		return;
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opHeartBeat);
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelAUnreliable(newMessage, 1);

	locationsCount = result->getRowCount();

	// build our reply
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opGetMapLocationsResponseMessage);
	gMessageFactory->addString(container->mPlanetName);

	gMessageFactory->addUint32((uint32)locationsCount);

	for (uint32 i = 0; i < locationsCount; i++)
	{
		result->GetNextRow(mDataBinding, (void*)&location);

		gMessageFactory->addUint64(location.mId);

		string tempString;
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

	client->SendChannelA(newMessage, 8, false);

  // Destroy our DB objects
  delete (container);
}


//======================================================================================================================
void PlanetMapHandler::_processMapLocationsRequest(Message* message, ConnectionClient* client)
{
  PlanetMapHandlerAsyncContainer* container = new PlanetMapHandlerAsyncContainer();

  // Set our client object
  container->mClient = client;

	// get the requested planet
  message->getStringAnsi(container->mPlanetName);

  // Send our job in.

  mDatabase->ExecuteSqlAsync(this, (void*)container, "select planetmap.id,planetmap.name,planetmap.x,planetmap.z,planetmapcategory.main,planetmapcategory.sub,planetmap.icon from planetmap inner join planetmapcategory on(planetmap.category_id = planetmapcategory.id) where planetmap.planet_id = (SELECT planet.planet_id from planet WHERE planet.name='%s')", container->mPlanetName.getAnsi());
}

//======================================================================================================================


