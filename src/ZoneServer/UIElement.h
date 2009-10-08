/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_UIELEMENT_H
#define ANH_ZONESERVER_UIELEMENT_H

#include "Utils/typedefs.h"
#include "UIEnums.h"

//================================================================================

class UIElement
{
	public:

		UIElement(){}
		UIElement(uint32 id,uint8 elementType);
		virtual ~UIElement();

		uint32			getId(){ return mId; }
		void			setId(uint32 id){ mId = id; }

		uint8			getElementType(){ return mElementType; }
		void			setElementType(uint8 type){ mElementType = type; }

		virtual void	addMessageData(){}
		virtual uint32	getPropertyCount(){ return(0); }

	protected:

		uint32	mId;
		uint8	mElementType;
};

#endif


