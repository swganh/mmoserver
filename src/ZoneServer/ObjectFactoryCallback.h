/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_OBJECTFACTORYCALLBACK_H
#define ANH_ZONESERVER_OBJECTFACTORYCALLBACK_H

#include "Utils/typedefs.h"

class Object;
class DispatchClient;

//======================================================================================================================

class ObjectFactoryCallback
{
public:
	ObjectFactoryCallback();
	virtual ~ObjectFactoryCallback();

	virtual void	handleObjectReady(Object* object,DispatchClient* client) {};
	virtual void	handleObjectReady(Object* object) {};
};

#endif

