/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "ResourceCollectionManager.h"
#include "PlayerObject.h"
#include "ResourceCollectionCommand.h"
#include "UIManager.h"
#include "WaypointObject.h"
#include "Inventory.h"
#include "Datapad.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/clock.h"
#include "Utils/rand.h"

bool ResourceCollectionManager::mInsFlag = false;
ResourceCollectionManager* ResourceCollectionManager::mSingleton = NULL;

//======================================================================================================================

ResourceCollectionManager::ResourceCollectionManager(Database* database) :
    mDatabase(database),
    mDBAsyncPool(sizeof(RCMAsyncContainer))
{
    _setupDatabindings();

    // load sample costs
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RCMAsyncContainer(RCMQuery_SampleCosts),
                               "select id, commandname, healthcost, actioncost, mindcost, damage_multiplier from command_table where commandname in ('dosample');");
    mDatabase->executeSqlAsync(this,new(mDBAsyncPool.ordered_malloc()) RCMAsyncContainer(RCMQuery_SurveyCosts),
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
    mCommandCostBinding = mDatabase->createDataBinding(6);
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
    mDatabase->destroyDataBinding(mCommandCostBinding);
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
            result->getNextRow(mCommandCostBinding, cCommand);
            this->sampleActionCost = cCommand->getActionCost();
            this->sampleHealthCost = cCommand->getHealthCost();
            this->sampleMindCost = cCommand->getMindCost();
            this->sampleRadioactiveDamageModifier = cCommand->getDamageModifier();

        }
    }
    break;

    case RCMQuery_SurveyCosts:
    {
        ResourceCollectionCommand* cCommand = new ResourceCollectionCommand();
        if (result->getRowCount())
        {

            result->getNextRow(mCommandCostBinding, cCommand);


            this->surveyActionCost = cCommand->getActionCost();
            this->surveyHealthCost = cCommand->getHealthCost();
            this->surveyMindCost = cCommand->getMindCost();

        }
    }
    break;

    default:
        break;
    }

    mDBAsyncPool.ordered_free(asyncContainer);
}

//=============================================================================
//
// handles any UIWindow callbacks for sampling events
//

void ResourceCollectionManager::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
}
