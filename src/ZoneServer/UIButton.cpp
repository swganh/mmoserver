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

#include "UIButton.h"
#include "NetworkManager/MessageFactory.h"

//================================================================================

UIButton::UIButton(uint32 id,BString name,bool enabled,BString text, bool three)
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
    BString	strFalse	= L"False";
    BString	strTrue		= L"True";

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


            if(m3B) // I wish I knew what 3B is.
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
                    if(mId >= 1 && mId <= 3)
                    {
                        //<Button IsDefaultButton='true' LocalText='[@ui_mission:details]'
                        //Location='233,267' Name='buttonDetails' PackLocation='npn,fff' PackLocationProp='0233/0449,-010/0001' PackSize='p,f' PackSizeProp='0090/0449,0002/0003' ScrollExtent='90,17' Size='90,17' Style='/Styles.New.buttons.hud.style' TextColor='#000000'>@ui_mission:details</Button>
                        gMessageFactory->addUint8(3);
                        gMessageFactory->addUint32(1);

						if(mId == 3)
							gMessageFactory->addString(BString(L"205,262")); //vertical, horizontal
						if(mId == 2)
							gMessageFactory->addString(BString(L"110,262")); //vertical, horizontal
						if(mId == 1)
							gMessageFactory->addString(BString(L"13,262")); //vertical, horizontal

                        gMessageFactory->addUint32(2);
                        gMessageFactory->addString(mName);
                        gMessageFactory->addString(BString("Location"));


                    }

                    gMessageFactory->addUint8(3);
                    gMessageFactory->addUint32(1);
                    
					for(unsigned short i=0; i < mId; i++)
						gMessageFactory->addString(BString(L"p,f"));

                    gMessageFactory->addUint32(2);
                    gMessageFactory->addString(mName);
                    gMessageFactory->addString(BString("PackSize"));


                    gMessageFactory->addUint8(3);
                    gMessageFactory->addUint32(1);

					for(unsigned short i = 0; i < mId; i++)
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


