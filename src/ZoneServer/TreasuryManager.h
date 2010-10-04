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
#include "UIManager.h"
#include "UICallback.h"
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
enum GalaxyBankAccount
{
    Account_CharacterCreation			=	1,
    Account_NewbieTutorial				=	2,
    Account_CustomerService				=	3,
    Account_DynamicMissionSystem		=	4,
    Account_PlayerMissionSystem			=	5,
    Account_BountyMissionSystem			=	6,
    Account_CloningSystem				=	7,
    Account_InsuranceSystem				=	8,
    Account_GalacticTravelCommission	=	9,
    Account_GalacticShippingCommission	=	10,
    Account_GalacticTradeCommission		=	11,
    Account_DispenserSystem				=	12,
    Account_SkillTrainingUnion			=	13,
    Account_Rebellion					=	14,
    Account_Empire						=	15,
    Account_JabbaTheHutt				=	16,
    Account_POISystem					=	17,
    Account_CorpseExpirationTracking	=	18,
    Account_Testing						=	19,
    Account_StructureMaintenance		=	20,
    Account_TipSurcharge				=	21,
    Account_VendorWages					=	22,
    Account_NPCLoot						=	23,
    Account_JunkDealer					=	24,
    Account_CantinaDrink				=	25,
    Account_BetaTestFund				=	26,
    Account_GroupSplitErrorAccount		=	27,
    Account_StandardSlotMachineAccount	=	28,
    Account_RouletteAccount				=	29,
    Account_SabaccAccount				=	30,
    Account_VehicleRepairSystem			=	31,
    Account_RelicDealer					=	32,
    Account_NewPlayerQuests				=	33,
    Account_ContrabandScanningFines		=	34,
    Account_Bank						=	35

};

enum TREMQueryType
{
    TREMQuery_NULL							=	0,
    TREMQuery_BankTipgetId					=	1,
    TREMQuery_BankTipTransaction			=	2,
    TREMQuery_BankTipUpdateGalaxyAccount	=	3

};

//======================================================================================================================

class TreasuryManagerAsyncContainer
{

public:

    TreasuryManagerAsyncContainer(TREMQueryType qt,DispatchClient* client) {
        mQueryType = qt;
        mClient = client;
    }
    ~TreasuryManagerAsyncContainer() {}

    TREMQueryType		mQueryType;
    DispatchClient*		mClient;

    uint32				amount;
    uint32				surcharge;
    PlayerObject*		player;
    PlayerObject*		target;
    uint64				targetId;
    BString				targetName;
};

//======================================================================================================================

class TreasuryManager : public DatabaseCallback, UICallback
{
public:

    TreasuryManager(Database* database);
    ~TreasuryManager();

    static TreasuryManager*		    Init(Database* database);
    static TreasuryManager*		    getSingletonPtr() {
        return mSingleton;
    }

    virtual void				    handleDatabaseJobComplete(void* ref,DatabaseResult* result);
    virtual void					handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window);

    void						    bankDepositAll(PlayerObject* playerObject);
    void						    bankWithdrawAll(PlayerObject* playerObject);
    void						    bankTransfer(int32 inventoryMoneyDelta, int32 bankMoneyDelta, PlayerObject* playerObject);
    std::shared_ptr<RadialMenu>   bankBuildTerminalRadialMenu(CreatureObject* creatureObject);
    void						    bankOpenSafetyDepositContainer(PlayerObject* playerObject);
    void						    bankQuit(PlayerObject* playerObject);
    void						    bankJoin(PlayerObject* playerObject);

    void						    saveAndUpdateInventoryCredits(PlayerObject* playerObject);
    void						    saveAndUpdateBankCredits(PlayerObject* playerObject);

    //handles bank tip to offline player
    void							handleBankTipSurchargeConfirmed(TreasuryManagerAsyncContainer* asyncContainer);
    void						    bankTipOffline(int32 amount, PlayerObject* playerObject, BString targetName);
    void						    inventoryTipOnline(int32 amount, PlayerObject* playerObject, PlayerObject* targetObject );
    void						    bankTipOnline(int32 amount, PlayerObject* playerObject, PlayerObject* targetObject );

private:

    static bool					mInsFlag;
    static TreasuryManager*		mSingleton;
    Database*					mDatabase;
};

#endif
