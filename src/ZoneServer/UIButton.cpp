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

UIButton::UIButton(uint32 id,string name,bool enabled,string text, bool three)
: UIElement(id,SUI_Element_Button),mEnabled(enabled)
{
	mName	= name.getAnsi();
	mText	= text.getAnsi();
	mText.convert(BSTRType_Unicode16);
	m3B = three;
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
			if(m3B)
				if(mId)
					return(7);
				else
					return(4);
			else
				return(2);
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
			/*
			gMessageFactory->addUint8(3);
			gMessageFactory->addUint32(1);
			gMessageFactory->addString(strTrue);
			gMessageFactory->addUint32(2);
			gMessageFactory->addString(mName);
			gMessageFactory->addString(BString("Enabled"));
			  */
			gMessageFactory->addUint8(3);
			gMessageFactory->addUint32(1);
			gMessageFactory->addString(strTrue);
			gMessageFactory->addUint32(2);
			gMessageFactory->addString(mName);
			gMessageFactory->addString(BString("visible"));

			gMessageFactory->addUint8(3);
			gMessageFactory->addUint32(1);
			gMessageFactory->addString(mText);
			gMessageFactory->addUint32(2);
			gMessageFactory->addString(mName);
			gMessageFactory->addString(BString("Text"));

	
			if(m3B)
			{
				
				gMessageFactory->addUint8(3);
				gMessageFactory->addUint32(1);
				gMessageFactory->addString(BString(L"86,20"));
				gMessageFactory->addUint32(2);
				gMessageFactory->addString(mName);
				gMessageFactory->addString(BString("ScrollExtent"));

					gMessageFactory->addUint8(3);
				gMessageFactory->addUint32(1);
				gMessageFactory->addString(BString(L"66,20"));
				gMessageFactory->addUint32(2);
				gMessageFactory->addString(mName);
				gMessageFactory->addString(BString("Size"));

				if(mId)
				{
					if(mId == 3)
					{
						//<Button IsDefaultButton='true' LocalText='[@ui_mission:details]' 
						//Location='233,267' Name='buttonDetails' PackLocation='npn,fff' PackLocationProp='0233/0449,-010/0001' PackSize='p,f' PackSizeProp='0090/0449,0002/0003' ScrollExtent='90,17' Size='90,17' Style='/Styles.New.buttons.hud.style' TextColor='#000000'>@ui_mission:details</Button>
						gMessageFactory->addUint8(3);
						gMessageFactory->addUint32(1);
						gMessageFactory->addString(BString(L"205,262")); //vertical, horizontal
															
						
						gMessageFactory->addUint32(2);
						gMessageFactory->addString(mName);
						gMessageFactory->addString(BString("Location"));
						
						
					}
					if(mId == 2)
					{
						gMessageFactory->addUint8(3);
						gMessageFactory->addUint32(1);
						gMessageFactory->addString(BString(L"110,262")); //vertical, horizontal
															
						
						gMessageFactory->addUint32(2);
						gMessageFactory->addString(mName);
						gMessageFactory->addString(BString("Location"));

					}			

					if(mId == 1)
					{
						gMessageFactory->addUint8(3);
						gMessageFactory->addUint32(1);
						gMessageFactory->addString(BString(L"13,262")); //vertical, horizontal
															
						
						gMessageFactory->addUint32(2);
						gMessageFactory->addString(mName);
						gMessageFactory->addString(BString("Location"));

					}			

					gMessageFactory->addUint8(3);
					gMessageFactory->addUint32(1);
					if(mId == 1)
						gMessageFactory->addString(BString(L"p,f"));
					if(mId == 2)
						gMessageFactory->addString(BString(L"p,f"));
					if(mId == 3)
						gMessageFactory->addString(BString(L"p,f"));

					gMessageFactory->addUint32(2);
					gMessageFactory->addString(mName);
					gMessageFactory->addString(BString("PackSize"));


					gMessageFactory->addUint8(3);
					gMessageFactory->addUint32(1);
					if(mId == 1)
						gMessageFactory->addString(BString(L"fpf,fff"));
					if(mId == 2)
						gMessageFactory->addString(BString(L"fpf,fff"));
					if(mId == 3)
						gMessageFactory->addString(BString(L"fpf,fff"));

					gMessageFactory->addUint32(2);
					gMessageFactory->addString(mName);
					gMessageFactory->addString(BString("PackLocation"));

					
				}
			}
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


