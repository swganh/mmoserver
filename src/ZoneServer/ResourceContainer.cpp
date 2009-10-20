 /*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ResourceContainer.h"
#include "PlayerObject.h"
#include "Resource.h"
#include "ResourceType.h"
#include "WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "MathLib/Quaternion.h"

//=============================================================================

ResourceContainer::ResourceContainer() : TangibleObject()
{
	mTanGroup	= TanGroup_ResourceContainer;
	mResource	= NULL;
	mAmount		= 0;
	mMaxAmount	= 100000;
}

//=============================================================================

ResourceContainer::~ResourceContainer()
{
}


uint64 ResourceContainer::getResourceId()
{
    return mResourceId;
}


void ResourceContainer::setResourceId(uint64 id)
{
    mResourceId = id;
}


Resource* ResourceContainer::getResource()
{
    return mResource;
}


void ResourceContainer::setResource(Resource* resource)
{
    mResource = resource;
}


uint32 ResourceContainer::getAmount()
{
    return mAmount;
}


void ResourceContainer::setAmount(uint32 amount)
{
    mAmount = amount;
}


uint32 ResourceContainer::getMaxAmount()
{
    return mMaxAmount;
}


void ResourceContainer::setMaxAmount(uint32 max)
{
    mMaxAmount = max;
}


string ResourceContainer::getBazaarTang()
{
    return getModelString();
}

//=============================================================================

string	ResourceContainer::getBazaarName()
{
	string value = string(BSTRType_ANSI,256);

	value.setLength(sprintf(value.getAnsi(),"%s (%s)",getResource()->getType()->getName().getAnsi(),getResource()->getName().getAnsi()));

	return value;
}


uint32 ResourceContainer::getCategoryBazaar()
{
    return getResource()->getType()->getCategoryBazaar();
}

//=============================================================================

void ResourceContainer::sendAttributes(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return;

	Message*	newMessage;
	string		tmpValueStr = string(BSTRType_Unicode16,64);
	string		value;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);

	uint32 attrCount = 4;

	for(uint8 i = 0;i < 11;i++)
	{
		if(mResource->getAttribute(i))
			++attrCount;
	}

	gMessageFactory->addUint32(attrCount + mAttributeMap.size());

	tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),20,L"%u/%u",mMaxCondition-mDamage,mMaxCondition));
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

	tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),20,L"%u/%u",mAmount,mMaxAmount));
	gMessageFactory->addString(BString("resource_contents"));
	gMessageFactory->addString(tmpValueStr);

	tmpValueStr = mResource->getName().getAnsi();
	tmpValueStr.convert(BSTRType_Unicode16);
	gMessageFactory->addString(BString("resource_name"));
	gMessageFactory->addString(tmpValueStr);

	tmpValueStr = ((mResource->getType())->getName()).getAnsi();
	tmpValueStr.convert(BSTRType_Unicode16);
	gMessageFactory->addString(BString("resource_class"));
	gMessageFactory->addString(tmpValueStr);

	for(uint8 i = 0;i < 11;i++)
	{
		uint16	attrValue = 0;
		string	attrName;

		if((attrValue = mResource->getAttribute(i)) != 0)
		{
			switch(i)
			{
				case ResAttr_OQ: attrName = "res_quality";				break;
				case ResAttr_CR: attrName = "res_cold_resist";			break;
				case ResAttr_CD: attrName = "res_conductivity";			break;
				case ResAttr_DR: attrName = "res_decay_resist";			break;
				case ResAttr_HR: attrName = "res_heat_resist";			break;
				case ResAttr_MA: attrName = "res_malleability";			break;
				case ResAttr_SR: attrName = "res_shock_resistance";		break;
				case ResAttr_UT: attrName = "res_toughness";			break;
				case ResAttr_ER: attrName = "entangle_resistance";		break;
				case ResAttr_PE: attrName = "res_potential_energy";		break;
				case ResAttr_FL: attrName = "res_flavor";				break;
			}

			tmpValueStr = string(BSTRType_Unicode16,64);
			tmpValueStr.setLength(swprintf(tmpValueStr.getUnicode16(),10,L"%u",attrValue));

			gMessageFactory->addString(attrName);
			gMessageFactory->addString(tmpValueStr);
		}
	}

	//gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(),  CR_Client, 9);
}

//=============================================================================


