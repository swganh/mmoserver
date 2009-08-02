/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIDROPDOWNBOX_H
#define ANH_ZONESERVER_UIDROPDOWNBOX_H

#include "Utils/typedefs.h"
#include "UIElement.h"

//================================================================================

class UIDropdownBox : public UIElement
{
	public:

		UIDropdownBox(){}
		UIDropdownBox(uint32 id,string name,bool enabled,const BStringVector elements,uint16 maxLength);
		virtual ~UIDropdownBox();

		string			getName(){ return mName; }
		void			setName(string name){ mName = name; }

		bool			getEnabled(){ return mEnabled; }
		void			setEnabled(bool e){ mEnabled = e; }

		uint16			getMaxLength(){ return mMaxLength; }
		void			setMaxLength(uint16 maxLength){ mMaxLength = maxLength; }

		BStringVector*	getElements(){ return &mElements; }

		virtual uint32	getPropertyCount();

		virtual void	addMessageData();

	private:

		string			mName;
		bool			mEnabled;
		BStringVector	mElements;
		uint16			mMaxLength;
};

#endif


