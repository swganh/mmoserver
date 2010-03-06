/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_TANGIBLE_CHARACTERBUILDERTERMINAL_H
#define ANH_ZONESERVER_TANGIBLE_CHARACTERBUILDERTERMINAL_H

#include "Terminal.h"
#include <vector>

typedef std::vector<std::pair<string,uint32> >	SortedList;
typedef std::vector<std::pair<uint32,uint32> >	ItemEntryList;
typedef	std::vector<uint32>						ResCatLookupList;

//=============================================================================

class CharacterBuilderTerminal : public Terminal
{
	friend class TerminalFactory;

public:

	CharacterBuilderTerminal();
	~CharacterBuilderTerminal();

	virtual void		prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount);
	void		handleObjectMenuSelect(uint8 messageType,Object* srcObject);
	void		handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window);

private:

	int32				mInputBoxAmount;
	BStringVector		mMainMenu;
	BStringVector		mCreditsMenu;
	BStringVector		mItemsMenu;
	ItemEntryList		mItemsTypes;
	BStringVector		mAttributesMenu;
	SortedList*			mSortedList;
};

//======================================================================================================================
//
//thats were the raw frog data goes
//


class ItemFrogItemClass
{
public:

	ItemFrogItemClass () {};      
	~ItemFrogItemClass (){}; 

	//void AddCommoditie(TypeListItem theCommoditie){Commoditie = theCommoditie;}
	string	GetName() {return(name);}
	void	SetName(string n) {name = n;}
	
	uint32	GetType(){return type;}
	void	SetType(uint32 n) {type = n;}

	uint32	GetFamily(){return family;}
	void	SetFamily(uint32 n) {family = n;}

	uint8	GetActive(){return active;}
	void	SetActive(uint8 n) {active = n;}

	uint64			id;
	uint32			type;
	uint32			family;
	string			name;
	uint8			active;
	string			f_name;

};

typedef std::vector<ItemFrogItemClass*> ItemFrogItemList;

class ItemFrogTypeClass
{
public:

	ItemFrogTypeClass () {};      
	~ItemFrogTypeClass (); 

	ItemFrogItemClass*			mItemFrogItemHandler;
	ItemFrogItemList			mItemFrogItemList;

	//void InsertItem(TypeListItem Commoditie);
	ItemFrogItemClass*	getItem(uint32 nr){return mItemFrogItemList[nr];}
	void				InsertItem(ItemFrogItemClass* item){mItemFrogItemList.push_back(item);}
	
	ItemFrogItemClass*	getType(uint32 type);

	uint32			family;
	string			f_name;

};


typedef std::vector<ItemFrogTypeClass*>			ItemFrogTypeList;

class ItemFrogClass
{
	public:
		ItemFrogTypeClass*				ItemFrogTypeHandler;
		ItemFrogTypeList				mItemFrogTypeList;

		ItemFrogClass(){}
		~ItemFrogClass();

		ItemFrogTypeClass*		GetFamily(uint32 nr){return mItemFrogTypeList[nr];}
		void					AddFamily(ItemFrogTypeClass* item){mItemFrogTypeList.push_back(item) ;}//adds an item to the list
		
		ItemFrogTypeClass*		LookUpFamily(uint32 family);
		ItemFrogItemClass*		LookUpType(uint32 type);
		 //TypeListItem	LookUpCommoditie(uint32 crc);
		 //uint32			getCategory(uint32 crc);
		 
		 //uint32			GetCount(){return(CommoditiesTypeCount);}
		 //uint32			CommoditiesTypeCount;
		
};

//=============================================================================

#endif

