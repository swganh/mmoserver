
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "PlayerStructure.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "WorldManager.h"


//=============================================================================

PlayerStructure::PlayerStructure() : TangibleObject()
{
	
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
		return(false);
	}

	if(!(this->getCurrentMaintenance() >= (this->getMaintenanceRate()*45)))
	{
		return(false);
	}
	return true;
}

//=============================================================================
//
//
void PlayerStructure::deleteStructureDBDataRead(uint64 playerId)
{
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId));

	gUIManager->createNewStructureDestroyBox(this,player, this, canRedeed());

}