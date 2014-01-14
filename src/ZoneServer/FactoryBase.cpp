/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "FactoryBase.h"
#include "Object.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"
#include "DatabaseManager/DataBinding.h"
#include "Utils/utils.h"




//=============================================================================

FactoryBase::FactoryBase(Database* database)
: mILCPool(sizeof(InLoadingContainer))
, mQueryContainerPool(sizeof(QueryContainerBase))
, mDatabase(database)
{
	mAttributeBinding = mDatabase->CreateDataBinding(3);
	mAttributeBinding->addField(DFT_bstring,offsetof(Attribute_QueryContainer,mKey),64,0);
	mAttributeBinding->addField(DFT_bstring,offsetof(Attribute_QueryContainer,mValue),128,1);
	mAttributeBinding->addField(DFT_uint8,offsetof(Attribute_QueryContainer,mInternal),1,2);
}

//=============================================================================

FactoryBase::~FactoryBase()
{
	mDatabase->DestroyDataBinding(mAttributeBinding);
}



//=============================================================================

InLoadingContainer* FactoryBase::_getObject(uint64 id)
{
	ObjectLoadMap::iterator it = mObjectLoadMap.find(id);

	if(it != mObjectLoadMap.end())
		return((*it).second);

	return(NULL);
}

//=============================================================================

bool FactoryBase::_removeFromObjectLoadMap(uint64 id)
{
	ObjectLoadMap::iterator it = mObjectLoadMap.find(id);

	if(it != mObjectLoadMap.end())
	{
		mObjectLoadMap.erase(it);
		return(true);
	}
	return(false);
}

//=============================================================================
// this should always be last in load order
void FactoryBase::_buildAttributeMap(Object* object,DatabaseResult* result)
{
	Attribute_QueryContainer	attribute;
	uint64						count = result->getRowCount();
	int8						str[256];
	BStringVector				dataElements;

	for(uint64 i = 0;i < count;i++)
	{
		result->GetNextRow(mAttributeBinding,(void*)&attribute);
		if(attribute.mKey.getCrc() == BString("cat_manf_schem_ing_resource").getCrc())
		{
			attribute.mValue.split(dataElements,' ');
			sprintf(str,"cat_manf_schem_ing_resource.\"%s",dataElements[0].getAnsi());

			attribute.mKey		= BString(str);
			attribute.mValue	= dataElements[1].getAnsi();

			//add key to the worldmanager
			if(gWorldManager->getAttributeKey(attribute.mKey.getCrc()) == "")
			{
				gWorldManager->mObjectAttributeKeyMap.insert(std::make_pair(attribute.mKey.getCrc(),attribute.mKey));
			}

		}

		if(attribute.mInternal)
			object->addInternalAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
		else
			object->addAttribute(attribute.mKey,std::string(attribute.mValue.getAnsi()));
	}

	object->setLoadState(LoadState_Loaded);
}

//=============================================================================

