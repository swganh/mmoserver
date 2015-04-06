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

#include "CloningTerminal.h"
#include "Bank.h"
#include "PlayerObject.h"
#include "TreasuryManager.h"
#include "ZoneServer/Tutorial.h"
#include "UIManager.h"
#include "ZoneServer/WorldConfig.h"
#include "WorldManager.h"
#include "MessageLib/MessageLib.h"

#include "DatabaseManager/Database.h"


// TODO: Make this a Var when we start use the 20% discount from player city with "Clone Lab" specializations.
const int32 cloningCost = 1000;

//=============================================================================

CloningTerminal::CloningTerminal() : Terminal()
{
    mRadialMenu = RadialMenuPtr(new RadialMenu());

    // any object with callbacks needs to handle those (received with menuselect messages) !
    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
    mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

CloningTerminal::~CloningTerminal()
{
}

//=============================================================================

void CloningTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    PlayerObject* playerObject = (PlayerObject*)srcObject;

    switch(messageType)
    {
    case radId_itemUse:
    {
        if (playerObject)
        {
            if (playerObject->isConnected())
            {
                if (gWorldConfig->isTutorial())
                {
                    if (this->getParentId() && gWorldManager->getObjectById(this->getParentId())->getParentId())
                    {
                        // We are located inside a building.

                        // Update player with pre-des cloning facility. It's a terminal in a cell in a building...
                        playerObject->setPreDesignatedCloningFacilityId(gWorldManager->getObjectById(this->getParentId())->getParentId());

                        // store the location where we are bind
                        playerObject->setBindPlanet((uint8)gWorldManager->getZoneId());

                        const glm::vec3& bindPosition = gWorldManager->getObjectById(playerObject->getPreDesignatedCloningFacilityId())->mPosition;
                        playerObject->setBindCoords(bindPosition);

                        // TODO: We need to save the current data before creating the clone data.

                        int8 sql[128];
                        sprintf(sql,"call %s.sp_CharacterCreateClone(%" PRIu64 ",%" PRIu64 ")",gWorldManager->getDatabase()->galaxy(), playerObject->getId(),playerObject->getPreDesignatedCloningFacilityId());
                        (gWorldManager->getDatabase())->executeProcedureAsync(NULL,NULL,sql);

                        // Clone location successfully updated
                        gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "clone_success"), playerObject);

                        // Inform Tutorial about the cloning.
                        playerObject->getTutorial()->tutorialResponse("cloneDataSaved");
                    }
                }
                else
                {
                    gUIManager->createNewMessageBox(this,"","@base_player:clone_confirm_title","@base_player:clone_confirm_prompt",playerObject, SUI_Window_MessageBox, SUI_MB_OKCANCEL);
                }
            }
        }
    }
    break;

    default:
        break;
    }
}


//=============================================================================


// void CloningTerminal::handleUIEvent(BString strInventoryCash, string strBankCash, UIWindow* window)
void CloningTerminal::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
    if(window == NULL)
    {
        return;
    }

    PlayerObject* playerObject = window->getOwner(); // window owner

    if(playerObject == NULL || !playerObject->isConnected() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->states.checkState(CreatureState_Combat))
    {
        return;
    }

    if (this->getParentId() && gWorldManager->getObjectById(this->getParentId())->getParentId())
    {
        // We are located inside a building.

        if (action != 1)
        {
            // This is the OK.  (action == 0)
            // TODO: If the player have the "coupon", they should get a special message.
            // For now, we skip the "coupon", because of the risk of player deleting them, how do they advance in the Tutorial then?

            int32 creditsAtBank = (dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->credits());

            // TODO: Some cities have 20% reduction of cloning fee, depending of city status
            if (creditsAtBank < cloningCost)
            {
                if (creditsAtBank == cloningCost - 1)
                {
                    // nsf_clone1       You lack the 1 additional credit required to cover the cost of cloning.
                    gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "nsf_clone1"), playerObject);
                }
                else
                {
                    // You lack the %DI additional credits required to cover the cost of cloning.
                    gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "nsf_clone", 0, 0, 0, cloningCost - creditsAtBank, 0.0f), playerObject);
                }
            }
            else if ((dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->updateCredits(-cloningCost)))
            {
                // The credits is drawn from the player bank.
                // System message: You successfully make a payment of %DI credits to %TO.
                ::common::ProsePackage prose("base_player", "prose_pay_acct_success");
                prose.to_stf_file = "terminal_name";
                prose.to_stf_label = "terminal_cloning";
                prose.di_integer = cloningCost;

                gMessageLib->SendSystemMessage(::common::OutOfBand(prose), playerObject);

                // Update player with pre-des cloning facility. It's a terminal in a cell in a building...
                playerObject->setPreDesignatedCloningFacilityId(gWorldManager->getObjectById(this->getParentId())->getParentId());

                // store the location where we are bind
                playerObject->setBindPlanet((uint8)gWorldManager->getZoneId());

                const glm::vec3& bindPosition = gWorldManager->getObjectById(playerObject->getPreDesignatedCloningFacilityId())->mPosition;
                playerObject->setBindCoords(bindPosition);

                int8 sql[128];
                sprintf(sql,"call %s.sp_CharacterCreateClone(%" PRIu64 ",%" PRIu64 ")",gWorldManager->getDatabase(), playerObject->getId(),playerObject->getPreDesignatedCloningFacilityId());
                (gWorldManager->getDatabase())->executeProcedureAsync(NULL, NULL, sql);


                // Clone location successfully updated
                gMessageLib->SendSystemMessage(::common::OutOfBand("base_player", "clone_success"), playerObject);

                // Re-enable when/if we starts to use the "coupon" for a free cloning in the Tutorial.
                // if (playerObject->isConnected() && gWorldConfig->isTutorial())
                // {
                // 	playerObject->getTutorial()->tutorialResponse("cloneDataSaved");
                // }
            }
            else
            {
                // This is a system error.
                // The player had no credits in his/her bank.

                // You lack the bank funds to complete this transaction request
                gMessageLib->SendSystemMessage(::common::OutOfBand("error_message", "insufficient_funds_bank"), playerObject);
            }
        }
    }
}
//=============================================================================
