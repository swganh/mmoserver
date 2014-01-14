/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "UIWindow.h"
#include "UICallback.h"
#include "Common/Message.h"


//================================================================================

UIWindow::UIWindow(UICallback* callback,uint32 id,uint8 windowType,const string windowTypeStr,const int8* eventStr, void* container)
: UIElement(id,SUI_Element_Window)
, mWindowTypeStr(windowTypeStr)
, mUICallback(callback)
, mWindowType(windowType)
{
	mEventStr = eventStr;
	mChildElements.reserve(5);
	mContainer = container;
}

//================================================================================

UIWindow::~UIWindow()
{
	Children::iterator it = mChildElements.begin();

	while(it != mChildElements.end())
	{
		delete(*it);
		mChildElements.erase(it);
		it = mChildElements.begin();
	}
}

//================================================================================

UIElement* UIWindow::getChildById(uint32 id)
{
	Children::iterator it = mChildElements.begin();

	while(it != mChildElements.end())
	{
		if((*it)->getId() == id)
			return(*it);

		++it;
	}

	return(NULL);
}

//================================================================================

bool UIWindow::removeChild(UIElement* element)
{
	Children::iterator it = mChildElements.begin();

	while(it != mChildElements.end())
	{
		if((*it) == element)
		{
			mChildElements.erase(it);
			return(true);
		}

		++it;
	}

	return(false);
}

//================================================================================

bool UIWindow::removeChild(uint32 id)
{
	Children::iterator it = mChildElements.begin();

	while(it != mChildElements.end())
	{
		if((*it)->getId() == id)
		{
			mChildElements.erase(it);
			return(true);
		}

		++it;
	}

	return(false);
}

//================================================================================

uint32 UIWindow::getChildrenPropertyCount()
{
	uint32 count = 0;
	Children::iterator childrenIt = mChildElements.begin();

	while(childrenIt != mChildElements.end())
	{
		count += (*childrenIt)->getPropertyCount();
		++childrenIt;
	}

	return(count);
}

//================================================================================


