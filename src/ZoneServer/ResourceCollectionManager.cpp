 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ResourceCollectionManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"
#include "LogManager/LogManager.h"
#include "ResourceCollectionCommand.h"

//======================================================================================================================

bool ResourceCollectionManager::mInsFlag = false;
ResourceCollectionManager* ResourceCollectionManager::mSingleton = NULL;

//======================================================================================================================

ResourceCollectionManager::ResourceCollectionManager(Database* database) :
mDatabase(database),
mDBAsyncPool(sizeof(RCMAsyncContainer))
{
	_setupDatabindings();

	// load sample costs
	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RCMAsyncContainer(RCMQuery_SampleCosts),
		"select id, commandname, healthcost, actioncost, mindcost, damage_multiplier from command_table where commandname in ('dosample');");
	
	mDatabase->ExecuteSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RCMAsyncContainer(RCMQuery_SurveyCosts),
		"select id, commandname, healthcost, actioncost, mindcost, damage_multiplier from command_table where commandname in ('requestSurvey');");

}

//======================================================================================================================

ResourceCollectionManager* ResourceCollectionManager::Init(Database* database)
{
	if(mInsFlag == false)
	{
		mSingleton = new ResourceCollectionManager(database);
		mInsFlag = true;
		return mSingleton;
	}
	else
		return mSingleton;
}

//======================================================================================================================

ResourceCollectionManager::~ResourceCollectionManager()
{
	_destroyDatabindings();

	mInsFlag = false;
	delete(mSingleton);
}

void ResourceCollectionManager::_setupDatabindings()
{
	mCommandCostBinding = mDatabase->CreateDataBinding(6);
	mCommandCostBinding->addField(DFT_uint32,offsetof(ResourceCollectionCommand,mId),4,0);
	mCommandCostBinding->addField(DFT_bstring,offsetof(ResourceCollectionCommand,mCommandName),255,1);
	mCommandCostBinding->addField(DFT_int32,offsetof(ResourceCollectionCommand,mHealthCost),4,2);
	mCommandCostBinding->addField(DFT_int32,offsetof(ResourceCollectionCommand,mActionCost),4,3);
	mCommandCostBinding->addField(DFT_int32,offsetof(ResourceCollectionCommand,mMindCost),4,4);
	mCommandCostBinding->addField(DFT_int32,offsetof(ResourceCollectionCommand,mDamageModifier),4,5);
}

//======================================================================================================================

void ResourceCollectionManager::_destroyDatabindings()
{
	mDatabase->DestroyDataBinding(mCommandCostBinding);
}

//======================================================================================================================
void ResourceCollectionManager::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
	RCMAsyncContainer* asyncContainer = reinterpret_cast<RCMAsyncContainer*>(ref);
	
	switch (asyncContainer->mQueryType)
	{
		case RCMQuery_SampleCosts:
			{
				ResourceCollectionCommand* cCommand = new ResourceCollectionCommand();

				if (result->getRowCount())
				{
					result->GetNextRow(mCommandCostBinding, cCommand);
					this->sampleActionCost = cCommand->getActionCost();
					this->sampleHealthCost = cCommand->getHealthCost();
					this->sampleMindCost = cCommand->getMindCost();
					this->sampleRadioactiveDamageModifier = cCommand->getDamageModifier();
					
				}

				if(result->getRowCount())
					gLogger->logMsgLoadSuccess("ResourceCollectionManager::Loading sample costs...",MSG_NORMAL);
				else
					gLogger->logMsgLoadFailure("ResourceCollectionManager::Loading sample costs...",MSG_NORMAL);					

			}
			break;

		case RCMQuery_SurveyCosts:
			{
				ResourceCollectionCommand* cCommand = new ResourceCollectionCommand();
				if (result->getRowCount())
				{
				
					result->GetNextRow(mCommandCostBinding, cCommand);

					
					this->surveyActionCost = cCommand->getActionCost();
					this->surveyHealthCost = cCommand->getHealthCost();
					this->surveyMindCost = cCommand->getMindCost();
					
				}
				
				if(result->getRowCount())
					gLogger->logMsgLoadSuccess("ResourceCollectionManager::Loading %u survey costs...",MSG_NORMAL,result->getRowCount());
				else
					gLogger->logMsgLoadFailure("ResourceCollectionManager::Loading survey costs...",MSG_NORMAL);					
			}
			break;

		default:break;
	}

	mDBAsyncPool.ordered_free(asyncContainer);
}