/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_CONVERSATION_H
#define ANH_ZONESERVER_CONVERSATION_H

#include "Utils/typedefs.h"
#include <vector>


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

		uint32				getId(){ return mId; }
		ConversationPage*	getFirstPage(){ return(mPages[0]);}
		ConversationPage*	getPage(uint32 pageNr){ return(mPages[pageNr - 1]);}

	private:

		uint32				mId;
		ConversationPages	mPages;
};

//=============================================================================

class ConversationOption
{
	public:

		uint32	mId;
		string	mCustomText;
		string	mStfFile;
		string	mStfVariable;
		uint32	mEvent;
		uint32	mPageLinkId;
};

//=============================================================================

class ConversationPage
{
	public:

		uint32					mId;
		string					mCustomText;
		string					mStfFile;
		string					mStfVariable;
		ConversationOptions		mOptions;
		uint32					mAnimation;
};

//=============================================================================

#endif

