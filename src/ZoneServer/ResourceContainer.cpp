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

#include "ResourceContainer.h"
#include "PlayerObject.h"
#include "Resource.h"
#include "ResourceType.h"
#include "WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"
#include "DatabaseManager/Database.h"
#include "MessageLib/MessageLib.h"

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
	//This cannot work until we do a) == instead of =
	//and b call our parent to destroy us
	//in which case we still would already be destroyed when we would have to finish the last line of code
	//PLUS if it actually worked, it would send the destroyobject several times to the client, thus crashing the zone
	//right now it crashes the server randomly when we try to access / delete the resource stack
	//if (mAmount = 0)// -> ==
		//ResourceContainer::deleteObject(this);//we are NOT a child of ourselves what you do is basically this->deleteObject(this);
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
//assign the ResourceContainer a new parentid
//

void ResourceContainer::setParentIdIncDB(uint64 parentId)
{ 
	mParentId = parentId; 
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE resource_containers SET parent_id=%"PRIu64" WHERE id=%"PRIu64"",mParentId,this->getId());
}

void ResourceContainer::upDateFactoryVolume(string amount)
{
	uint32 a = 0;
	a = boost::lexical_cast<uint32>(amount.getAnsi());
	
	if(a == this->getAmount())
	{
		return;
	}

	this->setAmount(a);

	TangibleObject* hopper = dynamic_cast<TangibleObject*>(gWorldManager->getObjectById(this->getParentId()));
	
	PlayerObjectSet*			knownPlayers	= hopper->getKnownPlayers();
	PlayerObjectSet::iterator	playerIt		= knownPlayers->begin();

	while(playerIt != knownPlayers->end())
	{
		PlayerObject* player = (*playerIt);
		if(player)
			gMessageLib->sendResourceContainerUpdateAmount(this,player);

		playerIt++;
	}

}


void ResourceContainer::updateWorldPosition()
{
	gWorldManager->getDatabase()->ExecuteSqlAsync(0,0,"UPDATE resource_containers SET parent_id ='%I64u', oX='%f', oY='%f', oZ='%f', oW='%f', x='%f', y='%f', z='%f' WHERE id='%I64u'",this->getParentId(), this->mDirection.x, this->mDirection.y, this->mDirection.z, this->mDirection.w, this->mPosition.x, this->mPosition.y, this->mPosition.z, this->getId());
	
}
