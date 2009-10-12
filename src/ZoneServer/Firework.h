/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_FIREWORK_H
#define ANH_ZONESERVER_TANGIBLE_FIREWORK_H

#include "Item.h"
#include <vector>

class FireworkEvent;

//=============================================================================
struct _fireworkShowEvent
{
	uint32 typeId;
	int32 delay;
	uint64 itemId;
};

typedef std::vector<_fireworkShowEvent> FireworkShowList;

class Firework : public Item
{
	friend class ItemFactory;

	public:

		Firework();
		virtual ~Firework();
		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);

		uint32			getDelay(){ return delay; };
		void			setDelay(uint32 _delay){ delay = _delay; };

	private:
		uint32			delay;

};

//=============================================================================

class FireworkShow : public Item
{
	friend class ItemFactory;

	public:

		FireworkShow();
		virtual ~FireworkShow();
		
		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		virtual void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void			handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		void			handleUIEvent(string available, string delay, UIWindow* window);
		void			onLaunch(const FireworkEvent* event);


	private:
		ObjectList*		_getInventoryFireworks(PlayerObject* playerObject);
		string			_getType(uint32 type);

		FireworkShowList	fireworkShowList;
		uint32				fireworkShowListModify; //Bit of a filthy hack, but couldn't find an alternative due to ui callback system

		//ObjectList*		objList;

};

//=============================================================================

#endif