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

#ifndef ANH_ZONESERVER_CONVERSATION_H
#define ANH_ZONESERVER_CONVERSATION_H

#include <vector>
#include "Utils/bstring.h"
#include "Utils/typedefs.h"


//=============================================================================

class ConversationPage;
class ConversationOption;

//=============================================================================

typedef std::vector<ConversationPage*>		ConversationPages;
typedef std::vector<ConversationOption*>	ConversationOptions;

//=============================================================================

enum ConversationEvent
{
    ConvEvent_FilterOptions	= 1,
    ConvEvent_TrainSkill	= 2
};

//=============================================================================

class Conversation
{
public:

    friend class ConversationManager;

    Conversation();
    ~Conversation();

    uint32				getId() {
        return mId;
    }
    ConversationPage*	getFirstPage() {
        return(mPages[0]);
    }
    ConversationPage*	getPage(uint32 pageNr) {
        return(mPages[pageNr - 1]);
    }

private:

    uint32				mId;
    ConversationPages	mPages;
};

//=============================================================================

class ConversationOption
{
public:

    uint32	mId;
    BString	mCustomText;
    BString	mStfFile;
    BString	mStfVariable;
    uint32	mEvent;
    uint32	mPageLinkId;
};

//=============================================================================

class ConversationPage
{
public:

    uint32					mId;
    BString					mCustomText;
    BString					mStfFile;
    BString					mStfVariable;
    ConversationOptions		mOptions;
    uint32					mAnimation;
};

//=============================================================================

#endif

