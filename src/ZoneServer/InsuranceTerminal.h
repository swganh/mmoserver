/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_INSURANCETERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_INSURANCETERMINAL_H

#include "Terminal.h"
#include "Inventory.h"

//=============================================================================

class InsuranceTerminal : public Terminal
{
	friend class TerminalFactory;

	public:

		InsuranceTerminal();
		~InsuranceTerminal();

		void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);
		void		handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

	private:
		void		getUninsuredItems(PlayerObject* playerObject, BStringVector* insuranceList);
		void		getInsuredItems(PlayerObject* playerObject, BStringVector* insuranceList);

		SortedInventoryItemList mSortedInsuranceList;

	
		

};

//=============================================================================

#endif

