
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MathLib/Quaternion.h"
#include "PlayerStructure.h"


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
//
//
uint32 PlayerStructure::getMaxMaintenance()
{
	if (this->hasAttribute("structure_maintenance_mod"))
	{
		uint32 maintenance = this->getAttribute<uint32>("structure_maintenance_mod");					
		gLogger->logMsgF("structure maintenance = %u", MSG_NORMAL, maintenance);
		return maintenance;
	}

	gLogger->logMsgF("structure maintenance not set!!!!", MSG_NORMAL);
	setMaxMaintenance(1);
	return 1;
}

//=============================================================================
//
//
void PlayerStructure::setMaxMaintenance(uint32 maintenance)
{
	if (this->hasAttribute("structure_maintenance_mod"))
	{
		this->setAttribute("structure_maintenance_mod",boost::lexical_cast<std::string>(maintenance));
		
	}

	this->addInternalAttribute("structure_maintenance_mod",boost::lexical_cast<std::string>(maintenance));
	gLogger->logMsgF("structure maintenance not set!!!!", MSG_NORMAL);

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

	if(!(this->getMaintenance() == this->getMaxCondition()))
	{
		return(false);
	}
	return true;
}