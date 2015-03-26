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
#include "UIMessageBox.h"
#include "PlayerObject.h"
#include "UIButton.h"
#include "UICallback.h"
#include "UIOpcodes.h"
#include "UIManager.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"


//================================================================================

UIMessageBox::UIMessageBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,const int8* caption,const int8* text,PlayerObject* playerObject,uint8 mbType, void* container)
    : UIWindow(callback,id,windowType,"Script.messageBox",eventStr, container),mMBType(mbType)
{
    mOwner		= playerObject;
    mCaption	= caption;
    mText		= text;
    mCaption.convert(BSTRType_Unicode16);
    mText.convert(BSTRType_Unicode16);

    _initChildren();
}

//================================================================================

UIMessageBox::~UIMessageBox()
{
}

//================================================================================

void UIMessageBox::handleEvent(Message* message)
{
    uint32 action = message->getUint32();

    // 0 = OK, 1 = Cancel

    if(mUICallback  != NULL)
        mUICallback->handleUIEvent(action,0,"",this);

    mOwner->removeUIWindow(mId);
    gUIManager->destroyUIWindow(mId);
}

//================================================================================

void UIMessageBox::sendCreate()
{
    if(!mOwner || mOwner->getConnectionState() != PlayerConnState_Connected)
        return;

    Message*	newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSuiCreatePageMessage);

    gMessageFactory->addUint32(mId);

    gMessageFactory->addString(mWindowTypeStr);

    uint32 propertyCount = 4 + getChildrenPropertyCount();

    gMessageFactory->addUint32(propertyCount);

    // main window properties
    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(3);
    gMessageFactory->addUint16(0);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint8(9);
    gMessageFactory->addString(mEventStr);

    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(3);
    gMessageFactory->addUint16(0);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint8(10);
    gMessageFactory->addString(mEventStr);

    // text
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mText);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("Prompt.lblPrompt"));
    gMessageFactory->addString(BString("Text"));

    // caption
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mCaption);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("bg.caption.lblTitle"));
    gMessageFactory->addString(BString("Text"));

    // child elements
    Children::iterator childrenIt = mChildElements.begin();

    while(childrenIt != mChildElements.end())
    {
        (*childrenIt)->addMessageData();
        ++childrenIt;
    }

    // unknown
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint32(0);


    newMessage = gMessageFactory->EndMessage();

    (mOwner->getClient())->SendChannelA(newMessage,mOwner->getAccountId(),CR_Client,2);
}

//================================================================================

void UIMessageBox::_initChildren()
{
    switch(mMBType)
    {
    case SUI_MB_OK:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",false));
        mChildElements.push_back(new UIButton(0,"btnRevert",false));
        mChildElements.push_back(new UIButton(0,"btnOk"));
    }
    break;

    case SUI_MB_OKCANCEL:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel"));
        mChildElements.push_back(new UIButton(0,"btnRevert",false));
        mChildElements.push_back(new UIButton(0,"btnOk"));
    }
    break;

    case SUI_MB_YESNO:
    {
        mChildElements.push_back(new UIButton(0,"btnCancel",true,"@no"));
        mChildElements.push_back(new UIButton(0,"btnRevert",false));
        mChildElements.push_back(new UIButton(0,"btnOk",true,"@yes"));
    }
    break;

    default:
        break;
    }
}

//================================================================================

