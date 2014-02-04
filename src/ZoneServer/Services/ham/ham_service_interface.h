// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include "anh/service/service_interface.h"

class CreatureObject;

namespace swganh
{
namespace ham
{
class HamServiceInterface : public swganh::service::BaseService
{
public:

	virtual			~HamServiceInterface() {}
	
	/*	@brief updates a creatures Ham
	*
	*	/param CreatureObject* creature
	*	/param uint8 propertyIndex			the property to access strength or will or whatever
	*	/param uint8 valueIndex				the index of our value (base strength; wounds to strength or maxstrength etc...
	*	/param int32 propertyDelta			the delta of the value were going to change
	*	/returns int32_t					the amount of healing done
	*
	*	This function will trigger delta and persist events
	*/
	//virtual	int32	updatePropertyValue(CreatureObject* creature, uint8 propertyIndex,uint8 valueIndex,int32 propertyDelta) = 0;
	
	virtual	int32_t	UpdateCurrentHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update) = 0;
	
	virtual	bool	UpdateBaseHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update) = 0;

	virtual int32_t		ApplyWound(CreatureObject* creature, uint16_t statIndex, uint32_t update) = 0;

	virtual int32_t		RemoveWound(CreatureObject* creature, uint16_t statIndex, uint32_t update) = 0;

	//virtual bool		ApplyHamCost(CreatureObject* creature, uint16_t statIndex, uint32_t cost) = 0;

	//virtual bool		ApplyHamCosts(CreatureObject* creature, uint32_t healthCost, uint32_t actionCost, uint32_t mindCost) = 0;

    

};
}
}
