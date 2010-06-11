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

#include "MessageLib.h"

#include "ZoneServer/ActiveConversation.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/DraftWeight.h"
#include "ZoneServer/DraftSlot.h"
#include "ZoneServer/DraftSchematic.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/SchematicManager.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/WaypointObject.h"
#include "ZoneServer/Weapon.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WeightsBatch.h"
#include "ZoneServer/WorldConfig.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ZoneTree.h"

#include "LogManager/LogManager.h"

#include "Common/atMacroString.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>


//======================================================================================================================
//
// Spatial Chat
//

void MessageLib::sendSpatialChat(CreatureObject* const srcObject,string chatMsg,char chatElement[5][32])
{
	using boost::lexical_cast;
    using boost::bad_lexical_cast;

	uint64 chatElementTarget;
	try
	{
		chatElementTarget	= boost::lexical_cast<uint64>(chatElement[0]);
	}
	catch(bad_lexical_cast &)
	{
		chatElementTarget	= 0;

	}
		
	uint16 chatElementMood1		= atoi(chatElement[1]);
	uint16 chatElementMood2		= atoi(chatElement[2]);

	if(!chatElementMood2)
		chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSpatialChat);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(chatElementTarget);
	mMessageFactory->addString(chatMsg);
	mMessageFactory->addUint16(0x32);
	mMessageFactory->addUint16(chatElementMood1);
	mMessageFactory->addUint16(chatElementMood2);
	mMessageFactory->addUint8(0);
	mMessageFactory->addUint8(static_cast<uint8>(srcObject->getLanguage()));
	mMessageFactory->addUint64(0);                    // unknown

	newMessage = mMessageFactory->EndMessage();


	PlayerObjectSet* inRangePlayers	= srcObject->getKnownPlayers();
	PlayerObjectSet::iterator it	= inRangePlayers->begin();
	uint32 loweredNameCrc			= 0;
	string loweredName;
	Message* clonedMessage;
	bool crcValid = false;

	// Get the source for this emote.
	if (srcObject->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(srcObject);

		if (srcPlayer->isConnected())
		{
			loweredName = srcPlayer->getFirstName().getAnsi();
			loweredName.toLower();
			loweredNameCrc = loweredName.getCrc();
			crcValid = true;
		}
	}

	while(it != inRangePlayers->end())
	{
		const PlayerObject* const player = (*it);

		// If player online, send emote.
		if (_checkPlayer(player))
		{
			if ((crcValid) && (player->checkIgnoreList(loweredNameCrc)))
 			{
				// I am at recivers ignore list.
				// Don't send any message.
			}
			else
			{
				// clone our message
				mMessageFactory->StartMessage();
				mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
				clonedMessage = mMessageFactory->EndMessage();

				// replace the target id
				int8* data = clonedMessage->getData() + 12;
				*((uint64*)data) = player->getId();

				//(player->getClient())->SendChannelA(clonedMessage,player->getAccountId(),CR_Client,5);
				(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
			}
		}
		++it;
	}

	// if we are a player, echo it back to ourself
	if(srcObject->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(srcObject);

		if (_checkPlayer(srcPlayer))
		{
			(srcPlayer->getClient())->SendChannelAUnreliable(newMessage,srcPlayer->getAccountId(),CR_Client,5);
			return;
		}
	}

	mMessageFactory->DestroyMessage(newMessage);

}


//======================================================================================================================
//
// Spatial Chat, used for tutorial
// We do send NPC chat to "player", or "player" is talking to himself.
//
/* Not good enough when we run instanced groups

void MessageLib::sendSpatialChat(CreatureObject* srcObject,string chatMsg,char chatElement[5][32], PlayerObject* player)
{
	uint64 chatElementTarget	= _atoi64(chatElement[0]);
	uint16 chatElementMood1		= atoi(chatElement[1]);
	uint16 chatElementMood2		= atoi(chatElement[2]);

	if(!chatElementMood2)
		chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSpatialChat);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(chatElementTarget);
	mMessageFactory->addString(chatMsg);
	mMessageFactory->addUint16(0x32);
	mMessageFactory->addUint16(chatElementMood1);
	mMessageFactory->addUint16(chatElementMood2);
	mMessageFactory->addUint8(0);
	mMessageFactory->addUint8(srcObject->getLanguage());
	mMessageFactory->addUint64(0);                    // unknown

	newMessage = mMessageFactory->EndMessage();

	// If player online, send emote.
	if (player->isConnected())
	{
		// Update tutorial about "chatActive"
		player->getTutorial()->tutorialResponse("chatActive");

		// replace the target id
		int8* data = newMessage->getData() + 12;
		*((uint64*)data) = player->getId();
		(player->getClient())->SendChannelA(newMessage,player->getAccountId(),CR_Client,5,false);
	}
	else
	{
		mMessageFactory->DestroyMessage(newMessage);
	}
}
*/

void MessageLib::sendSpatialChat(const CreatureObject* const srcObject,string chatMsg,char chatElement[5][32], const PlayerObject* const playerObject) const
{

	using boost::lexical_cast;
    using boost::bad_lexical_cast;

	uint64 chatElementTarget;
	try
	{
		chatElementTarget	= boost::lexical_cast<uint64>(chatElement[0]);
	}
	catch(bad_lexical_cast &)
	{
		chatElementTarget	= 0;

	}

	uint16 chatElementMood1		= atoi(chatElement[1]);
	uint16 chatElementMood2		= atoi(chatElement[2]);

	if(!chatElementMood2)
		chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSpatialChat);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(chatElementTarget);
	mMessageFactory->addString(chatMsg);
	mMessageFactory->addUint16(0x32);
	mMessageFactory->addUint16(chatElementMood1);
	mMessageFactory->addUint16(chatElementMood2);
	mMessageFactory->addUint8(0);
	mMessageFactory->addUint8(static_cast<uint8>(srcObject->getLanguage()));
	mMessageFactory->addUint64(0);                    // unknown

	newMessage = mMessageFactory->EndMessage();

	uint32 loweredNameCrc			= 0;
	string loweredName;
	bool crcValid = false;

	// Get the source for this emote.
	if (srcObject->getType() == ObjType_Player)
	{
		const PlayerObject* const srcPlayer = dynamic_cast<const PlayerObject*>(srcObject);

		if (srcPlayer->isConnected())
		{
			loweredName = srcPlayer->getFirstName().getAnsi();
			loweredName.toLower();
			loweredNameCrc = loweredName.getCrc();
			crcValid = true;	// crcValid true means that the source is a connected player.
		}
	}

	if (_checkPlayer(playerObject))
	{
		if (gWorldConfig->isTutorial() && crcValid)		// Don't trig the Tutorial at npc talk.
		{
			// Update tutorial about "chatActive"
			playerObject->getTutorial()->tutorialResponse("chatActive");
		}

		PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(true);
		PlayerList::iterator it	= inRangeMembers.begin();
		Message* clonedMessage;

		while (it != inRangeMembers.end())
		{
			const PlayerObject* const player = (*it);

			// If player online, send emote.
			if (_checkPlayer(player))
			{
				if ((crcValid) && (player->checkIgnoreList(loweredNameCrc)))
 				{
					// I am at recivers ignore list.
					// Don't send any message.
				}
				else
				{
					// clone our message
					mMessageFactory->StartMessage();
					mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
					clonedMessage = mMessageFactory->EndMessage();

					// replace the target id
					int8* data = clonedMessage->getData() + 12;
					*((uint64*)data) = player->getId();

					(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
				}
			}
			++it;
		}
	}
	mMessageFactory->DestroyMessage(newMessage);
}

bool MessageLib::sendSpatialChat(const CreatureObject* const srcObject,const PlayerObject* const playerObject,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId) const
{
	uint16 chatElementMood2	= srcObject->getMoodId();

	Message*	newMessage;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSpatialChat);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addString(L"");
	mMessageFactory->addUint16(0x32);
	mMessageFactory->addUint16(0);
	mMessageFactory->addUint16(chatElementMood2);
	mMessageFactory->addUint8(0);
	mMessageFactory->addUint8(static_cast<uint8>(srcObject->getLanguage()));

	// simple message
	if(customMessage.getLength())
	{
		mMessageFactory->addString(customMessage);
	}
	// templated message
	else
	{
		uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

		mMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + ttCustom.getLength());

		if(realSize % 2)
			mMessageFactory->addUint16(1);
		else
			mMessageFactory->addUint16(0);

		mMessageFactory->addUint8(1);
		mMessageFactory->addUint32(0xFFFFFFFF);
		mMessageFactory->addString(mainFile);
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(mainVar);
		mMessageFactory->addUint64(tuId);

		mMessageFactory->addUint16(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint16(0);

		mMessageFactory->addUint32(0);

		mMessageFactory->addUint64(ttId);
		mMessageFactory->addString(ttFile);
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(ttVar);
		mMessageFactory->addString(ttCustom);
		mMessageFactory->addUint64(toId);
		mMessageFactory->addString(toFile);
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(toVar);
		mMessageFactory->addString(toCustom);
		mMessageFactory->addInt32(di);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint8(0);

		if(realSize % 2)
			mMessageFactory->addUint8(0);
	}

	mMessageFactory->addUint32(0);

	newMessage = mMessageFactory->EndMessage();

	uint32 loweredNameCrc			= 0;
	string loweredName;
	bool crcValid = false;

	// Get the source for this emote.
	if (srcObject->getType() == ObjType_Player)
	{
		const PlayerObject* const srcPlayer = dynamic_cast<const PlayerObject*>(srcObject);

		if (srcPlayer->isConnected())
		{
			loweredName = srcPlayer->getFirstName().getAnsi();
			loweredName.toLower();
			loweredNameCrc = loweredName.getCrc();
			crcValid = true;
		}
	}

	if (_checkPlayer(playerObject))
	{
		if (gWorldConfig->isTutorial())
		{
			// Update tutorial about "chatActive"
			playerObject->getTutorial()->tutorialResponse("chatActive");
		}

		PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(true);
		PlayerList::iterator it	= inRangeMembers.begin();
		Message* clonedMessage;

		while (it != inRangeMembers.end())
		{
			const PlayerObject* const player = (*it);

			// If player online, send emote.
			if (player->isConnected())
			{
				if ((crcValid) && (player->checkIgnoreList(loweredNameCrc)))
 				{
					// I am at recivers ignore list.
					// Don't send any message.
				}
				else
				{
					// clone our message
					mMessageFactory->StartMessage();
					mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
					clonedMessage = mMessageFactory->EndMessage();

					// replace the target id
					int8* data = clonedMessage->getData() + 12;
					*((uint64*)data) = player->getId();

					(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
				}
			}
			++it;
		}
	}

	mMessageFactory->DestroyMessage(newMessage);

return true;
}

//======================================================================================================================
//
// Spatial Emote
//

void MessageLib::sendSpatialEmote(CreatureObject* srcObject,uint16 emoteId,uint16 sendText,uint64 emoteTarget)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSpatialEmote);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);                    // unknown
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(emoteTarget);
	mMessageFactory->addUint16(emoteId);
	mMessageFactory->addUint8(0);					  // unknown
	mMessageFactory->addUint16(sendText);

	Message* newMessage = mMessageFactory->EndMessage();

	const PlayerObjectSet* const inRangePlayers	= srcObject->getKnownPlayers();
	PlayerObjectSet::const_iterator it	= inRangePlayers->begin();
	uint32 loweredNameCrc			= 0;
	string loweredName;
	Message* clonedMessage;
	bool crcValid = false;

	// Get the source for this emote.
	if(srcObject->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(srcObject);

		if(srcPlayer->isConnected())
		{
			loweredName = srcPlayer->getFirstName().getAnsi();
			loweredName.toLower();
			loweredNameCrc = loweredName.getCrc();
			crcValid = true;
		}
	}

	while(it != inRangePlayers->end())
	{
		const PlayerObject* const player = (*it);

		// If player online, send emote.
		if(player->isConnected())
		{
			if ((crcValid) && (player->checkIgnoreList(loweredNameCrc)))
 			{
				// I am at receivers ignore list.
				// Don't send any message.
			}
			else
			{
				// clone our message
				mMessageFactory->StartMessage();
				mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
				clonedMessage = mMessageFactory->EndMessage();

				// replace the target id
				int8* data = clonedMessage->getData() + 12;
				*((uint64*)data) = player->getId();

				(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
			}
		}
		++it;
	}

	// if we are a player, echo it back to ourself
	if(srcObject->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(srcObject);

		if(srcPlayer->isConnected())
		{
			(srcPlayer->getClient())->SendChannelAUnreliable(newMessage,srcPlayer->getAccountId(),CR_Client,5);
			return;
		}
	}

	mMessageFactory->DestroyMessage(newMessage);

}

//======================================================================================================================
//
// sit on chair/bank
//

void MessageLib::sendSitOnObject(CreatureObject* creatureObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opSitOnObject);
	mMessageFactory->addUint64(creatureObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(creatureObject->getParentId());
	mMessageFactory->addFloat(creatureObject->mPosition.x);
	mMessageFactory->addFloat(creatureObject->mPosition.y);
	mMessageFactory->addFloat(creatureObject->mPosition.z);

	_sendToInRange(mMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
//
// watch entertainer
//

void MessageLib::sendWatchEntertainer(PlayerObject* playerObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001b);
	mMessageFactory->addUint32(0x0000022B);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// flourish
//

void MessageLib::sendperformFlourish(PlayerObject* playerObject,uint32 flourish)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(27);
	mMessageFactory->addUint32(opEntertainerFlourish);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(flourish);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0);

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// animate a creature
//

void MessageLib::sendCreatureAnimation(CreatureObject* srcObject,string animation)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opSendAnimation);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(animation);

	_sendToInRange(mMessageFactory->EndMessage(),srcObject,5);
}

//======================================================================================================================
//
// animate a creature, used by tutorial
//

void MessageLib::sendCreatureAnimation(CreatureObject* srcObject,string animation, PlayerObject* player)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opSendAnimation);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(animation);

	_sendToInstancedPlayers(mMessageFactory->EndMessage(),5, player);
}
//======================================================================================================================
//
// posture update
//

void MessageLib::sendSelfPostureUpdate(PlayerObject* playerObject)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opPosture);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint8(playerObject->getPosture());
	mMessageFactory->addUint8(1);

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
//
// radial response
//

bool MessageLib::sendObjectMenuResponse(Object* object,PlayerObject* targetObject,uint8 responseNr)
{
	if(!(targetObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opObjectMenuResponse);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint64(targetObject->getId());

	RadialMenuPtr radialMenu = object->getRadialMenu();

	if(radialMenu != NULL)
	{
		RadialItemList* itemList = radialMenu->getItemList();
		uint32 elementCount = itemList->size();
		RadialItemList::iterator it = itemList->begin();

		mMessageFactory->addUint32(elementCount);

		while(it != itemList->end())
		{
			RadialMenuItem* item = (*it);
			string description = item->mExtendedDescription.getAnsi();

			mMessageFactory->addUint8(item->mIndex);
			mMessageFactory->addUint8(item->mParentItem);
			mMessageFactory->addUint8(item->mIdentifier);
			mMessageFactory->addUint8(item->mAction);

			if(description.getLength())
			{
				description.convert(BSTRType_Unicode16);
				mMessageFactory->addString(description);
			}
			else
				mMessageFactory->addUint32(0);

			++it;
		}
	}
	// no custom menu items
	else
	{
		mMessageFactory->addUint32(0);
	}

	mMessageFactory->addUint8(responseNr);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// empty radial response
//

bool MessageLib::sendEmptyObjectMenuResponse(uint64 requestedId,PlayerObject* targetObject,uint8 responseNr, MenuItemList mMenuItemList)
{
	if(!(targetObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opObjectMenuResponse);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(requestedId);
	mMessageFactory->addUint64(targetObject->getId());

	mMessageFactory->addUint32(mMenuItemList.size());  // list counter

	MenuItemList::iterator it = mMenuItemList.begin();
	while(it != mMenuItemList.end())
	{
		mMessageFactory->addUint8((*it)->sItem);// item index
		mMessageFactory->addUint8((*it)->sSubMenu);  // no submenu
		mMessageFactory->addUint8((*it)->sIdentifier);  // item identifier, 7 = examine
		mMessageFactory->addUint8((*it)->sOption); // send custom description, 1 = off,3 = on
		mMessageFactory->addUint32(0);// unicode custom description
		it++;
	}
	mMessageFactory->addUint8(responseNr);   // response nr

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// starting location list
//

bool MessageLib::sendStartingLocationList(PlayerObject* player, uint8 tatooine, uint8 corellia, uint8 talus, uint8 rori, uint8 naboo)
{
	gLogger->log(LogManager::DEBUG,"Sending Starting Location List\n");

	if(!(player->isConnected()))
	{
		return(false);
	}

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opStartingLocationList);
	mMessageFactory->addUint64(player->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint32(16);  // list counter

	//tatooine =====================================
	mMessageFactory->addString("bestine");
	mMessageFactory->addString("tatooine");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.tatooine.bestine");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(tatooine);

	mMessageFactory->addString("mos_espa");
	mMessageFactory->addString("tatooine");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.tatooine.mos_espa");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(tatooine);

	mMessageFactory->addString("mos_eisley");
	mMessageFactory->addString("tatooine");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.tatooine.mos_eisley");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(tatooine);

	mMessageFactory->addString("mos_entha");
	mMessageFactory->addString("tatooine");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.tatooine.mos_entha");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(tatooine);

	//corellia =======================================
	mMessageFactory->addString("coronet");
	mMessageFactory->addString("corellia");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.corellia.coronet");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(corellia);

	mMessageFactory->addString("tyrena");
	mMessageFactory->addString("corellia");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.corellia.tyrena");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(corellia);

	mMessageFactory->addString("kor_vella");
	mMessageFactory->addString("corellia");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.corellia.kor_vella");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(corellia);

	mMessageFactory->addString("doaba_guerfel");
	mMessageFactory->addString("corellia");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.corellia.doaba_guerfel");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(corellia);

	//talus =============================================
	mMessageFactory->addString("dearic");
	mMessageFactory->addString("talus");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.talus.dearic");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(talus);

	mMessageFactory->addString("nashal");
	mMessageFactory->addString("talus");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.talus.nashal");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(talus);

	//rori ==============================================
	mMessageFactory->addString("narmle");
	mMessageFactory->addString("rori");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.rori.narmle");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(rori);

	mMessageFactory->addString("restuss");
	mMessageFactory->addString("rori");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.rori.restuss");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(rori);

	// naboo ============================================
	mMessageFactory->addString("theed");
	mMessageFactory->addString("naboo");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.naboo.theed");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(naboo);

	mMessageFactory->addString("moenia");
	mMessageFactory->addString("naboo");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.naboo.moenia");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(naboo);

	mMessageFactory->addString("keren");
	mMessageFactory->addString("naboo");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.naboo.keren");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(naboo);

	mMessageFactory->addString("kaadara");
	mMessageFactory->addString("naboo");
	mMessageFactory->addFloat(0);
	mMessageFactory->addFloat(0);
	mMessageFactory->addString("");
	mMessageFactory->addString("styles.location.naboo.kaadara");
	mMessageFactory->addString("");
	mMessageFactory->addUint8(naboo);

	(player->getClient())->SendChannelA(mMessageFactory->EndMessage(),player->getAccountId(),CR_Client,5);

	return(true);
}


//======================================================================================================================
//
// combat action
//

void MessageLib::sendCombatAction(CreatureObject* attacker,Object* defender,uint32 animation,uint8 trail1,uint8 trail2,uint8 hit)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000001B);
	mMessageFactory->addUint32(opCombatAction);
	mMessageFactory->addUint64(attacker->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(animation);
	mMessageFactory->addUint64(attacker->getId());

	if(Weapon* weapon = dynamic_cast<Weapon*>(attacker->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hold_Left)))
	{
		mMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		mMessageFactory->addUint64(0);
	}

	mMessageFactory->addUint8(attacker->getPosture());
	mMessageFactory->addUint8(trail1);
	mMessageFactory->addUint8(trail2);

	if(defender)
	{
		mMessageFactory->addUint16(1);
		mMessageFactory->addUint64(defender->getId());

		if(CreatureObject* creature = dynamic_cast<CreatureObject*>(defender))
		{
			if (creature->getCreoGroup() == CreoGroup_AttackableObject)
			{
				mMessageFactory->addUint8(0);
			}
			else
			{
				mMessageFactory->addUint8(creature->getPosture());
			}
		}
		else
		{
			mMessageFactory->addUint8(0);
		}

		mMessageFactory->addUint8(hit);
		mMessageFactory->addUint8(0);
	}
	else
	{
		mMessageFactory->addUint16(0);
	}

	_sendToInRange(mMessageFactory->EndMessage(),attacker,5);
}

//======================================================================================================================
//
// draftslots response
//

bool MessageLib::sendDraftslotsResponse(DraftSchematic* schematic,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDraftSlotsQueryResponse);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(schematic->getId());

	mMessageFactory->addUint32(schematic->getComplexity());
	mMessageFactory->addUint32(schematic->getDataSize());
	mMessageFactory->addUint8(1);

	DraftSlots*	slots = schematic->getDraftSlots();

	mMessageFactory->addUint32(slots->size());

	DraftSlots::iterator it = slots->begin();

	while(it != slots->end())
	{
		DraftSlot* slot = (*it);

		mMessageFactory->addString(slot->getComponentFile());
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(slot->getComponentName());
		mMessageFactory->addUint8(slot->getOptional());
		mMessageFactory->addUint32(1);
		mMessageFactory->addString(slot->getComponentFile());
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(slot->getComponentName());
		mMessageFactory->addString(slot->getResourceName());

		if((slot->getType())== DST_IdentComponent)
		{
			mMessageFactory->addUint8(slot->getType());
			mMessageFactory->addUint32(slot->getNecessaryAmount());
			mMessageFactory->addUint16(0);
		}
		else
		{
			mMessageFactory->addUint8(slot->getType());
			mMessageFactory->addUint32(slot->getNecessaryAmount());
		}


		++it;
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// draftweights response
//

bool MessageLib::sendDraftWeightsResponse(DraftSchematic* schematic,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opResourceWeights);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(schematic->getId());

	// assembly list
	WeightsBatches* batches = schematic->getAssemblyWeights();
	WeightsBatches::iterator batchIt = batches->begin();

	mMessageFactory->addUint8(batches->size());

	while(batchIt != batches->end())
	{
		WeightsBatch* batch = (*batchIt);
		DraftWeights* batchWeights = batch->getWeights();
		DraftWeights::iterator weightsIt = batchWeights->begin();

		mMessageFactory->addUint8(batchWeights->size());

		while(weightsIt != batchWeights->end())
		{
			DraftWeight* weight = (*weightsIt);

			mMessageFactory->addUint8(weight->getData());

			++weightsIt;
		}
		++batchIt;
	}

	// experiment list
	batches = schematic->getExperimentWeights();
	batchIt = batches->begin();

	mMessageFactory->addUint8(batches->size());

	while(batchIt != batches->end())
	{
		WeightsBatch* batch = (*batchIt);
		DraftWeights* batchWeights = batch->getWeights();
		DraftWeights::iterator weightsIt = batchWeights->begin();

		mMessageFactory->addUint8(batchWeights->size());

		while(weightsIt != batchWeights->end())
		{
			DraftWeight* weight = (*weightsIt);

			mMessageFactory->addUint8(weight->getData());

			++weightsIt;
		}
		++batchIt;
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

// move (moving???)object in cell
//
//we need 0x0000000B to move players in elevators ... 0x00000053 might be specifically for static items ???
//evtly divide between object and movingobject ????
void MessageLib::sendDataTransformWithParent0B(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDataTransformWithParent);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	uint32 u = object->incDataTransformCounter();
	mMessageFactory->addUint32(u);

	mMessageFactory->addUint64(object->getParentId());
	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
//
// move object in world
//

void MessageLib::sendDataTransform0B(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDataTransform);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
//
// move object in world
//

void MessageLib::sendDataTransform053(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x00000053);
	mMessageFactory->addUint32(opDataTransform);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

void MessageLib::sendDataTransform071(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x00000071);
	mMessageFactory->addUint32(opDataTransform);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

void MessageLib::sendDataTransform023(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x00000023);
	mMessageFactory->addUint32(opDataTransform);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
//
// move object in cell
//

void MessageLib::sendDataTransformWithParent053(Object* object)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x00000053);
	mMessageFactory->addUint32(opDataTransformWithParent);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	uint32 u = object->incDataTransformCounter();
	mMessageFactory->addUint32(u);

	mMessageFactory->addUint64(object->getParentId());
	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(mMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
//
// move object in world, used with Tutorial
//

void MessageLib::sendDataTransform(Object* object, PlayerObject* player)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDataTransform);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);	//speed

	_sendToInstancedPlayersUnreliable(mMessageFactory->EndMessage(),5, player);//thats movement thats supposed to be fast !!!!!!!!!!!!!!!!!!!!!!!
}	//so unreliable needs to be true!!!!!

//======================================================================================================================
//
// move object in cell, used with Tutorial
//

void MessageLib::sendDataTransformWithParent(Object* object, PlayerObject* player)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDataTransformWithParent);
	mMessageFactory->addUint64(object->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(object->incDataTransformCounter());

	mMessageFactory->addUint64(object->getParentId());
	mMessageFactory->addFloat(object->mDirection.x);
	mMessageFactory->addFloat(object->mDirection.y);
	mMessageFactory->addFloat(object->mDirection.z);
	mMessageFactory->addFloat(object->mDirection.w);
	mMessageFactory->addFloat(object->mPosition.x);
	mMessageFactory->addFloat(object->mPosition.y);
	mMessageFactory->addFloat(object->mPosition.z);
	mMessageFactory->addUint32(0);

	_sendToInstancedPlayersUnreliable(mMessageFactory->EndMessage(),5, player);
}

//======================================================================================================================
//
// send biography
//

bool MessageLib::sendBiography(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opBiographyUpdate);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addString(targetObject->getBiography());

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(false);
}

//======================================================================================================================
//
// character match results
//

bool MessageLib::sendCharacterMatchResults(const PlayerList* const matchedPlayers, const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	PlayerList::const_iterator	playersIt	= matchedPlayers->begin();
	// PlayerObject*			player		= NULL;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opPlayersNearYou);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint32(matchedPlayers->size());

	while(playersIt != matchedPlayers->end())
	{
		const PlayerObject* const player = (*playersIt);

		mMessageFactory->addUint32(4);
		mMessageFactory->addUint32(player->getPlayerFlags());
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint32(0);

		string playerName = player->getFirstName().getAnsi();

		if(player->getLastName().getLength())
		{
			playerName << " ";
			playerName << player->getLastName().getAnsi();
		}

		playerName.convert(BSTRType_Unicode16);

		mMessageFactory->addString(playerName);
		mMessageFactory->addUint32(player->getRaceId());

		// only cities for now
		ObjectSet				regions;
		gWorldManager->getSI()->getObjectsInRange(player,&regions,ObjType_Region,1);

		ObjectSet::iterator	objIt = regions.begin();
		string				regionName;

		while(objIt != regions.end())
		{
			RegionObject* region = dynamic_cast<RegionObject*>(*objIt);

			if(region->getRegionType() == Region_City)
			{
				regionName = "@";
				regionName << region->getNameFile().getAnsi();
				regionName << ":";
				regionName << region->getRegionName().getAnsi();

				break;
			}

			++objIt;
		}

		mMessageFactory->addString(regionName);

		mMessageFactory->addString(BString(gWorldManager->getPlanetNameThis()));

		// guild
		mMessageFactory->addUint16(0);

		mMessageFactory->addString(player->getTitle());

		++playersIt;
	}

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// trade
//

bool MessageLib::sendSecureTrade(PlayerObject* targetPlayer,PlayerObject* srcObject,uint32 error)
{
	if(!(targetPlayer->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSecureTrade);
	mMessageFactory->addUint64(targetPlayer->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(error);

	mMessageFactory->addUint64(srcObject->getId());   // sender
	mMessageFactory->addUint64(targetPlayer->getId());// recipient												  //

	(targetPlayer->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// open npc dialog
//

bool MessageLib::sendStartNPCConversation(NPCObject* srcObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opStartNpcConversation);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint16(0);
	mMessageFactory->addUint8(0);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// stop a conversation
//

bool MessageLib::sendStopNPCConversation(NPCObject* srcObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opStopNpcConversation);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint32(0);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// update a conversations page text
//

bool MessageLib::sendNPCDialogMessage(ActiveConversation* av,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	ConversationPage*	currentPage = av->getCurrentPage();

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opNpcConversationMessage);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);

	// custom text
	if(currentPage->mCustomText.getLength())
	{
		mMessageFactory->addString(currentPage->mCustomText);
	}
	// templated text
	else
	{
		uint32	realSize = av->getTOStfFile().getLength() + av->getTOStfVariable().getLength() + currentPage->mStfFile.getLength() + currentPage->mStfVariable.getLength();

		mMessageFactory->addUint32(43 + ((uint32)ceil(((double)realSize) / 2.0)) + av->getTOCustom().getLength());
		mMessageFactory->addUint16(0);

		if(realSize % 2)
			mMessageFactory->addUint16(1);
		else
			mMessageFactory->addUint16(0);

		mMessageFactory->addUint8(1);
		mMessageFactory->addUint32(0xFFFFFFFF);
		mMessageFactory->addString(currentPage->mStfFile);
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(currentPage->mStfVariable);
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint64(0);
		mMessageFactory->addUint64(0);
		mMessageFactory->addUint64(0);
		mMessageFactory->addUint64(0);
		mMessageFactory->addUint64(0);
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(av->getTOStfFile());
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(av->getTOStfVariable());
		mMessageFactory->addString(av->getTOCustom());
		mMessageFactory->addUint32(av->getDI());
		mMessageFactory->addUint32(0);
		mMessageFactory->addUint8(0);

		if(realSize % 2)
			mMessageFactory->addUint8(0);
	}

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// conversation options
//

bool MessageLib::sendNPCDialogOptions(std::vector<ConversationOption*>* options,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	ConversationOption* option;
	uint32				realSize = 0;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opNpcConversationOptions);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint8(options->size());

	std::vector<ConversationOption*>::iterator it = options->begin();

	while(it != options->end())
	{
		option = (*it);

		if(option->mCustomText.getLength())
		{
			mMessageFactory->addString(option->mCustomText);
		}
		else
		{
			realSize = option->mStfFile.getLength() + option->mStfVariable.getLength();

			mMessageFactory->addUint32(43 + ((uint32)ceil(((double)realSize) / 2.0)));

			mMessageFactory->addUint16(0);

			if(realSize % 2)
				mMessageFactory->addUint16(1);
			else
				mMessageFactory->addUint16(0);

			mMessageFactory->addUint8(1);
			mMessageFactory->addUint32(0xFFFFFFFF);
			mMessageFactory->addString(option->mStfFile);
			mMessageFactory->addUint32(0);
			mMessageFactory->addString(option->mStfVariable);
			mMessageFactory->addUint32(0);
			mMessageFactory->addUint32(0);
			mMessageFactory->addUint32(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint64(0);
			mMessageFactory->addUint32(0);
			mMessageFactory->addUint32(0);
			mMessageFactory->addUint8(0);

			if(realSize % 2)
				mMessageFactory->addUint8(0);
		}
		++it;
	}

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// filtered draft schematics list, send when starting a crafting session
//

bool MessageLib::sendDraftSchematicsList(CraftingTool* tool,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	SchematicsIdList*			schemIdList = playerObject->getSchematicsIdList();
	SchematicsIdList*			filteredIdList = playerObject->getFilteredSchematicsIdList();
	SchematicsIdList::iterator	schemIt		= schemIdList->begin();
	DraftSchematic*				schematic;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opDraftSchematics);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(tool->getId());
	mMessageFactory->addUint64(0); // station ?

	// filter by tool / station properties
	uint32 toolGroupMask		= tool->getInternalAttribute<uint32>("craft_tool_typemask");

	uint32 availableComplexity	= tool->getInternalAttribute<uint32>("complexity"); // + stationComplexity
	if(playerObject->getNearestCraftingStation())
	{
		//TODO check for private stations!!
		availableComplexity = 25;
	}


	uint32 filteredCount		= 0;
	uint32 subCategory			= 0;

	while(schemIt != schemIdList->end())
	{
		schematic	= gSchematicManager->getSchematicBySlotId((*schemIt) >> 32);
		subCategory = schematic->getSubCategory();

		if(((toolGroupMask & subCategory) == subCategory) && availableComplexity >= schematic->getComplexity())
		{
			filteredCount++;
		}

		++schemIt;
	}

	// build and update the filtered list
	filteredIdList->clear();

	schemIt	= schemIdList->begin();

	mMessageFactory->addUint32(filteredCount);

	while(schemIt != schemIdList->end())
	{
		schematic = gSchematicManager->getSchematicBySlotId(*schemIt >> 32);
		subCategory = schematic->getSubCategory();

		if(((toolGroupMask & subCategory) == subCategory) && availableComplexity >= schematic->getComplexity())
		{
			filteredIdList->push_back(*schemIt);
			mMessageFactory->addUint64(*schemIt);
			mMessageFactory->addUint32(subCategory);
		}

		++schemIt;
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// shared net message
//

bool MessageLib::sendSharedNetworkMessage(PlayerObject* playerObject,uint32 unknown1,uint32 unknown2)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opSharedNetworkMessage);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(unknown1);
	mMessageFactory->addUint32(unknown2);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// manufacture slots
//

bool MessageLib::sendManufactureSlots(ManufacturingSchematic* manSchem,CraftingTool* tool,TangibleObject* item,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opManufactureSlots);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint64(tool->getId());
	mMessageFactory->addUint64(manSchem->getId());
	mMessageFactory->addUint64(item->getId());
	mMessageFactory->addUint32(1);
	mMessageFactory->addUint8(1);

	ManufactureSlots*	slots = manSchem->getManufactureSlots();

	mMessageFactory->addUint32(slots->size());

	ManufactureSlots::iterator it = slots->begin();

	while(it != slots->end())
	{
		DraftSlot* slot = (*it)->mDraftSlot;

		mMessageFactory->addString(slot->getComponentFile());
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(slot->getComponentName());
		mMessageFactory->addUint8(slot->getOptional());
		mMessageFactory->addUint32(1);
		mMessageFactory->addString(slot->getComponentFile());
		mMessageFactory->addUint32(0);
		mMessageFactory->addString(slot->getComponentName());
		mMessageFactory->addString(slot->getResourceName());

		if((slot->getType()) == DST_IdentComponent)
		{
			mMessageFactory->addUint8(slot->getType());
			mMessageFactory->addUint32(slot->getNecessaryAmount());
			mMessageFactory->addUint16(0);
		}
		else
		{
			mMessageFactory->addUint8(slot->getType());
			mMessageFactory->addUint32(slot->getNecessaryAmount());
		}

		++it;
	}

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// craft acks
//

bool MessageLib::sendCraftAcknowledge(uint32 ackType,uint32 errorId,uint8 counter,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opCraftAcknowledge);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(ackType);
	mMessageFactory->addUint32(errorId);
	mMessageFactory->addUint8(counter);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// craft experiment response
//

bool MessageLib::sendCraftExperimentResponse(uint32 ackType,uint32 resultId,uint8 counter,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opCraftExperimentResponse);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(ackType);
	mMessageFactory->addUint32(resultId);
	mMessageFactory->addUint8(counter);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// generic response
//

bool MessageLib::sendGenericIntResponse(uint32 value,uint8 counter,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opGenericIntResponse);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0x00000109);
	mMessageFactory->addUint32(value);
	mMessageFactory->addUint8(counter);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// combat spam
//

void MessageLib::sendCombatSpam(Object* attacker,Object* defender,int32 damage,string stfFile,string stfVar,Item* item,uint8 colorFlag,string customText)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opCombatSpam);
	mMessageFactory->addUint64(attacker->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint64(attacker->getId());
	mMessageFactory->addUint64(defender->getId());

	if(!item)
		mMessageFactory->addUint64(0);
	else
		mMessageFactory->addUint64(item->getId());

	mMessageFactory->addUint32(damage);

	mMessageFactory->addString(stfFile);
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(stfVar);

	mMessageFactory->addUint8(colorFlag);
	mMessageFactory->addString(customText);

	Message* newMessage = mMessageFactory->EndMessage();
	//this is fastpath
	_sendToInRangeUnreliable(newMessage,attacker,5,true);

		/*
	PlayerObjectSet* inRangePlayers	= attacker->getKnownPlayers();
	PlayerObjectSet::iterator it	= inRangePlayers->begin();

	Message* clonedMessage;

	while(it != inRangePlayers->end())
	{
		PlayerObject* player = (*it);

		if(player->isConnected())
		{
			mMessageFactory->StartMessage();
			mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
			clonedMessage = mMessageFactory->EndMessage();

			// replace the target id
			int8* data = clonedMessage->getData() + 12;
			*((uint64*)data) = player->getId();

			(player->getClient())->SendChannelA(clonedMessage,player->getAccountId(),CR_Client,5,false);
		}

		++it;
	}

	// if we are a player, echo it back to ourself
	if(attacker->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(attacker);

		if(srcPlayer->isConnected())
		{
			(srcPlayer->getClient())->SendChannelA(newMessage,srcPlayer->getAccountId(),CR_Client,5,false);
			return;
		}

	}

	mMessageFactory->DestroyMessage(newMessage);
	*/

}

//======================================================================================================================
//
// fly text
//

// We may have to use flytexts on more things than just Creatures...
// void MessageLib::sendFlyText(CreatureObject* srcCreature,string stfFile,string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display)
void MessageLib::sendFlyText(Object* srcCreature,string stfFile,string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opShowFlyText);
	mMessageFactory->addUint64(srcCreature->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint64(srcCreature->getId());
	mMessageFactory->addString(stfFile);
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(stfVar);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint8(red);
	mMessageFactory->addUint8(green);
	mMessageFactory->addUint8(blue);
	mMessageFactory->addUint8(display);

	Message* newMessage = mMessageFactory->EndMessage();

	const PlayerObjectSet* const inRangePlayers	= srcCreature->getKnownPlayers();
	PlayerObjectSet::const_iterator it	= inRangePlayers->begin();

	Message* clonedMessage;

	while(it != inRangePlayers->end())
	{
		const PlayerObject* const player = (*it);

		if(player->isConnected())
		{
			mMessageFactory->StartMessage();
			mMessageFactory->addData(newMessage->getData(),newMessage->getSize());
			clonedMessage = mMessageFactory->EndMessage();

			// replace the target id
			int8* data = clonedMessage->getData() + 12;
			*((uint64*)data) = player->getId();

			(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
		}

		++it;
	}

	// if we are a player, echo it back to ourself
	if(srcCreature->getType() == ObjType_Player)
	{
		PlayerObject* srcPlayer = dynamic_cast<PlayerObject*>(srcCreature);

		if(srcPlayer->isConnected())
		{
			(srcPlayer->getClient())->SendChannelAUnreliable(newMessage,srcPlayer->getAccountId(),CR_Client,5);
			return;
		}

	}

	mMessageFactory->DestroyMessage(newMessage);

}

//======================================================================================================================
//
// fly text, to be used by Tutorial or other instances
//

void MessageLib::sendFlyText(Object* srcCreature, PlayerObject* playerObject, string stfFile,string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opShowFlyText);
	mMessageFactory->addUint64(srcCreature->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint64(srcCreature->getId());
	mMessageFactory->addString(stfFile);
	mMessageFactory->addUint32(0);
	mMessageFactory->addString(stfVar);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint8(red);
	mMessageFactory->addUint8(green);
	mMessageFactory->addUint8(blue);
	mMessageFactory->addUint8(display);

	Message* message = mMessageFactory->EndMessage();

	PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(true);
	PlayerList::iterator player	= inRangeMembers.begin();
	Message* clonedMessage;

	while (player != inRangeMembers.end())
	{
		if ((*player)->isConnected())
		{
			// Clone the message.
			mMessageFactory->StartMessage();
			mMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = mMessageFactory->EndMessage();

			// replace the target id
			int8* data = clonedMessage->getData() + 12;
			*((uint64*)data) = (*player)->getId();

			((*player)->getClient())->SendChannelAUnreliable(clonedMessage,(*player)->getAccountId(),CR_Client,5);
		}
		++player;
	}
	mMessageFactory->DestroyMessage(message);

	// _sendToInstancedPlayers(mMessageFactory->EndMessage(),5, player);
}


//======================================================================================================================
//
// combat queue remove
//

bool MessageLib::sendCommandQueueRemove(uint32 sequence,float tickCounter,uint32 reply1,uint32 reply2,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opCommandQueueRemove);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(sequence);
	mMessageFactory->addFloat(tickCounter);
	mMessageFactory->addUint32(reply1);
	mMessageFactory->addUint32(reply2);

	(playerObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================

void MessageLib::sendImageDesignStartMessage(PlayerObject* srcObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return;

	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opImageDesignStartMessage);
	mMessageFactory->addUint64(targetObject->getId());
	mMessageFactory->addUint32(0);                    // unknown
	mMessageFactory->addUint64(srcObject->getId());
	mMessageFactory->addUint64(targetObject->getId());

	uint64 parentID = 0;

	if(targetObject->getParentId() != 0)
	{
		Object* object = gWorldManager->getObjectById(targetObject->getParentId());

		if(object)
		{
			parentID = object->getParentId();
		}

		Object* parentObject = gWorldManager->getObjectById(parentID);

		if(parentObject)
		{
			// string modelname = object->getModelString();
			if(strstr(parentObject->getModelString().getAnsi(),"salon") == NULL)
			{
				parentID = 0;
			}
		}
		else
		{
			parentID = 0;
		}
	}

	mMessageFactory->addUint64(parentID);
	mMessageFactory->addUint16(0);                    // unknown

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),targetObject->getAccountId(), CR_Client, 5);


	if(srcObject != targetObject && srcObject->isConnected())
	{
		mMessageFactory->StartMessage();
		mMessageFactory->addUint32(opObjControllerMessage);
		mMessageFactory->addUint32(0x0000000B);
		//needs to be 0x0000000B otherwise window wont open..
		mMessageFactory->addUint32(opImageDesignStartMessage);
		mMessageFactory->addUint64(srcObject->getId());
		mMessageFactory->addUint32(0);                    // unknown
		mMessageFactory->addUint64(srcObject->getId());
		mMessageFactory->addUint64(targetObject->getId());

		mMessageFactory->addUint64(parentID);
		mMessageFactory->addUint16(0);

		(srcObject->getClient())->SendChannelA(mMessageFactory->EndMessage(),srcObject->getAccountId(), CR_Client, 5);
	}
}

//======================================================================================================================

void MessageLib::sendIDChangeMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 customerAccept, uint8 designerCommit, uint8 flag3,uint32 smTimer, uint8 flagHair, uint64 parentId,string holoEmote)
{
	if(!(targetObject->isConnected()))
		return;

	// thats send in both directions
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opImageDesignChangeMessage);
	mMessageFactory->addUint64(targetObject->getId());//the object were manipulating
	mMessageFactory->addUint32(0);                    // unknown

	mMessageFactory->addUint64(srcObject->getId()); //the recipient
	mMessageFactory->addUint64(otherObject->getId());   //the manipulator

	mMessageFactory->addUint64(parentId);

	mMessageFactory->addUint8(flagHair); //flag

	if(flagHair)
		mMessageFactory->addString(hair);
	else
		mMessageFactory->addUint16(0);


	mMessageFactory->addUint16(0);
	//not a list/string when send to customer???

	//mMessageFactory->addUint64(1);        //
	mMessageFactory->addUint32(smTimer); //timer
	//in pub 14 only 4 minute timers for stat migration - bitmap timerlength set clientside


	mMessageFactory->addUint32(iDsession);
	mMessageFactory->addUint32(moneyDemanded);
	mMessageFactory->addUint32(moneyOffered);

	mMessageFactory->addUint8(designerCommit);        //flag ID accepted
	mMessageFactory->addUint32(customerAccept);        //flag customer accepted
	mMessageFactory->addUint8(flag3);       //flag stat migration
	mMessageFactory->addUint32(srcObject->getSkillModValue(SMod_markings));
	mMessageFactory->addUint32(srcObject->getSkillModValue(SMod_hair));
	mMessageFactory->addUint32(srcObject->getSkillModValue(SMod_body));
	mMessageFactory->addUint32(srcObject->getSkillModValue(SMod_face));

	//body options
	AttributesList* aList = targetObject->getIdAttributesList();
	AttributesList::iterator it = aList->begin();
	mMessageFactory->addUint32(aList->size());

	while(it != aList->end())
	{
		mMessageFactory->addString((*it).first);
		mMessageFactory->addFloat((*it).second);
		++it;
	}

	//body options
	ColorList* cList = targetObject->getIdColorList();
	ColorList::iterator cIt = cList->begin();
	mMessageFactory->addUint32(cList->size());

	while(cIt != cList->end())
	{
		mMessageFactory->addString((*cIt).first);
		mMessageFactory->addUint32((*cIt).second);
		++cIt;
	}

	mMessageFactory->addString(holoEmote);   //holoemote string
	mMessageFactory->addUint8(0);//xff);        //
	mMessageFactory->addUint8(0);//xff);        //
	mMessageFactory->addUint8(0);//xff);        //
	mMessageFactory->addUint8(0);//xff);        //

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void MessageLib::sendIDEndMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 unknown2, uint8 flag2, uint8 flag3,uint32 counter1)
{
	if(!(targetObject->isConnected()))
		return;

	//thats send in both directions
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opImageDesignStopMessage);
	mMessageFactory->addUint64(targetObject->getId());//the recipient
	mMessageFactory->addUint32(0);                    // unknown

	mMessageFactory->addUint64(otherObject->getId()); //the recipient
	mMessageFactory->addUint64(srcObject->getId());   //the manipulator
	mMessageFactory->addUint64(otherObject->getParentId()-1);

	if(hair.getLength() > 0)
		mMessageFactory->addUint8(0); //flag
	else
		mMessageFactory->addUint8(0); //flag

	mMessageFactory->addString(hair);

	mMessageFactory->addUint16(0);
	mMessageFactory->addUint32(counter1);
	mMessageFactory->addUint32(iDsession);
	mMessageFactory->addUint32(moneyDemanded);
	mMessageFactory->addUint32(moneyOffered);

	mMessageFactory->addUint8(flag2);
	mMessageFactory->addUint32(unknown2);
	mMessageFactory->addUint8(flag3);
	mMessageFactory->addUint32(srcObject->checkSkill(SMod_markings));
	mMessageFactory->addUint32(srcObject->checkSkill(SMod_hair));
	mMessageFactory->addUint32(srcObject->checkSkill(SMod_body));
	mMessageFactory->addUint32(srcObject->checkSkill(SMod_face));

	//body options
	AttributesList* aList = srcObject->getIdAttributesList();
	AttributesList::iterator it = aList->begin();
	mMessageFactory->addUint32(aList->size());

	while(it != aList->end())
	{
		mMessageFactory->addString((*it).first);
		mMessageFactory->addFloat((*it).second);
		++it;
	}

	//body options
	ColorList* cList = srcObject->getIdColorList();
	ColorList::iterator cIt = cList->begin();
	mMessageFactory->addUint32(cList->size());

	while(cIt != cList->end())
	{
		mMessageFactory->addString((*cIt).first);
		mMessageFactory->addUint32((*cIt).second);
		++cIt;
	}
	mMessageFactory->addUint16(77);
	mMessageFactory->addUint16(77);
	mMessageFactory->addUint16(77);
	mMessageFactory->addUint32(1);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void MessageLib::sendStatMigrationStartMessage(PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return;

	//thats send in both directions
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opStatMigrationStart);
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Health));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Strength));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Constitution));

	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Action));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Quickness));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Stamina));

	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Mind));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Focus));
	mMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Willpower));
	//remaining points need to be zero otherwise client messes up
	mMessageFactory->addUint32(0);

	(targetObject->getClient())->SendChannelA(mMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Buffs
//

void MessageLib::sendPlayerAddBuff(PlayerObject* playerObject, int32 CRC, float Duration)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opApplyBuff);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint32(CRC);
	mMessageFactory->addFloat(Duration);

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,1);
}
//======================================================================================================================
//
// Buffs
//

void MessageLib::sendPlayerRemoveBuff(PlayerObject* playerObject, int32 CRC)
{
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x0000000B);
	mMessageFactory->addUint32(opRemoveBuff);
	mMessageFactory->addUint64(playerObject->getId());
	mMessageFactory->addUint32(0);

	mMessageFactory->addUint32(CRC);

	_sendToInRange(mMessageFactory->EndMessage(),playerObject,1);
}

//======================================================================================================================

void MessageLib::sendSetWaypointActiveStatus(WaypointObject* waypointObject, bool active, PlayerObject* targetObject)
{
	if(!waypointObject || !targetObject) return;
	else if(!targetObject->isConnected()) return;

	Message* message;
	mMessageFactory->StartMessage();
	mMessageFactory->addUint32(opObjControllerMessage);
	mMessageFactory->addUint32(0x00000023);
	mMessageFactory->addUint32(opCommandQueueEnqueue);
	mMessageFactory->addUint64(targetObject->getPlayerObjId());
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(0);
	mMessageFactory->addUint32(opOCsetwaypointactivestatus);
	mMessageFactory->addUint64(waypointObject->getId());
	mMessageFactory->addString(active ? L"on" : L"off");

	message = mMessageFactory->EndMessage();

	targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return;
}
//======================================================================================================================
