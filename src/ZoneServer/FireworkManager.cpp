#include "FireworkManager.h"

#include "WorldManager.h"
#include "UIManager.h"
#include "Item_Enums.h"
#include "nonPersistantObjectFactory.h"

FireworkManager*	FireworkManager::mSingleton = NULL;

FireworkManager::~FireworkManager(void)
{
}


//bool FireworkManager::createFirework(uint32 typeId, PlayerObject* player, bool isShow)
bool FireworkManager::createFirework(uint32 typeId, PlayerObject* player, Anh_Math::Vector3 position, bool isShow)
{
	StaticObject* firework = new StaticObject();
	firework->mPosition = position;//player->mPosition;
	firework->mDirection.mX = 0;
	firework->mDirection.mY = 0;
	firework->mDirection.mZ = 0;
	firework->mDirection.mW = 1;
	firework->setId(gNonPersistantObjectFactory->getId());

	switch(typeId)
	{
	case ItemType_Firework_Type_1: 
		firework->setModelString("object/static/firework/shared_fx_01.iff");
		break;
	case ItemType_Firework_Type_2:
		firework->setModelString("object/static/firework/shared_fx_02.iff");
		break;
	case ItemType_Firework_Type_3: 
		firework->setModelString("object/static/firework/shared_fx_03.iff");
		break;
	case ItemType_Firework_Type_4: 
		firework->setModelString("object/static/firework/shared_fx_04.iff");
		break;
	case ItemType_Firework_Type_5: 
		firework->setModelString("object/static/firework/shared_fx_05.iff");
		break;
	case ItemType_Firework_Type_10: 
		firework->setModelString("object/static/firework/shared_fx_10.iff");
		break;
	case ItemType_Firework_Type_11: 
		firework->setModelString("object/static/firework/shared_fx_11.iff");
		break;
	case ItemType_Firework_Type_18: 
		firework->setModelString("object/static/firework/shared_fx_18.iff");
		break;
	case ItemType_Firework_Show: 
		firework->setModelString("object/static/firework/shared_show_launcher.iff");
		break;

	default:
		{
			gLogger->logMsgF("Error creating firework, type:%u",MSG_NORMAL, typeId);
			return false;
		}
	}

	//gWorldManager->addObject(firework);


	if(player->isConnected())
		gMessageLib->sendHeartBeat(player->getClient());

	if(isShow==false)
	{
		player->setPosture(CreaturePosture_Crouched);
		player->getHam()->updateRegenRates();
		player->toggleStateOff(CreatureState_SittingOnChair);
		player->updateMovementProperties();

		gMessageLib->sendUpdateMovementProperties(player);
		gMessageLib->sendPostureAndStateUpdate(player);
		gMessageLib->sendSelfPostureUpdate(player);
	}


	PlayerObjectSet*			inRangePlayers	= player->getKnownPlayers();
	PlayerObjectSet::iterator	it				= inRangePlayers->begin();
	while(it != inRangePlayers->end())
	{
		PlayerObject* targetObject = (*it);
		gMessageLib->sendCreateObjectByCRC(firework,targetObject,false);
		gMessageLib->sendBaselinesSTAO_3(firework,targetObject);
		gMessageLib->sendBaselinesSTAO_6(firework,targetObject);
		gMessageLib->sendEndBaselines(firework->getId(),targetObject);
		++it;
	}

	gMessageLib->sendCreateObjectByCRC(firework,player,false);
	gMessageLib->sendBaselinesSTAO_3(firework,player);
	gMessageLib->sendBaselinesSTAO_6(firework,player);
	gMessageLib->sendEndBaselines(firework->getId(),player);
	delete firework;

	return true;
}
