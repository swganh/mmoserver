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

#include "ChatMessageLib.h"
#include "ChatOpcodes.h"
#include "Player.h"
#include "TradeManagerHelp.h"
#include "StructureManagerChat.h"




#include "Utils/logger.h"

#include "Common/atMacroString.h"
#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include <cstring>

void ChatMessageLib::sendCanceLiveAuctionResponseMessage(DispatchClient* client, uint32 error, uint64 mAuctionID) const
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

void ChatMessageLib::sendBidAuctionResponse(DispatchClient* client, uint32 mError, uint64 mAuctionID) const
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

void ChatMessageLib::sendIsVendorOwnerResponseMessage(DispatchClient* client, BString mBazaarString, uint32 mPermission, uint32 mError, uint64 mId) const
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
void ChatMessageLib::processSendCreateItem(DispatchClient* client, uint64 mPlayerID, uint64 mItemID,uint32 mItemGroup, uint32 mPlanetID) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opProcessSendCreateItem);
    gMessageFactory->addUint64(mPlayerID);
    gMessageFactory->addUint64(mItemID);
    gMessageFactory->addUint32(mItemGroup);
    Message* newMessage = gMessageFactory->EndMessage();
    //planetID+8 is the zones ID
    client->SendChannelA(newMessage, client->getAccountId(), static_cast<uint8>(mPlanetID)+8, 6);
}

//=======================================================================================================================
void ChatMessageLib::SendGetAuctionDetailsResponse(TradeManagerAsyncContainer* asynContainer, AttributesList* mAttributesList) const
{

    BString mDescriptionString;
    mDescriptionString = asynContainer->mItemDescription->Description;
    mDescriptionString.convert(BSTRType_Unicode16);

    // default reply
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opGetAuctionDetailsResponse);
    gMessageFactory->addUint64(asynContainer->mItemDescription->ItemID);
    gMessageFactory->addString(mDescriptionString);



    AttributesList::iterator it = mAttributesList->begin();
    BString value,name;
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

//=======================================================================================================================
void ChatMessageLib::sendBankTipDeductMessage(DispatchClient* client, uint64 receiverId,   uint32 amount, Player*  mPlayer) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opBankTipDeduct);
    gMessageFactory->addUint64(receiverId);
    gMessageFactory->addUint32(amount);

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), static_cast<uint8>(mPlayer->getPlanetId())+8 , 6);


}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//gives the zone server information on a won instant
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//=======================================================================================================================
void ChatMessageLib::sendBazaarTransactionMessage(DispatchClient* client, AuctionItem mAuctionItem, uint64 mBuyerId,   uint32 mTime, Player*  mPlayer, Bazaar* mBazaarInfo) const
{

    char *token;
    char separation[] = ".#,";
    BString mString, planet, x, y, region;

    if (!mBazaarInfo)
    {
        planet = "";
        x = "";
        y = "";
        region = "";
        DLOG(WARNING) << "ChatMessageLib :: Bazaar not found";
    }
    else
    {
        mString = mBazaarInfo->string;

        token = strtok(mString.getRawData(),separation); //
        planet = BString(token);

        //we need the first character capped for the EMail but lower case for the waypoint!!
        //so I scrapped it here and move it to the zone where the EMail gets send
        //planet.toUpperFirst();

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
    gMessageFactory->addUint64(mBuyerId);									// Buyer ID
    gMessageFactory->addUint64(mAuctionItem.OwnerID);						// Seller ID
    gMessageFactory->addUint64(mAuctionItem.ItemID);						// Item ID
    gMessageFactory->addUint32(mAuctionItem.Price);							// Price
    gMessageFactory->addUint32(mTime);										// Time Left?
    gMessageFactory->addString(BString(mAuctionItem.Name));					// Item Name
    gMessageFactory->addString(planet);										// Planet
    gMessageFactory->addString(region);										// Region
    gMessageFactory->addString(BString(mAuctionItem.Owner));				// Owner Name
    gMessageFactory->addString(x);											// x
    gMessageFactory->addString(y);											// y

    Message* newMessage = gMessageFactory->EndMessage();
    client->SendChannelA(newMessage, client->getAccountId(), static_cast<uint8>(mPlayer->getPlanetId())+8 , 6);
}

//=======================================================================================================================
//
// sends the relevant delta to the client to update hopper contents
//
void ChatMessageLib::SendHarvesterHopperUpdate(StructureManagerAsyncContainer* asynContainer, HopperResourceList* mHopperList) const
{

    HopperResourceList::iterator it = mHopperList->begin();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint16(2);														// 2 updated vars
    gMessageFactory->addUint16(12);														// var Nr 12 = hopper update flag
    gMessageFactory->addUint8(1);														//
    gMessageFactory->addUint16(13);														// var Nr 12 = hopper resourcelist
    gMessageFactory->addUint32(mHopperList->size());
    gMessageFactory->addUint32(asynContainer->updateCounter+mHopperList->size());

    gMessageFactory->addUint8(3);
    gMessageFactory->addUint16(mHopperList->size());

    asynContainer->updateCounter += mHopperList->size();
    while(it != mHopperList->end())
    {
		gMessageFactory->addUint64((*it)->ResourceID);
        //gMessageFactory->addFloat((*it)->Quantity);
        gMessageFactory->addFloat((float)asynContainer->updateCounter);
        it++;
    }

    Message* fragment = gMessageFactory->EndMessage();

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opDeltasMessage);
    gMessageFactory->addUint64(asynContainer->harvesterID);
    gMessageFactory->addUint32(opHINO);
    gMessageFactory->addUint8(7);
    gMessageFactory->addUint32(fragment->getSize());
    gMessageFactory->addData(fragment->getData(),fragment->getSize());

    fragment->setPendingDelete(true);
    asynContainer->mClient->SendChannelA(gMessageFactory->EndMessage(), asynContainer->mClient->getAccountId(),  CR_Client, 4);

}

//=======================================================================================================================
void ChatMessageLib::sendStructureConditionActiveUpdate(DispatchClient* client, uint64 structureID, uint32 mPlanetID, uint32 condition) const
{
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opISMUpdateStructureCondition);
    gMessageFactory->addUint64(structureID);
    gMessageFactory->addUint32(condition);
    gMessageFactory->addUint32(0);
    Message* newMessage = gMessageFactory->EndMessage();
    //planetID+8 is the zones ID
    client->SendChannelA(newMessage, client->getAccountId(), static_cast<uint8>(mPlanetID)+8, 6);

}
