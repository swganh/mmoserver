/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"
#include "LogManager/LogManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"
#include "Common/Message.h"
#include "Common/DispatchClient.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include <boost/lexical_cast.hpp>
#include "Common/atMacroString.h"
#include "ZoneServer/ActiveConversation.h"
#include "ZoneServer/Tutorial.h"
#include "ZoneServer/WorldConfig.h"

//======================================================================================================================
//
// Spatial Chat
// 

void MessageLib::sendSpatialChat(CreatureObject* const srcObject,string chatMsg,char chatElement[5][32])
{
	//TODO Is chat really unreliable ??

	uint64 chatElementTarget	= _atoi64(chatElement[0]);
	uint16 chatElementMood1		= atoi(chatElement[1]);
	uint16 chatElementMood2		= atoi(chatElement[2]);

	if(!chatElementMood2)
		chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);          
	gMessageFactory->addUint32(0x0000000B);          
	gMessageFactory->addUint32(opSpatialChat);         
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);                   
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(chatElementTarget);
	gMessageFactory->addString(chatMsg);			 
	gMessageFactory->addUint16(0x32);               
	gMessageFactory->addUint16(chatElementMood1);   
	gMessageFactory->addUint16(chatElementMood2);   
	gMessageFactory->addUint8(0);              
	gMessageFactory->addUint8(srcObject->getLanguage());              
	gMessageFactory->addUint64(0);                    // unknown

	newMessage = gMessageFactory->EndMessage();


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
				gMessageFactory->StartMessage();
				gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
				clonedMessage = gMessageFactory->EndMessage();

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
	
	gMessageFactory->DestroyMessage(newMessage);
	
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

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);          
	gMessageFactory->addUint32(0x0000000B);          
	gMessageFactory->addUint32(opSpatialChat);         
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);                   
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(chatElementTarget);
	gMessageFactory->addString(chatMsg);			 
	gMessageFactory->addUint16(0x32);               
	gMessageFactory->addUint16(chatElementMood1);   
	gMessageFactory->addUint16(chatElementMood2);   
	gMessageFactory->addUint8(0);              
	gMessageFactory->addUint8(srcObject->getLanguage());              
	gMessageFactory->addUint64(0);                    // unknown

	newMessage = gMessageFactory->EndMessage();

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
		gMessageFactory->DestroyMessage(newMessage);
	}
}
*/

void MessageLib::sendSpatialChat(const CreatureObject* const srcObject,string chatMsg,char chatElement[5][32], const PlayerObject* const playerObject) const
{
	uint64 chatElementTarget	= _atoi64(chatElement[0]);
	uint16 chatElementMood1		= atoi(chatElement[1]);
	uint16 chatElementMood2		= atoi(chatElement[2]);

	if(!chatElementMood2)
		chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);          
	gMessageFactory->addUint32(0x0000000B);          
	gMessageFactory->addUint32(opSpatialChat);         
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);                   
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(chatElementTarget);
	gMessageFactory->addString(chatMsg);			 
	gMessageFactory->addUint16(0x32);               
	gMessageFactory->addUint16(chatElementMood1);   
	gMessageFactory->addUint16(chatElementMood2);   
	gMessageFactory->addUint8(0);              
	gMessageFactory->addUint8(srcObject->getLanguage());              
	gMessageFactory->addUint64(0);                    // unknown

	newMessage = gMessageFactory->EndMessage();

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
					gMessageFactory->StartMessage();
					gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
					clonedMessage = gMessageFactory->EndMessage();

					// replace the target id
					int8* data = clonedMessage->getData() + 12;
					*((uint64*)data) = player->getId();

					(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
				}
			}
			++it;
		}
	}
	gMessageFactory->DestroyMessage(newMessage);
}

bool MessageLib::sendSpatialChat(const CreatureObject* const srcObject,const PlayerObject* const playerObject,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId) const
{
	uint16 chatElementMood2	= chatElementMood2 = srcObject->getMoodId();

	Message*	newMessage;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);          
	gMessageFactory->addUint32(0x0000000B);          
	gMessageFactory->addUint32(opSpatialChat);         
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);                   
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addString(L"");			 
	gMessageFactory->addUint16(0x32);               
	gMessageFactory->addUint16(0);   
	gMessageFactory->addUint16(chatElementMood2);   
	gMessageFactory->addUint8(0);              
	gMessageFactory->addUint8(srcObject->getLanguage());              
    
	// simple message
	if(customMessage.getLength())
	{
		gMessageFactory->addString(customMessage);				 
	}
	// templated message
	else
	{
		uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

		gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + ttCustom.getLength());

		if(realSize % 2)
			gMessageFactory->addUint16(1);
		else
			gMessageFactory->addUint16(0);

		gMessageFactory->addUint8(1);
		gMessageFactory->addUint32(0xFFFFFFFF);
		gMessageFactory->addString(mainFile);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(mainVar);
		gMessageFactory->addUint64(tuId);
		
		gMessageFactory->addUint16(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint16(0);

		gMessageFactory->addUint32(0);
		
		gMessageFactory->addUint64(ttId);
		gMessageFactory->addString(ttFile);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(ttVar);
		gMessageFactory->addString(ttCustom);
		gMessageFactory->addUint64(toId);
		gMessageFactory->addString(toFile);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(toVar);
		gMessageFactory->addString(toCustom);
		gMessageFactory->addInt32(di);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint8(0);

		if(realSize % 2)
			gMessageFactory->addUint8(0);
	}

	gMessageFactory->addUint32(0);

	newMessage = gMessageFactory->EndMessage();

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
					gMessageFactory->StartMessage();
					gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
					clonedMessage = gMessageFactory->EndMessage();

					// replace the target id
					int8* data = clonedMessage->getData() + 12;
					*((uint64*)data) = player->getId();

					(player->getClient())->SendChannelAUnreliable(clonedMessage,player->getAccountId(),CR_Client,5);
				}
			}
			++it;
		}
	}

	gMessageFactory->DestroyMessage(newMessage);

return true;
}

//======================================================================================================================
// 
// Spatial Emote
//

void MessageLib::sendSpatialEmote(CreatureObject* srcObject,uint16 emoteId,uint16 sendText,uint64 emoteTarget)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opSpatialEmote);           
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);                    // unknown
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(emoteTarget); 
	gMessageFactory->addUint16(emoteId);	
	gMessageFactory->addUint8(0);					  // unknown
	gMessageFactory->addUint16(sendText);

	Message* newMessage = gMessageFactory->EndMessage();

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
				gMessageFactory->StartMessage();
				gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
				clonedMessage = gMessageFactory->EndMessage();

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
	
	gMessageFactory->DestroyMessage(newMessage);
	
}

//======================================================================================================================
// 
// sit on chair/bank
//

void MessageLib::sendSitOnObject(CreatureObject* creatureObject)
{
	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);           
	gMessageFactory->addUint32(opSitOnObject);           
	gMessageFactory->addUint64(creatureObject->getId());
	gMessageFactory->addUint32(0);                    
	gMessageFactory->addUint64(creatureObject->getParentId());					
	gMessageFactory->addFloat(creatureObject->mPosition.mX);	
	gMessageFactory->addFloat(creatureObject->mPosition.mY);				 
	gMessageFactory->addFloat(creatureObject->mPosition.mZ);				 	

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5);
}

//======================================================================================================================
// 
// watch entertainer
//

void MessageLib::sendWatchEntertainer(PlayerObject* playerObject)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001b);           
	gMessageFactory->addUint32(0x0000022B);           
	gMessageFactory->addUint64(playerObject->getId()); 
	gMessageFactory->addUint32(0);           

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
// 
// flourish
//

void MessageLib::sendperformFlourish(PlayerObject* playerObject,uint32 flourish)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(27);           
	gMessageFactory->addUint32(opEntertainerFlourish);           
	gMessageFactory->addUint64(playerObject->getId()); 
	gMessageFactory->addUint32(0);           
	gMessageFactory->addUint32(flourish);           
	gMessageFactory->addUint32(0);           
	gMessageFactory->addUint32(0);           

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
// 
// animate a creature
//

void MessageLib::sendCreatureAnimation(CreatureObject* srcObject,string animation)
{
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);
	gMessageFactory->addUint32(opSendAnimation);
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(animation);

	_sendToInRange(gMessageFactory->EndMessage(),srcObject,5);
}

//======================================================================================================================
// 
// animate a creature, used by tutorial
//

void MessageLib::sendCreatureAnimation(CreatureObject* srcObject,string animation, PlayerObject* player)
{
	// gLogger->logMsg("MessageLib::sendCreatureAnimation: For tutorial");
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);
	gMessageFactory->addUint32(opSendAnimation);
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(animation);

	_sendToInstancedPlayers(gMessageFactory->EndMessage(),5, player);
}
//======================================================================================================================
// 
// posture update
//

void MessageLib::sendSelfPostureUpdate(PlayerObject* playerObject)
{
	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);           
	gMessageFactory->addUint32(opPosture);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);                    
	gMessageFactory->addUint8(playerObject->getPosture());					
	gMessageFactory->addUint8(1);	

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,5);
}

//======================================================================================================================
// 
// radial response
//

bool MessageLib::sendObjectMenuResponse(Object* object,PlayerObject* targetObject,uint8 responseNr)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opObjectMenuResponse);           
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);                    
	gMessageFactory->addUint64(object->getId());	
	gMessageFactory->addUint64(targetObject->getId());

	RadialMenuPtr radialMenu = object->getRadialMenu();

	if(radialMenu != NULL)
	{
		RadialItemList* itemList = radialMenu->getItemList();
		uint32 elementCount = itemList->size();
		RadialItemList::iterator it = itemList->begin();

		gMessageFactory->addUint32(elementCount);

		while(it != itemList->end())
		{
			RadialMenuItem* item = (*it);
			string description = item->mExtendedDescription.getAnsi();

			gMessageFactory->addUint8(item->mIndex);
			gMessageFactory->addUint8(item->mParentItem);
			gMessageFactory->addUint8(item->mIdentifier);
			gMessageFactory->addUint8(item->mAction);

			if(description.getLength())
			{
				description.convert(BSTRType_Unicode16);
				gMessageFactory->addString(description);
			}
			else
				gMessageFactory->addUint32(0);

			++it;
		}
	}
	// no custom menu items
	else
	{
		gMessageFactory->addUint32(0);  
	}

	gMessageFactory->addUint8(responseNr);  

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

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

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opObjectMenuResponse);           
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);                    
	gMessageFactory->addUint64(requestedId);	
	gMessageFactory->addUint64(targetObject->getId());

	gMessageFactory->addUint32(mMenuItemList.size());  // list counter
	uint8 index = 1;
	MenuItemList::iterator it = mMenuItemList.begin();
	while(it != mMenuItemList.end())
	{
		gMessageFactory->addUint8((*it)->sItem);// item index
		gMessageFactory->addUint8((*it)->sSubMenu);  // no submenu
		gMessageFactory->addUint8((*it)->sIdentifier);  // item identifier, 7 = examine
		gMessageFactory->addUint8((*it)->sOption); // send custom description, 1 = off,3 = on
		gMessageFactory->addUint32(0);// unicode custom description
		delete((*it));
		(*it) = NULL;
		it++;
	}
	mMenuItemList.clear();
	gMessageFactory->addUint8(responseNr);   // response nr

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
// 
// starting location list
//

bool MessageLib::sendStartingLocationList(PlayerObject* player)
{
	if(!(player->isConnected()))
	{
		return(false);
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);           
	gMessageFactory->addUint32(opStartingLocationList);           
	gMessageFactory->addUint64(player->getId());
	gMessageFactory->addUint32(0);           

	gMessageFactory->addUint32(16);  // list counter

	//tatooine =====================================
	gMessageFactory->addString("bestine");
	gMessageFactory->addString("tatooine");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.tatooine.bestine");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("mos_espa");
	gMessageFactory->addString("tatooine");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.tatooine.mos_espa");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("mos_eisley");
	gMessageFactory->addString("tatooine");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.tatooine.mos_eisley");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("mos_entha");
	gMessageFactory->addString("tatooine");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.tatooine.mos_entha");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	//corellia =======================================
	gMessageFactory->addString("coronet");
	gMessageFactory->addString("corellia");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.corellia.coronet");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("tyrena");
	gMessageFactory->addString("corellia");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.corellia.tyrena");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("kor_vella");
	gMessageFactory->addString("corellia");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.corellia.kor_vella");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("doaba_guerfel");
	gMessageFactory->addString("corellia");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.corellia.doaba_guerfel");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	//talus =============================================
	gMessageFactory->addString("dearic");
	gMessageFactory->addString("talus");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.talus.dearic");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("nashal");
	gMessageFactory->addString("talus");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.talus.nashal");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	//rori ==============================================
	gMessageFactory->addString("narmle");
	gMessageFactory->addString("rori");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.rori.narmle");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("restuss");
	gMessageFactory->addString("rori");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.rori.restuss");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	// naboo ============================================
	gMessageFactory->addString("theed");
	gMessageFactory->addString("naboo");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.naboo.theed");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("moenia");
	gMessageFactory->addString("naboo");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.naboo.moenia");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("keren");
	gMessageFactory->addString("naboo");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.naboo.keren");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	gMessageFactory->addString("kaadara");
	gMessageFactory->addString("naboo");
	gMessageFactory->addFloat(0);
	gMessageFactory->addFloat(0);
	gMessageFactory->addString("");
	gMessageFactory->addString("styles.location.naboo.kaadara");
	gMessageFactory->addString("");
	gMessageFactory->addUint8(1);

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(),player->getAccountId(),CR_Client,5);

	return(true);
}


//======================================================================================================================
// 
// combat action
//

void MessageLib::sendCombatAction(CreatureObject* attacker,Object* defender,uint32 animation,uint8 trail1,uint8 trail2,uint8 hit)
{
	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000001B);           
	gMessageFactory->addUint32(opCombatAction);           
	gMessageFactory->addUint64(attacker->getId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint32(animation);
	gMessageFactory->addUint64(attacker->getId());

	if(Weapon* weapon = dynamic_cast<Weapon*>(attacker->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon)))
	{
		gMessageFactory->addUint64(weapon->getId());
	}
	else
	{
		gMessageFactory->addUint64(0);  
	}

	gMessageFactory->addUint8(attacker->getPosture());
	gMessageFactory->addUint8(trail1);
	gMessageFactory->addUint8(trail2);

	if(defender)
	{
		gMessageFactory->addUint16(1);
		gMessageFactory->addUint64(defender->getId());

		if(CreatureObject* creature = dynamic_cast<CreatureObject*>(defender))
		{
			if (creature->getCreoGroup() == CreoGroup_AttackableObject)
			{
				gMessageFactory->addUint8(0);
			}
			else
			{
				gMessageFactory->addUint8(creature->getPosture());
			}
		}
		else
		{
			gMessageFactory->addUint8(0);
		}

		gMessageFactory->addUint8(hit);
		gMessageFactory->addUint8(0);
	}
	else
	{
		gMessageFactory->addUint16(0);
	}

	_sendToInRange(gMessageFactory->EndMessage(),attacker,5);
}

//======================================================================================================================
// 
// draftslots response
//

bool MessageLib::sendDraftslotsResponse(DraftSchematic* schematic,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opDraftSlotsQueryResponse);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint64(schematic->getId());

	gMessageFactory->addUint32(schematic->getComplexity());  
	gMessageFactory->addUint32(schematic->getDataSize());
	gMessageFactory->addUint8(1); 

	DraftSlots*	slots = schematic->getDraftSlots();

	gMessageFactory->addUint32(slots->size());

	DraftSlots::iterator it = slots->begin();

	while(it != slots->end())
	{
		DraftSlot* slot = (*it);

		gMessageFactory->addString(slot->getComponentFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(slot->getComponentName());
		gMessageFactory->addUint8(slot->getOptional()); 
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(slot->getComponentFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(slot->getComponentName());
		gMessageFactory->addString(slot->getResourceName());

		if((slot->getType())==2)
		{
			gMessageFactory->addUint8(2);
			gMessageFactory->addUint32(slot->getAmount());
			gMessageFactory->addUint16(0);
		}
		else
		{
			gMessageFactory->addUint8(slot->getType());
			gMessageFactory->addUint32(slot->getAmount());
		}


		++it;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(), CR_Client, 5);

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

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opResourceWeights);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint64(schematic->getId());

	// assembly list
	WeightsBatches* batches = schematic->getAssemblyWeights();
	WeightsBatches::iterator batchIt = batches->begin();

	gMessageFactory->addUint8(batches->size());

	while(batchIt != batches->end())
	{
		WeightsBatch* batch = (*batchIt);
		DraftWeights* batchWeights = batch->getWeights();
		DraftWeights::iterator weightsIt = batchWeights->begin();

		gMessageFactory->addUint8(batchWeights->size());

		while(weightsIt != batchWeights->end())
		{
			DraftWeight* weight = (*weightsIt);

			gMessageFactory->addUint8(weight->getData());

			++weightsIt;
		}
		++batchIt;
	}

	// experiment list
	batches = schematic->getExperimentWeights();
	batchIt = batches->begin();

	gMessageFactory->addUint8(batches->size());

	while(batchIt != batches->end())
	{
		WeightsBatch* batch = (*batchIt);
		DraftWeights* batchWeights = batch->getWeights();
		DraftWeights::iterator weightsIt = batchWeights->begin();

		gMessageFactory->addUint8(batchWeights->size());

		while(weightsIt != batchWeights->end())
		{
			DraftWeight* weight = (*weightsIt);

			gMessageFactory->addUint8(weight->getData());

			++weightsIt;
		}
		++batchIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
// 
// move object in world
//

void MessageLib::sendDataTransform(Object* object)
{
	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opDataTransform);           
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	gMessageFactory->addFloat(object->mDirection.mX);
	gMessageFactory->addFloat(object->mDirection.mY);
	gMessageFactory->addFloat(object->mDirection.mZ);
	gMessageFactory->addFloat(object->mDirection.mW);
	gMessageFactory->addFloat(object->mPosition.mX);
	gMessageFactory->addFloat(object->mPosition.mY);
	gMessageFactory->addFloat(object->mPosition.mZ);
	gMessageFactory->addUint32(0);

	_sendToInRangeUnreliable(gMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
// 
// move object in cell
//

void MessageLib::sendDataTransformWithParent(Object* object)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opDataTransformWithParent);           
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint64(object->getParentId());
	gMessageFactory->addFloat(object->mDirection.mX);
	gMessageFactory->addFloat(object->mDirection.mY);
	gMessageFactory->addFloat(object->mDirection.mZ);
	gMessageFactory->addFloat(object->mDirection.mW);
	gMessageFactory->addFloat(object->mPosition.mX);
	gMessageFactory->addFloat(object->mPosition.mY);
	gMessageFactory->addFloat(object->mPosition.mZ);
	gMessageFactory->addUint32(0);

	//_sendToInRange(gMessageFactory->EndMessage(),object,5);
	_sendToInRangeUnreliable(gMessageFactory->EndMessage(),object,5);
}

//======================================================================================================================
// 
// move object in world, used with Tutorial
//

void MessageLib::sendDataTransform(Object* object, PlayerObject* player)
{
	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opDataTransform);           
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	gMessageFactory->addFloat(object->mDirection.mX);
	gMessageFactory->addFloat(object->mDirection.mY);
	gMessageFactory->addFloat(object->mDirection.mZ);
	gMessageFactory->addFloat(object->mDirection.mW);
	gMessageFactory->addFloat(object->mPosition.mX);
	gMessageFactory->addFloat(object->mPosition.mY);
	gMessageFactory->addFloat(object->mPosition.mZ);
	gMessageFactory->addUint32(0);

	_sendToInstancedPlayersUnreliable(gMessageFactory->EndMessage(),5, player);//thats movement thats supposed to be fast !!!!!!!!!!!!!!!!!!!!!!!
}	//so unreliable needs to be true!!!!!

//======================================================================================================================
// 
// move object in cell, used with Tutorial
//

void MessageLib::sendDataTransformWithParent(Object* object, PlayerObject* player)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opDataTransformWithParent);           
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint64(object->getParentId());
	gMessageFactory->addFloat(object->mDirection.mX);
	gMessageFactory->addFloat(object->mDirection.mY);
	gMessageFactory->addFloat(object->mDirection.mZ);
	gMessageFactory->addFloat(object->mDirection.mW);
	gMessageFactory->addFloat(object->mPosition.mX);
	gMessageFactory->addFloat(object->mPosition.mY);
	gMessageFactory->addFloat(object->mPosition.mZ);
	gMessageFactory->addUint32(0);
	
	_sendToInstancedPlayersUnreliable(gMessageFactory->EndMessage(),5, player);
}

//======================================================================================================================
// 
// send biography
//

bool MessageLib::sendBiography(PlayerObject* playerObject,PlayerObject* targetObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opBiographyUpdate);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addString(targetObject->getBiography());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opPlayersNearYou);           
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint32(matchedPlayers->size());

	while(playersIt != matchedPlayers->end())
	{
		const PlayerObject* const player = (*playersIt);

		gMessageFactory->addUint32(4);
		gMessageFactory->addUint32(player->getPlayerFlags());
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint32(0);

		string playerName = player->getFirstName().getAnsi();

		if(player->getLastName().getLength())
		{
			playerName << " ";
			playerName << player->getLastName().getAnsi();
		}

		playerName.convert(BSTRType_Unicode16);

		gMessageFactory->addString(playerName);
		gMessageFactory->addUint32(player->getRaceId());

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

		gMessageFactory->addString(regionName);

		gMessageFactory->addString(BString(gWorldManager->getPlanetNameThis()));

		// guild
		gMessageFactory->addUint16(0);

		gMessageFactory->addString(player->getTitle());

		++playersIt;
	}

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);          
	gMessageFactory->addUint32(0x0000000B);          
	gMessageFactory->addUint32(opSecureTrade);     
	gMessageFactory->addUint64(targetPlayer->getId());
	gMessageFactory->addUint32(0);                   
	gMessageFactory->addUint32(error);          

	gMessageFactory->addUint64(srcObject->getId());   // sender
	gMessageFactory->addUint64(targetPlayer->getId());// recipient												  //  

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Client, 5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opStartNpcConversation);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint8(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opStopNpcConversation);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(srcObject->getId());
	gMessageFactory->addUint32(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opNpcConversationMessage);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);

	// custom text
	if(currentPage->mCustomText.getLength())
	{
		gMessageFactory->addString(currentPage->mCustomText);
	}
	// templated text
	else
	{
		uint32	realSize = av->getTOStfFile().getLength() + av->getTOStfVariable().getLength() + currentPage->mStfFile.getLength() + currentPage->mStfVariable.getLength();

		gMessageFactory->addUint32(43 + ((uint32)ceil(((double)realSize) / 2.0)) + av->getTOCustom().getLength());
		gMessageFactory->addUint16(0);

		if(realSize % 2)
			gMessageFactory->addUint16(1);
		else
			gMessageFactory->addUint16(0);

		gMessageFactory->addUint8(1);
		gMessageFactory->addUint32(0xFFFFFFFF);
		gMessageFactory->addString(currentPage->mStfFile);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(currentPage->mStfVariable);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint64(0);
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(av->getTOStfFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(av->getTOStfVariable());
		gMessageFactory->addString(av->getTOCustom());
		gMessageFactory->addUint32(av->getDI());
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint8(0);

		if(realSize % 2)
			gMessageFactory->addUint8(0);
	}

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opNpcConversationOptions);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint8(options->size()); 

	std::vector<ConversationOption*>::iterator it = options->begin();

	while(it != options->end())
	{
		option = (*it);

		if(option->mCustomText.getLength())
		{
			gMessageFactory->addString(option->mCustomText);
		}
		else
		{
			realSize = option->mStfFile.getLength() + option->mStfVariable.getLength();

			gMessageFactory->addUint32(43 + ((uint32)ceil(((double)realSize) / 2.0)));

			gMessageFactory->addUint16(0);

			if(realSize % 2)
				gMessageFactory->addUint16(1);
			else
				gMessageFactory->addUint16(0);

			gMessageFactory->addUint8(1);
			gMessageFactory->addUint32(0xFFFFFFFF);
			gMessageFactory->addString(option->mStfFile);
			gMessageFactory->addUint32(0);
			gMessageFactory->addString(option->mStfVariable);
			gMessageFactory->addUint32(0);
			gMessageFactory->addUint32(0);
			gMessageFactory->addUint32(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint64(0);
			gMessageFactory->addUint32(0);
			gMessageFactory->addUint32(0);
			gMessageFactory->addUint8(0);

			if(realSize % 2)
				gMessageFactory->addUint8(0);
		}
		++it;
	}

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opDraftSchematics);
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint64(tool->getId());
	gMessageFactory->addUint64(0); // station ?

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

	gMessageFactory->addUint32(filteredCount);

	while(schemIt != schemIdList->end())
	{
		schematic = gSchematicManager->getSchematicBySlotId(*schemIt >> 32);
		subCategory = schematic->getSubCategory();

		if(((toolGroupMask & subCategory) == subCategory) && availableComplexity >= schematic->getComplexity())
		{
			filteredIdList->push_back(*schemIt);
			gMessageFactory->addUint64(*schemIt);
			gMessageFactory->addUint32(subCategory);
		}

		++schemIt;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);
	gMessageFactory->addUint32(opSharedNetworkMessage);
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(unknown1);
	gMessageFactory->addUint32(unknown2);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

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

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opManufactureSlots);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint64(tool->getId());
	gMessageFactory->addUint64(manSchem->getId());  
	gMessageFactory->addUint64(item->getId());
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint8(1); 

	ManufactureSlots*	slots = manSchem->getManufactureSlots();

	gMessageFactory->addUint32(slots->size());

	ManufactureSlots::iterator it = slots->begin();

	while(it != slots->end())
	{
		DraftSlot* slot = (*it)->mDraftSlot;

		gMessageFactory->addString(slot->getComponentFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(slot->getComponentName());
		gMessageFactory->addUint8(slot->getOptional()); 
		gMessageFactory->addUint32(1);
		gMessageFactory->addString(slot->getComponentFile());
		gMessageFactory->addUint32(0);
		gMessageFactory->addString(slot->getComponentName());
		gMessageFactory->addString(slot->getResourceName());

		if((slot->getType()) == 2)
		{
			gMessageFactory->addUint8(2);
			gMessageFactory->addUint32(slot->getAmount());
			gMessageFactory->addUint16(0);
		}
		else
		{
			gMessageFactory->addUint8(slot->getType());
			gMessageFactory->addUint32(slot->getAmount());
		}

		++it;
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client,5);

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

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opCraftAcknowledge);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(ackType);  
	gMessageFactory->addUint32(errorId);
	gMessageFactory->addUint8(counter); 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

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

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opCraftExperimentResponse);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(ackType);  
	gMessageFactory->addUint32(resultId);
	gMessageFactory->addUint8(counter); 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

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

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opGenericIntResponse);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(0x00000109);  
	gMessageFactory->addUint32(value);
	gMessageFactory->addUint8(counter); 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
// 
// combat spam
//

void MessageLib::sendCombatSpam(Object* attacker,Object* defender,int32 damage,string stfFile,string stfVar,Item* item,uint8 colorFlag,string customText)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opCombatSpam);       
	gMessageFactory->addUint64(attacker->getId());
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint64(attacker->getId()); 
	gMessageFactory->addUint64(defender->getId());

	if(!item)
		gMessageFactory->addUint64(0); 
	else
		gMessageFactory->addUint64(item->getId());

	gMessageFactory->addUint32(damage);

	gMessageFactory->addString(stfFile);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(stfVar);

	gMessageFactory->addUint8(colorFlag);
	gMessageFactory->addString(customText);

	Message* newMessage = gMessageFactory->EndMessage();
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
			gMessageFactory->StartMessage();
			gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
			clonedMessage = gMessageFactory->EndMessage();

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
	
	gMessageFactory->DestroyMessage(newMessage);
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
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opShowFlyText); 
	gMessageFactory->addUint64(srcCreature->getId());
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint64(srcCreature->getId());
	gMessageFactory->addString(stfFile);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(stfVar);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(red);
	gMessageFactory->addUint8(green);
	gMessageFactory->addUint8(blue);
	gMessageFactory->addUint8(display);

	Message* newMessage = gMessageFactory->EndMessage();

	const PlayerObjectSet* const inRangePlayers	= srcCreature->getKnownPlayers();
	PlayerObjectSet::const_iterator it	= inRangePlayers->begin();

	Message* clonedMessage;

	while(it != inRangePlayers->end())
	{
		const PlayerObject* const player = (*it);

		if(player->isConnected())
		{
			gMessageFactory->StartMessage();
			gMessageFactory->addData(newMessage->getData(),newMessage->getSize());
			clonedMessage = gMessageFactory->EndMessage();

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
	
	gMessageFactory->DestroyMessage(newMessage);
	
}

//======================================================================================================================
// 
// fly text, to be used by Tutorial or other instances
//

void MessageLib::sendFlyText(Object* srcCreature, PlayerObject* playerObject, string stfFile,string stfVar,uint8 red,uint8 green,uint8 blue,uint8 display)
{
	gMessageFactory->StartMessage();               
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opShowFlyText); 
	gMessageFactory->addUint64(srcCreature->getId());
	gMessageFactory->addUint32(0);

	gMessageFactory->addUint64(srcCreature->getId());
	gMessageFactory->addString(stfFile);
	gMessageFactory->addUint32(0);
	gMessageFactory->addString(stfVar);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(red);
	gMessageFactory->addUint8(green);
	gMessageFactory->addUint8(blue);
	gMessageFactory->addUint8(display);

	Message* message = gMessageFactory->EndMessage();

	PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(true);
	PlayerList::iterator player	= inRangeMembers.begin();
	Message* clonedMessage;

	while (player != inRangeMembers.end())
	{
		if ((*player)->isConnected()) 
		{
			// Clone the message.
			gMessageFactory->StartMessage();
			gMessageFactory->addData(message->getData(),message->getSize());
			clonedMessage = gMessageFactory->EndMessage();

			// replace the target id
			int8* data = clonedMessage->getData() + 12;
			*((uint64*)data) = (*player)->getId();

			((*player)->getClient())->SendChannelAUnreliable(clonedMessage,(*player)->getAccountId(),CR_Client,5);
		}
		++player;
	}
	gMessageFactory->DestroyMessage(message);

	// _sendToInstancedPlayers(gMessageFactory->EndMessage(),5, player);
}


//======================================================================================================================
// 
// combat queue remove
//

bool MessageLib::sendCommandQueueRemove(uint32 sequence,float tickCounter,uint32 reply1,uint32 reply2,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opCommandQueueRemove);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(sequence);
	gMessageFactory->addFloat(tickCounter);
	gMessageFactory->addUint32(reply1); 
	gMessageFactory->addUint32(reply2); 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================

void MessageLib::sendImageDesignStartMessage(PlayerObject* srcObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opImageDesignStartMessage);           
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint32(0);                    // unknown
	gMessageFactory->addUint64(srcObject->getId());   
	gMessageFactory->addUint64(targetObject->getId()); 

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

	gMessageFactory->addUint64(parentID); 
	gMessageFactory->addUint16(0);                    // unknown
	
	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(), CR_Client, 5);
		
	
	if(srcObject != targetObject && srcObject->isConnected())
	{
		gMessageFactory->StartMessage();
		gMessageFactory->addUint32(opObjControllerMessage);  
		gMessageFactory->addUint32(0x0000000B);           
		//needs to be 0x0000000B otherwise window wont open..
		gMessageFactory->addUint32(opImageDesignStartMessage);           
		gMessageFactory->addUint64(srcObject->getId());
		gMessageFactory->addUint32(0);                    // unknown
		gMessageFactory->addUint64(srcObject->getId());   
		gMessageFactory->addUint64(targetObject->getId()); 

		gMessageFactory->addUint64(parentID); 
		gMessageFactory->addUint16(0); 

		(srcObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),srcObject->getAccountId(), CR_Client, 5);
	}
}

//======================================================================================================================

void MessageLib::sendIDChangeMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 customerAccept, uint8 designerCommit, uint8 flag3,uint32 smTimer, uint8 flagHair, uint64 parentId,string holoEmote)
{
	if(!(targetObject->isConnected()))
		return;

	// thats send in both directions
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opImageDesignChangeMessage);           
	gMessageFactory->addUint64(targetObject->getId());//the object were manipulating
	gMessageFactory->addUint32(0);                    // unknown
	
	gMessageFactory->addUint64(srcObject->getId()); //the recipient
	gMessageFactory->addUint64(otherObject->getId());   //the manipulator

	gMessageFactory->addUint64(parentId); 

	gMessageFactory->addUint8(flagHair); //flag

	if(flagHair)
		gMessageFactory->addString(hair); 
	else
		gMessageFactory->addUint16(0);
	
	
	gMessageFactory->addUint16(0);        
	//not a list/string when send to customer???

	//gMessageFactory->addUint64(1);        //
	gMessageFactory->addUint32(smTimer); //timer
	//in pub 14 only 4 minute timers for stat migration - bitmap timerlength set clientside
	
	
	gMessageFactory->addUint32(iDsession);
	gMessageFactory->addUint32(moneyDemanded); 
	gMessageFactory->addUint32(moneyOffered);        

	gMessageFactory->addUint8(designerCommit);        //flag ID accepted
	gMessageFactory->addUint32(customerAccept);        //flag customer accepted
	gMessageFactory->addUint8(flag3);       //flag stat migration 
	gMessageFactory->addUint32(srcObject->getSkillModValue(SMod_markings));        
	gMessageFactory->addUint32(srcObject->getSkillModValue(SMod_hair));        
	gMessageFactory->addUint32(srcObject->getSkillModValue(SMod_body));        
	gMessageFactory->addUint32(srcObject->getSkillModValue(SMod_face));        
		
	//body options
	AttributesList* aList = targetObject->getIdAttributesList();
	AttributesList::iterator it = aList->begin();
	gMessageFactory->addUint32(aList->size());

	while(it != aList->end())
	{
		gMessageFactory->addString((*it).first); 
		gMessageFactory->addFloat((*it).second);        
		++it;
	}

	//body options
	ColorList* cList = targetObject->getIdColorList();
	ColorList::iterator cIt = cList->begin();
	gMessageFactory->addUint32(cList->size());

	while(cIt != cList->end())
	{
		gMessageFactory->addString((*cIt).first); 
		gMessageFactory->addUint32((*cIt).second);         
		++cIt;
	}

	gMessageFactory->addString(holoEmote);   //holoemote string
	gMessageFactory->addUint8(0);//xff);        //
	gMessageFactory->addUint8(0);//xff);        //
	gMessageFactory->addUint8(0);//xff);        //
	gMessageFactory->addUint8(0);//xff);        //
	
	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void MessageLib::sendIDEndMessage(PlayerObject* targetObject,PlayerObject* srcObject,PlayerObject* otherObject, string hair, uint32 iDsession,uint32 moneyOffered, uint32 moneyDemanded,uint32 unknown2, uint8 flag2, uint8 flag3,uint32 counter1)
{
	if(!(targetObject->isConnected()))
		return;

	//thats send in both directions
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opImageDesignStopMessage);           
	gMessageFactory->addUint64(targetObject->getId());//the object were manipulating
	gMessageFactory->addUint32(0);                    // unknown
	
	gMessageFactory->addUint64(srcObject->getId()); //the recipient
	gMessageFactory->addUint64(otherObject->getId());   //the manipulator
	gMessageFactory->addUint64(otherObject->getParentId()-1); 

	if(hair.getLength() > 0)
		gMessageFactory->addUint8(0); //flag
	else
		gMessageFactory->addUint8(0); //flag

	gMessageFactory->addString(hair); 
	
	gMessageFactory->addUint16(0);        
	gMessageFactory->addUint32(counter1);        
	gMessageFactory->addUint32(iDsession);
	gMessageFactory->addUint32(moneyDemanded); 
	gMessageFactory->addUint32(moneyOffered);        

	gMessageFactory->addUint8(flag2);        
	gMessageFactory->addUint32(unknown2);        
	gMessageFactory->addUint8(flag3);        
	gMessageFactory->addUint32(srcObject->checkSkill(SMod_markings));        
	gMessageFactory->addUint32(srcObject->checkSkill(SMod_hair));        
	gMessageFactory->addUint32(srcObject->checkSkill(SMod_body));        
	gMessageFactory->addUint32(srcObject->checkSkill(SMod_face));        
		
	//body options
	AttributesList* aList = srcObject->getIdAttributesList();
	AttributesList::iterator it = aList->begin();
	gMessageFactory->addUint32(aList->size());

	while(it != aList->end())
	{
		gMessageFactory->addString((*it).first); 
		gMessageFactory->addFloat((*it).second);        
		++it;
	}

	//body options
	ColorList* cList = srcObject->getIdColorList();
	ColorList::iterator cIt = cList->begin();
	gMessageFactory->addUint32(cList->size());

	while(cIt != cList->end())
	{
		gMessageFactory->addString((*cIt).first); 
		gMessageFactory->addUint32((*cIt).second);         
		++cIt;
	}
	gMessageFactory->addUint16(77);   
	gMessageFactory->addUint16(77);   
	gMessageFactory->addUint16(77);   
	gMessageFactory->addUint32(1);   
	
	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================

void MessageLib::sendStatMigrationStartMessage(PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return;

	//thats send in both directions
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opStatMigrationStart);  
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Health));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Strength));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Constitution));           

	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Action));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Quickness));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Stamina));           

	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Mind));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Focus));           
	gMessageFactory->addUint32(targetObject->getHam()->getTargetStatValue(HamBar_Willpower));            
	//remaining points need to be zero otherwise client messes up
	gMessageFactory->addUint32(0);           
	
	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Buffs
//

void MessageLib::sendPlayerAddBuff(PlayerObject* playerObject, int32 CRC, float Duration)
{
	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opApplyBuff);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);  
	
	gMessageFactory->addUint32(CRC);
	gMessageFactory->addFloat(Duration);

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,1);
}
//======================================================================================================================
//
// Buffs
//

void MessageLib::sendPlayerRemoveBuff(PlayerObject* playerObject, int32 CRC)
{
	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x0000000B);           
	gMessageFactory->addUint32(opRemoveBuff);           
	gMessageFactory->addUint64(playerObject->getId());
	gMessageFactory->addUint32(0);  
	
	gMessageFactory->addUint32(CRC);

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,1);
}

//======================================================================================================================

void MessageLib::sendSetWaypointActiveStatus(WaypointObject* waypointObject, bool active, PlayerObject* targetObject)
{
	if(!waypointObject || !targetObject) return;
	else if(!targetObject->isConnected()) return;

	Message* message;
	gMessageFactory->StartMessage();          
	gMessageFactory->addUint32(opObjControllerMessage);  
	gMessageFactory->addUint32(0x00000023);           
	gMessageFactory->addUint32(opCommandQueueEnqueue);           
	gMessageFactory->addUint64(targetObject->getPlayerObjId());
	gMessageFactory->addUint32(0);  
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint32(opOCsetwaypointactivestatus);
	gMessageFactory->addUint64(waypointObject->getId());
	gMessageFactory->addString(active ? L"on" : L"off");

	message = gMessageFactory->EndMessage();

	targetObject->getClient()->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

return;
}
//======================================================================================================================
