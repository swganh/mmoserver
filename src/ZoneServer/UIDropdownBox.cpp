/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "UIDropdownBox.h"
#include "Common/MessageFactory.h"

//================================================================================

UIDropdownBox::UIDropdownBox(uint32 id,string name,bool enabled,const BStringVector elements,uint16 maxLength)
: UIElement(id,SUI_Element_DropdownBox),mEnabled(enabled),mMaxLength(maxLength)
{
	mName		= name.getAnsi();
	mElements	= elements;
}

//================================================================================

UIDropdownBox::~UIDropdownBox()
{
}

//================================================================================

uint32 UIDropdownBox::getPropertyCount()
{
	if(mEnabled)
	{
		//fixme
		return(0);
	}
	else
		return(3);
}

//================================================================================

void UIDropdownBox::addMessageData()
{
	string	strFalse	= L"False";
	string	strTrue		= L"True";

	string lengthStr;
	lengthStr.setLength(sprintf(lengthStr.getAnsi(),"%u",mMaxLength));
	lengthStr.convert(BSTRType_Unicode16);

	if(mEnabled)
	{
		//fixme
		return;
	}
	else
	{
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(strFalse);
		gMessageFactory->addUint32(2);
		gMessageFactory->addString(mName);
		gMessageFactory->addString(BString("Enabled"));

		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(strFalse);
		gMessageFactory->addUint32(2);
		gMessageFactory->addString(mName);
		gMessageFactory->addString(BString("visible"));

		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(lengthStr);
		gMessageFactory->addUint32(2);
		gMessageFactory->addString(mName);
		gMessageFactory->addString(BString("MaxLength"));
	}
}

//================================================================================


