/*
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "ChatMessageLib.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "Common/atMacroString.h"
#include "ChatOpcodes.h"

void ChatMessageLib::sendCanceLiveAuctionResponseMessage(DispatchClient* client, uint32 error, uint64 mAuctionID)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opCanceLiveAuctionResponseMessage);
	//AuctionID
	gMessageFactory->addUint64(mAuctionID);
	gMessageFactory->addUint32(error);
	gMessageFactory->addUint8(0);// ??
	Message* newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, client->getAccountId(),  CR_Client, 6);
}

//=======================================================================================================================

void ChatMessageLib::sendBidAuctionResponse(DispatchClient* client, uint32 mError, uint64 mAuctionID)
{
	Message* newMessage;
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBidAuctionResponseMessage);
	gMessageFactory->addUint64(mAuctionID);
	gMessageFactory->addUint32(mError);
	newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage,client->getAccountId(),CR_Client,6);
}

//=======================================================================================================================

void ChatMessageLib::sendIsVendorOwnerResponseMessage(DispatchClient* client, string mBazaarString, uint32 mPermission, uint32 mError, uint64 mId)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opIsVendorOwnerResponseMessage);

	//permissionflag
	gMessageFactory->addUint32(mPermission); 

	//some kind of error message at this time no error anticipated, we could
	gMessageFactory->addUint32(mError);

	//our terminals / or vendors ID
	//0 if vendor hasnt been initialized
	gMessageFactory->addUint64(mId);

	//string containing Bazaar Id, location, etc
	gMessageFactory->addString(mBazaarString);
	gMessageFactory->addUint64(0);


	Message* newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage,client->getAccountId(),  CR_Client, 6);

}


//=======================================================================================================================

void ChatMessageLib::processSendCreateItem(DispatchClient* client, uint64 mPlayerID, uint64 mItemID,uint32 mItemGroup, uint32 mPlanetID)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opProcessSendCreateItem);
	gMessageFactory->addUint64(mPlayerID);
	gMessageFactory->addUint64(mItemID);
	gMessageFactory->addUint32(mItemGroup);
	Message* newMessage = gMessageFactory->EndMessage();
	//planetID+8 is the zones ID 
	client->SendChannelA(newMessage, client->getAccountId(), mPlanetID+8, 6);

}

//=======================================================================================================================

void ChatMessageLib::SendGetAuctionDetailsResponse(TradeManagerAsyncContainer* asynContainer, AttributesList* mAttributesList)
{

	string mDescriptionString;
	mDescriptionString = asynContainer->mItemDescription->Description;
	mDescriptionString.convert(BSTRType_Unicode16);

	// default reply 
	gMessageFactory->StartMessage();           
	gMessageFactory->addUint32(opGetAuctionDetailsResponse);
	gMessageFactory->addUint64(asynContainer->mItemDescription->ItemID);
	gMessageFactory->addString(mDescriptionString);



	AttributesList::iterator it = mAttributesList->begin();
	string value,name;
	uint32 size = mAttributesList->size();
	gMessageFactory->addUint32(size);

	while(it != mAttributesList->end())
	{
		gMessageFactory->addString(BString((*it)->name));
		value = (*it)->value;
		value.convert(BSTRType_Unicode16);
		gMessageFactory->addString(value);
		it++;
	}

	gMessageFactory->addString(BString(asynContainer->mItemDescription->tang));
	gMessageFactory->addUint32(0);

	Message* newMessage = gMessageFactory->EndMessage();
	asynContainer->mClient->SendChannelA(newMessage, asynContainer->mClient->getAccountId(),  CR_Client, 4);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//adds the amount for a banktip to a players bank account on the respective zone
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ChatMessageLib::sendBankTipDeductMessage(DispatchClient* client, uint64 receiverId,   uint32 amount, Player*  mPlayer)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBankTipDeduct);
	gMessageFactory->addUint64(receiverId);				
	gMessageFactory->addUint32(amount);	

	Message* newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, client->getAccountId(), mPlayer->getPlanetId()+8 , 6);


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//gives the zone server information on a won instant
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ChatMessageLib::sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mBuyerId,   uint32 mTime, Player*  mPlayer, Bazaar* mBazaarInfo)
{

	char *token;
	char separation[] = ".#,";
	string mString, planet, x, y, region;
	
	if (!mBazaarInfo)
	{
		planet = "";
		x = "";
		y = "";
		region = "";
		gLogger->logMsgF("ChatMessageLib :: Bazaar not found",MSG_NORMAL);
	}
	else
	{
		mString = mBazaarInfo->string;

		token = strtok(mString.getRawData(),separation); //
		planet = BString(token);
		// No, no.. as long as toUpperFisrt change the type of data of the BString object, this is a disaster... 
		// planet.toUpperFirst();

		token = strtok( NULL,separation); 
		region = BString(token);
		//thats now the terminal name part
		token = strtok( NULL,separation); 
		//thats now the terminal id part
		token = strtok( NULL,separation); 
		//x/y
		token = strtok( NULL,separation); 
		x =  BString(token);
		token = strtok( NULL,separation); 
		y =  BString(token);
	}
	


	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opDeductMoneyMessage);
	gMessageFactory->addUint64(mBuyerId);				//buyer Id
	gMessageFactory->addUint64(mAuctionItem.OwnerID);	//seller Id
	gMessageFactory->addUint64(mAuctionItem.ItemID);
	gMessageFactory->addUint32(mAuctionItem.Price);
	gMessageFactory->addUint32(mTime);
	gMessageFactory->addString(BString(mAuctionItem.Name));//itemname
	gMessageFactory->addString(planet);//Planet
	gMessageFactory->addString(region);//Region
	gMessageFactory->addString(BString(mAuctionItem.Owner));
	
	gMessageFactory->addString(x);//x
	gMessageFactory->addString(y);//y

	Message* newMessage = gMessageFactory->EndMessage();
	client->SendChannelA(newMessage, client->getAccountId(), mPlayer->getPlanetId()+8 , 6);

}

//======================================================================================================================

