/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_MISSION_FACTORY_H
#define ANH_ZONESERVER_MISSION_FACTORY_H

#define		gMissionFactory	MissionFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;

//=============================================================================

class MissionFactory //: public FactoryBase
{
	public:

		static	MissionFactory*	getSingletonPtr() { return mSingleton; }
		static	MissionFactory*	Init(Database* database);

		~MissionFactory();

	private:
		
		MissionFactory();

		static MissionFactory*		mSingleton;
		static bool					mInsFlag;

};

//=============================================================================


#endif