/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "Common/atMacroString.h"


//======================================================================================================================

void MessageLib::sendBanktipMail(PlayerObject* playerObject, PlayerObject* targetObject, uint32 amount)
{

	atMacroString* aMS = new atMacroString();

	aMS->addMBstf("base_player","prose_wire_mail_target");
	aMS->addDI(amount);
	aMS->addTO(playerObject->getFirstName());
	aMS->addTextModule();


	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmSendSystemMailMessage);
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addString(targetObject->getFirstName());
	gMessageFactory->addString(BString("@base_player:wire_mail_subject"));
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(aMS->assemble());
	delete aMS;


	Message* newMessage = gMessageFactory->EndMessage();
	playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6, false);



	aMS = new atMacroString();

	aMS->addMBstf("base_player","prose_wire_mail_self");
	aMS->addDI(amount);
	aMS->addTO(targetObject->getFirstName());
	aMS->addTextModule();


	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmSendSystemMailMessage);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addString(playerObject->getFirstName());
	gMessageFactory->addString(BString("@base_player:wire_mail_subject"));
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(aMS->assemble());
	delete aMS;


	newMessage = gMessageFactory->EndMessage();
	playerObject->getClient()->SendChannelA(newMessage, playerObject->getAccountId(), CR_Chat, 6, false);

}


//======================================================================================================================

void MessageLib::sendBoughtInstantMail(PlayerObject* newOwner, string ItemName, string SellerName, uint32 Credits, string planet, string region, int32 mX, int32 mY)
{

	atMacroString* aMS = new atMacroString();

	aMS->addMBstf("auction","buyer_success");
	aMS->addTO(ItemName);
	aMS->addTT(SellerName);
	aMS->addDI(Credits);
	aMS->addTextModule();


	aMS->addMBstf("auction","buyer_success_location");
	aMS->addTT(region);
	aMS->addTO(planet);
	aMS->addTextModule();

	gLogger->logMsgF("send EMail : %s",MSG_NORMAL,planet.getAnsi());
	aMS->setPlanetString(planet);
	aMS->setWP(mX,mY,0,ItemName);
	aMS->addWaypoint();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmSendSystemMailMessage);
	gMessageFactory->addUint64(newOwner->getId());
	gMessageFactory->addUint64(newOwner->getId());
	gMessageFactory->addString(BString("auctioner"));
	gMessageFactory->addString(BString("@auction:subject_auction_buyer"));
	gMessageFactory->addUint32(0);
	string attachment = aMS->assemble();
	gMessageFactory->addString(attachment);
	
	Message* newMessage = gMessageFactory->EndMessage();

	
	newOwner->getClient()->SendChannelA(newMessage, newOwner->getAccountId(), CR_Chat, 6, false);

	delete aMS;

}

//======================================================================================================================

void MessageLib::sendSoldInstantMail(uint64 oldOwner, PlayerObject* newOwner, string ItemName, uint32 Credits, string planet, string region)
{
	//seller_success       Your auction of %TO has been sold to %TT for %DI credits

	atMacroString* aMS = new atMacroString();

	aMS->addMBstf("auction","seller_success");
	aMS->addTO(ItemName);
	aMS->addTT(newOwner->getFirstName());
	aMS->addDI(Credits);
	aMS->addTextModule();

	aMS->addMBstf("auction","seller_success_location");
	aMS->addTT(region);
	aMS->addTO(planet);
	aMS->addTextModule();

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsmSendSystemMailMessage);
	gMessageFactory->addUint64(oldOwner);
	gMessageFactory->addUint64(oldOwner);
	gMessageFactory->addString(BString("auctioner"));
	gMessageFactory->addString(BString("@auction:subject_instant_seller"));
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(aMS->assemble());
	delete aMS;

	Message* newMessage = gMessageFactory->EndMessage();
	newOwner->getClient()->SendChannelA(newMessage, newOwner->getAccountId(), CR_Chat, 6, false);

}

//======================================================================================================================


