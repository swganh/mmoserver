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

	/**	@brief adds or removes BattleFatiguee. Performs sanity checks on bf size and updates.
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint32_t update is the amount
	*/
	virtual int32_t		UpdateBattleFatigue(CreatureObject* creature, const int32_t update) = 0;

	/**	@brief calculates the hamcost for an action based on the creatures hamstats and applies it to the ham if all costs can be met
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param int32_t *_cost is the raw cost for the relevant Main Hambar
	*	@returns whether the calculated costs could be applied
	*/
	virtual bool		ApplyModifiedHamCosts(CreatureObject* creature, int32_t healthCost, int32_t actionCost, int32_t mindCost) = 0;

	/**	@brief adds a wound to the relevant ham element. Performs sanity checks on wound size and updates the hitpoints if necessary
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*/
	virtual int32_t		ApplyWound(CreatureObject* creature, uint16_t statIndex, uint32_t update) = 0;

	/**	@brief adds or removes a wound to the relevant ham element. Performs sanity checks on wound size and updates the hitpoints if necessary
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@returns int32_tz the amount of wounds applied / removed
	*/
	virtual int32_t		UpdateWound(CreatureObject* creature, uint16_t statIndex, int32_t update) = 0;

	/**	@brief Modifies a Hambars MaxHam Value. The currenthitpoints will be adapted. This will trigger a persist and serialize event
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param const uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@param const int32_t update is the amount we are going to change the MaxHam for.
	*/
	virtual int32_t		ModifyMaxHam(CreatureObject* creature, const uint16_t statIndex, const int32_t update) = 0;
	
	/**	@brief heals a wound of the relevant ham element This will trigger a persist and serialize event
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param const uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@param const int32_t update is the amount we are going to change the MaxHam for.
	*/
	virtual int32_t		RemoveWound(CreatureObject* creature, uint16_t statIndex, uint32_t update) = 0;
	
	/**	@brief updates a creatures current hitpoints (hit while doing combat)
	*
	*	@param CreatureObject* creature
	*	@param uint16_t						statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@param int32_t update				the delta of the value we are going to change
	*
	*	This function will trigger delta and persist events
	*/
	virtual	int32_t	UpdateCurrentHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update) = 0;
	
	/*	@brief updates a creatures base hitpoints (eg image design)
	*	@param CreatureObject* creature
	*	@param uint16_t statIndex			the property to access strength or will or whatever
	*	@param int32_t update				the delta of the value we are going to change
	*
	*	This function will trigger delta and persist events
	*	the current hitpoints will be affected as well
	*/
	virtual	bool	UpdateBaseHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update) = 0;

	/**
     * This method checks the main pools and verifies that each is
     * currently greater than respective value that is passed in.
	 * @param  CreatureObject* creature is the creatureobject we want the check being made for
     * @param health Value that the health pool should be greater than.
     * @param action Value that the action pool should be greater than.
     * @param mind Value that the mind pool should be greater than.
     * @returns True if the main pools are exceed the specified values, false if not.
     */
	virtual bool	checkMainPools(CreatureObject* creature, const int32_t health,const int32_t action, const int32_t mind) = 0;

	/**
     * @brief This method checks the main pools and verifies that each is currently greater than respective value that is passed in.
	 * @param  CreatureObject* creature is the creatureobject we want the check being made for
     * @param value Value that the tested pool should be greater than.
	 * @param statIndex the stat we want to be testing
     * @returns True if the main pools are exceed the specified values, false if not.
     */
    virtual bool	checkMainPool(CreatureObject* creature, const uint16_t statIndex, const int32_t value) = 0;

	                                                                                    
	//virtual	bool	ApplyHamCosts(CreatureObject* creature, uint32_t healthCost, uint32_t actionCost, uint32_t mindCost) = 0;

	//virtual bool	ApplyHamCost(CreatureObject* creature, uint16_t statIndex, uint32_t cost) = 0;

	//virtual bool		ApplyHamCost(CreatureObject* creature, uint16_t statIndex, uint32_t cost) = 0;

	//virtual bool		ApplyHamCosts(CreatureObject* creature, uint32_t healthCost, uint32_t actionCost, uint32_t mindCost) = 0;

    

};
}
}
