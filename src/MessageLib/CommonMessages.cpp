/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

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

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 5, false);

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

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 2, false);

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

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 3, false);

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

// What is the use of "self"? 
// it means whether we send a message to all surrounding objects including or excludingus
// ... for example if object is a player
// its however not needed for destroy object
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

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 4, false);

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

	client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 1, true);

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

	(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 2, false);

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
	gMessageFactory->addUint16(object->mPosition.mX * 4.0f + 0.5f);     
	gMessageFactory->addUint16(object->mPosition.mY * 4.0f + 0.5f);      
	gMessageFactory->addUint16(object->mPosition.mZ * 4.0f + 0.5f);      
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                
	gMessageFactory->addUint8(object->mDirection.getAnglesToSend()); 

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
	gMessageFactory->addUint16(object->mPosition.mX * 8.0f + 0.5f);     
	gMessageFactory->addUint16(object->mPosition.mY * 8.0f + 0.5f);    
	gMessageFactory->addUint16(object->mPosition.mZ * 8.0f + 0.5f);    
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                       
	gMessageFactory->addUint8(object->mDirection.getAnglesToSend());          
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
	gMessageFactory->addUint16(object->mPosition.mX * 4.0f + 0.5f);     
	gMessageFactory->addUint16(object->mPosition.mY * 4.0f + 0.5f);      
	gMessageFactory->addUint16(object->mPosition.mZ * 4.0f + 0.5f);      
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                
	gMessageFactory->addUint8(object->mDirection.getAnglesToSend());    

	_sendToInstancedPlayers(gMessageFactory->EndMessage(), 8, player, true);
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
	gMessageFactory->addUint16(object->mPosition.mX * 8.0f + 0.5f);     
	gMessageFactory->addUint16(object->mPosition.mY * 8.0f + 0.5f);    
	gMessageFactory->addUint16(object->mPosition.mZ * 8.0f + 0.5f);    
	gMessageFactory->addUint32(object->getInMoveCount()); 
	long double mag = sqrt((object->mPosition.mX*object->mPosition.mX) + (object->mPosition.mY*object->mPosition.mY) + (object->mPosition.mZ*object->mPosition.mZ));
	gMessageFactory->addUint8((uint8)(mag * 4.0f + 0.5f));                       
	gMessageFactory->addUint8(object->mDirection.getAnglesToSend());  

	//Movement is THE fastpathpacket!!!
	_sendToInstancedPlayers(gMessageFactory->EndMessage(), 8, player, true);
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

	client->SendChannelA(gMessageFactory->EndMessage(),client->getAccountId(),CR_Client,2,false);

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

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 2, false);

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

	(player->getClient())->SendChannelA(gMessageFactory->EndMessage(), player->getAccountId(), CR_Client, 9, false);

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

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 2, false);

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

	client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Client, 1, false);

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

		client->SendChannelA(gMessageFactory->EndMessage(), client->getAccountId(), CR_Chat, 4, false);
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

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4, false);

	return(true);
}

//=======================================================================================================================
//
// system message
//

bool MessageLib::sendSystemMessage(PlayerObject* playerObject,string customMessage,string mainFile,string mainVar,string toFile,string toVar,string toCustom,int32 di,string ttFile,string ttVar,string ttCustom,uint64 ttId,uint64 toId,uint64 tuId,string tuFile,string tuVar,string tuCustom )
{
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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);

	return(true);
}

//======================================================================================================================
//
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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 3, false);

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
		(player->getClient())->SendChannelA(gMessageFactory->EndMessage(),player->getAccountId(),CR_Client,3,false);
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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 3, false);

	return(true);
}

//======================================================================================================================
//
// play a clienteffect
//

bool MessageLib::sendPlayClientEffectObjectMessage(string effect,string location,PlayerObject* targetObject)
{
	if(!(targetObject->isConnected()))
		return(false);

	gMessageFactory->StartMessage();   
	gMessageFactory->addUint32(opPlayClientEffectObjectMessage);
	gMessageFactory->addString(effect);
	gMessageFactory->addString(location);
	gMessageFactory->addUint64(targetObject->getId());
	gMessageFactory->addUint16(0); 

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,8,true);		
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5, false);

	return(true);
}

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
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5, false);

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
	//(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 5, false);

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
	gMessageFactory->addFloat(pos.mY - 0.2);
	gMessageFactory->addFloat(pos.mZ);
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(0); 

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 8, true);

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

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 2, false);

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

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,3,false);

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
		gLogger->logMsgF("MessageLib::sendPlayMusicMessage Player is not connected, no music sent.", MSG_NORMAL);
		return(false);
	}

	gMessageFactory->StartMessage();
	gMessageFactory->addUint32(opPlayMusicMessage);
	gMessageFactory->addString(gWorldManager->getSound(soundId));
	gMessageFactory->addUint64(0);
	gMessageFactory->addUint32(1);
	gMessageFactory->addUint8(0);

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),targetObject->getAccountId(),CR_Client,5,false);

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

	_sendToInRange(gMessageFactory->EndMessage(),creatureObject,5,false,false);		
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

		gMessageFactory->addFloat((int32)bindLoc.mX);
		gMessageFactory->addFloat((int32)bindLoc.mY);
		gMessageFactory->addFloat((int32)bindLoc.mZ);
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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(),playerObject->getAccountId(),CR_Client,5,false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4, false);

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

	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 4, false);

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

	(targetObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetObject->getAccountId(), CR_Client, 4, false);

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

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetPlayer->getAccountId(), CR_Client, 5, false);

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

	(targetPlayer->getClient())->SendChannelA(gMessageFactory->EndMessage(), targetPlayer->getAccountId(),  CR_Client, 6, false);
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

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,4,true,false);		
	//(targetObject->getClient())->SendChannelA(message, targetObject->getAccountId(), CR_Client, 4, false);

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

	_sendToInRange(gMessageFactory->EndMessage(),targetObject,4,false,false);		
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
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);
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
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);

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
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);

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
	(playerObject->getClient())->SendChannelA(gMessageFactory->EndMessage(), playerObject->getAccountId(), CR_Client, 5, false);

}
