// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#pragma once

#include "ham_service_interface.h"
#include "anh/app/swganh_kernel.h"
#include <boost/thread/mutex.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <map>
//#include <list>
#include <cstdint>
#include <memory>

namespace Anh_Utils
{
class Clock;
class Scheduler;
}

typedef std::pair<boost::posix_time::ptime, uint64> RegenerationQueueType;

enum BarIndex
{
    HamBar_Health		=	0,
    HamBar_Strength		=	1,
    HamBar_Constitution	=	2,
    HamBar_Action		=	3,
    HamBar_Quickness	=	4,
    HamBar_Stamina		=	5,
    HamBar_Mind			=	6,
    HamBar_Focus		=	7,
    HamBar_Willpower	=	8
};

enum ValueIndex
{
    //Current hitpoints is what you currently have after that MOB bit a bite out of you
    HamProperty_CurrentHitpoints	=	0,

    //MaxHitpoints are the Hitpoints you can have maximally with Modifiers applied (NO WOUNDS)
    HamProperty_MaxHitpoints		=	1,
    HamProperty_Modifier			=	2,
    HamProperty_Encumbrance			=	3,
    HamProperty_Wounds				=	4,

    //Modified Hitpoints are the virtual maximal hitpoints you have available
    //basically its Maxhitpoints without Wounds
    HamProperty_ModifiedHitpoints	=	5,

    //Basehitpoints are the UNMODIFIED base hitpoints you have without Wounds or Buffs or loss of hitpoints
    HamProperty_BaseHitpoints		=	6
};

namespace swganh
{
namespace ham
{

class HamService: public swganh::ham::HamServiceInterface
{
public:

    HamService(swganh::app::SwganhKernel* kernel);
    ~HamService();

    virtual void Initialize();
	
	/**	@brief adds or removes BattleFatiguee. Performs sanity checks on bf size and updates.
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint32_t update is the amount
	*/
	int32_t				UpdateBattleFatigue(CreatureObject* creature, const int32_t update);

	/**	@brief calculates the hamcost for an action based on the creatures hamstats and applies it to the ham if all costs can be met
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param int32_t *_cost is the raw cost for the relevant Main Hambar
	*	@returns whether the calculated costs could be applied
	*/
	bool				ApplyModifiedHamCosts(CreatureObject* creature, int32_t healthCost, int32_t actionCost, int32_t mindCost);

	/**	@brief adds a wound to the relevant ham element. Performs sanity checks on wound size and updates the hitpoints if necessary
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*/
	virtual int32_t		ApplyWound(CreatureObject* creature, uint16_t statIndex, uint32_t update);
	
	/**	@brief adds or removes a wound to the relevant ham element. Performs sanity checks on wound size and updates the hitpoints if necessary
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@returns int32_tz the amount of wounds applied / removed
	*/
	virtual int32_t		UpdateWound(CreatureObject* creature, uint16_t statIndex, int32_t update);

	/**	@brief Modifies a Hambars MaxHam Value. The currenthitpoints will be adapted. This will trigger a persist and serialize event
	*	@param CreatureObject* creature The CreatureObject we will be manipulating
	*	@param const uint16_t statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@param const int32_t update is the amount we are going to change the MaxHam for.
	*/
	virtual int32_t		ModifyMaxHam(CreatureObject* creature, const uint16_t statIndex, const int32_t update);
	

	//heals a wound of the relevant ham element
	//performs sanity checks on wound size and updates the hitpoints if necessary
	//
	virtual int32_t		RemoveWound(CreatureObject* creature, uint16_t statIndex, uint32_t update);


	/*	@brief updates a creatures Ham
	*
	*	@param CreatureObject* creature
	*	@param uint8 propertyIndex			the property to access strength or will or whatever
	*	@param uint8 valueIndex				the index of our value (base strength; wounds to strength or maxstrength etc...
	*	@param int32 propertyDelta			the delta of the value were going to change
	*
	*	This function will trigger delta and persist events
	*/
	//virtual	int32	updatePropertyValue(CreatureObject* creature, uint8 propertyIndex,uint8 valueIndex,int32 propertyDelta);

	/*	@brief updates a creatures current hitpoints (hit while doing combat)
	*
	*	@param CreatureObject* creature
	*	@param uint16_t						statIndex desctribes the stat we want to manipulate (will / health action whatever)
	*	@param int32_t update				the delta of the value we are going to change
	*
	*	This function will trigger delta and persist events
	*/
	virtual	int32_t	UpdateCurrentHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update);

	/*	@brief updates a creatures base hitpoints (eg image design)
	*
	*	@param CreatureObject* creature
	*	@param uint16_t statIndex			the property to access strength or will or whatever
	*	@param int32_t update				the delta of the value we are going to change
	*
	*	This function will trigger delta and persist events
	*	the current hitpoints will be affected as well
	*/
	virtual	bool	UpdateBaseHitpoints(CreatureObject* creature, const uint16_t statIndex, const int32_t update);

	/*	@brief this will check whether a creature is already on the regeneration queue and add it in case it isnt
	*	@param uint64 id this is the idea of the creature we want to add
	*/
	void			addToRegeneration(uint64 id);

	uint32_t		regenerationModifier(CreatureObject* creature, uint16_t mainstatIndex);
	
	/*	@brief calcModifiedHitPoints will recalculate the modified hitpoints after changes to the creatures ham
	*			have been applied
	*	@param CreatureObject* creature this is the creature we are manipulating
	*	@param const uint16_t statIndex this is the stat of the ham we are manipulating
	*/
	void			calcCurrentHitPoints(CreatureObject* creature, const uint16_t statIndex);

	int32			getCurrentDamage(CreatureObject* creature, const uint16_t statIndex);
    
	/*	@brief this will handle the regeneration tick.
	*	We iterate through the map with the Ids and apply regeneration to every creature on it
	*/
	void			handleTick_(const boost::system::error_code& e);

	/*	@brief calculates the modified hitpoints. These are calculated from the max possible - wounds - encumbrance
	*	@param CreatureObject* creature the creature that gets initialized
	*	@param const uint16_t statIndex this is the stat of the ham we want to calculate the modified hitpoints for
	*/
	int32			getModifiedHitPoints(CreatureObject* creature, const uint16_t statIndex);

    /**
     * This method checks the main pools and verifies that each is
     * currently greater than respective value that is passed in.
     *
	 * @param  CreatureObject* creature is the creatureobject we want the check being made for
     *
     * @param health Value that the health pool should be greater than.
     *
     * @param action Value that the action pool should be greater than.
	 *
     * @param mind Value that the mind pool should be greater than.
     *
     * @returns True if the main pools are exceed the specified values, false if not.
     */
	bool			checkMainPools(CreatureObject* creature, const int32 health,const int32 action, const int32 mind);

	/**
     * @brief This method checks the main pools and verifies that each is currently greater than respective value that is passed in.
     *
	 * @param  CreatureObject* creature is the creatureobject we want the check being made for
     *
     * @param value Value that the tested pool should be greater than.
     *
	 * @param statIndex the stat we want to be testing
	 *
     * @returns True if the main pools are exceed the specified values, false if not.
     */
    bool			checkMainPool(CreatureObject* creature, const uint16_t statIndex, const int32 value);


	bool			ApplyHamCosts(CreatureObject* creature, uint32_t healthCost, uint32_t actionCost, uint32_t mindCost);

	bool			ApplyHamCost(CreatureObject* creature, uint16_t statIndex, uint32_t cost);

	/*	@brief SetHam initializes the Ham Values (Max Base and Current) to the given Value. This is used to initialize creatures on respawn. Optimally a creature where this method is used isnt part of the simulation
	*	
	*	@param CreatureObject* creature Is the creature we want to modify
	*
	*	@param const uint16_t statIndex is the hamindex we want to modify
	*
	*	@param const int32_t value is the value we want to set
	*/
	void			SetHam(CreatureObject* creature, const uint16_t statIndex, const int32_t value);

private:
	/*	@brief regenerate_ will handle the regeneration event for the given creature
	*	@param uint64 id the id of the creature that wants to regenerate
	*	@returns true if the creature is to be removed from the map
	*/
	bool			regenerate_(uint64 id);
	
	boost::asio::deadline_timer					timer_;

	boost::mutex								mutex_;
    std::map<uint64_t,RegenerationQueueType>	reg_;
    boost::mutex								ham_mutex_;
    swganh::app::SwganhKernel*					kernel_;
	
};
}
}
