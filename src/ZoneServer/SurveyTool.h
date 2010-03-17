/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_SURVEYTOOL_H
#define ANH_ZONESERVER_TANGIBLE_SURVEYTOOL_H

#include "Item.h"

//=============================================================================

class SurveyTool : public Item
{
	friend class SurveyToolObjectFactory;

	public:

		SurveyTool();
		virtual ~SurveyTool();

		virtual void	prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
		void	handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void	handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);
		void	StartUsing(PlayerObject* player);


	private:

		void	_createRangeMenu(PlayerObject* playerObject, bool sample = false);

		bool	mSampleAfterSet;
};

//=============================================================================

#endif



