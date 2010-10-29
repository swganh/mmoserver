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

#include "UITransferBox.h"
#include "UIOpcodes.h"
#include "UIManager.h"
#include "UIButton.h"
#include "UIInputField.h"
#include "UIDropdownBox.h"
#include "UICallback.h"
#include "PlayerObject.h"

#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"

#include <boost/lexical_cast.hpp>

//================================================================================

UITransferBox::UITransferBox(UICallback* callback,uint32 id,const int8* eventStr,const int8* caption,const int8* text
                             ,const int8* leftTitle,const int8* rightTitle, uint32 leftValue, uint32 rightValue, PlayerObject* playerObject, uint8 windowType)
    : UIWindow(callback,id,windowType,"Script.transfer",eventStr)
{
    mOwner		= playerObject;
    mCaption	= caption;
    mPrompt		= text;
    mLeftTitle  = leftTitle;
    mRightTitle = rightTitle;

    mLeftValue	= boost::lexical_cast<std::string>(leftValue).c_str();
    mRightValue = boost::lexical_cast<std::string>(rightValue).c_str();

    mCaption.convert(BSTRType_Unicode16);
    mPrompt.convert(BSTRType_Unicode16);
    mLeftTitle.convert(BSTRType_Unicode16);
    mRightTitle.convert(BSTRType_Unicode16);
    mLeftValue.convert(BSTRType_Unicode16);
    mRightValue.convert(BSTRType_Unicode16);
}



//================================================================================

UITransferBox::~UITransferBox()
{
}

//================================================================================

void UITransferBox::handleEvent(Message* message)
{
    uint32	action				= message->getUint32();
    uint32	items				= message->getUint32();
    BString	leftValue;
    BString	rightValue;

    if(items)
    {
        message->getUint32(); // item count again
        message->getStringUnicode16(leftValue);  // left side value
        message->getStringUnicode16(rightValue); // right side value
    }

    if(mUICallback != NULL && !action) // also check if OK was pressed
        mUICallback->handleUIEvent(leftValue,rightValue,this);

    mOwner->removeUIWindow(mId);
    gUIManager->destroyUIWindow(mId);
}

//================================================================================

void UITransferBox::sendCreate()
{

    if(!mOwner || mOwner->getConnectionState() != PlayerConnState_Connected)
        return;

    Message*	newMessage;

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opSuiCreatePageMessage);

    gMessageFactory->addUint32(mId);

    gMessageFactory->addString(mWindowTypeStr);

    gMessageFactory->addUint32(12); //propertyCount

    // main window properties
    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(7);
    gMessageFactory->addUint16(0);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint8(9);
    gMessageFactory->addString(mEventStr); //handleDepositWithdraw
    gMessageFactory->addString(BString("transaction.txtInputFrom"));
    gMessageFactory->addString(BString("Text"));
    gMessageFactory->addString(BString("transaction.txtInputTo"));
    gMessageFactory->addString(BString("Text"));


    gMessageFactory->addUint8(5);
    gMessageFactory->addUint32(0);
    gMessageFactory->addUint32(7);
    gMessageFactory->addUint16(0);
    gMessageFactory->addUint16(1);
    gMessageFactory->addUint8(10);
    gMessageFactory->addString(mEventStr); //handleDepositWithdraw
    gMessageFactory->addString(BString("transaction.txtInputFrom"));
    gMessageFactory->addString(BString("Text"));
    gMessageFactory->addString(BString("transaction.txtInputTo"));
    gMessageFactory->addString(BString("Text"));

    // prompt
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mCaption);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("bg.caption.lblTitle"));
    gMessageFactory->addString(BString("Text"));

    // caption
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mPrompt);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("Prompt.lblPrompt"));
    gMessageFactory->addString(BString("Text"));

    // From label
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mLeftTitle);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.lblFrom"));
    gMessageFactory->addString(BString("Text"));

    // To label
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mRightTitle);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.lblTo"));
    gMessageFactory->addString(BString("Text"));


    // Default value left
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mLeftValue);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.lblStartingFrom"));
    gMessageFactory->addString(BString("Text"));


    // Default value right
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mRightValue);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.lblStartingTo"));
    gMessageFactory->addString(BString("Text"));

    // Min left
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mLeftValue);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.txtInputFrom"));
    gMessageFactory->addString(BString("Text"));

    // Max left
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addString(mRightValue);
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction.txtInputTo"));
    gMessageFactory->addString(BString("Text"));

    //Ratio
    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(1);
    gMessageFactory->addInt16(0x31); //1
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction"));
    gMessageFactory->addString(BString("ConversionRatioFrom"));

    gMessageFactory->addUint8(3);
    gMessageFactory->addUint32(1);
    gMessageFactory->addUint32(1);
    gMessageFactory->addInt16(0x31); //1
    gMessageFactory->addUint32(2);
    gMessageFactory->addString(BString("transaction"));
    gMessageFactory->addString(BString("ConversionRatioTo"));

    // unknown
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint64(0);
    gMessageFactory->addUint32(0);

    newMessage = gMessageFactory->EndMessage();

    (mOwner->getClient())->SendChannelA(newMessage,mOwner->getAccountId(),CR_Client,2);
}
