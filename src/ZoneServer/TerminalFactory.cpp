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
#include "TerminalFactory.h"

#include "BankTerminal.h"
#include "PlayerStructureTerminal.h"
#include "BazaarTerminal.h"
#include "CharacterBuilderTerminal.h"
#include "CloningTerminal.h"
#include "ElevatorTerminal.h"
#include "InsuranceTerminal.h"
#include "MissionTerminal.h"
#include "ObjectFactoryCallback.h"
#include "TravelTerminal.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"

#include "Utils/utils.h"

//=============================================================================

bool				TerminalFactory::mInsFlag    = false;
TerminalFactory*	TerminalFactory::mSingleton  = NULL;

//======================================================================================================================

TerminalFactory*	TerminalFactory::Init(Database* database)
{
	if(!mInsFlag)
	{
		mSingleton = new TerminalFactory(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//=============================================================================

TerminalFactory::TerminalFactory(Database* database) : FactoryBase(database)
{
	_setupDatabindings();
}

//=============================================================================

TerminalFactory::~TerminalFactory()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

//=============================================================================

void TerminalFactory::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	QueryContainerBase* asyncContainer = reinterpret_cast<QueryContainerBase*>(ref);

	switch(asyncContainer->mQueryType)
	{
		case TFQuery_MainData:
		{
			Terminal* terminal = _createTerminal(result);

			if(terminal->getLoadState() == LoadState_Loaded && asyncContainer->mOfCallback)
				asyncContainer->mOfCallback->handleObjectReady(terminal,asyncContainer->mClient);
			else
			{
				switch(terminal->getTangibleType())
				{
					case TanType_ElevatorTerminal:
					case TanType_ElevatorUpTerminal:
					case TanType_ElevatorDownTerminal:
					{
						ElevatorTerminal* elTerminal = dynamic_cast<ElevatorTerminal*>(terminal);

						switch(elTerminal->mLoadState)
						{
							case LoadState_Tangible_Data:
							{
								QueryContainerBase* asContainer = new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(asyncContainer->mOfCallback,TFQuery_ElevatorData,asyncContainer->mClient);
								asContainer->mObject = elTerminal;

								mDatabase->ExecuteSqlAsync(this,asContainer,"SELECT * FROM terminal_elevator_data WHERE id=%"PRIu64" ORDER BY direction",elTerminal->getId());
							}
							break;

							default:break;
						}
					}
					break;

					default:break;
				}
			}
		}
		break;

		case TFQuery_ElevatorData:
		{
			ElevatorTerminal* terminal = dynamic_cast<ElevatorTerminal*>(asyncContainer->mObject);

			//uint64 count = result->getRowCount();
			//assert(count < 3 && count > 0);

			// we order by direction in select, though up is first
			if(terminal->mTanType == TanType_ElevatorUpTerminal || terminal->mTanType == TanType_ElevatorTerminal)
				result->GetNextRow(mElevetorDataUpBinding,(void*)terminal);

			if(terminal->mTanType == TanType_ElevatorDownTerminal || terminal->mTanType == TanType_ElevatorTerminal)
				result->GetNextRow(mElevetorDataDownBinding,(void*)terminal);

			terminal->setLoadState(LoadState_Loaded);

			asyncContainer->mOfCallback->handleObjectReady(terminal,asyncContainer->mClient);
		}
		break;

		default:break;
	}

	mQueryContainerPool.free(asyncContainer);
}

//=============================================================================

void TerminalFactory::requestObject(ObjectFactoryCallback* ofCallback,uint64 id,uint16 subGroup,uint16 subType,DispatchClient* client)
{
	mDatabase->ExecuteSqlAsync(this,new(mQueryContainerPool.ordered_malloc()) QueryContainerBase(ofCallback,TFQuery_MainData,client),
					"SELECT terminals.id,terminals.parent_id,terminals.oX,terminals.oY,terminals.oZ,terminals.oW,terminals.x,"
					"terminals.y,terminals.z,terminals.terminal_type,terminal_types.object_string,terminal_types.name,terminal_types.file,"
					"terminals.dataStr,terminals.dataInt1,terminals.customName"
					" FROM terminals INNER JOIN terminal_types ON (terminals.terminal_type = terminal_types.id)"
					" WHERE (terminals.id = %"PRIu64")",id);
}

//=============================================================================

Terminal* TerminalFactory::_createTerminal(DatabaseResult* result)
{
	Terminal*		terminal(0);
	TangibleType	tanType;

	DataBinding* typeBinding = mDatabase->CreateDataBinding(1);
	typeBinding->addField(DFT_uint32,0,4,9);

	uint64 count = result->getRowCount();

	result->GetNextRow(typeBinding,&tanType);
	result->ResetRowIndex();

	mDatabase->DestroyDataBinding(typeBinding);

	switch(tanType)
	{
		case TanType_HQTerminal:
		case TanType_SpaceTerminal:
		case TanType_NewsNetTerminal:
		case TanType_BestineQuest1Terminal:
		case TanType_BestineQuest2Terminal:
		case TanType_BallotBoxTerminal:
		case TanType_BountyDroidTerminal:
		case TanType_GuildTerminal:
		case TanType_MissionStatueTerminal:
		case TanType_NewbieClothingTerminal:
		case TanType_NewbieFoodTerminal:
		case TanType_NewbieInstrumentTerminal:
		case TanType_NewbieMedicineTerminal:
		case TanType_NewbieToolTerminal:
		case TanType_HQRebelTerminal:
		case TanType_HQImperialTerminal:
		case TanType_PMRegisterTerminal:
		case TanType_SKillTerminal:
		case TanType_CityTerminal:
		case TanType_PlayerStructureNoSnapTerm:
		case TanType_CityVoteTerminal:
		case TanType_PlayerStructureNoSnapMini:
		case TanType_NymCaveTerminal:
		case TanType_CommandConsoleTerminal:
		case TanType_GeoBunkerTerminal:
		case TanType_BestineQuests3:
		case TanType_HQTurrentControlTermainl:
		case TanType_ImageDesignTerminal:
		case TanType_WaterPressureTerminal:
		case TanType_Light_Enc_VotingTerminal:
		case TanType_Dark_Enc_ChallengeTerminal:
		case TanType_Dark_Enc_VotingTerminal:
		case TanType_ShipInteriorSecurity1:
		case TanType_POBShipTerminal:
		case TanType_Light_Enc_ChallengeTerminal:
		case TanType_CampTerminal:
		{			
			terminal = new Terminal();
			terminal->setTangibleType(tanType);

			DataBinding* terminalBinding = mDatabase->CreateDataBinding(14);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mId),8,0);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mParentId),8,1);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.x),4,2);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.y),4,3);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.z),4,4);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.w),4,5);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.x),4,6);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.y),4,7);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.z),4,8);
			terminalBinding->addField(DFT_uint32,offsetof(Terminal,mTerminalType),4,9);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mModel),256,10);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mName),64,11);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mNameFile),64,12);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mCustomName),256,15);

			result->GetNextRow(terminalBinding,(void*)terminal);

			mDatabase->DestroyDataBinding(terminalBinding);

			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_PlayerStructureTerminal:		
		{
			terminal = new PlayerStructureTerminal();
			terminal->setTangibleType(tanType);

			DataBinding* terminalBinding = mDatabase->CreateDataBinding(14);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mId),8,0);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mParentId),8,1);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.x),4,2);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.y),4,3);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.z),4,4);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.w),4,5);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.x),4,6);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.y),4,7);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.z),4,8);
			terminalBinding->addField(DFT_uint32,offsetof(Terminal,mTerminalType),4,9);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mModel),256,10);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mName),64,11);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mNameFile),64,12);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mCustomName),256,15);

			result->GetNextRow(terminalBinding,(void*)terminal);

			mDatabase->DestroyDataBinding(terminalBinding);

			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_BankTerminal:
		{
			terminal = new BankTerminal();
			terminal->setTangibleType(tanType);
			result->GetNextRow(mBankMainDataBinding,(void*)terminal);
			terminal->setLoadState(LoadState_Loaded);

		}
		break;

		case TanType_CharacterBuilderTerminal:
		{
			terminal = new CharacterBuilderTerminal();
			terminal->setTangibleType(tanType);
			result->GetNextRow(mCharacterBuilderMainDataBinding,(void*)terminal);
			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_BazaarTerminal:
		{
			terminal = new BazaarTerminal();
			terminal->setTangibleType(tanType);
			result->GetNextRow(mBazaarMainDataBinding,(void*)terminal);
			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_CloningTerminal:
		{
			terminal = new CloningTerminal();
			terminal->setTangibleType(tanType);
			result->GetNextRow(mCloningMainDataBinding,(void*)terminal);
			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_InsuranceTerminal:
		{
			terminal = new InsuranceTerminal();
			terminal->setTangibleType(tanType);
			result->GetNextRow(mInsuranceMainDataBinding,(void*)terminal);
			terminal->setLoadState(LoadState_Loaded);
		}
		break;


		case TanType_MissionTerminal:
		case TanType_EntertainerMissionTerminal:
		case TanType_BountyMissionTerminal:
		case TanType_RebelMissionTerminal:
		case TanType_ImperialMissionTerminal:
		case TanType_ScoutMissionTerminal:
		case TanType_ArtisanMissionTerminal:
		case TanType_MissionNewbieTerminal:
		{
			terminal = new MissionTerminal();
			terminal->setTangibleType(tanType);

			result->GetNextRow(mMissionMainDataBinding,(void*)terminal);

			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_TravelTerminal:
		{
			terminal = new TravelTerminal();
			terminal->setTangibleType(tanType);

			result->GetNextRow(mTravelMainDataBinding,(void*)terminal);

			terminal->setLoadState(LoadState_Loaded);
		}
		break;

		case TanType_ElevatorUpTerminal:
		case TanType_ElevatorDownTerminal:
		case TanType_ElevatorTerminal:
		{
			terminal = new ElevatorTerminal();
			terminal->setTangibleType(tanType);

			result->GetNextRow(mElevatorMainDataBinding,(void*)terminal);

			//((ElevatorTerminal*)(terminal))->prepareRadialMenu();
			terminal->setLoadState(LoadState_Tangible_Data);
		}
		break;

		default:
		{
			gLogger->log(LogManager::DEBUG,"TerminalFactory::_createTerminal: unknown eType: %u",tanType);

			terminal = new Terminal();
			terminal->setTangibleType(tanType);

			DataBinding* terminalBinding = mDatabase->CreateDataBinding(14);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mId),8,0);
			terminalBinding->addField(DFT_uint64,offsetof(Terminal,mParentId),8,1);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.x),4,2);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.y),4,3);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.z),4,4);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mDirection.w),4,5);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.x),4,6);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.y),4,7);
			terminalBinding->addField(DFT_float,offsetof(Terminal,mPosition.z),4,8);
			terminalBinding->addField(DFT_uint32,offsetof(Terminal,mTerminalType),4,9);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mModel),256,10);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mName),64,11);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mNameFile),64,12);
			terminalBinding->addField(DFT_bstring,offsetof(Terminal,mCustomName),256,15);

			result->GetNextRow(terminalBinding,(void*)terminal);

			mDatabase->DestroyDataBinding(terminalBinding);

			terminal->setLoadState(LoadState_Loaded);
		}
		break;
	}

	terminal->mTypeOptions = 0x108;

	return terminal;
}

//=============================================================================

void TerminalFactory::_setupDatabindings()
{
	// mission terminal
	mMissionMainDataBinding = mDatabase->CreateDataBinding(13);
	mMissionMainDataBinding->addField(DFT_uint64,offsetof(MissionTerminal,mId),8,0);
	mMissionMainDataBinding->addField(DFT_uint64,offsetof(MissionTerminal,mParentId),8,1);
	mMissionMainDataBinding->addField(DFT_bstring,offsetof(MissionTerminal,mModel),256,10);
	mMissionMainDataBinding->addField(DFT_bstring,offsetof(MissionTerminal,mName),64,11);
	mMissionMainDataBinding->addField(DFT_bstring,offsetof(MissionTerminal,mNameFile),64,12);
	mMissionMainDataBinding->addField(DFT_bstring,offsetof(MissionTerminal,mCustomName),256,15);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mDirection.x),4,2);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mDirection.y),4,3);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mDirection.z),4,4);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mDirection.w),4,5);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mPosition.x),4,6);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mPosition.y),4,7);
	mMissionMainDataBinding->addField(DFT_float,offsetof(MissionTerminal,mPosition.z),4,8);

	// bazaar terminal
	mBazaarMainDataBinding = mDatabase->CreateDataBinding(13);
	mBazaarMainDataBinding->addField(DFT_uint64,offsetof(BazaarTerminal,mId),8,0);
	mBazaarMainDataBinding->addField(DFT_uint64,offsetof(BazaarTerminal,mParentId),8,1);
	mBazaarMainDataBinding->addField(DFT_bstring,offsetof(BazaarTerminal,mModel),256,10);
	mBazaarMainDataBinding->addField(DFT_bstring,offsetof(BazaarTerminal,mName),64,11);
	mBazaarMainDataBinding->addField(DFT_bstring,offsetof(BazaarTerminal,mNameFile),64,12);
	mBazaarMainDataBinding->addField(DFT_bstring,offsetof(BazaarTerminal,mCustomName),256,15);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mDirection.x),4,2);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mDirection.y),4,3);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mDirection.z),4,4);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mDirection.w),4,5);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mPosition.x),4,6);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mPosition.y),4,7);
	mBazaarMainDataBinding->addField(DFT_float,offsetof(BazaarTerminal,mPosition.z),4,8);

	// cloning terminal
	mCloningMainDataBinding = mDatabase->CreateDataBinding(13);
	mCloningMainDataBinding->addField(DFT_uint64,offsetof(CloningTerminal,mId),8,0);
	mCloningMainDataBinding->addField(DFT_uint64,offsetof(CloningTerminal,mParentId),8,1);
	mCloningMainDataBinding->addField(DFT_bstring,offsetof(CloningTerminal,mModel),256,10);
	mCloningMainDataBinding->addField(DFT_bstring,offsetof(CloningTerminal,mName),64,11);
	mCloningMainDataBinding->addField(DFT_bstring,offsetof(CloningTerminal,mNameFile),64,12);
	mCloningMainDataBinding->addField(DFT_bstring,offsetof(CloningTerminal,mCustomName),256,15);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mDirection.x),4,2);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mDirection.y),4,3);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mDirection.z),4,4);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mDirection.w),4,5);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mPosition.x),4,6);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mPosition.y),4,7);
	mCloningMainDataBinding->addField(DFT_float,offsetof(CloningTerminal,mPosition.z),4,8);

	// insurance terminal
	mInsuranceMainDataBinding = mDatabase->CreateDataBinding(13);
	mInsuranceMainDataBinding->addField(DFT_uint64,offsetof(InsuranceTerminal,mId),8,0);
	mInsuranceMainDataBinding->addField(DFT_uint64,offsetof(InsuranceTerminal,mParentId),8,1);
	mInsuranceMainDataBinding->addField(DFT_bstring,offsetof(InsuranceTerminal,mModel),256,10);
	mInsuranceMainDataBinding->addField(DFT_bstring,offsetof(InsuranceTerminal,mName),64,11);
	mInsuranceMainDataBinding->addField(DFT_bstring,offsetof(InsuranceTerminal,mNameFile),64,12);
	mInsuranceMainDataBinding->addField(DFT_bstring,offsetof(InsuranceTerminal,mCustomName),256,15);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mDirection.x),4,2);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mDirection.y),4,3);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mDirection.z),4,4);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mDirection.w),4,5);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mPosition.x),4,6);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mPosition.y),4,7);
	mInsuranceMainDataBinding->addField(DFT_float,offsetof(InsuranceTerminal,mPosition.z),4,8);

	// character builder terminal
	mCharacterBuilderMainDataBinding = mDatabase->CreateDataBinding(13);
	mCharacterBuilderMainDataBinding->addField(DFT_uint64,offsetof(CharacterBuilderTerminal,mId),8,0);
	mCharacterBuilderMainDataBinding->addField(DFT_uint64,offsetof(CharacterBuilderTerminal,mParentId),8,1);
	mCharacterBuilderMainDataBinding->addField(DFT_bstring,offsetof(CharacterBuilderTerminal,mModel),256,10);
	mCharacterBuilderMainDataBinding->addField(DFT_bstring,offsetof(CharacterBuilderTerminal,mName),64,11);
	mCharacterBuilderMainDataBinding->addField(DFT_bstring,offsetof(CharacterBuilderTerminal,mNameFile),64,12);
	mCharacterBuilderMainDataBinding->addField(DFT_bstring,offsetof(CharacterBuilderTerminal,mCustomName),256,15);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mDirection.x),4,2);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mDirection.y),4,3);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mDirection.z),4,4);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mDirection.w),4,5);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mPosition.x),4,6);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mPosition.y),4,7);
	mCharacterBuilderMainDataBinding->addField(DFT_float,offsetof(CharacterBuilderTerminal,mPosition.z),4,8);

	// travel terminal
	mTravelMainDataBinding = mDatabase->CreateDataBinding(15);
	mTravelMainDataBinding->addField(DFT_uint64,offsetof(TravelTerminal,mId),8,0);
	mTravelMainDataBinding->addField(DFT_uint64,offsetof(TravelTerminal,mParentId),8,1);
	mTravelMainDataBinding->addField(DFT_bstring,offsetof(TravelTerminal,mModel),256,10);
	mTravelMainDataBinding->addField(DFT_bstring,offsetof(TravelTerminal,mName),64,11);
	mTravelMainDataBinding->addField(DFT_bstring,offsetof(TravelTerminal,mNameFile),64,12);
	mTravelMainDataBinding->addField(DFT_bstring,offsetof(TravelTerminal,mPositionDescriptor),128,13);
	mTravelMainDataBinding->addField(DFT_uint32,offsetof(TravelTerminal,mPortType),4,14);
	mTravelMainDataBinding->addField(DFT_bstring,offsetof(TravelTerminal,mCustomName),256,15);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mDirection.x),4,2);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mDirection.y),4,3);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mDirection.z),4,4);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mDirection.w),4,5);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mPosition.x),4,6);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mPosition.y),4,7);
	mTravelMainDataBinding->addField(DFT_float,offsetof(TravelTerminal,mPosition.z),4,8);


	// bank terminal
	mBankMainDataBinding = mDatabase->CreateDataBinding(13);
	mBankMainDataBinding->addField(DFT_uint64,offsetof(BankTerminal,mId),8,0);
	mBankMainDataBinding->addField(DFT_uint64,offsetof(BankTerminal,mParentId),8,1);
	mBankMainDataBinding->addField(DFT_bstring,offsetof(BankTerminal,mModel),256,10);
	mBankMainDataBinding->addField(DFT_bstring,offsetof(BankTerminal,mName),64,11);
	mBankMainDataBinding->addField(DFT_bstring,offsetof(BankTerminal,mNameFile),64,12);
	mBankMainDataBinding->addField(DFT_bstring,offsetof(BankTerminal,mCustomName),256,15);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mDirection.x),4,2);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mDirection.y),4,3);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mDirection.z),4,4);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mDirection.w),4,5);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mPosition.x),4,6);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mPosition.y),4,7);
	mBankMainDataBinding->addField(DFT_float,offsetof(BankTerminal,mPosition.z),4,8);

	// elevator terminal main data
	mElevatorMainDataBinding = mDatabase->CreateDataBinding(13);
	mElevatorMainDataBinding->addField(DFT_uint64,offsetof(ElevatorTerminal,mId),8,0);
	mElevatorMainDataBinding->addField(DFT_uint64,offsetof(ElevatorTerminal,mParentId),8,1);
	mElevatorMainDataBinding->addField(DFT_bstring,offsetof(ElevatorTerminal,mModel),256,10);
	mElevatorMainDataBinding->addField(DFT_bstring,offsetof(ElevatorTerminal,mName),64,11);
	mElevatorMainDataBinding->addField(DFT_bstring,offsetof(ElevatorTerminal,mNameFile),64,12);
	mElevatorMainDataBinding->addField(DFT_bstring,offsetof(ElevatorTerminal,mCustomName),256,15);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDirection.x),4,2);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDirection.y),4,3);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDirection.z),4,4);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDirection.w),4,5);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mPosition.x),4,6);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mPosition.y),4,7);
	mElevatorMainDataBinding->addField(DFT_float,offsetof(ElevatorTerminal,mPosition.z),4,8);

	// elevator terminal upper destination
	mElevetorDataUpBinding = mDatabase->CreateDataBinding(9);
	mElevetorDataUpBinding->addField(DFT_uint64,offsetof(ElevatorTerminal,mDstCellUp),8,1);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirUp.x),4,2);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirUp.y),4,3);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirUp.z),4,4);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirUp.w),4,5);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosUp.x),4,6);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosUp.y),4,7);
	mElevetorDataUpBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosUp.z),4,8);
	mElevetorDataUpBinding->addField(DFT_uint32,offsetof(ElevatorTerminal,mEffectUp),4,9);

	// elevator terminal lower destination
	mElevetorDataDownBinding = mDatabase->CreateDataBinding(9);
	mElevetorDataDownBinding->addField(DFT_uint64,offsetof(ElevatorTerminal,mDstCellDown),8,1);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirDown.x),4,2);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirDown.y),4,3);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirDown.z),4,4);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstDirDown.w),4,5);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosDown.x),4,6);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosDown.y),4,7);
	mElevetorDataDownBinding->addField(DFT_float,offsetof(ElevatorTerminal,mDstPosDown.z),4,8);
	mElevetorDataDownBinding->addField(DFT_uint32,offsetof(ElevatorTerminal,mEffectDown),4,9);
}

//=============================================================================

void TerminalFactory::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mBazaarMainDataBinding);
	mDatabase->DestroyDataBinding(mCloningMainDataBinding);
	mDatabase->DestroyDataBinding(mInsuranceMainDataBinding);
	mDatabase->DestroyDataBinding(mMissionMainDataBinding);
	mDatabase->DestroyDataBinding(mTravelMainDataBinding);
	mDatabase->DestroyDataBinding(mBankMainDataBinding);
	mDatabase->DestroyDataBinding(mCharacterBuilderMainDataBinding);
	mDatabase->DestroyDataBinding(mElevatorMainDataBinding);
	mDatabase->DestroyDataBinding(mElevetorDataUpBinding);
	mDatabase->DestroyDataBinding(mElevetorDataDownBinding);
}

//=============================================================================

