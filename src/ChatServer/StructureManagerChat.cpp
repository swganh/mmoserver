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
#include "ChatOpcodes.h"
#include "Mail.h"
#include "Player.h"
#include "StructureManagerChat.h"
#include "TradeManagerChat.h"

#include "ZoneServer/TangibleEnums.h"




#include "Utils/logger.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/DatabaseResult.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include "Utils/utils.h"
#include "Utils/Timer.h"

#include <boost/lexical_cast.hpp>

#include <cstring>
#include <ctime>


//=========================================================================================

bool								StructureManagerChatHandler::mInsFlag    = false;
StructureManagerChatHandler*		StructureManagerChatHandler::mSingleton  = NULL;

//=========================================================================================


//======================================================================================================================

StructureManagerChatHandler::StructureManagerChatHandler(Database* database, MessageDispatch* dispatch, ChatManager* chatManager)
{

    mDatabase = database;
    mChatManager = chatManager;
    mMessageDispatch = dispatch;
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();
    //StructureManagerAsyncContainer* asyncContainer;

    mMessageDispatch->RegisterMessageCallback(opIsmHarvesterUpdate,std::bind(&StructureManagerChatHandler::ProcessAddHarvesterHopperUpdate, this, std::placeholders::_1, std::placeholders::_2));






    // load our bazaar terminals
    //asyncContainer = new TradeManagerAsyncContainer(TRMQuery_LoadBazaar, 0);
    //mDatabase->ExecuteSqlAsync(this,asyncContainer,"SELECT * FROM commerce_bazaar");

    mTimerQueueProcessTimeLimit = 10;

    //move to Handle dispatch message at some time

    //TODO

    //implement configmanager in the chatserver

    std::tr1::shared_ptr<Timer> factory_timer(new Timer(SRMTimer_CheckFactory,this,8000,NULL));
    std::tr1::shared_ptr<Timer> hopper_timer(new Timer(SRMTimer_CheckHarvesterHopper,this,60000,NULL));
    std::tr1::shared_ptr<Timer> maintenance_timer(new Timer(SRMTimer_CheckHarvesterMaintenance,this,3600*1000,NULL));
    std::tr1::shared_ptr<Timer> power_timer(new Timer(SRMTimer_CheckHarvesterPower,this,3600*1000,NULL));
    //std::tr1::shared_ptr<Timer> tick_preserve_timer(new Timer(CMTimer_TickPreserve,this,ServerTimeInterval*10000,NULL));
    //std::tr1::shared_ptr<Timer> check_auctions_timer(new Timer(CMTimer_CheckAuctions,this,ServerTimeInterval*10000,NULL));

    mTimers.push_back(factory_timer);
    mTimers.push_back(hopper_timer);
    mTimers.push_back(maintenance_timer);
    mTimers.push_back(power_timer);

}


//======================================================================================================================
StructureManagerChatHandler::~StructureManagerChatHandler()
{
    mInsFlag = false;

}
//======================================================================================================================
StructureManagerChatHandler*	StructureManagerChatHandler::Init(Database* database, MessageDispatch* dispatch, ChatManager* chatManager)
{
    if(!mInsFlag)
    {
        mSingleton = new StructureManagerChatHandler(database,dispatch,chatManager);
        mInsFlag = true;
        return mSingleton;
    }
    else
        return mSingleton;

}

//======================================================================================================================
void StructureManagerChatHandler::Shutdown()
{
    mMessageDispatch->UnregisterMessageCallback(opIsmHarvesterUpdate);

}

//=======================================================================================================================


void StructureManagerChatHandler::handleDatabaseJobComplete(void* ref,DatabaseResult* result)
{
    StructureManagerAsyncContainer* asynContainer = (StructureManagerAsyncContainer*)ref;

    switch(asynContainer->mQueryType)
    {


        //=================================================
        //
        //the structures condition reached zero
        //
    case STRMQuery_StructureMailCondZero:
    {
        structure st;

        DataBinding* binding = mDatabase->createDataBinding(9);
        binding->addField(DFT_uint64,offsetof(structure,owner),8,0);
        binding->addField(DFT_bstring,offsetof(structure,file),64,1);
        binding->addField(DFT_bstring,offsetof(structure,dir),64,2);
        binding->addField(DFT_float,offsetof(structure,x),4,3);
        binding->addField(DFT_float,offsetof(structure,z),4,4);
        binding->addField(DFT_bstring,offsetof(structure,planet),32,5);
        binding->addField(DFT_uint32,offsetof(structure,maxcondition),4,6);
        binding->addField(DFT_uint32,offsetof(structure,maint),4,7);
        binding->addField(DFT_uint64,offsetof(structure,lastMail),8,8);

        if (result->getRowCount()) {
        	return;
        }

        result->getNextRow(binding,&st);

        if(st.lastMail < (gTradeManagerChat->getGlobalTickCount() + (24*3600*1000)))
        {
            //last Mail less than 24hours no need to send it again
            mDatabase->destroyDataBinding(binding);
            return;

        }

        //send the relevant EMail

        atMacroString* aMS = new atMacroString();

        aMS->addMBstf("player_structure","structure_condemned_body");
        aMS->addTTstf(st.file.getAnsi(),st.dir.getAnsi());
        aMS->addDI((uint32)(st.maxcondition*(st.maint/168)));
        aMS->addTextModule();

        aMS->setPlanetString(st.planet);
        aMS->setWP(st.x,0,st.z,"Structure");
        aMS->addWaypoint();

        Mail* mail = new Mail();
        mail->setSender(BString("@player_structure:management"));
        mail->setSubject(BString("@player_structure:structure_condemned_subject "));
        mail->setText(BString(""));
        mail->setStatus(MailStatus_New);
        mail->setTime(static_cast<uint32>(time(NULL)));
        mail->setAttachments(aMS->assemble());

        gChatManager->sendSystemMailMessage(mail,st.owner);

        mDatabase->destroyDataBinding(binding);

        int8 sql[250];
        // Now update the time of the last EMail
        sprintf(sql,"UPDATE %s.structures SET structures.lastMail = %" PRIu64 " WHERE ID = %" PRIu64 "",mDatabase->galaxy(), gTradeManagerChat->getGlobalTickCount(), asynContainer->harvesterID);

        mDatabase->executeAsyncSql(sql);

    }
    break;



    //=================================================
    //
    //the structure is getting damaged
    //
    case STRMQuery_StructureMailDamage:
    {
        structure st;

        DataBinding* binding = mDatabase->createDataBinding(9);
        binding->addField(DFT_uint64,offsetof(structure,owner),8,0);
        binding->addField(DFT_bstring,offsetof(structure,file),64,1);
        binding->addField(DFT_bstring,offsetof(structure,dir),64,2);
        binding->addField(DFT_float,offsetof(structure,x),4,3);
        binding->addField(DFT_float,offsetof(structure,z),4,4);
        binding->addField(DFT_bstring,offsetof(structure,planet),32,5);
        binding->addField(DFT_uint32,offsetof(structure,maxcondition),4,6);
        binding->addField(DFT_uint32,offsetof(structure,condition),4,7);
        binding->addField(DFT_uint64,offsetof(structure,lastMail),8,8);

        if (!result->getRowCount()) {
           	return;
        }

        result->getNextRow(binding,&st);

        if(st.lastMail < (gTradeManagerChat->getGlobalTickCount() + (24*3600*1000)))
        {
            //last Mail less than 24hours no need to send it again
            mDatabase->destroyDataBinding(binding);
            return;

        }

        //send the relevant EMail

        atMacroString* aMS = new atMacroString();

        aMS->addMBstf("player_structure","mail_structure_damage");
        aMS->addTTstf(st.file.getAnsi(),st.dir.getAnsi());
        aMS->addDI((uint32)((st.maxcondition-st.condition)/(st.maxcondition/100)));
        aMS->addTextModule();

        aMS->setPlanetString(st.planet);
        aMS->setWP(st.x,0,st.z,"Structure");
        aMS->addWaypoint();

        Mail* mail = new Mail();
        mail->setSender(BString("@player_structure:management"));
        mail->setSubject(BString("@player_structure:mail_structure_damage_sub"));
        mail->setText(BString(""));
        mail->setStatus(MailStatus_New);
        mail->setTime(static_cast<uint32>(time(NULL)));
        mail->setAttachments(aMS->assemble());

        gChatManager->sendSystemMailMessage(mail,st.owner);

        mDatabase->destroyDataBinding(binding);

        int8 sql[250];
        // Now update the time of the last EMail
        sprintf(sql,"UPDATE %s.structures SET structures.lastMail = %" PRIu64 " WHERE ID = %" PRIu64 "",mDatabase->galaxy(), gTradeManagerChat->getGlobalTickCount(), asynContainer->harvesterID);

        mDatabase->executeAsyncSql(sql);

    }
    break;


    //=================================================
    //
    //the structure is out of maintenance - the bank account will be used
    //we just need to send the appropriate mail
    case STRMQuery_StructureMailOOFMaint:
    {

        structure st;

        DataBinding* binding = mDatabase->createDataBinding(7);
        binding->addField(DFT_uint64,offsetof(structure,owner),8,0);
        binding->addField(DFT_bstring,offsetof(structure,file),64,1);
        binding->addField(DFT_bstring,offsetof(structure,dir),64,2);
        binding->addField(DFT_float,offsetof(structure,x),4,3);
        binding->addField(DFT_float,offsetof(structure,z),4,4);
        binding->addField(DFT_bstring,offsetof(structure,planet),32,5);
        binding->addField(DFT_uint64,offsetof(structure,lastMail),8,6);

        if (!result->getRowCount()) {
        	return;
        }

        result->getNextRow(binding,&st);

        if(st.lastMail < (gTradeManagerChat->getGlobalTickCount() + (24*3600*1000)))
        {
            //last Mail less than 24hours no need to send it again
            mDatabase->destroyDataBinding(binding);
            return;

        }


        //send the relevant EMail

        atMacroString* aMS = new atMacroString();

        aMS->addMBstf("player_structure","structure_maintenance_empty_body");
        aMS->addTTstf(st.file.getAnsi(),st.dir.getAnsi());
        aMS->addTextModule();

        aMS->setPlanetString(st.planet);
        aMS->setWP(st.x,0,st.z,"Structure");
        aMS->addWaypoint();

        Mail* mail = new Mail();
        mail->setSender(BString("@player_structure:management"));
        mail->setSubject(BString("@player_structure:structure_maintenance_empty_subject"));
        mail->setText(BString(""));
        mail->setStatus(MailStatus_New);
        mail->setTime(static_cast<uint32>(time(NULL)));
        mail->setAttachments(aMS->assemble());

        gChatManager->sendSystemMailMessage(mail,st.owner);

        mDatabase->destroyDataBinding(binding);

        int8 sql[250];
        // Now update the time of the last EMail
        sprintf(sql,"UPDATE %s.structures SET structures.lastMail = %" PRIu64 " WHERE ID = %" PRIu64 "",mDatabase->galaxy(), gTradeManagerChat->getGlobalTickCount(), asynContainer->harvesterID);

        mDatabase->executeSqlAsync(0 ,0 ,sql);


    }
    break;

    //=================================================
    //
    //we deducted the maintenance - or tried too
    //read out the answer and proceed
    case STRMQuery_DoneStructureMaintenance:
    {
        uint32 exitCode;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        if (!result->getRowCount()) {
           	return;
        }

        //return codes :
        // 0 everything ok
        // 1 structure is out of maintenance  - taking maint out of the bank
        // 2 structure got damaged
        // 3 condition is zero


        result->getNextRow(binding,&exitCode);

        if(exitCode == 1)// 1 structure is out of maintenance
        {
            // get the Owners ID
            int8 sql[500];

            //inform the owner on the maintenance issue
            sprintf(sql,"SELECT s.owner, st.stf_file, st.stf_name, s.x, s.z, p.name, s.lastMail FROM %s.structures s INNER JOIN %s.structure_type_data st ON (s.type = st.type) INNER JOIN %s.planet p ON (p.planet_id = s.zone)WHERE ID = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),asynContainer->harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_StructureMailOOFMaint,0);
            asyncContainer->harvesterID = asynContainer->harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

        }

        if(exitCode == 2)// 2 structure got damaged
        {
            // get the Owners ID
            int8 sql[500];

            //start by using power
            sprintf(sql,"SELECT s.owner, st.stf_file, st.stf_name, s.x, s.z, p.name, st.max_condition, s.condition, s.lastMail FROM %s.structures s INNER JOIN %s.structure_type_data st ON (s.type = st.type) INNER JOIN %s.planet p ON (p.planet_id = s.zone)WHERE ID = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),asynContainer->harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_StructureMailDamage,0);
            asyncContainer->harvesterID = asynContainer->harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

        }

        if(exitCode == 3)// 1 structure is out of maintenance
        {
            // get the Owners ID
            int8 sql[500];

            //start by using power
            sprintf(sql,"SELECT s.owner, st.stf_file, st.stf_name, s.x, s.z, p.name, st.max_condition, st.maint_cost_wk, s.lastMail FROM %s.structures s INNER JOIN %s.structure_type_data st ON (s.type = st.type) INNER JOIN %s.planet p ON (p.planet_id = s.zone)WHERE ID = %" PRIu64 "",mDatabase->galaxy(),mDatabase->galaxy(),mDatabase->galaxy(),asynContainer->harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_StructureMailCondZero,0);
            asyncContainer->harvesterID = asynContainer->harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

        }


        if(exitCode > 3)
        {
            //unspecified db error

            //most likely the structure reached condition zero and awaits destruction
        }


        mDatabase->destroyDataBinding(binding);

    }
    break;

    case STRMQuery_DoneHarvesterUsePower:
    {
        uint32 exitCode;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        if (!result->getRowCount()) {
           	return;
        }

        //return codes :
        // 0 everything ok
        // 1 structure is out of power
        // 2
        // 3 unspecified db error


        result->getNextRow(binding,&exitCode);

        // cant notify zone when no player is online
        // zone will update harvesterstatus once per hour

        // only reason for us tzo handle this event here  would be if we wanted to inform the player

        if(exitCode == 3)
        {
            //unspecified db error
            LOG(WARNING) << "StructureManagerChat::HarvesterPowerUsage "<< asynContainer->harvesterID <<" unspecified db error" ;
        }


        mDatabase->destroyDataBinding(binding);

    }
    break;

    case STRMQuery_DoneFactoryUpdate:
    {
        uint32 exitCode;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint64 count;
        count = result->getRowCount();

        //return codes :
        // 0 everything ok
        // 1
        // 2
        // 3


        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&exitCode);

            if(exitCode == 1)
            {
                //resource never existed in the first place
            }
            if(exitCode == 2)
            {
                //resource never existed in the first place
            }
            if(exitCode == 3)
            {
                //resource never existed in the first place
            }

        }

        mDatabase->destroyDataBinding(binding);


    }
    break;

    case STRMQuery_DoneHarvestUpdate:
    {
        uint32 exitCode;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint32,0,4);

        uint64 count;
        count = result->getRowCount();

        //return codes :
        // 0 everything ok
        // 1 hopper full harvester turned of
        // 2 resource isnt active anymore
        // 3 resource doesnt exist in the first place


        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&exitCode);

            if(exitCode == 1)
            {
                //resource never existed in the first place
				DLOG(INFO ) << "StructureMabagerChat::Harvester "<<asynContainer->harvesterID << " hopper full";
            }
            if(exitCode == 2)
            {
                //resource never existed in the first place
                DLOG(INFO) << "StructureMabagerChat::Harvester "<< asynContainer->harvesterID << " resourcechange";
            }
            if(exitCode == 3)
            {
                //resource never existed in the first place
               DLOG(INFO) << "StructureMabagerChat::Harvester "<< asynContainer->harvesterID <<" harvested an invalid resource";
            }

        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case STRMQuery_MaintenanceUpdate:
    {
        uint64 harvesterID;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,0,8);

        uint64 count;
        count = result->getRowCount();

        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&harvesterID);

            int8 sql[100];

            // then use maintenance
            sprintf(sql, "SELECT %s.sf_HarvesterUseMaintenance(%" PRIu64 ")", mDatabase->galaxy(),  harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_DoneStructureMaintenance,0);
            asyncContainer->harvesterID = harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

        }

        mDatabase->destroyDataBinding(binding);
    }
    break;

    case STRMQuery_PowerUpdate:
    {
        uint64 harvesterID;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,0,8);

        uint64 count;
        count = result->getRowCount();

        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&harvesterID);

            int8 sql[100];

            //start by using power
            sprintf(sql, "SELECT %s.sf_HarvesterUsePower(%" PRIu64 ")", mDatabase->galaxy(), harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_DoneHarvesterUsePower,0);
            asyncContainer->harvesterID = harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

            //return codes :
            // 0 everything ok
            // 1 hopper full harvester turned of
            // 2 resource isnt active anymore
            // 3 resource doesnt exist in the first place

        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case STRMQuery_FactoryUpdate:
    {
        uint64 factoryID;
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,0,8);

        uint64 count;
        count = result->getRowCount();

        for(uint64 i=0; i <count; i++)
        {
            result->getNextRow(binding,&factoryID);

            int8 sql[100];

            //now harvest
            sprintf(sql, "SELECT %s.sf_FactoryProduce(%" PRIu64 ")", mDatabase->galaxy(), factoryID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_DoneFactoryUpdate,0);
            asyncContainer->harvesterID = factoryID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

            //return codes :
            // 0 everything ok
            // 1 single item created no crate
            // 2 hopper full
            // 3 other fault - attrib / table not found

        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    case STRMQuery_HopperUpdate:
    {
        
        DataBinding* binding = mDatabase->createDataBinding(1);
        binding->addField(DFT_uint64,0,8);

        uint64 count;
        count = result->getRowCount();

        for(uint64 i=0; i <count; i++)
        {
            // clear out the value
            uint64 harvesterID = 0;

            result->getNextRow(binding,&harvesterID);
            if (harvesterID == 0)
                continue;

            int8 sql[100];

            //now harvest
            sprintf(sql, "SELECT %s.sf_HarvestResource(%" PRIu64 ")", mDatabase->galaxy(), harvesterID);
            StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_DoneHarvestUpdate,0);
            asyncContainer->harvesterID = harvesterID;

            mDatabase->executeSqlAsync(this,asyncContainer,sql);

            //return codes :
            // 0 everything ok
            // 1 hopper full harvester turned of
            // 2 resource isnt active anymore
            // 3 resource doesnt exist in the first place



        }

        mDatabase->destroyDataBinding(binding);

    }
    break;

    default:
        break;
    }
    SAFE_DELETE(asynContainer);
}

//=======================================================================================================================

//=======================================================================================================================


//=======================================================================================================================
void StructureManagerChatHandler::handleTimer(uint32 id, void* container)
{
    boost::mutex::scoped_lock lock(mTimerMutex);
    mTimerEventQueue.push(id);
}

//=======================================================================================================================
void StructureManagerChatHandler::processTimerEvents()
{
    uint64	startTime = Anh_Utils::Clock::getSingleton()->getLocalTime();
    uint64	processTime = 0;
    while(mTimerEventQueue.size() && processTime < mTimerQueueProcessTimeLimit)
    {
        boost::mutex::scoped_lock lock(mTimerMutex);
        uint32 id = mTimerEventQueue.front();
        mTimerEventQueue.pop();
        lock.unlock();

        switch(id)
        {
            //handles the harvesters Maintenance updates
        case SRMTimer_CheckHarvesterMaintenance:
        {
            handleCheckHarvesterMaintenance();
        }
        break;

        //handles the harvesters hopper and power -updates
        case SRMTimer_CheckHarvesterHopper:
        {
            handleCheckHarvesterHopper();
        }
        break;

        case SRMTimer_CheckFactory:
        {
            handleFactoryUpdate();
        }
        break;

        case SRMTimer_CheckHarvesterPower:
        {
            handleCheckHarvesterPower();
        }
        break;

        default:
            break;
        }

        processTime = Anh_Utils::Clock::getSingleton()->getLocalTime() - startTime;
    }
}

//=======================================================================================================================
//
// iterates through the list of hoppers we need to update on a regular basis
// and reads the relevant resource data - then sends it to the client
//
void StructureManagerChatHandler::handleCheckHarvesterPower()
{

    StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_PowerUpdate,0);

    int8 sql[100];
    sprintf(sql,"SELECT h.ID FROM %s.harvesters h WHERE h.active > 0 ",mDatabase->galaxy());

    mDatabase->executeSqlAsync(this,asyncContainer,sql);

}


//=======================================================================================================================
//
// iterates through the list of hoppers we need to update on a regular basis
// and reads the relevant resource data - then sends it to the client
//
void StructureManagerChatHandler::handleCheckHarvesterHopper()
{

    StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_HopperUpdate,0);

    int8 sql[100];
    sprintf(sql,"SELECT h.ID FROM %s.harvesters h WHERE h.active > 0 ",mDatabase->galaxy());

    mDatabase->executeSqlAsync(this,asyncContainer,sql);

}

//=======================================================================================================================
//
// iterates through the list of hoppers we need to update on a regular basis
// and reads the relevant resource data - then sends it to the client
//
void StructureManagerChatHandler::handleFactoryUpdate()
{

    StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_FactoryUpdate,0);

    int8 sql[100];
    sprintf(sql,"SELECT f.ID FROM %s.factories f WHERE f.active > 0 ",mDatabase->galaxy());

    mDatabase->executeSqlAsync(this,asyncContainer,sql);

}


//=======================================================================================================================
//
// iterates through all structures in structures.sql to take off maintenance
//
void StructureManagerChatHandler::handleCheckHarvesterMaintenance()
{

    StructureManagerAsyncContainer* asyncContainer = new StructureManagerAsyncContainer(STRMQuery_MaintenanceUpdate,0);

    int8 sql[100];
    sprintf(sql,"SELECT s.ID FROM %s.structures s",mDatabase->galaxy());

    mDatabase->executeSqlAsync(this,asyncContainer,sql);

}

//=======================================================================================================================
void StructureManagerChatHandler::Process()
{
    processTimerEvents();
}


//=======================================================================================================================
//
// This adds or deletes a harvester to /from the hopperupdate list
// All harvesters on this list will have a hopperupdate send every few seconds
//
void StructureManagerChatHandler::ProcessAddHarvesterHopperUpdate(Message* message,DispatchClient* client)
{
    mPlayerAccountMap = mChatManager->getPlayerAccountMap();

    uint64	harvesterID		= message->getUint64();
    uint8   activation		= message->getUint8();
    uint32  updateCounter	= message->getUint32();

    if(activation)
    {
        HarvesterList* hList = getHarvesterList();

        HarvesterItem* item = new(HarvesterItem);
        item->HarvesterID		= harvesterID;
        item->PlayerAccount		= client->getAccountId();
        item->UpdateCounter		= updateCounter;

        hList->push_back(item);
    }
    else
    {

        //TODO update the zone on the new update counter
        HarvesterList* hList = getHarvesterList();
        HarvesterList::iterator it = hList->begin();
        while(it != hList->end())
        {
            if((*it)->HarvesterID == harvesterID)
            {
                delete (*it);
                it = hList->erase(it);
                return;
            }
            it++;
        }

    }
}


/*
Harvesters
when a harvester gets placed the relevant attributes of the deed are copied over to the harvester.
these attributes are :

381 examine_hoppersize
382 examine_maintenance
383 examine_maintenance_rate
384 examine_power
397 extractrate
1004 harvester_efficiency
1005 harvester_maintenance
1006 harvester_speed

forget 1004 to 1006 were using the former set

then the harvester is created in the db by putting the structure details in the structure table and the
harvester details - like active / current resource in the harvesters table
*/
