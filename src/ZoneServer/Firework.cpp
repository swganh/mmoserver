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

#include "Firework.h"
#include "FireworkEvent.h"
#include "FireworkManager.h"
#include "Inventory.h"
#include "Item.h"
#include "ObjectFactory.h"
#include "WorldManager.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "MessageLib/MessageLib.h"
#include "DatabaseManager/Database.h"

//=============================================================================

Firework::Firework() : Item()
{

}

//=============================================================================

Firework::~Firework()
{
}

//=============================================================================


void Firework::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
    mRadialMenu	= RadialMenuPtr(new RadialMenu());
    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
    mRadialMenu->addItem(2,0,radId_examine,radAction_ObjCallback,"");
    mRadialMenu->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
    this->delay=0;

}

//=============================================================================

void Firework::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
    {
        switch(messageType)
        {
            // When player hits launch on fireworks
        case radId_itemUse:
        {
            //make sure it is a firework
            if(this->getItemType() >= ItemType_Firework_Type_5 || this->getItemType() <= ItemType_Firework_Type_2)
            {
                //Player must be standing or Kneeling to launch
                if(!playerObject->states.checkPosture(CreaturePosture_Upright) && !playerObject->states.checkPosture(CreaturePosture_Crouched))
                {
                    gMessageLib->SendSystemMessage(L"You must be standing or kneeling to start a firework.", playerObject);
                    return;
                }

				//player can not be swimming -> not working yet. The server do not update the state when walking in water
				if (playerObject->states.checkState(CreatureState_Swimming))
				{
					gMessageLib->SendSystemMessage(L"You can not do that while swimming!", playerObject);
					return;
				}

                else if(playerObject->getParentId())
                {
                    gMessageLib->SendSystemMessage(L"You can not do this while indoors.", playerObject);
                    return;
                }
                //Create the Firework in the world
                TangibleObject* fireWorkInworld = gFireworkManager->createFirework(this->getItemType(),playerObject,playerObject->mPosition);

                int charges = (int)this->getAttribute<float>("charges");
                charges--;

                if(fireWorkInworld)
                {
                    if (charges)
                    {
                        this->setAttribute("charges",boost::lexical_cast<std::string>(charges));
                        gWorldManager->getDatabase()->executeSqlAsync(0,0,"UPDATE %s.item_attributes SET value='%f' WHERE item_id=%" PRIu64 " AND attribute_id=%u",gWorldManager->getDatabase()->galaxy(),charges,this->getId(), AttrType_Charges);
                        
                        //now update the uses display
                        gMessageLib->sendUpdateUses(this,playerObject);
                        return;
                    }
                    else
                    {
                        playerObject->getController()->destroyObject(this->getId());
                    }
                }

                //now add timer somewhere to make them start

                //then delete the
            }
        }
        break;

        default:
            break;
        }
    }
}

//=============================================================================


//=============================================================================

FireworkShow::FireworkShow() : Item()
{
    mRadialMenu	= RadialMenuPtr(new RadialMenu());
    mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
    mRadialMenu->addItem(2,0,radId_examine,radAction_ObjCallback,"");
    mRadialMenu->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
    mRadialMenu->addItem(4,0,radId_fireworkshowData,radAction_ObjCallback,"@firework:mnu_show_data");
    mRadialMenu->addItem(5,4,radId_fireworkshowRemove,radAction_ObjCallback,"@firework:mnu_remove_event");
    mRadialMenu->addItem(6,4,radId_fireworkshowModify,radAction_ObjCallback,"@firework:mnu_modify_event");
    mRadialMenu->addItem(7,4,radId_fireworkshowReorder,radAction_ObjCallback,"@firework:mnu_reorder_show");
    mRadialMenu->addItem(8,4,radId_fireworkshowAdd,radAction_ObjCallback,"@firework:mnu_add_event");

    registerEventFunction(this,&FireworkShow::onLaunch);
    this->fireworkShowListModify=0;

    //objList = new ObjectList();
}

//=============================================================================

FireworkShow::~FireworkShow()
{
}

//=============================================================================


void FireworkShow::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
}

//=============================================================================

void FireworkShow::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
    if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
    {
        gMessageLib->SendSystemMessage(L"Firework Show Packagaes are still under development and not ready for testing", playerObject);
        return;
    }
    /*
    if(PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
    {
        switch(messageType)
        {
            // When player hits launch on fireworks
            case radId_itemUse:
            {
                if(this->getItemType() == ItemType_Firework_Show)
                {
                    //If we are inside show an error and return;
                    if(playerObject->getParentId())
                    {
                        gMessageLib->sendSystemMessage(playerObject,L"Launching fireworks indoors? Sorry, but something might catch fire!");
                        return;
                    }

                    //If the show is empty show an error and return
                    if(fireworkShowList.size()<=0)
                    {
                        gMessageLib->sendSystemMessage(playerObject,L"Firework show is empty, please load some fireworks!");
                        return;
                    }

                    //Create firework show
                    gFireworkManager->createFirework(ItemType_Firework_Show,playerObject,playerObject->mPosition);

                    //Loop thourg list and add show events
                    FireworkShowList::iterator containerObjectIt = fireworkShowList.begin();
                    int fireworkNumber=0;
                    int totalDelay=0;
                    while (containerObjectIt != fireworkShowList.end())
                    {
                        _fireworkShowEvent event = (*containerObjectIt);
                        this->getController()->addEvent(new FireworkEvent(event.typeId,playerObject,playerObject->mPosition), event.delay*100);

                        playerObject->getController()->destroyObject(event.itemId);
                        totalDelay+=event.delay*100;
                        ++containerObjectIt;
                    }
                    fireworkShowList.clear();
                    //event: destroy fireworkshow in world, at: totalDelay
                    //gFireworkManager->createFirework(ItemType_Firework_Show,playerObject,playerObject->mPosition);
                    //this->getController()->addEvent(new FireworkEvent(1756,playerObject,playerObject->mPosition), 2000);
                    //this->getController()->addEvent(new FireworkEvent(1756,playerObject,playerObject->mPosition), 4000);
                    //this->getController()->addEvent(new FireworkEvent(1756,playerObject,playerObject->mPosition), 6000);
                    //this->getController()->addEvent(new FireworkEvent(1756,playerObject,playerObject->mPosition), 8000);
                }

            }
            break;

            case radId_fireworkshowAdd:
            {
                BStringVector		mFireworkList;
                mFireworkList.clear();
                int8 text[64];


                ObjectList* fireworks = _getInventoryFireworks(playerObject);
                ObjectList::iterator containerObjectIt = fireworks->begin();
                while (containerObjectIt != fireworks->end())
                {
                    Object* object = (*containerObjectIt);
                    if (Item* item = dynamic_cast<Item*>(object))
                    {
                        sprintf(text,"%s",item->getCustomName().getAnsi());
                        mFireworkList.push_back(text);
                    }
                    ++containerObjectIt;
                }
                gUIManager->createNewListBox(this,"handleFireworkAdd","Select Show Addition","Select the Firework to append to the end of the show package.", mFireworkList,playerObject,SUI_Window_FireworkShow_Add,SUI_LB_OKCANCEL);
                }
            break;

            case radId_fireworkshowRemove:
            {

                BStringVector		mFireworkList;
                mFireworkList.clear();
                int8 text[512];

                FireworkShowList::iterator containerObjectIt = fireworkShowList.begin();
                int fireworkNumber=0;
                while (containerObjectIt != fireworkShowList.end())
                {
                    _fireworkShowEvent event = (*containerObjectIt);

                    float delay = event.delay;
                    delay/=10;

                    sprintf(text,"(%i:%0.1fs) %s",fireworkNumber++,delay,_getType(event.typeId).getAnsi());

                    mFireworkList.push_back(text);
                    ++containerObjectIt;
                }
                gUIManager->createNewListBox(this,"handleFireworkRemove","Select Remove","Select the Firework to Remove.", mFireworkList,playerObject,SUI_Window_FireworkShow_Remove,SUI_LB_OKCANCEL);

            }
            break;

            case radId_fireworkshowModify:
            {
                this->fireworkShowListModify=0;	//Bit of a filthy hack, but couldn't find an alternative due to ui callback system

                BStringVector		mFireworkList;
                mFireworkList.clear();
                int8 text[512];

                FireworkShowList::iterator containerObjectIt = fireworkShowList.begin();
                int fireworkNumber=0;
                while (containerObjectIt != fireworkShowList.end())
                {
                    _fireworkShowEvent event = (*containerObjectIt);

                    float delay = event.delay;
                    delay/=10;

                    sprintf(text,"(%i:%0.1fs) %s",fireworkNumber++,delay,_getType(event.typeId).getAnsi());

                    mFireworkList.push_back(text);
                    ++containerObjectIt;
                }
                gUIManager->createNewListBox(this,"handleFireworkModify","@firework:modify_index_title","@firework:modify_index_prompt", mFireworkList,playerObject,SUI_Window_FireworkShow_Modify,SUI_LB_OKCANCEL);


            }
            break;

            case radId_fireworkshowReorder:
            {

            }
            break;

            default: break;
        }
    }
    */
}

void FireworkShow::handleUIEvent(uint32 action,int32 element,BString inputStr,UIWindow* window)
{
}

void FireworkShow::handleUIEvent(BString strAvailable, BString strDelay, UIWindow* window)
{
    if(window == NULL)
    {
        return;
    }
    //uint32 delay = atoi(strDelay.getAnsi());
    strDelay.convert(BSTRType_ANSI);
    int32 delay = atoi(strDelay.getAnsi());
    fireworkShowList.at(this->fireworkShowListModify).delay = delay;

}
void FireworkShow::onLaunch(const FireworkEvent* event)
{
    gFireworkManager->createFirework(1756,event->getPlayerObject(),event->getPosition());
}

ObjectList* FireworkShow::_getInventoryFireworks(PlayerObject* playerObject)
{
    ObjectList* returnList = new ObjectList();

    Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
    ObjectIDList*			objList				= inventory->getObjects();
    ObjectIDList::iterator	containerObjectIt	= objList->begin();

    while (containerObjectIt != objList->end())
    {
        Object* object = gWorldManager->getObjectById((*containerObjectIt));
        if (Item* item = dynamic_cast<Item*>(object))
        {
            if(item->getItemFamily()==ItemFamily_FireWork && item->getItemType()!= ItemType_Firework_Show)
            {
                returnList->push_back(item);
            }
        }
        ++containerObjectIt;
    }


    return returnList;
}

BString FireworkShow::_getType(uint32 type)
{
    switch(type)
    {
    case ItemType_Firework_Type_1:
        return BString("Starburst - Light");
    case ItemType_Firework_Type_2:
        return BString("Concussion Rings");
    case ItemType_Firework_Type_3:
        return BString("Chandelier");
    case ItemType_Firework_Type_4:
        return BString("Spiral Tears");
    case ItemType_Firework_Type_5:
        return BString("Starburst - Heavy");
    case ItemType_Firework_Type_10:
        return BString("Type 10 Effect");
    case ItemType_Firework_Type_11:
        return BString("Type 10 Effect");
    case ItemType_Firework_Type_18:
        return BString("Type 10 Effect");
    default:
        return BString("UNKNOWN");
    }
}
//=============================================================================
