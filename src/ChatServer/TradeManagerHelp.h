/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TRADEMANAGERHELP_H
#define ANH_ZONESERVER_TRADEMANAGERHELP_H

#include <map>
#include <vector>

#include "Utils/typedefs.h"
#include "ZoneServer/TradeManager.h"

class DispatchClient;

//======================================================================================================================

enum TRMTimer
{
	CMTimer_GlobalTick		=	0,
	CMTimer_TickPreserve	=	1,
	CMTimer_CheckAuctions	=	2
};

//======================================================================================================================

struct ItemDescriptionAttributes
{
		int8			name[64];
		int8			value[128];
};

//======================================================================================================================

struct ResItemDescriptionAttributes
{
		uint64			id;
		uint32			amount;
		int8			name[32];
		int8			er[32];
		int8			cr[32];
		int8			cd[32];
		int8			dr[32];
		int8			fl[32];
		int8			hr[32];
		int8			ma[32];
		int8			oq[32];
		int8			sr[32];
		int8			ut[32];
		int8			pe[32];

};

//======================================================================================================================
//
//thats were the raw Auction db data goes
//

struct TypeListItem
{		uint32			crc;
		uint32			mask;
		int8			directory[40];
		int8			name[60];
};

//======================================================================================================================

struct ListStringStruct
{
		string Name;
		uint32 id;
};

//======================================================================================================================

struct Query
{
		uint32 Region;
		int8   RegionQuery[128];

		uint32 UpdateCounter;
		uint32 Windowtype;
		int8   WindowQuery[256];

		uint32 Category;
		int8   CategoryQuery[128];

		uint32 ItemTyp;
		int8   ItemTypQuery[64];

		string searchstring;
		uint32 unknown;
		uint32 minprice;
		uint32 maxprice;
		uint8 entrancefee;
		uint64 vendorID;
		uint8 unknown2;
		uint16 start;
};

//======================================================================================================================

typedef int8 str32[32];
typedef int8 str256[256];

class ListStringClass
{
public:
	ListStringClass () {ID = 0; Name = ""; Stringid = 0;}
	~ListStringClass () {}

	uint64 GetID(){return(ID);}
	string GetString(){return(Name);}
	uint32 GetStringID(){return(Stringid);}

	void   InsertString(uint64 id,string name,uint32 stringid){ID = id;Name = name;Stringid = stringid;}

private:
		uint64			ID;
		string			Name;
		uint32			Stringid;

};


class NameStringClass
{
public:

	NameStringClass () {}
	~NameStringClass (){};

	//hark! every Bazaarstring is followed by a list of sellers for it, so that everyAuctionBazaarObject
	//owns a variable amount of AuctionSellerStrings

	// oh and delete the lists in the destructor.....!!!

	void InsertName(string Des,uint32 num){Name = Des;nr = num;}
	string GetName() {return(Name);}
	uint32 GetNr() {return(nr);}
	void SetNr(uint32 Nr) {nr = Nr;}

	uint32			nr;
	string			Name;

};

class AuctionStringClass
{
public:

	AuctionStringClass () {}
	~AuctionStringClass (){};

	// oh and delete the lists in the destructor.....!!!

	void InsertAuction(AuctionItem Auction,uint32 BazaarStrNr,uint32 SellerStrNr, uint32 DescriptionStrNr,uint32 BidderStrNr);
	uint32 GetSellerListID(){return(SellerListID);}
	uint32 GetBidderListID(){return(BidderListID);}
	uint32 GetBazaarListID(){return(BazaarListID);}
	uint32 GetNameListID(){return(NameID);}

	uint64 GetOwnerID(){return(AuctionVar.OwnerID);}
	uint64 GetAuctionID(){return(AuctionVar.ItemID);}
	uint8 GetType(){return(static_cast<uint8>(AuctionVar.AuctionTyp));}
	uint64 GetTime(){return(AuctionVar.EndTime);}
	uint8 GetPremium(){return(static_cast<uint8>(AuctionVar.Premium));}
	uint32 GetCategory()
	{
		if (AuctionVar.Category != 0)
		{
			return(AuctionVar.Category);
		}
		else
		{

			return(AuctionVar.itemcategory);
		}
	}

	uint32 GetItemTyp(){return(AuctionVar.ItemTyp );}
	uint32 GetPrice(){return(AuctionVar.Price);}
	uint32 GetBid(){return(AuctionVar.HighBid);}
	uint32 GetProxy(){return(AuctionVar.HighProxy);}
	string GetHighBidder(){return(AuctionVar.bidder_name);}

	AuctionItem		AuctionVar;
	uint32			SellerListID;
	uint32			BidderListID;
	uint32			BazaarListID;
	uint32			NameID;


};

typedef std::vector<ListStringClass*> ListStringList;
typedef std::vector<NameStringClass*> NameStringList;
typedef std::vector<AuctionStringClass*> AuctionStringList;

class AuctionClass
{
	public:

		ListStringClass*		ListStringHandler;
		ListStringList			mListStringList;

		NameStringClass*			NameStringHandler;
		NameStringList				mNameStringList;

		AuctionStringClass*			AuctionStringHandler;
		AuctionStringList			mAuctionStringList;


		AuctionClass(){
						AuctionStringCount = 0;
						NameStringCount = 0;
						SellerStringCount = 0;
						Stringid = 0;}

		~AuctionClass();

		void AddAuction(AuctionItem Auction);//adds an auction to the list
		uint32 getStringCount(){return Stringid;}//count of bazaar and sellerstrings

		AuctionItem     AuctionItemVar;
		uint32			Auctions;
		uint32			AuctionStringCount;
		uint32			SellerStringCount;
		uint32			NameStringCount;

		uint32			BazaarStringNr;
		uint32			AuctionStringNr;
		uint32			SellerStringNr;
		uint32			BidderStringNr;
		uint32			NameStringNr;
		uint32			Stringid;
};
class CommoditiesItemClass
{
public:

	CommoditiesItemClass () {};
	~CommoditiesItemClass (){};

	void AddCommoditie(TypeListItem theCommoditie){Commoditie = theCommoditie;}
	string GetName() {return(Commoditie.name);}
	string GetDirectory() {return(Commoditie.directory);}
	uint32 GetMask(){return(Commoditie.mask );}
	uint32 GetCrc(){return(Commoditie.crc );}
	TypeListItem GetCommoditie(){return Commoditie;}
	uint32			getKey(){ return mKey; }
	void			setKey(uint32 key) { mKey = key; }

	TypeListItem    Commoditie;
	uint32			mKey;

};

typedef std::vector<CommoditiesItemClass*> CommoditiesItemList;

class CommoditiesTypeClass
{
public:

	CommoditiesTypeClass () {};
	~CommoditiesTypeClass ();

	CommoditiesItemClass*			CommoditiesItemHandler;
	CommoditiesItemList				mCommoditiesItemList;

	void InsertCommoditie(TypeListItem Commoditie);
	void InsertCommoditieClass(CommoditiesItemClass* CommoditiesClass);
	void InsertType(uint32 theMask){Mask = theMask;CommoditiesItemCount = 0;}
	uint32 GetMask() {return(Mask);}
	uint32 GetCount() {return(CommoditiesItemCount);}

	uint32			CommoditiesItemCount;
	uint32			Mask;

};

typedef std::map<uint32,CommoditiesItemClass*>		CommoditiesItemHash;
typedef std::vector<CommoditiesTypeClass*>			CommoditiesTypeList;

class CommoditiesClass
{
	public:

		CommoditiesItemClass*			CommoditiesHashHandler;
		CommoditiesItemHash				mCommoditiesHashList;

		CommoditiesItemClass*			CommoditiesItemHandler;
		CommoditiesItemList				mCommoditiesItemList;

		CommoditiesTypeClass*			CommoditiesTypeHandler;
		CommoditiesTypeList				mCommoditiesTypeList;




		CommoditiesClass(){}

		~CommoditiesClass();

		 void			AddCommoditie(TypeListItem Commoditie);//adds a Commoditie to the list
		 TypeListItem	LookUpCommoditie(uint32 crc);
		 uint32			getCategory(uint32 crc);

		 uint32			GetCount(){return(CommoditiesTypeCount);}
		 uint32			CommoditiesTypeCount;

};

#endif

