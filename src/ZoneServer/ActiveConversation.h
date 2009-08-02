/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_ACTIVE_CONVERSATION_H
#define ANH_ZONESERVER_ACTIVE_CONVERSATION_H

#include "Utils/typedefs.h"
#include "Conversation.h"
#include <vector>


//=============================================================================

class PlayerObject;
class NPCObject;
class ConversationPage;
class ConversationOption;

//=============================================================================

class ActiveConversation
{
	public:

		ActiveConversation(Conversation* baseConv,PlayerObject* player,NPCObject* npc);
		~ActiveConversation();

		ConversationPage*		getCurrentPage(){ return mCurrentPage; }
		void					setCurrentPage(uint32 pageLink);
		void					prepareFilteredOptions();

		void					updateCurrentPage(uint32 selectId);
		bool					preProcessConversation();
		void					postProcessCurrentPage();
		

		ConversationOptions*	getFilteredOptions(){ return &mSelectOptionMap; }

		NPCObject*				getNpc(){ return mNpc; }

		int32					getDI(){ return mDI; }
		string					getTTStfFile(){ return mTTStfFile; }
		string					getTTStfVariable(){ return mTTStfVariable; }
		string					getTTCustom(){ return mTTCustom; }
		uint64					getTTId(){ return mTTId; }
		string					getTOStfFile(){ return mTOStfFile; }
		string					getTOStfVariable(){ return mTOStfVariable; }
		string					getTOCustom(){ return mTOCustom; }
		uint64					getTOId(){ return mTOId; }
		
	private:

		// void				_prepareFilteredOptions();

		Conversation*		mBaseConversation;
		PlayerObject*		mPlayer;
		NPCObject*			mNpc;

		ConversationPage*	mCurrentPage;

		ConversationOptions	mSelectOptionMap;

		int32				mDI;
		string				mTTStfFile;
		string				mTTStfVariable;
		string				mTTCustom;
		uint64				mTTId;
		string				mTOStfFile;
		string				mTOStfVariable;
		string				mTOCustom;
		uint64				mTOId;
};

//=============================================================================

#endif

