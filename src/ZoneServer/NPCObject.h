/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_ZONESERVER_NPC_OBJECT_H
#define ANH_ZONESERVER_NPC_OBJECT_H

#include "CreatureObject.h"
#include "NPC_Enums.h"


#define NPC_CHAT_SPAM_PROTECTION_TIME	10000

//=============================================================================

class ActiveConversation;
class ConversationPage;
class ConversationOption;
class PlayerObject;
class Conversation;

//=============================================================================

class NPCObject : public CreatureObject
{
	public:

		friend class PersistentNPCFactory;
		friend class NonPersistentNpcFactory;

		typedef enum _Npc_AI_State
		{
			NpcIsDormant = 0,
			NpcIsReady,
			NpcIsActive,
		} Npc_AI_State;

		NPCObject();
		virtual ~NPCObject();

		uint32			getNpcFamily() const { return mNpcFamily; }
		void			setNpcFamily(uint32 family){ mNpcFamily = family; }

		string			getTitle() const { return mTitle; }
		void			setTitle(string title){ mTitle = title; }

		virtual void	filterConversationOptions(ConversationPage* page,std::vector<ConversationOption*>* filteredOptions,PlayerObject* player){}
		virtual uint32	handleConversationEvent(ActiveConversation* av,ConversationPage* page,ConversationOption* option,PlayerObject* player){return 0;}
		virtual	bool	preProcessfilterConversation(ActiveConversation* av, Conversation* conversation, PlayerObject* player) {return false;}
		virtual void	postProcessfilterConversation(ActiveConversation* av,ConversationPage* page,PlayerObject* player) {}
		virtual void	prepareConversation(PlayerObject* player) {}
		virtual void	stopConversation(PlayerObject* player) {}
		void			restoreDefaultDirection() {mDirection = mDefaultDirection;}
		void			storeDefaultDirection() {mDefaultDirection = mDirection;}
		virtual void	restorePosition(PlayerObject* player) {}

		virtual void	handleEvents(void) { }
		virtual uint64	handleState(uint64 timeOverdue) {return 0; }


		// Used for NPC movements
		void			setPositionOffset(Anh_Math::Vector3 positionOffset) {mPositionOffset = positionOffset;}
		Anh_Math::Vector3	getPositionOffset() const {return mPositionOffset;}

		Anh_Math::Vector3 getRandomPosition(Anh_Math::Vector3& currentPos, int32 offsetX, int32 offsetZ) const;
		float			getHeightAt2DPosition(float xPos, float zPos) const;
		void			setDirection(float deltaX, float deltaZ);
		
		void			moveAndUpdatePosition(void);
		void			updatePosition(uint64 parentId, Anh_Math::Vector3 newPosition);

		uint64			getLastConversationTarget()const { return mLastConversationTarget; }
		uint64			getLastConversationRequest() const { return mLastConversationRequest; }

		void			setLastConversationTarget(uint64 target){ mLastConversationTarget = target; }
		void			setLastConversationRequest(uint64 request){ mLastConversationRequest = request; }

		Npc_AI_State	getAiState(void) const;

	private:
	

	protected:
		void			setAiState(Npc_AI_State state);
		

		uint32	mNpcFamily;
		string	mTitle; 

		uint64	mLastConversationTarget;
		uint64	mLastConversationRequest;
		

	private:
		Npc_AI_State	mAiState;
		Anh_Math::Quaternion	mDefaultDirection;	// Default direction for npc-objects. Needed when players start turning the npc around.
		Anh_Math::Vector3		mPositionOffset;

};

//=============================================================================


class	NpcConversionTime
{
	public:
		NPCObject*	mNpc;
		uint64		mTargetId;		// This is used when we run instances, and to differ
		uint64		mGroupId;		// Player belonging to same group, will be handled as one "unit" regarding the timeer.
		uint64		mInterval;
};

#endif