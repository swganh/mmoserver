/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIBUTTON_H
#define ANH_ZONESERVER_UIBUTTON_H

#include "Utils/typedefs.h"
#include "UIElement.h"

//================================================================================

class UIButton : public UIElement
{
	public:

		UIButton(){}
		UIButton(uint32 id,string name,bool enabled = true,string text = "", bool three = false);
		virtual ~UIButton();

		string			getName(){ return mName; }
		void			setName(string name){ mName = name; }

		string			getText(){ return mText; }
		void			setText(string text){ mText = text; }
		
		bool			getEnabled(){ return mEnabled; }
		void			setEnabled(bool e){ mEnabled = e; }

		virtual uint32	getPropertyCount();

		virtual void	addMessageData();

	private:

		string	mName;
		string	mText;
		bool	mEnabled;
		bool	m3B;
};

#endif


