/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib.h"

#include "ZoneServer/Bank.h"
#include "ZoneServer/CellObject.h"
#include "ZoneServer/CharSheetManager.h"
#include "ZoneServer/Conversation.h"
#include "ZoneServer/CraftingTool.h"
#include "ZoneServer/CurrentResource.h"
#include "ZoneServer/ManufacturingSchematic.h"
#include "ZoneServer/NPCObject.h"
#include "ZoneServer/ObjectControllerOpcodes.h"
#include "ZoneServer/ObjectFactory.h"
#include "ZoneServer/PlayerObject.h"
#include "ZoneServer/ResourceCollectionManager.h"
#include "ZoneServer/TravelTerminal.h"
#include "ZoneServer/UIOpcodes.h"
#include "ZoneServer/Wearable.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"

#include "LogManager/LogManager.h"

#include "Common/atMacroString.h"
#include "Common/DispatchClient.h"
#include "Common/Message.h"
#include "Common/MessageDispatch.h"
#include "Common/MessageFactory.h"
#include "Common/MessageOpcodes.h"

#include <boost/lexical_cast.hpp>


//======================================================================================================================
//
// create function, used for all objects
//

bool MessageLib::sendCreateObjectByCRC(Object* object,const PlayerObject* const targetObject,bool player) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*				message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneCreateObjectByCrc); 

	if(!player)
		gMessageFactory->addUint64(object->getId());
	else
		gMessageFactory->addUint64(dynamic_cast<PlayerObject*>(object)->getPlayerObjId());

	// direction
	gMessageFactory->addFloat(object->mDirection.mX);
	gMessageFactory->addFloat(object->mDirection.mY);
	gMessageFactory->addFloat(object->mDirection.mZ);
	gMessageFactory->addFloat(object->mDirection.mW);

	// position
	gMessageFactory->addFloat(object->mPosition.mX);     
	gMessageFactory->addFloat(object->mPosition.mY);       
	gMessageFactory->addFloat(object->mPosition.mZ);     

	if(!player)
		gMessageFactory->addUint32(object->getModelString().getCrc());
	else
		gMessageFactory->addUint32(0x619bae21); // shared_player.iff

	gMessageFactory->addUint8(0);         
	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// End Baselines
// 

bool MessageLib::sendEndBaselines(uint64 objectId,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneEndBaselines);   
	gMessageFactory->addUint64(objectId);   

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 2);

	return(true);
}

//======================================================================================================================
//
// Scene Destroy Object
// 
// 

bool MessageLib::sendDestroyObject(uint64 objectId, PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneDestroyObject);   
	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint8(0);

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 3);

	return(true);
}

bool MessageLib::sendDestroyObject(uint64 objectId, CreatureObject* owner)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneDestroyObject);   
	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint8(0);

	_sendToInRange(gMessageFactory->EndMessage(), owner, 3, false);
	return(true);
}


// What is the use of "owner"? The info is send to the know Objects....
bool MessageLib::sendDestroyObject_InRange(uint64 objectId, PlayerObject* const owner, bool self)
{
	if(!(owner->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneDestroyObject);   
	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint8(0);

	//message = gMessageFactory->EndMessage();

	//(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 3, false);

	_sendToInRange(gMessageFactory->EndMessage(), owner, 3,self);
	return(true);
}

//==============================================================================================================
//
// this deletes an object from the client
//
bool MessageLib::sendDestroyObject_InRangeofObject(Object* object)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opSceneDestroyObject);   
	gMessageFactory->addUint64(object->getId());  
	gMessageFactory->addUint8(0);

	//message = gMessageFactory->EndMessage();

	//(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 3, false);

	_sendToInRange(gMessageFactory->EndMessage(), object, 3,false);
	return(true);
}

//======================================================================================================================
//
// updates an object parent<->child relationship
//

bool MessageLib::sendContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,const PlayerObject* const targetObject) const
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateContainmentMessage);  

	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint64(parentId);
	gMessageFactory->addUint32(linkType);				

	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 4);

	return(true);
}

//same with broadcast to in Range
bool MessageLib::sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateContainmentMessage);  

	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint64(parentId);
	gMessageFactory->addUint32(linkType);				

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,5);
	//(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 4, false);

	return(true);
}

bool MessageLib::sendContainmentMessage_InRange(uint64 objectId,uint64 parentId,uint32 linkType,CreatureObject* targetObject)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateContainmentMessage);  

	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint64(parentId);
	gMessageFactory->addUint32(linkType);				

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,5);
	return(true);
}



//======================================================================================================================
//
// Heartbeat, simple keep alive
//

bool MessageLib::sendHeartBeat(DispatchClient* client)
{
	if(!client)
		return(false);

	Message* newMessage;
	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opHeartBeat); 
	newMessage = gMessageFactory->EndMessage();

	client->SendChannelAUnreliable(newMessage, client->getAccountId(), CR_Client, 1);

	return(true);
}


//======================================================================================================================
//
// opened container
//

bool MessageLib::sendOpenedContainer(uint64 objectId, PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	Message*		message;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opOpenedContainer);  
	gMessageFactory->addUint32(0xFFFFFFFF);  
	gMessageFactory->addUint64(objectId);				
	gMessageFactory->addUint16(0);
	gMessageFactory->addUint8(0);
	message = gMessageFactory->EndMessage();

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 2);

	return(true);
}

//======================================================================================================================
//
// world position update
//

void MessageLib::sendUpdateTransformMessage(MovingObject* object)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateTransformMessage);          
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mX * 4.0f + 0.5f));     
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mY * 4.0f + 0.5f));      
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mZ * 4.0f + 0.5f));      
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                
	gMessageFactory->addUint8(static_cast<uint8>(object->mDirection.getAnglesToSend())); 

	_sendToInRangeUnreliable(gMessageFactory->EndMessage(),object,8,true);


}

//======================================================================================================================
//
// cell position update
//

void MessageLib::sendUpdateTransformMessageWithParent(MovingObject* object)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateTransformMessageWithParent);   
	gMessageFactory->addUint64(object->getParentId());
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mX * 8.0f + 0.5f));     
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mY * 8.0f + 0.5f));    
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mZ * 8.0f + 0.5f));    
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                       
	gMessageFactory->addUint8(static_cast<uint8>(object->mDirection.getAnglesToSend()));          
	_sendToInRangeUnreliable(gMessageFactory->EndMessage(),object,8,false);		
}

//======================================================================================================================
//
// world position update, to be used with Tutorial
//

void MessageLib::sendUpdateTransformMessage(MovingObject* object, PlayerObject* player)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateTransformMessage);          
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mX * 4.0f + 0.5f));     
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mY * 4.0f + 0.5f));      
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mZ * 4.0f + 0.5f));      
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                
	gMessageFactory->addUint8(static_cast<uint8>(object->mDirection.getAnglesToSend()));    

	_sendToInstancedPlayersUnreliable(gMessageFactory->EndMessage(), 8, player);
}

//======================================================================================================================
//
// cell position update, to be used with Tutorial
//

void MessageLib::sendUpdateTransformMessageWithParent(MovingObject* object, PlayerObject* player)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateTransformMessageWithParent);   
	gMessageFactory->addUint64(object->getParentId());
	gMessageFactory->addUint64(object->getId());
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mX * 8.0f + 0.5f));     
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mY * 8.0f + 0.5f));    
	gMessageFactory->addUint16(static_cast<uint16>(object->mPosition.mZ * 8.0f + 0.5f));    
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                       
	gMessageFactory->addUint8(static_cast<uint8>(object->mDirection.getAnglesToSend()));  

	//Movement is THE fastpathpacket!!!
	_sendToInstancedPlayersUnreliable(gMessageFactory->EndMessage(), 8, player);
}

//======================================================================================================================
//
// TODO: figure out unknown values
//

bool MessageLib::sendChatServerStatus(uint8 unk1,uint8 unk2,DispatchClient* client)
{
	if(!client)
	{
		return(false);
	}

	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opChatServerStatus);
	gMessageFactory->addUint8(unk1);             
	gMessageFactory->addUint8(unk2);

	client->SendChannelA(gMessageFactory->EndMessage(),client->getAccountId(),CR_Client,2);

	return(true);
}

//======================================================================================================================
//
// parameters, unknown
//

bool MessageLib::sendParameters(uint32 parameters,DispatchClient* client)
{
	if(!client)
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opParametersMessage);   
	gMessageFactory->addUint32(parameters);   

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 2);

	return(true);
}

//=======================================================================================================================
//
// start scene
//

bool MessageLib::sendStartScene(uint64 zoneId,PlayerObject* player)
{
	if(!(player->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opCmdStartScene);   
	gMessageFactory->addUint8(0);             
	gMessageFactory->addUint64(player->getId());           

	string mapName = gWorldManager->getTrnFileThis();
	gMessageFactory->addString(mapName);

	gMessageFactory->addFloat(player->mPosition.mX);     
	gMessageFactory->addFloat(player->mPosition.mY);    
	gMessageFactory->addFloat(player->mPosition.mZ);   

	gMessageFactory->addString(player->getModelString());
	gMessageFactory->addUint64(zoneId);

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 9);

	return(true);
}

//=======================================================================================================================
//
// planet time update
//

bool MessageLib::sendServerTime(uint64 time,DispatchClient* client)
{
	if(!client)
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opServerTimeMessage);  
	gMessageFactory->addUint64(time);

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 2);

	return(true);
}

//=======================================================================================================================
//
// scene ready acknowledge
//

bool MessageLib::sendSceneReady(DispatchClient* client)
{
	if(!client)
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opCmdSceneReady);  

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 1);

	return(true);
}

//=======================================================================================================================
//
// scene ready acknowledge to chatseerver
//

void MessageLib::sendSceneReadyToChat(DispatchClient* client)
{
	if (client)
	{
		gMessageFactory->StartMessage(); 
		gMessageFactory->addUint32(opChatNotifySceneReady);  

		client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Chat, 4);
	}
}
//=======================================================================================================================
//
// open the ticket terminal ui
//

bool MessageLib::sendEnterTicketPurchaseModeMessage(TravelTerminal* terminal,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	string planet = gWorldManager->getPlanetNameThis();

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opEnterTicketPurchaseModeMessage);  
	gMessageFactory->addString(planet);
	gMessageFactory->addString(terminal->getPosDescriptor());
	gMessageFactory->addUint8(0); // unknown

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4);

	return(true);
}

//=======================================================================================================================
//
// system message
//

bool MessageLib::sendSystemMessageInRange(PlayerObject* playerObject,bool toSelf, string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId,string tuFile,string tuVar,string tuCustom )
{
		if(!playerObject)
		return(false);

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	// simple message
	if(customMessage.getLength())
	{
		gMessageFactory->addString(customMessage);
		gMessageFactory->addUint32(0);				 
	}
	// templated message
	else
	{ 
		gMessageFactory->addUint32(0);				 

		// WRONG!
		// The real size is the size of ALL parameters.
		// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();
		uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength() + tuFile.getLength() + tuVar.getLength();

		gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + ttCustom.getLength() + tuCustom.getLength());

		if(realSize % 2)
			gMessageFactory->addUint16(1);
		else
			gMessageFactory->addUint16(0);

		gMessageFactory->addUint8(1);
		gMessageFactory->addUint32(0xFFFFFFFF);

		//main message		
		gMessageFactory->addString(mainFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(mainVar);

		//object 1
		gMessageFactory->addUint64(tuId);
		gMessageFactory->addString(tuFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tuVar);
		gMessageFactory->addString(tuCustom);

		//object 2		
		gMessageFactory->addUint64(ttId);  //object id2
		gMessageFactory->addString(ttFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(ttVar);
		gMessageFactory->addString(ttCustom);

		//object 3
		gMessageFactory->addUint64(toId);
		gMessageFactory->addString(toFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(toVar);
		gMessageFactory->addString(toCustom);

		gMessageFactory->addInt32(di);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint8(0);

		if(realSize % 2)
			gMessageFactory->addUint8(0);
	}

	_sendToInRange(gMessageFactory->EndMessage(),playerObject,8,toSelf);		

	return(true);
}

//=======================================================================================================================
//
// system message
//

bool MessageLib::sendSystemMessage(PlayerObject* playerObject,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId,string tuFile,string tuVar,string tuCustom )
{
	if(!playerObject)
		return(false);

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	// simple message
	if(customMessage.getLength())
	{
		gMessageFactory->addString(customMessage);
		gMessageFactory->addUint32(0);				 
	}
	// templated message
	else
	{ 
		gMessageFactory->addUint32(0);				 

		// WRONG!
		// The real size is the size of ALL parameters.
		// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();
		uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength() + tuFile.getLength() + tuVar.getLength();

		gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + ttCustom.getLength() + tuCustom.getLength());

		if(realSize % 2)
			gMessageFactory->addUint16(1);
		else
			gMessageFactory->addUint16(0);

		gMessageFactory->addUint8(1);
		gMessageFactory->addUint32(0xFFFFFFFF);

		//main message		
		gMessageFactory->addString(mainFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(mainVar);

		//object 1
		gMessageFactory->addUint64(tuId);
		gMessageFactory->addString(tuFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tuVar);
		gMessageFactory->addString(tuCustom.getUnicode16());

		//object 2		
		gMessageFactory->addUint64(ttId);  //object id2
		gMessageFactory->addString(ttFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(ttVar);
		gMessageFactory->addString(ttCustom.getUnicode16());

		//object 3
		gMessageFactory->addUint64(toId);
		gMessageFactory->addString(toFile);
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(toVar);
		gMessageFactory->addString(toCustom.getUnicode16());

		gMessageFactory->addInt32(di);
		gMessageFactory->addUint32(0);
		gMessageFactory->addUint8(0);

		if(realSize % 2)
			gMessageFactory->addUint8(0);
	}

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
// system message
//

bool MessageLib::sendMacroSystemMessage(PlayerObject* playerObject,string message,string macro)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);
	gMessageFactory->addString(message);
	gMessageFactory->addString(macro);				 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// system message,
//
// Option: Can be directed to chat only.

bool MessageLib::sendSystemMessage(PlayerObject* playerObject, string message, bool chatOnly)
{
	if ((!playerObject) || (!(playerObject->isConnected())))
	{
		return(false);
	}

	if (message.getLength())
	{
		// User requested to send no data, and we managed to accomplish that.
		// return true;
	}

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	if (chatOnly)
	{
		gMessageFactory->addUint8(2);
	}
	else
	{
		gMessageFactory->addUint8(0);
	}
	gMessageFactory->addString(message);
	gMessageFactory->addUint32(0);				 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
	return(true);
}


//======================================================================================================================
//
// error message
//

bool MessageLib::sendErrorMessage(PlayerObject* playerObject,string errType,string errMsg,uint8 fatal)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();        
	gMessageFactory->addUint32(opErrorMessage);  
	gMessageFactory->addString(errType);
	gMessageFactory->addString(errMsg);
	gMessageFactory->addUint8(fatal);				 

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 3);

	return(true);
}

//======================================================================================================================
//
// weather update
//

void MessageLib::sendWeatherUpdate(Anh_Math::Vector3 cloudVec,uint32 weatherType,PlayerObject* player)
{
	gMessageFactory->StartMessage();    
	gMessageFactory->addUint32(opServerWeatherMessage);  
	gMessageFactory->addUint32(weatherType);           
	gMessageFactory->addFloat(cloudVec.mX);
	gMessageFactory->addFloat(cloudVec.mY);
	gMessageFactory->addFloat(cloudVec.mZ);

	if(player && player->isConnected())
	{
		(player->getClient())->SendChannelA(gMessageFactory->EndMessage(),player->getAccountId(),CR_Client,3);
	}
	else
	{
		_sendToAll(gMessageFactory->EndMessage(),3);
	}
}

//======================================================================================================================
//
// update cell permissions
//

bool MessageLib::sendUpdateCellPermissionMessage(CellObject* cellObject,uint8 permission,PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();     
	gMessageFactory->addUint32(opUpdateCellPermissionMessage);  
	gMessageFactory->addUint8(permission);
	gMessageFactory->addUint64(cellObject->getId());

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 3);

	return(true);
}

//======================================================================================================================
//
// play a clienteffect
//

bool MessageLib::sendPlayClientEffectObjectMessage(string effect,string location,PlayerObject* originObject,CreatureObject* targetObject)
{
	if(!(originObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opPlayClientEffectObjectMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(location);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint16(0); 

	_sendToInRange(gMessageFactory->EndMessage(),originObject,8,true);		
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

bool MessageLib::sendPlayClientEffectObjectMessage(string effect,string location, CreatureObject* targetObject)
{
	// No need to restrict this function for PlayerObject* only.

	// We use getKnownPlayers() when we send this.
	// if(!(originObject->isConnected()))
	// 	return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opPlayClientEffectObjectMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(location);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint16(0); 

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,8,false);		
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendPlayClientEffectObjectMessage(string effect,string location, Object* targetObject)
{
	// No need to restrict this function for PlayerObject* only.

	// We use getKnownPlayers() when we send this.
	// if(!(originObject->isConnected()))
	// 	return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opPlayClientEffectObjectMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(location);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint16(0); 

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,8,false);		
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5);

	return(true);
}

bool MessageLib::sendPlayClientEffectObjectMessage(string effect,string location, Object* targetObject, PlayerObject* player)
{
	// No need to restrict this function for PlayerObject* only.

	 if(!(player->isConnected()))
	 	return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opPlayClientEffectObjectMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(location);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint16(0); 

	//_sendToInRange(gMessageFactory->EndMessage(),targetObject,8,false);		
	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 5);

	return(true);
}




//======================================================================================================================
//
// play a clienteffect at location
//

bool MessageLib::sendPlayClientEffectLocMessage(string effect,Anh_Math::Vector3 pos,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	string		planet = gWorldManager->getPlanetNameThis();

	gMessageFactory->StartMessage();            
	gMessageFactory->addUint32(opPlayClientEffectLocMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(planet);
	gMessageFactory->addFloat(pos.mX);
	//gMessageFactory->addFloat(static_cast<float>(pos.mY - 0.2));
	gMessageFactory->addFloat(pos.mY);
	gMessageFactory->addFloat(pos.mZ);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0); 

	//this is probably a reliable isnt it ?
	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 8);

	return(true);
}

//======================================================================================================================
//
// survey response
//

ResourceLocation MessageLib::sendSurveyMessage(uint16 range,uint16 points,CurrentResource* resource,PlayerObject* targetObject)
{
	float				posX,posZ,ratio;	
	ResourceLocation	highestDist;

	// init to lowest possible value
	uint8		 step		= range / (points - 1);
	highestDist.ratio		= -1.0f;
	// using mY of highest ratio vector, to determine if resource actually was found
	highestDist.position.mY = 0.0f;

	range = (range >> 1);

	gMessageFactory->StartMessage();         
	gMessageFactory->addUint32(opSurveyMessage);   

	gMessageFactory->addUint32(points*points);

	for(int16 i = -range;i <= range;i+=step)
	{
		for(int16 j = -range;j <= range;j+=step)
		{
			posX	= targetObject->mPosition.mX + (float)i;
			posZ	= targetObject->mPosition.mZ + (float)j;
			ratio	= resource->getDistribution((int)posX + 8192,(int)posZ + 8192);

			if(ratio > highestDist.ratio)
			{				
				highestDist.position.mX = posX;
				highestDist.position.mZ = posZ;
				highestDist.ratio = ratio;
			}

			gMessageFactory->addFloat(posX);
			gMessageFactory->addFloat(0.0f);
			gMessageFactory->addFloat(posZ);
			gMessageFactory->addFloat(ratio);  
		}
	}

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 2);

	if(highestDist.ratio >= 0.1f)
		highestDist.position.mY = 5.0f;

	return highestDist;
}

//======================================================================================================================
//
// send current badges
//

bool MessageLib::sendBadges(PlayerObject* srcObject,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	uint32		badgeMap[15];

	for(uint32 i = 0;i < 15;i++)
		memset(&badgeMap[i],0,sizeof(badgeMap[i]));

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBadgesResponseMessage);
	gMessageFactory	->addUint64(srcObject->getId());
	gMessageFactory->addUint32(15);

	BadgesList* badges = srcObject->getBadges();

	BadgesList::iterator it = badges->begin();

	while(it != badges->end())
	{
		uint32 index = (uint32)floor((double)((*it)/32));
		badgeMap[index] = badgeMap[index] ^ (1 << ((*it)%32));
		++it;
	}

	for(uint32 i = 0;i < 15;i++)
		gMessageFactory->addUint32(badgeMap[i]);

	// unknown
	gMessageFactory->addUint8(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,3);

	return(true);
}

//======================================================================================================================
//
// play music message
//

bool MessageLib::sendPlayMusicMessage(uint32 soundId,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
	{
		return(false);
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opPlayMusicMessage);
	gMessageFactory->addString(gWorldManager->getSound(soundId));
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint8(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// play music message, used by non-player objects.
//

bool MessageLib::sendPlayMusicMessage(uint32 soundId, Object* creatureObject)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opPlayMusicMessage);
	gMessageFactory->addString(gWorldManager->getSound(soundId));
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint8(0);

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5,false);		
	return(true);
}




//======================================================================================================================
//
// character sheet
//

bool MessageLib::sendCharacterSheetResponse(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opCharacterSheetResponseMessage);  

	gMessageFactory->addUint64(0);

	if(playerObject->getBindPlanet() == -1)
	{
		gMessageFactory->addFloat(0);
		gMessageFactory->addFloat(0);
		gMessageFactory->addFloat(0);
		gMessageFactory->addUint16(0);
	}
	else
	{
		Anh_Math::Vector3 bindLoc = playerObject->getBindCoords();

		gMessageFactory->addFloat(bindLoc.mX);
		gMessageFactory->addFloat(bindLoc.mY);
		gMessageFactory->addFloat(bindLoc.mZ);
		string bindPlanet(gWorldManager->getPlanetNameById(playerObject->getBindPlanet()));
		gMessageFactory->addString(bindPlanet);
	}

	// unknown (the unused bank position)
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0);

	// bank
	Bank* bank = dynamic_cast<Bank*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank));

	if(!bank || bank->getPlanet() == -1)
	{
		gMessageFactory->addString(BString("unknown"));
	}
	else
	{
		gMessageFactory->addString(BString(gWorldManager->getPlanetNameById(bank->getPlanet())));
	}

	if(playerObject->getHomePlanet() == -1)
	{
		gMessageFactory->addFloat(0);
		gMessageFactory->addFloat(0);
		gMessageFactory->addFloat(0);
		gMessageFactory->addUint16(0);
	}
	else
	{
		Anh_Math::Vector3 homeLoc = playerObject->getHomeCoords();

		gMessageFactory->addFloat(homeLoc.mX);
		gMessageFactory->addFloat(homeLoc.mY);
		gMessageFactory->addFloat(homeLoc.mZ);
		gMessageFactory->addString(BString(gWorldManager->getPlanetNameById(playerObject->getHomePlanet())));
	} 

	gMessageFactory->addString(playerObject->getMarriage());
	gMessageFactory->addUint32(playerObject->getLots());

	// neutral
	if(playerObject->getFaction().getCrc() == 0x1fdc3051)
		gMessageFactory->addUint32(0);
	else
		gMessageFactory->addUint32(playerObject->getFaction().getCrc());

	// Faction State see wiki for details
	gMessageFactory->addUint32(0);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5);

	return(true);
}

//======================================================================================================================
//
// trade, remove item from window
//

bool MessageLib::sendDeleteItemMessage(PlayerObject* playerObject,uint64 ItemId)
{
	if(!(playerObject->isConnected()))
		return(false);

	//deletes an item in trade window

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opRemoveItemMessage);  
	gMessageFactory->addUint64(ItemId);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// bid auction response
//

bool MessageLib::sendBidAuctionResponseMessage(PlayerObject* playerObject, uint64 AuctionId, uint32 error)
{	
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opBidAuctionResponseMessage);
	gMessageFactory->addUint64(AuctionId);
	gMessageFactory->addUint32(error);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}			

//======================================================================================================================
//
// trade, accept
//

bool MessageLib::sendAcceptTradeMessage(PlayerObject* playerObject)
{	
	if(!(playerObject->isConnected()))
		return(false);

	//sets the accepted flag
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opAcceptTransactionMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, give money
//

bool MessageLib::sendGiveMoneyMessage(PlayerObject* playerObject,uint32 Money)
{	
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opGiveMoneyMessage);  
	gMessageFactory->addUint32(Money);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, cancel
//

bool MessageLib::sendUnacceptTradeMessage(PlayerObject* playerObject)
{	
	if(!(playerObject->isConnected()))
		return(false);

	//sets the accepted flag
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opUnacceptTransactionMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, verify
//

bool MessageLib::sendBeginVerificationMessage(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opBeginVerificationMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, complete
//

bool MessageLib::sendTradeCompleteMessage(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opTradeCompleteMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, verify
//

bool MessageLib::sendVerificationMessage(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opVerifyTradeMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

// again ?
bool MessageLib::sendVerifyTradeMessage(PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	//cancels the trade and clears the trade window
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opVerifyTradeMessage);  

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4);

	return(true);
}


//======================================================================================================================
//
// trade, abort
//

bool MessageLib::sendAbortTradeMessage(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return(false);

	//cancels the trade and clears the trade window
	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opAbortTradeMessage);  

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, begin
//

bool MessageLib::sendBeginTradeMessage(PlayerObject* targetObject,PlayerObject* srcObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();      
	gMessageFactory->addUint32(opBeginTradeMessage);  
	gMessageFactory->addUint64(srcObject->getId());  

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4);

	return(true);
}

//======================================================================================================================
//
// trade, add item
//

bool MessageLib::sendAddItemMessage(PlayerObject* targetPlayer,TangibleObject* Object)
{
	if(!(targetPlayer->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opAddItemMessage);          
	gMessageFactory->addUint64(Object->getId());           // 

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Client, 5);

	return(true);
}

//======================================================================================================================
//
// auction, item response
//

void MessageLib::sendCreateAuctionItemResponseMessage(PlayerObject* targetPlayer,uint64 AuctionId,uint32 error)
{
	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opCreateAuctionMessageResponseMessage);
	gMessageFactory->addUint64(AuctionId);
	gMessageFactory->addUint32(error);

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetPlayer->getAccountId(),  CR_Client, 6);
}

//======================================================================================================================
//
// updates an object parent<->child relationship
//

bool MessageLib::broadcastContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateContainmentMessage);  

	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint64(parentId);
	gMessageFactory->addUint32(linkType);				

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,4,true);		
	//(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 4);

	return(true);

}

//======================================================================================================================
//
// updates an object parent<->child relationship
// Used when Creatures updates their cell positions.
//

bool MessageLib::broadcastContainmentMessage(uint64 objectId,uint64 parentId,uint32 linkType,Object* targetObject)
{
	// gLogger->logMsg("MessageLib::broadcastContainmentMessage");

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opUpdateContainmentMessage);  

	gMessageFactory->addUint64(objectId);  
	gMessageFactory->addUint64(parentId);
	gMessageFactory->addUint32(linkType);				

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,4,false);		
	return(true);

}


//======================================================================================================================
//
//	Methods related to the Tutorial
//


//======================================================================================================================
//
// update tutorial trigger
//

void MessageLib::sendUpdateTutorialRequest(PlayerObject* playerObject, string request)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opNewbieTutorialRequest);  
	gMessageFactory->addString(request);
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);
}

//======================================================================================================================
//
// Open Holocron
//

void MessageLib::sendOpenHolocron(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opOpenHolocronToPageMessage);  
	gMessageFactory->addUint16(0);
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

}

//======================================================================================================================
//
// enable hud element
//

void MessageLib::sendEnableHudElement(PlayerObject* playerObject, string hudElement)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(0xCA375124);  
	gMessageFactory->addString(hudElement);
	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0);
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

}

//======================================================================================================================
//
// disable hud element
//

void MessageLib::sendDisableHudElement(PlayerObject* playerObject, string hudElement)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(0xCA375124);  
	gMessageFactory->addString(hudElement);
	gMessageFactory->addUint8(0);
	gMessageFactory->addUint32(0);
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

}


//======================================================================================================================
//
// Logout
//

void MessageLib::sendLogout(PlayerObject* playerObject)
{
	if(!(playerObject->isConnected()))
		return;

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opLogoutMessage);  
	
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

}

/*
bool MessageLib::sendSystemMessage(PlayerObject* playerObject,string mainFile,string mainVar,TangibleObject* to, CreatureObject* tt, CreatureObject* tu, int32 di)
{
	if(!playerObject)
	{
		gLogger->logMsg("bool MessageLib::sendSystemMessage (Objects) :: player is NULL");
		return(false);
	}

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	
	// templated message
	gMessageFactory->addUint32(0);				 

	// WRONG!
	// The real size is the size of ALL parameters.
	// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

	string tuFullName;
	string ttFullName;
	string toCustom;

	uint32	realSize = mainFile.getLength() + mainVar.getLength();
	if(to)
	{
		realSize +=		to->getNameFile().getLength() + to->getName().getLength();
		toCustom =		to->getCustomName();
		toCustom.convert(BSTRType_Unicode16);
	}

	if(tu)
	{
		realSize +=  tu->getSpeciesString().getLength() + tu->getSpeciesGroup().getLength();
		if(tu->getFirstName().getLength() > 1)
		{
			tuFullName << tu->getFirstName().getAnsi();
		}
		if(tu->getLastName().getLength() > 1)
		{
			tuFullName << tu->getLastName().getAnsi();
		}

		tuFullName.convert(BSTRType_Unicode16);
	}

	if(tt)
	{
		realSize +=  tt->getSpeciesString().getLength() + tt->getSpeciesGroup().getLength();
		if(tt->getFirstName().getLength() > 1)
		{
			ttFullName << tt->getFirstName().getAnsi();
		}
		if(tt->getLastName().getLength() > 1)
		{
			ttFullName << tt->getLastName().getAnsi();
		}

		ttFullName.convert(BSTRType_Unicode16);

	}
	

	gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + tuFullName.getLength() + ttFullName.getLength());

	if(realSize % 2)
		gMessageFactory->addUint16(1);
	else
		gMessageFactory->addUint16(0);

	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0xFFFFFFFF);

	//main message		
	gMessageFactory->addString(mainFile);
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(mainVar);

	//object 1
	if(tu)
	{
		gMessageFactory->addUint64(tu->getId());
		gMessageFactory->addString(tu->getSpeciesGroup().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tu->getSpeciesString().getAnsi());
		gMessageFactory->addString(tuFullName.getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//
	}

	//object 2		
	if(tt)
	{
		gMessageFactory->addUint64(tt->getId());  //object id2
		gMessageFactory->addString(tt->getSpeciesGroup().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tt->getSpeciesString().getAnsi());
		gMessageFactory->addString(ttFullName.getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	//object 3
	if(to)
	{
		gMessageFactory->addUint64(to->getId());
		gMessageFactory->addString(to->getNameFile().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(to->getName().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		//gMessageFactory->addString(to->getCustomName().getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	gMessageFactory->addInt32(di);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);

	if(realSize % 2)
		gMessageFactory->addUint8(0);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}


bool MessageLib::sendSystemMessage(PlayerObject* playerObject,string mainFile,string mainVar,TangibleObject* to, TangibleObject* tt, CreatureObject* tu, int32 di)
{
	if(!playerObject)
	{
		gLogger->logMsg("bool MessageLib::sendSystemMessage (Objects) :: player is NULL");
		return(false);
	}

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	
	// templated message
	gMessageFactory->addUint32(0);				 

	// WRONG!
	// The real size is the size of ALL parameters.
	// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

	string tuFullName;
	string ttCustom;
	string toCustom;

	uint32	realSize = mainFile.getLength() + mainVar.getLength();
	if(to)
	{
		realSize +=		to->getNameFile().getLength() + to->getName().getLength();
		toCustom =		to->getCustomName();
		toCustom.convert(BSTRType_Unicode16);
	}

	if(tu)
	{
		realSize +=  tu->getSpeciesString().getLength() + tu->getSpeciesGroup().getLength();
		if(tu->getFirstName().getLength() > 1)
		{
			tuFullName << tu->getFirstName().getAnsi();
		}
		if(tu->getLastName().getLength() > 1)
		{
			tuFullName << tu->getLastName().getAnsi();
		}

		tuFullName.convert(BSTRType_Unicode16);
	}

	if(tt)
	{
		realSize +=		tt->getNameFile().getLength() + tt->getName().getLength();
		ttCustom =		tt->getCustomName();
		ttCustom.convert(BSTRType_Unicode16);
	}
	

	gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + tuFullName.getLength() + ttCustom.getLength());

	if(realSize % 2)
		gMessageFactory->addUint16(1);
	else
		gMessageFactory->addUint16(0);

	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0xFFFFFFFF);

	//main message		
	gMessageFactory->addString(mainFile);
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(mainVar);

	//object 1
	if(tu)
	{
		gMessageFactory->addUint64(tu->getId());
		gMessageFactory->addString(tu->getSpeciesGroup().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tu->getSpeciesString().getAnsi());
		gMessageFactory->addString(tuFullName.getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//
	}

	//object 2		
	if(tt)
	{
		gMessageFactory->addUint64(tt->getId());
		gMessageFactory->addString(tt->getNameFile().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tt->getName().getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	//object 3
	if(to)
	{
		gMessageFactory->addUint64(to->getId());
		gMessageFactory->addString(to->getNameFile().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(to->getName().getAnsi());
		gMessageFactory->addUint32(0);//spacer
		//gMessageFactory->addString(to->getCustomName().getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	gMessageFactory->addInt32(di);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);

	if(realSize % 2)
		gMessageFactory->addUint8(0);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}
					  
bool MessageLib::sendSystemMessage(PlayerObject* playerObject,string mainFile,string mainVar, int32 di)
{
	if(!playerObject)
	{
		gLogger->logMsg("bool MessageLib::sendSystemMessage (Objects) :: player is NULL");
		return(false);
	}

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	
	// templated message
	gMessageFactory->addUint32(0);				 

	// WRONG!
	// The real size is the size of ALL parameters.
	// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

	uint32	realSize = mainFile.getLength() + mainVar.getLength();

	gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)));

	if(realSize % 2)
		gMessageFactory->addUint16(1);
	else
		gMessageFactory->addUint16(0);

	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0xFFFFFFFF);

	//main message		
	gMessageFactory->addString(mainFile);
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(mainVar);

	//object 1

	gMessageFactory->addUint64(0);
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//

	gMessageFactory->addUint64(0);
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//spacer

	gMessageFactory->addUint64(0);
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(BString("").getAnsi());
	gMessageFactory->addUint32(0);//spacer
	

	gMessageFactory->addInt32(di);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);

	if(realSize % 2)
		gMessageFactory->addUint8(0);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}

		  */
bool MessageLib::sendSysMsg(PlayerObject* playerObject,string mainFile,string mainVar,Object* to, Object* tt, Object* tu, int32 di)
{
	if(!playerObject)
	{
		gLogger->logMsg("bool MessageLib::sendSystemMessage (Objects) :: player is NULL");
		return(false);
	}

	if(!(playerObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage(); 
	gMessageFactory->addUint32(opChatSystemMessage);  
	gMessageFactory->addUint8(0);

	
	// templated message
	gMessageFactory->addUint32(0);				 

	// WRONG!
	// The real size is the size of ALL parameters.
	// uint32	realSize = mainFile.getLength() + mainVar.getLength() + toFile.getLength() + toVar.getLength() + ttFile.getLength() + ttVar.getLength();

	string ttCustom, toCustom, tuCustom;
	string ttdir, tudir, todir;
	string ttfile, tufile, tofile;
	

	uint32	realSize = mainFile.getLength() + mainVar.getLength();
	
	TangibleObject* tto = dynamic_cast<TangibleObject*>(to);
	
	if(tto)
	{
		realSize +=		tto->getNameFile().getLength() + tto->getName().getLength();
		toCustom =		tto->getCustomName();
		toCustom.convert(BSTRType_Unicode16);
		
		tofile	= tto->getNameFile();
		todir	= tto->getName();
	}
	else
	{
		CreatureObject* cto = dynamic_cast<CreatureObject*>(to);
		if(cto)
		{
			realSize +=  cto->getSpeciesString().getLength() + cto->getSpeciesGroup().getLength();
			if(cto->getFirstName().getLength() > 1)
			{
				toCustom << cto->getFirstName().getAnsi();
			}
			if(cto->getLastName().getLength() > 1)
			{
				toCustom<< cto->getLastName().getAnsi();
			}

			toCustom.convert(BSTRType_Unicode16);

			tofile	= cto->getSpeciesGroup();
			todir	= cto->getSpeciesString();
		}
	}

	TangibleObject* ttu = dynamic_cast<TangibleObject*>(tu);
	
	if(ttu)
	{
		realSize +=		ttu->getNameFile().getLength() + ttu->getName().getLength();
		tuCustom =		ttu->getCustomName();
		tuCustom.convert(BSTRType_Unicode16);
		tufile	= ttu->getNameFile();
		tudir	= ttu->getName();
	}
	else
	{
		CreatureObject* ctu = dynamic_cast<CreatureObject*>(tu);
		if(ctu)
		{
			realSize +=  ctu->getSpeciesString().getLength() + ctu->getSpeciesGroup().getLength();
			if(ctu->getFirstName().getLength() > 1)
			{
				tuCustom << ctu->getFirstName().getAnsi();
			}
			if(ctu->getLastName().getLength() > 1)
			{
				tuCustom<< ctu->getLastName().getAnsi();
			}

			tuCustom.convert(BSTRType_Unicode16);

			tufile	= ctu->getSpeciesGroup();
			tudir	= ctu->getSpeciesString();
		}
	}

	
	TangibleObject* ttt = dynamic_cast<TangibleObject*>(tt);

	if(ttt)
	{
		realSize +=		ttt->getNameFile().getLength() + ttt->getName().getLength();
		ttCustom =		ttt->getCustomName();
		ttCustom.convert(BSTRType_Unicode16);

		ttfile	= ttt->getNameFile();
		ttdir	= ttt->getName();
	}
	else
	{
		CreatureObject* ctt = dynamic_cast<CreatureObject*>(tt);
		if(ctt)
		{
			realSize +=  ctt->getSpeciesString().getLength() + ctt->getSpeciesGroup().getLength();
			if(ctt->getFirstName().getLength() > 1)
			{
				ttCustom<< ctt->getFirstName().getAnsi();
			}
			if(ctt->getLastName().getLength() > 1)
			{
				ttCustom<< ctt->getLastName().getAnsi();
			}

			ttCustom.convert(BSTRType_Unicode16);

			ttfile	= ctt->getSpeciesGroup();
			ttdir	= ctt->getSpeciesString();
		}
	}

	gMessageFactory->addUint32(42 + ((uint32)ceil(((double)realSize) / 2.0)) + toCustom.getLength() + tuCustom.getLength() + ttCustom.getLength());

	if(realSize % 2)
		gMessageFactory->addUint16(1);
	else
		gMessageFactory->addUint16(0);

	gMessageFactory->addUint8(1);
	gMessageFactory->addUint32(0xFFFFFFFF);

	//main message		
	gMessageFactory->addString(mainFile);
	gMessageFactory->addUint32(0);//spacer
	gMessageFactory->addString(mainVar);

	//object 1
	if(tu)
	{
		gMessageFactory->addUint64(tu->getId());
		gMessageFactory->addString(tufile.getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(tudir.getAnsi());
		gMessageFactory->addString(tuCustom.getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//
	}

	//object 2		
	if(tt)
	{
		gMessageFactory->addUint64(tt->getId());
		gMessageFactory->addString(ttfile.getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(ttdir.getAnsi());
		gMessageFactory->addString(ttCustom.getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	//object 3
	if(to)
	{
		gMessageFactory->addUint64(to->getId());
		gMessageFactory->addString(tofile.getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(todir.getAnsi());
		gMessageFactory->addString(toCustom.getUnicode16());
		//gMessageFactory->addString(to->getCustomName().getUnicode16());
	}
	else
	{
		gMessageFactory->addUint64(0);
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
		gMessageFactory->addString(BString("").getAnsi());
		gMessageFactory->addUint32(0);//spacer
	}

	gMessageFactory->addInt32(di);
	gMessageFactory->addUint32(0);
	gMessageFactory->addUint8(0);

	if(realSize % 2)
		gMessageFactory->addUint8(0);

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5);

	return(true);
}
