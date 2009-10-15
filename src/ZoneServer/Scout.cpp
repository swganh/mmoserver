		 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Scout.h"
#include "Camp.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"
#include "PlayerObject.h"
#include "ScoutManager.h"
#include "StructureManager.h"
#include "Worldmanager.h"
#include "ZoneOpcodes.h"

#include "MathLib/Quaternion.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

//=============================================================================

Scout::Scout() : Item()
{
}

//=============================================================================

Scout::~Scout()
{
}

//=============================================================================
//handles the radial selection

void Scout::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
	{	
		switch(messageType)
		{
			case radId_itemUse: 
			{
				//make sure it is a camp
				if(this->getItemType() >= ItemType_Camp_basic || this->getItemType() <= ItemType_Camp_quality)
				{
					//place camp TODO check whether camp can be placed ie whether we are in an urban area
					//ie create in the world for all known players
					//Camps use BUIO 3 and 6				
			
					gScoutManager->createCamp(this->getItemType(),0,player->mPosition,"",player);

				}
				
			}
		}
	}
}

//=============================================================================

void Scout::sendAttributes(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return;

	Message* newMessage;

	gMessageFactory->StartMessage();           
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);

	gMessageFactory->addUint32(1 + mAttributeMap.size());

	string	tmpValueStr = string(BSTRType_Unicode16,64);
	string	value;

	tmpValueStr.setLength(swprintf(reinterpret_cast<wchar_t*>(tmpValueStr.getUnicode16()),L"%u/%u",mMaxCondition - mDamage,mMaxCondition));
	
	gMessageFactory->addString(BString("condition"));
	gMessageFactory->addString(tmpValueStr);

	AttributeMap::iterator			mapIt;
	AttributeOrderList::iterator	orderIt = mAttributeOrderList.begin();

	while(orderIt != mAttributeOrderList.end())
	{
		mapIt = mAttributeMap.find(*orderIt);

		gMessageFactory->addString(gWorldManager->getAttributeKey((*mapIt).first));

		value = (*mapIt).second.c_str();
		value.convert(BSTRType_Unicode16);

		gMessageFactory->addString(value);

		++orderIt;
	}
                  
	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================

void Scout::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	
	RadialMenu* radial	= new RadialMenu();
			
	radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
	radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}

