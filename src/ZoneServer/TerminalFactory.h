/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TERMINAL_OBJECT_FACTORY_H
#define ANH_ZONESERVER_TERMINAL_OBJECT_FACTORY_H

#include "FactoryBase.h"

#define 	gTerminalFactory	TerminalFactory::getSingletonPtr()

//=============================================================================

class Database;
class DataBinding;
class DispatchClient;
class ObjectFactoryCallback;
class Terminal;

//=============================================================================

enum TFQuery
{
	TFQuery_MainData		= 1,
	TFQuery_ElevatorData	= 2
};

//=============================================================================

class TerminalFactory : public FactoryBase
{
public:

	static TerminalFactory*	getSingletonPtr() { return mSingleton; }
	static TerminalFactory*	Init(Database* database);

	~TerminalFactory();

	void			handleDatabaseJobComplete(void* ref,DatabaseResult* result);
	void			requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client);

private:

	TerminalFactory(Database* database);

	void			_setupDatabindings();
	void			_destroyDatabindings();

	Terminal*		_createTerminal(DatabaseResult* result);

	static TerminalFactory*		mSingleton;
	static bool					mInsFlag;

	DataBinding*				mBazaarMainDataBinding;
	DataBinding*				mCloningMainDataBinding;
	DataBinding*				mInsuranceMainDataBinding;
	DataBinding*				mCharacterBuilderMainDataBinding;
	DataBinding*				mMissionMainDataBinding;
	DataBinding*				mTravelMainDataBinding;
	DataBinding*				mBankMainDataBinding;
	DataBinding*				mElevatorMainDataBinding;
	DataBinding*				mElevetorDataUpBinding;
	DataBinding*				mElevetorDataDownBinding;
};


//=============================================================================


#endif

