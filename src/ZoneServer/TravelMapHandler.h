/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TRAVELMAPHANDLER_H
#define ANH_ZONESERVER_TRAVELMAPHANDLER_H

#include "UICallback.h"
#include "Common/MessageDispatchCallback.h"
#include "DatabaseManager/DatabaseCallback.h"
#include <boost/pool/pool.hpp>

#include <vector>

#define 	gTravelMapHandler	TravelMapHandler::getSingletonPtr()

//======================================================================================================================

class Database;
class Message;
class MessageDispatch;
class PlayerObject;
class Shuttle;
class TravelMapAsyncContainer;
class TravelTicket;

//======================================================================================================================

enum TMQueryType
{
	TMQuery_PointsInWorld	=	1,
	TMQuery_PointsInCells	=	2,
	TMQuery_PlanetRoutes	=	3
};

//======================================================================================================================

class TravelMapAsyncContainer
{
	public:

		TravelMapAsyncContainer(TMQueryType qt){ mQueryType = qt;}
		~TravelMapAsyncContainer(){}

		TMQueryType	mQueryType;
};

//======================================================================================================================

class TravelPoint
{
	public:

		TravelPoint(){}
		~TravelPoint(){}

		float	x;
		float	y;
		float	z;
		uint8	portType;
		uint32	taxes;
		int8	descriptor[64];
		uint16	planetId;
		float	spawnX;
		float	spawnY;
		float	spawnZ;
};

//======================================================================================================================

class TravelRoute
{
	public:

		TravelRoute(){}
		~TravelRoute(){}

		uint16	srcId;
		uint16	destId;
		int32	price;
};

//======================================================================================================================

class TicketProperties
{
	public:

		TicketProperties(){}
		~TicketProperties(){}

		uint16			srcPlanetId;
		uint16			dstPlanetId;
		TravelPoint*	srcPoint;
		TravelPoint*	dstPoint;
		int32			price;
};

//======================================================================================================================

typedef std::vector<TravelPoint*> TravelPointList;
typedef std::vector<std::pair<uint16,int32> > TravelRoutes;

//======================================================================================================================

class TravelMapHandler : public MessageDispatchCallback, public DatabaseCallback, public UICallback
{
public:

		static TravelMapHandler*	getSingletonPtr() { return mSingleton; }
		static TravelMapHandler*	Init(Database* database,MessageDispatch* dispatch,uint32 zoneId);

		~TravelMapHandler();

		void				Shutdown();

		void				getTicketInformation(BStringVector vQuery,TicketProperties* ticketProperties);
		TravelPoint*		getTravelPoint(uint16 planetId,string name);

		virtual void		handleDispatchMessage(uint32 opcode,Message* message,DispatchClient* client);
		virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);
		bool				findTicket(PlayerObject* player, string port);
		void				createTicketSelectMenu(PlayerObject* playerObject, Shuttle* shuttle, string port);
		void				useTicket(PlayerObject* player, TravelTicket* ticket,Shuttle* shuttle);

		virtual void		handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

	private:

		TravelMapHandler(Database* database,MessageDispatch* dispatch,uint32 zoneId);

		void				_processTravelPointListRequest(Message* message,DispatchClient* client);

		static TravelMapHandler*	mSingleton;
		static bool					mInsFlag;

		boost::pool<>					mDBAsyncPool;
		TravelPointList				mTravelPoints[50];
		TravelRoutes					mTravelRoutes[50];
		Database*							mDatabase;
		MessageDispatch*			mMessageDispatch;
		uint32								mPointCount;
		uint32								mRouteCount;
		uint32								mZoneId;
		bool									mCellPointsLoaded;
		bool									mRoutesLoaded;
		bool									mWorldPointsLoaded;
};

//======================================================================================================================

#endif


