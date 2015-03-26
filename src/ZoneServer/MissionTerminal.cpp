/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
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
#include "MissionTerminal.h"
#include "PlayerObject.h"
#include "Tutorial.h"
#include "WorldConfig.h"

//=============================================================================

MissionTerminal::MissionTerminal() : Terminal()
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());
}

//=============================================================================

MissionTerminal::~MissionTerminal()
{
}

//=============================================================================

void MissionTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    if (gWorldConfig->isTutorial())
    {
#if defined(_MSC_VER)
        if (this->getId() == 4294968328)
#else
        if (this->getId() == 4294968328LLU)
#endif
        {
            // This is the Misson Terminal in the tutorial.
            PlayerObject* player = dynamic_cast<PlayerObject*>(creatureObject);
            if (player)
            {
                Tutorial* tutorial = player->getTutorial();

                // We will not display this when tutorial is in state 21 or 22. LOL...
                if ((tutorial->getSubState() != 21) && (tutorial->getSubState() != 22))
                {
                    // "You cannot take a mission from this terminal.  To take a mission, you will need to find a mission terminal on a planet."
                    tutorial->scriptSystemMessage("@newbie_tutorial/system_messages:mission_terminal");
                    tutorial->scriptPlayMusic(3397);		// sound/tut_00_mission_terminal.snd
                }
            }
        }
    }
}


//=============================================================================

