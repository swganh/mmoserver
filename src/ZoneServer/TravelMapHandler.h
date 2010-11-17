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

#ifndef ANH_ZONESERVER_TRAVELMAPHANDLER_H
#define ANH_ZONESERVER_TRAVELMAPHANDLER_H

#include "UICallback.h"
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
class DispatchClient;

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

    TravelMapAsyncContainer(TMQueryType qt) {
        mQueryType = qt;
    }
    ~TravelMapAsyncContainer() {}

    TMQueryType	mQueryType;
};

//======================================================================================================================

class TravelPoint
{
public:

    TravelPoint() {}
    ~TravelPoint() {}

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

    TravelRoute() {}
    ~TravelRoute() {}

    uint16	srcId;
    uint16	destId;
    int32	price;
};

//======================================================================================================================

class TicketProperties
{
public:

    TicketProperties() {}
    ~TicketProperties() {}

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

class TravelMapHandler : public DatabaseCallback, public UICallback
{
public:

    static TravelMapHandler*	getSingletonPtr() {
        return mSingleton;
    }
    static TravelMapHandler*	Init(Database* database,MessageDispatch* dispatch,uint32 zoneId);

    ~TravelMapHandler();

    void				Shutdown();

    void				getTicketInformation(BStringVector vQuery,TicketProperties* ticketProperties);
    TravelPoint*		getTravelPoint(uint16 planetId,BString name);

    virtual void		handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    bool				findTicket(PlayerObject* player, BString port);
    void				createTicketSelectMenu(PlayerObject* playerObject, Shuttle* shuttle, BString port);
    void				useTicket(PlayerObject* player, TravelTicket* ticket,Shuttle* shuttle);

    virtual void		handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);

private:

    TravelMapHandler(Database* database,MessageDispatch* dispatch,uint32 zoneId);

    void				_processTravelPointListRequest(Message* message,DispatchClient* client);
    void				_processTutorialTravelList(Message* message, DispatchClient* client);

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


