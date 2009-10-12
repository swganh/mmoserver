/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "Resource.h"
#include "PlayerObject.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"


//=============================================================================

Resource::Resource()
{
}

//=============================================================================

Resource::~Resource()
{
}

//=============================================================================


void Resource::sendAttributes(PlayerObject* playerObject)
{
	if(playerObject->getConnectionState() != PlayerConnState_Connected)
		return;

	Message* newMessage;

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opAttributeListMessage);
	gMessageFactory->addUint64(mId);

	uint32 attrCount = 0;

	for(uint8 i = 0;i < 11;i++)
	{
		if(mAttributes[i])
			attrCount++;
	}

	gMessageFactory->addUint32(attrCount);

	char	tmpStr[128];
	string	tmpValueStr;

	for(uint8 i = 0;i < 11;i++)
	{
		uint16	attrValue = 0;
		string	attrName;

		if((attrValue = mAttributes[i]) != 0)
		{
			switch(i)
			{
				case ResAttr_OQ: attrName = "res_quality";			break;
				case ResAttr_CR: attrName = "res_cold_resist";		break;
				case ResAttr_CD: attrName = "res_conductivity";		break;
				case ResAttr_DR: attrName = "res_decay_resist";		break;
				case ResAttr_HR: attrName = "res_heat_resist";		break;
				case ResAttr_MA: attrName = "res_malleability";		break;
				case ResAttr_SR: attrName = "res_shock_resistance"; break;
				case ResAttr_UT: attrName = "res_toughness";		break;
				case ResAttr_ER: attrName = "entangle_resistance";	break;
				case ResAttr_PE: attrName = "res_potential_energy"; break;
				case ResAttr_FL: attrName = "res_flavor";			break;
			}

			sprintf(tmpStr,"%u",attrValue);
			tmpValueStr = tmpStr;
			tmpValueStr.convert(BSTRType_Unicode16);

			gMessageFactory->addString(attrName);
			gMessageFactory->addString(tmpValueStr);
		}
	}

	gMessageFactory->addUint32(0xffffffff);
           
	newMessage = gMessageFactory->EndMessage();

	//(playerObject->getClient())->SendChannelA(newMessage, playerObject->getAccountId(), CR_Client, 9,false);
	(playerObject->getClient())->SendChannelAUnreliable(newMessage, playerObject->getAccountId(), CR_Client, 9);
}

//=============================================================================

