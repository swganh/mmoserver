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

#ifndef ANH_ZONESERVER_TREASURYMANAGER_H
#define ANH_ZONESERVER_TREASURYMANAGER_H

#define 	gTreasuryManager	TreasuryManager::getSingletonPtr()

#include "RadialMenu.h"

#include "Utils/typedefs.h"
#include "DatabaseManager/DatabaseCallback.h"

#if defined(__GNUC__)
// GCC implements tr1 in the <tr1/*> headers. This does not conform to the TR1
// spec, which requires the header without the tr1/ prefix.
#include <tr1/memory>
#else
#include <memory>
#endif

//======================================================================================================================

class CreatureObject;
class Database;
class DatabaseCallback;
class DatabaseResult;
class DispatchClient;
class PlayerObject;

//======================================================================================================================

enum TREMQueryType
{
	TREMQuery_NULL					=	0,
	TREMQuery_BankTipgetId			=	1,
	TREMQuery_BankTipTransaction	=	2

};

//======================================================================================================================

class TreasuryManagerAsyncContainer
{

	public:

		TreasuryManagerAsyncContainer(TREMQueryType qt,DispatchClient* client){ mQueryType = qt; mClient = client; }
		~TreasuryManagerAsyncContainer(){}

		TREMQueryType		mQueryType;
		DispatchClient*		mClient;

		uint32				amount;
		PlayerObject*		player;
		PlayerObject*		target;
		uint64				targetId;
		string				targetName;
};

//======================================================================================================================

class TreasuryManager : public DatabaseCallback
{
	public:

		TreasuryManager(Database* database);
		~TreasuryManager();

		static TreasuryManager*		    Init(Database* database);
		static TreasuryManager*		    getSingletonPtr() { return mSingleton; }

		virtual void				    handleDatabaseJobComplete(void* ref,DatabaseResult* result);

		void						    bankDepositAll(PlayerObject* playerObject);
		void						    bankWithdrawAll(PlayerObject* playerObject);
		void						    bankTransfer(int32 inventoryMoneyDelta, int32 bankMoneyDelta, PlayerObject* playerObject);
        std::tr1::shared_ptr<RadialMenu>   bankBuildTerminalRadialMenu(CreatureObject* creatureObject);
		void						    bankOpenSafetyDepositContainer(PlayerObject* playerObject);
		void						    bankQuit(PlayerObject* playerObject);
		void						    bankJoin(PlayerObject* playerObject);

		void						    saveAndUpdateInventoryCredits(PlayerObject* playerObject);
		void						    saveAndUpdateBankCredits(PlayerObject* playerObject);

		//handles bank tip to offline player
		void						    bankTipOffline(int32 amount, PlayerObject* playerObject, string targetName);
		void						    inventoryTipOnline(int32 amount, PlayerObject* playerObject, PlayerObject* targetObject );
		void						    bankTipOnline(int32 amount, PlayerObject* playerObject, PlayerObject* targetObject );

	private:

		static bool					mInsFlag;
		static TreasuryManager*		mSingleton;
		Database*					mDatabase;
};

#endif
