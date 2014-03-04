/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2014 The SWG:ANH Team
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
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/GameSystemManagers/UI Manager/UIManager.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"

#include "Common/atMacroString.h"

#include "UIInputBox.h"
#include "UIListBox.h"
#include "UIMessageBox.h"
#include "UIOpcodes.h"


//#include "NetworkManager/DispatchClient.h"
//#include "NetworkManager/Message.h"
//#include "NetworkManager/MessageDispatch.h"
//#include "NetworkManager/MessageFactory.h"

//======================================================================================================================


//======================================================================================================================
void UIManager::createNewDiagnoseListBox(UICallback* callback,PlayerObject* Medic, PlayerObject* Patient, StringVector vector_string)
{

    std::string first_name = Patient->GetCreature()->getFirstName();
	
	uint8	id = 0;
	std::locale loc;
	for (std::string::size_type i=0; i < first_name.length(); ++i)
		first_name[i]= std::toupper(first_name[i],loc);

	
    std::string last_name = Patient->GetCreature()->getLastName();
    for (std::string::size_type i=0; i < last_name.length(); ++i)
		last_name[i]= std::toupper(last_name[i],loc);

    int8 title[64];
    sprintf(title,"PATIENT %s %s'S WOUNDS",first_name, last_name);

    int8 desc[512];
    sprintf(desc, "Below is a listing of the Wound and Battle Fatigue levels of %s %s. Wounds are healed through /tendWound or use of Wound Medpacks. High levels of Battle Fatigue can inhibit the healing process, and Battle Fatigue can only be healed by the patient choosing to watch performing entertainers",Patient->GetCreature()->getFirstName(), Patient->GetCreature()->getLastName());

    createNewListBox(callback,"handleDiagnoseMenu",title, desc, vector_string, Medic, SUI_Window_ListBox,SUI_LB_OK,Patient->getId());
}
