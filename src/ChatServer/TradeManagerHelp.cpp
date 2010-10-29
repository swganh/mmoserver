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

#include "TradeManagerHelp.h"

#include <cstring>

void AuctionStringClass::InsertAuction(AuctionItem Auction,uint32 BazaarStrNr,uint32 SellerStrNr, uint32 NameStrNr,uint32 BidderStrNr)
{
    SellerListID	= SellerStrNr;
    BidderListID	= BidderStrNr;
    BazaarListID	= BazaarStrNr;
    NameID			= NameStrNr;
    AuctionVar		= Auction;

}

//=============================================================================
void AuctionClass::AddAuction(AuctionItem Auction)
{
//listen bauen
    //1) Terminal List - take the terminal id here need to send the string to the client though

    bool found = false;
    BidderStringNr=0;

    //ListStringList::iterator it = mListStringList.begin();
    NameStringList::iterator itD = mNameStringList.begin();

    //insert bazaar and seller into the lists
    //so every bazaar and every seller are only once in memory
    //as they are also only once send

    //first bazaar
    ListStringList::iterator itL = mListStringList.begin();

    while(itL != mListStringList.end())
    {
        if ((*itL)->GetID() == Auction.BazaarID )//Bazaar known, return id
        {
            found = true;
            BazaarStringNr = ((*itL)->GetStringID());
            break;
        }
        ++itL;
    }
    if (found == false) {
        ListStringHandler = new ListStringClass();
        Stringid ++;
        BazaarStringNr = Stringid;
        ListStringHandler->InsertString(Auction.BazaarID ,Auction.BazaarName,Stringid);
        mListStringList.push_back(ListStringHandler);
    }

    //then the seller
    found = false;
    itL = mListStringList.begin();
    while(itL != mListStringList.end())
    {
        if ((*itL)->GetID() == Auction.OwnerID )//Owner known, return id
        {
            found = true;
            SellerStringNr = ((*itL)->GetStringID());
            break;
        }
        itL++;
    }
    if (found == false) {
        ListStringHandler = new ListStringClass();
        Stringid ++;
        SellerStringNr = Stringid;
        ListStringHandler->InsertString(Auction.OwnerID ,Auction.SellerName,Stringid);
        mListStringList.push_back(ListStringHandler);
    }

    //and now the High bidder /in case its an auction
    //then the seller
    if (Auction.AuctionTyp == 0) {
        found = false;
        itL = mListStringList.begin();
        while(itL != mListStringList.end())
        {
            if (strcmp((*itL)->GetString().getAnsi(),Auction.bidder_name)== 0)
            {
                found = true;
                BidderStringNr = ((*itL)->GetStringID());
                break;
            }
            itL++;
        }
        if (found == false) {
            ListStringHandler = new ListStringClass();
            Stringid ++;
            BidderStringNr = Stringid;
            ListStringHandler->InsertString(0 ,Auction.bidder_name,Stringid);
            mListStringList.push_back(ListStringHandler);
        }
    }
    //thats for our Auction Names
    found = false;
    while(itD != mNameStringList.end())
    {
        BString s = (*itD)->GetName();
        if (strcmp(s.getAnsi(),Auction.Name ) == 0)//Des known s.getAnsi() == Auction.Description )
        {
            //get the id of our strings to save it for the auction
            NameStringNr = (*itD)->GetNr();
            found = true;
        }
        itD++;
    }
    if (found == false) {

        NameStringCount ++;
        NameStringHandler = new NameStringClass();
        NameStringHandler->InsertName(Auction.Name,NameStringCount);
        NameStringHandler->SetNr(NameStringCount);
        NameStringNr = NameStringCount;
        mNameStringList.push_back(NameStringHandler);
    }

    AuctionStringCount ++;
    AuctionStringHandler = new AuctionStringClass();
    AuctionStringHandler->InsertAuction(Auction,BazaarStringNr,SellerStringNr,NameStringNr,BidderStringNr);
    mAuctionStringList.push_back(AuctionStringHandler);

}

//=============================================================================
AuctionClass::~AuctionClass()
{
    ListStringList::iterator it = mListStringList.begin();
    while(it != mListStringList.end())
    {
        delete(*it);
        mListStringList.erase(it);
        it = mListStringList.begin();
    }


    NameStringList::iterator itD = mNameStringList.begin();
    while(itD != mNameStringList.end())
    {
        delete(*itD);
        mNameStringList.erase(itD);
        itD = mNameStringList.begin();
    }


    AuctionStringList::iterator itA = mAuctionStringList.begin();
    while(itA != mAuctionStringList.end())
    {
        delete(*itA);
        mAuctionStringList.erase(itA);
        itA = mAuctionStringList.begin();
    }


}

//=============================================================================
void CommoditiesTypeClass::InsertCommoditie(TypeListItem Commoditie)
{
    CommoditiesItemList::iterator it = mCommoditiesItemList.begin();

    bool found = false;
    while(it != mCommoditiesItemList.end())
    {
        if ((*it)->GetCrc() == Commoditie.crc )
        {
            // its already in our list
            found = true;
        }
        if (found == true) break;
        it++;
    }
    //Ok we need to enter this new one
    if (found == false)
    {
        CommoditiesItemCount  ++;
        CommoditiesItemHandler = new CommoditiesItemClass();
        CommoditiesItemHandler->AddCommoditie(Commoditie);
        mCommoditiesItemList.push_back(CommoditiesItemHandler);
    }

}

//=============================================================================
void CommoditiesTypeClass::InsertCommoditieClass(CommoditiesItemClass* CommoditiesClass)
{
    CommoditiesItemList::iterator it = mCommoditiesItemList.begin();

    bool found = false;
    while(it != mCommoditiesItemList.end())
    {
        if ((*it)->GetCrc() == CommoditiesClass->GetCrc())
        {
            // its already in our list
            found = true;
        }
        if (found == true) break;
        it++;
    }
    //Ok we need to enter this new one
    if (found == false)
    {
        CommoditiesItemCount  ++;
        mCommoditiesItemList.push_back(CommoditiesClass);
    }

}

//=============================================================================
uint32 CommoditiesClass::getCategory(uint32 crc)
{
    CommoditiesItemHash::iterator itH = mCommoditiesHashList.find(crc);
    if(itH != mCommoditiesHashList.end())
        return (*itH).second->GetCommoditie().mask;
    else
        //this happens for example for resources
        //or if there is no valid entry in the table
        return 0;
}

//=============================================================================

void CommoditiesClass::AddCommoditie(TypeListItem Commoditie)
{
    //1) for every subcategory a CommoditiesType List

    //get the crc we are using for identification
    BString crcstring = Commoditie.name;
    uint32 crc = crcstring.getCrc();

    CommoditiesItemClass* CommoditiesHashClass = new(CommoditiesItemClass);
    Commoditie.crc = crc;
    CommoditiesHashClass->setKey(crc);
    CommoditiesHashClass->AddCommoditie(Commoditie);
    mCommoditiesHashList.insert(std::make_pair(crc,CommoditiesHashClass));

    bool found = false;
    CommoditiesTypeList::iterator it = mCommoditiesTypeList.begin();

    while(it != mCommoditiesTypeList.end())
    {
        if ((*it)->GetMask() == Commoditie.mask)//SubCategory already known; perhaps new Commoditie Item
        {
            found = true;
            //inserts new Commoditie
            (*it)->InsertCommoditie(Commoditie);
        }
        if (found == true) break;//no need now to iterate further
        it++;
    }
    if (found == false) {
        // we need to enter a new CommoditieType
        CommoditiesTypeCount  ++;
        CommoditiesTypeHandler = new CommoditiesTypeClass();
        CommoditiesTypeHandler->InsertType(Commoditie.mask);
        CommoditiesTypeHandler->InsertCommoditieClass(CommoditiesHashClass);
        mCommoditiesTypeList.push_back(CommoditiesTypeHandler);
    }


}

//=============================================================================
CommoditiesClass::~CommoditiesClass()
{
    //mCommoditiesHashList.clear();

    CommoditiesTypeList::iterator it = mCommoditiesTypeList.begin();
    while(it != mCommoditiesTypeList.end())
    {
        SAFE_DELETE(*it);
        mCommoditiesTypeList.erase(it);
        it = mCommoditiesTypeList.begin();
    }

}

//=============================================================================
CommoditiesTypeClass::~CommoditiesTypeClass()
{
    CommoditiesItemList::iterator it = mCommoditiesItemList.begin();
    while(it != mCommoditiesItemList.end())
    {
        delete(*it);
        mCommoditiesItemList.erase(it);
        it = mCommoditiesItemList.begin();
    }

}
