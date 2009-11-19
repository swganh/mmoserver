
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "PlayerStructure.h"
#include "PlayerObject.h"
#include "UICallback.h"
#include "UIManager.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"
#include "MathLib/Quaternion.h"

#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
//=============================================================================

PlayerStructure::PlayerStructure() : TangibleObject()
{
	mMaxCondition	= 1000;
	mCondition		= 1000;
	mWillRedeed		= false;
	
}

//=============================================================================
//
//
PlayerStructure::~PlayerStructure()
{

}

//=============================================================================
// gets the maintenance paid into the harvester - CAVE we need to query this asynch!
// as we modify the maintenance pool minutely  through the chatserver
uint32 PlayerStructure::getCurrentMaintenance()
{
	if (this->hasAttribute("examine_maintenance"))
	{
		uint32 maintenance = this->getAttribute<uint32>("examine_maintenance");					
		gLogger->logMsgF("structure maintenance = %u", MSG_NORMAL, maintenance);
		return maintenance;
	}

	gLogger->logMsgF("PlayerStructure::getMaintenance structure maintenance not set!!!!", MSG_NORMAL);
	setCurrentMaintenance(1);
	return 1;
}


//=============================================================================
//
//
void PlayerStructure::setCurrentMaintenance(uint32 maintenance)
{
	if (this->hasAttribute("examine_maintenance"))
	{
		this->setAttribute("examine_maintenance",boost::lexical_cast<std::string>(maintenance));
		
	}

	this->addAttribute("examine_maintenance",boost::lexical_cast<std::string>(maintenance));
	gLogger->logMsgF("PlayerStructure::setMaintenanceRate structure maintenance rate not set!!!!", MSG_NORMAL);

}


//=============================================================================
//   gets the maintenance to be paid hourly for the structures upkeep
//
uint32 PlayerStructure::getMaintenanceRate()
{
	if (this->hasAttribute("examine_maintenance_rate"))
	{
		uint32 maintenance = this->getAttribute<uint32>("examine_maintenance_rate");					
		gLogger->logMsgF("structure maintenance = %u", MSG_NORMAL, maintenance);
		return maintenance;
	}

	gLogger->logMsgF("PlayerStructure::getMaintenanceRate structure maintenance not set!!!!", MSG_NORMAL);
	setMaintenanceRate(1);
	return 1;
}

//=============================================================================
//
//
void PlayerStructure::setMaintenanceRate(uint32 maintenance)
{
	if (this->hasAttribute("examine_maintenance_rate"))
	{
		this->setAttribute("examine_maintenance_rate",boost::lexical_cast<std::string>(maintenance));
		
	}

	this->addAttribute("examine_maintenance_rate",boost::lexical_cast<std::string>(maintenance));
	gLogger->logMsgF("PlayerStructure::setMaintenanceRate structure maintenance rate not set!!!!", MSG_NORMAL);

}

//=============================================================================
//
//
bool PlayerStructure::canRedeed()
{
	if(!(this->getCondition() == this->getMaxCondition()))
	{
		setRedeed(false);
		return(false);
	}

	if(!(this->getCurrentMaintenance() >= (this->getMaintenanceRate()*45)))
	{
		setRedeed(false);
		return(false);
	}
	setRedeed(true);
	return true;
}

//=============================================================================
// now we have the maintenance data we can proceed with the delete UI
//
void PlayerStructure::deleteStructureDBDataRead(uint64 playerId)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	gUIManager->createNewStructureDestroyBox(this,player, this, canRedeed());

}

//=============================================================================
// now we have the adminlist we can proceed to display it
//
void PlayerStructure::sendStructureAdminList(uint64 playerId)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	gMessageLib->sendAdminList(this,player);


}

//=============================================================================
// now we have the hopperlist we can proceed to display it
//
void PlayerStructure::sendStructureHopperList(uint64 playerId)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	gMessageLib->sendHopperList(this,player);


}


//=============================================================================
// 

void PlayerStructure::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{

	PlayerObject* playerObject = window->getOwner();

	// action is zero for ok !!!

	if(!playerObject || action || playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	switch(window->getWindowType())
	{

		case SUI_Window_Structure_Delete:
		{
			//================================
			// now that a decision has been made get confirmation
			gUIManager->createNewStructureDeleteConfirmBox(this,playerObject,this );
		
		}
		break;

		case SUI_Window_Structure_Rename:
		{

			inputStr.convert(BSTRType_ANSI);
			
			if(!inputStr.getLength())
			{
				//hmmm no answer - remain as it is?
				return;
			}

			if(inputStr.getLength() > 68)
			{
				//hmmm no answer - remain as it is?
				gMessageLib->sendSystemMessage(playerObject,L"","player_structure","not_valid_name"); 
				return;

			}
			
			//inputStr.convert(BSTRType_Unicode16);
			this->setCustomName(inputStr.getAnsi());

			gMessageLib->sendNewHarvesterName(this);

			//update db!!!
			// pull the db query

		
			int8	sql[255],end[128],*sqlPointer;

			sprintf(sql,"UPDATE structures SET structures.name = '");
			sprintf(end,"' WHERE structures.ID = %I64u",this->getId());
			sqlPointer = sql + strlen(sql);
			sqlPointer += gWorldManager->getDatabase()->Escape_String(sqlPointer,inputStr.getAnsi(),inputStr.getLength());
			strcat(sql,end);

			gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,sql);

			gLogger->logMsgF("PlayerStructure::Rename Structure sql : %s", MSG_NORMAL,sql);

		}
		break;

		case SUI_Window_Structure_Delete_Confirm:
		{
			inputStr.convert(BSTRType_ANSI);
			if(inputStr.getCrc() == this->getCode().getCrc())
			{
				//delete it
				mTTS.todo = ttE_Delete;
				gStructureManager->addStructureforDestruction(this->getId());
				gMessageLib->sendSystemMessage(playerObject,L"","player_structure","deed_reclaimed");
			}
			else
			{
				int8 text[255];
				sprintf(text,"@player_structure:incorrect_destroy_code");
				gUIManager->createNewMessageBox(NULL,"","SWG::ANH",text,playerObject);
			}
			//we need to get the input
		}
	}
}


