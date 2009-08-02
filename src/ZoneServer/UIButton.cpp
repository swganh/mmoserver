/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "UIButton.h"
#include "Common/MessageFactory.h"

//================================================================================

UIButton::UIButton(uint32 id,string name,bool enabled,string text)
: UIElement(id,SUI_Element_Button),mEnabled(enabled)
{
	mName	= name.getAnsi();
	mText	= text.getAnsi();
	mText.convert(BSTRType_Unicode16);
}

//================================================================================

UIButton::~UIButton()
{
}

//================================================================================

uint32 UIButton::getPropertyCount()
{
	if(mEnabled)
	{
		if(mText.getLength())
			return(1);
		else
			return(0);
	}
	else
		return(2);
}

//================================================================================

void UIButton::addMessageData()
{
	string	strFalse	= L"False";
	string	strTrue		= L"True";

	if(mEnabled)
	{
		if(mText.getLength())
		{
			gMessageFactory->addUint8(3);
			gMessageFactory->addUint32(1);
			gMessageFactory->addString(mText);
			gMessageFactory->addUint32(2);
			gMessageFactory->addString(mName);
			gMessageFactory->addString(BString("Text"));
		}
		else
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
	}
}

//================================================================================


