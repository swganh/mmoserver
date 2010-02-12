/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "FactoryCrate.h"
#include "Inventory.h"
#include "Item_Enums.h"
#include "ObjectFactory.h"
#include "PlayerObject.h"
#include "StructureManager.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"

#include "MessageLib/MessageLib.h"

#include "Common/Message.h"
#include "Common/MessageFactory.h"

//#include "MathLib/Quaternion.h"
#include "DatabaseManager/Database.h"
//=============================================================================

FactoryCrate::FactoryCrate() : Item()
{
}

//=============================================================================

FactoryCrate::~FactoryCrate()
{
}

//=============================================================================
//handles the radial selection

void FactoryCrate::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(PlayerObject* player = dynamic_cast<PlayerObject*>(srcObject))
	{
		switch(messageType)
		{
			case radId_itemUse:
			{
				
			}

		}

	}
}

//=============================================================================

void FactoryCrate::sendAttributes(PlayerObject* playerObject)
{

	if(!(playerObject->isConnected()))
		return;

	AttributeMap*				iAttributeMap		= this->getLinkedObject()->getAttributeMap();
	AttributeOrderList*			iAttributeOrderList	= this->getLinkedObject()->getAttributeOrder();

	Message* newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);

	gMessageFactory->addUint32(2 + mAttributeMap.size()+iAttributeMap->size());

	string	tmpValueStr = string(BSTRType_Unicode16,64);
	string	value,aStr;

	tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),50,L"%u/%u",mMaxCondition - mDamage,mMaxCondition));

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

	gMessageFactory->addString(BString("factory_attribs"));
	aStr = "\\#ff0000 --------------";
	aStr.convert(BSTRType_Unicode16);
	gMessageFactory->addString(aStr);

	orderIt = iAttributeOrderList->begin();

	while(orderIt != iAttributeOrderList->end())
	{
		mapIt = iAttributeMap->find(*orderIt);

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

void FactoryCrate::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{

	RadialMenu* radial	= new RadialMenu();

	radial->addItem(1,0,radId_itemUse,radAction_ObjCallback,"");
	radial->addItem(2,0,radId_examine,radAction_ObjCallback,"");
	radial->addItem(3,0,radId_itemDestroy,radAction_ObjCallback,"");
	RadialMenuPtr radialPtr(radial);
	mRadialMenu = radialPtr;
}


TangibleObject*	FactoryCrate::getLinkedObject()
{
	ObjectIDList*			ol = this->getObjects();
	ObjectIDList::iterator	it = ol->begin();

	//just get the first linked object - crates only have the one

	TangibleObject* tO = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById((*it)));
	if(!tO)
	{
		assert(false);
		return NULL;
	}

	return tO;

}

int32 FactoryCrate::decreaseContent(uint32 amount)
{
	uint32 crateAmount = 0;
	if(this->hasAttribute("factory_count"))
	{
		crateAmount = this->getAttribute<int>("factory_count");
	}

	int32 newAmount = crateAmount - amount;

	if(newAmount < 0)
	{
		assert(false);
		return -1;
	}

	this->setAttribute("factory_count",boost::lexical_cast<std::string>(newAmount));
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE item_attributes SET value='%i' WHERE item_id=%"PRIu64" AND attribute_id=%u",newAmount,this->getId(),AttrType_factory_count);

	return newAmount;
}

