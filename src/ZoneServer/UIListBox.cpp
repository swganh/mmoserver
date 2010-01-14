/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/
#include "UIListBox.h"
#include "PlayerObject.h"
#include "UIButton.h"
#include "UICallback.h"
#include "UIOpcodes.h"
#include "UIManager.h"

#include "LogManager/LogManager.h"
#include "Common/Message.h"
#include "Common/MessageFactory.h"

//================================================================================

UIListBox::UIListBox(UICallback* callback,uint32 id,uint8 windowType,const int8* eventStr,string caption,BString prompt,const BStringVector dataItems,PlayerObject* playerObject,uint8 lbType, float distance, uint64 object, void* container)
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
	string	selectedDataItemStr;
	string	caption;
	int32	selectedItem		= -1;

	if(items)
	{
		message->getUint32(); // item count again
		message->getStringUnicode16(selectedDataItemStr);

		if(swscanf(selectedDataItemStr.getUnicode16(),L"%i",&selectedItem) != 1)
			gLogger->logMsg("UIListBox::handleEvent: item mismatch");

		message->getStringUnicode16(caption);
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

	//uint32 propertyCount = 5 + (datasize << 1) + getChildrenPropertyCount();
	uint32 propertyCount = 6 + (datasize << 1) + getChildrenPropertyCount();

	gMessageFactory->addUint32(propertyCount);

	// main window properties
	gMessageFactory->addUint8(5);
	gMessageFactory->addUint32(0);	//listsize
	gMessageFactory->addUint32(7);	 //listsize
	gMessageFactory->addUint16(0);	 //string
	gMessageFactory->addUint16(1);	 //string
	gMessageFactory->addUint8(9);
	gMessageFactory->addString(mEventStr);
	gMessageFactory->addString(BString("List.lstList"));
	gMessageFactory->addString(BString("SelectedRow"));
	gMessageFactory->addString(BString("bg.caption.lblTitle"));
	gMessageFactory->addString(BString("Text"));

	gMessageFactory->addUint8(5);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(7);
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint16(1);
	gMessageFactory->addUint8(10);
	gMessageFactory->addString(mEventStr);
	gMessageFactory->addString(BString("List.lstList"));
	gMessageFactory->addString(BString("SelectedRow"));
	gMessageFactory->addString(BString("bg.caption.lblTitle"));
	gMessageFactory->addString(BString("Text"));

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

	// child elements
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
		string indexStr;
		indexStr.setLength(sprintf(indexStr.getAnsi(),"%u",index));

		string itemName = "List.dataList.";
		itemName << indexStr.getAnsi();

		indexStr.convert(BSTRType_Unicode16);

		string item = (*it).getAnsi();
		item.convert(BSTRType_Unicode16);

		gMessageFactory->addUint8(4);
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(indexStr);
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
	/*gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(1);
	gMessageFactory->addString(BString(L"True"));
	gMessageFactory->addUint32(2);
	gMessageFactory->addString(mName);
	gMessageFactory->addString(BString("visible"));
	  */
	gMessageFactory->addUint8(3);
	gMessageFactory->addUint32(1);
	gMessageFactory->addString(BString(L"@cancel"));
	gMessageFactory->addUint32(2);
	gMessageFactory->addString(BString("btnCancel"));
	gMessageFactory->addString(BString("Text"));

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
			//mChildElements.push_back(new UIButton(1,"btnRevert",true,"@cancel"));
			mChildElements.push_back(new UIButton(0,"btnOther",true,"@remove_schematic"));
			mChildElements.push_back(new UIButton(0,"btnOk",true,"@use_schematic"));
			
			
			
		}
		break;

		case SUI_LB_CANCEL_SCHEMATIC_USE:
		{
			mChildElements.push_back(new UIButton(0,"btnCancel",true,"@cancel"));
			mChildElements.push_back(new UIButton(0,"btnOk",true,"@use_schematic"));
			
		}
		break;
		
		default:break;
	}
}

//================================================================================




