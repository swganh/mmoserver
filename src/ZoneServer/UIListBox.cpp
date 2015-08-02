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
#include "UIListBox.h"
#include "PlayerObject.h"
#include "UIButton.h"
#include "UICallback.h"
#include "UIOpcodes.h"
#include "UIManager.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

//================================================================================

UIListBox::UIListBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,BString caption,BString prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType, float distance, uint64 object, void* container)
    : UIWindow(callback,id,windowType,"Script.listBox",eventStr,container),mLbType(lbType)
{
    mDistance	= distance;
    mObjectID	= object;
    mOwner		= playerObject;
    mCaption	= caption;
    mPrompt		= prompt;
    mDataItems	= dataItems;
    mCaption.convert(BSTRType_Unicode16);
    mPrompt.convert(BSTRType_Unicode16);

    mOption3 = "";
    mOption4 = "";

    _initChildren();
}

//================================================================================

UIListBox::~UIListBox()
{
}

//================================================================================

void UIListBox::handleEvent(Message* message)
{
    uint32	action				= message->getUint32();
    uint32	items				= message->getUint32();
    BString	selectedDataItemStr;
    BString	caption;
    int32	selectedItem		= -1;

    if(items)
    {
        message->getUint32(); // item count again
        message->getStringUnicode16(selectedDataItemStr);

        if(swscanf(selectedDataItemStr.getUnicode16(),L"%i",&selectedItem) != 1)
            DLOG(INFO) << "UIListBox::handleEvent: item mismatch";

        if(items >= 2)
            message->getStringUnicode16(caption);

        if(items >= 3)
            message->getStringUnicode16(mOption3);

        if(items >= 4)
            message->getStringUnicode16(mOption4);
    }

    if(mUICallback != NULL)
        mUICallback->handleUIEvent(action,selectedItem,selectedDataItemStr,this);

    mOwner->removeUIWindow(mId);
    gUIManager->destroyUIWindow(mId);
}

//================================================================================

void UIListBox::sendCreate()
{
    if(!mOwner || mOwner->getConnectionState() != PlayerConnState_Connected)
        return;

    Message*	newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSuiCreatePageMessage);

    gMessageFactory->addUint32(mId);

    gMessageFactory->addString(mWindowTypeStr);
    uint32 datasize = mDataItems.size();
    if(datasize > 250)
        datasize = 250;

    uint32 propertyCount =5 + (datasize << 1) + getChildrenPropertyCount();

    gMessageFactory->addUint32(propertyCount);

    // main window properties
    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);	//listsize
    gMessageFactory->addUint32(9);	 //listsize
    gMessageFactory->addUint16(0);	 //string
    gMessageFactory->addUint16(1);	 //string
    gMessageFactory->addUint8(9);
    gMessageFactory->addString(mEventStr);
    gMessageFactory->addString(BString("List.lstList"));//return 1
    gMessageFactory->addString(BString("SelectedRow"));
    gMessageFactory->addString(BString("bg.caption.lblTitle"));	  //return 2
    gMessageFactory->addString(BString("Text"));

    gMessageFactory->addString(BString("this")); //return 4
    gMessageFactory->addString(BString("otherPressed"));

    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(9);
    gMessageFactory->addUint16(0);   //string 1

    gMessageFactory->addUint16(1);   //string 2
    gMessageFactory->addUint8(10);	 //text string 2

    gMessageFactory->addString(mEventStr);//string 3

    gMessageFactory->addString(BString("List.lstList"));		//
    gMessageFactory->addString(BString("SelectedRow"));

    gMessageFactory->addString(BString("bg.caption.lblTitle"));
    gMessageFactory->addString(BString("Text"));

    gMessageFactory->addString(BString("this"));//ok this is rather straightforward
    gMessageFactory->addString(BString("otherPressed"));




    //with the 05 we set the returntype. if we set it like the other boxes it always returns the caption
    //by adding the last 2 strings (this and otherPressed) it will return true when btnOther was pressed or false if btnOther was NOT pressed
    //I could not get the UI to return any other value than zero in the return int when any other button than cancel was pressed.

    // caption
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mCaption);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("bg.caption.lblTitle"));
    gMessageFactory->addString(BString("Text"));

    // prompt
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mPrompt);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("Prompt.lblPrompt"));
    gMessageFactory->addString(BString("Text"));


    Children::iterator childrenIt = mChildElements.begin();

    while(childrenIt != mChildElements.end())
    {
        (*childrenIt)->addMessageData();
        ++childrenIt;
    }

    // data items
    gMessageFactory->addUint8(1);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(BString("List.dataList"));

    BStringVector::iterator it = mDataItems.begin();
    uint8 index = 0;
    uint32 count = 0;
    while(it != mDataItems.end())
    {
        count ++;
        char indexStr[64];
        sprintf(indexStr,"%u",index);
		std::string indexString(indexStr);
		std::wstring windexStr(indexString.begin(), indexString.end());

        std::string itemName("List.dataList.");
        itemName.append(indexStr);

        BString item = (*it).getAnsi();
        item.convert(BSTRType_Unicode16);

        gMessageFactory->addUint8(4);
        gMessageFactory->addUint32(1);
        gMessageFactory->addString(windexStr);
        gMessageFactory->addUint32(2);
        gMessageFactory->addString(BString("List.dataList"));
        gMessageFactory->addString(BString("Name"));

        gMessageFactory->addUint8(3);
        gMessageFactory->addUint32(1);
        gMessageFactory->addString(item);
        gMessageFactory->addUint32(2);
        gMessageFactory->addString(itemName);
        gMessageFactory->addString(BString("Text"));

        ++it;
        index++;
        if(count == datasize)
            break;
    }

    //Size='66,17'

    // unknown
    gMessageFactory->addUint64(mObjectID);
    gMessageFactory->addFloat(mDistance);
    gMessageFactory->addUint64(0);

    newMessage = gMessageFactory->EndMessage();

    (mOwner->getClient())->SendChannelA(newMessage,mOwner->getAccountId(),CR_Client,2);
}

//================================================================================

void UIListBox::_initChildren()
{
    switch(mLbType)
    {
    case SUI_LB_OK:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",false));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@ok"));
    }
    break;

    case SUI_LB_OKCANCEL:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",true,"@cancel"));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@ok"));
    }
    break;

    case SUI_LB_CANCELREFRESH:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",true,"@cancel"));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@refresh"));
    }
    break;

    case SUI_LB_CANCEL_SCHEMATIC_REMOVEUSE:
    {
        mChildElements.push_back(new UIButton(1,"btnCancel",true,"@cancel",true));
        mChildElements.push_back(new UIButton(2,"btnOk",true,"@use_schematic",true));
        mChildElements.push_back(new UIButton(3,"btnOther",true,"@remove_schematic",true));



    }
    break;

    case SUI_LB_CANCEL_SCHEMATIC_USE:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",true,"@cancel"));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@use_schematic"));

    }
    break;


    case SUI_LB_YESNO:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",true,"@no"));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@yes"));
    }
    break;

    default:
        break;
    }
}

//================================================================================




