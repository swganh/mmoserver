/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "UIInputField.h"
#include "Common/MessageFactory.h"

//================================================================================

UIInputField::UIInputField(uint32 id,string name,bool enabled,uint16 maxLength)
: UIElement(id,SUI_Element_InputField),mEnabled(enabled),mMaxLength(maxLength)
{
	mName	= name.getAnsi();
}

//================================================================================

UIInputField::~UIInputField()
{
}

//================================================================================

uint32 UIInputField::getPropertyCount()
{
	if(mEnabled)
		return(3);
	else
		return(3);
}

//================================================================================

void UIInputField::addMessageData()
{
	string	strFalse	= L"False";
	string	strTrue		= L"True";

	string lengthStr;
	lengthStr.setLength(sprintf(lengthStr.getAnsi(),"%u",mMaxLength));
	lengthStr.convert(BSTRType_Unicode16);

	if(mEnabled)
	{
		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(strTrue);
		gMessageFactory->addUint32(2);
		gMessageFactory->addString(mName);
		gMessageFactory->addString(BString("Enabled"));

		gMessageFactory->addUint8(3);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(strTrue);
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


