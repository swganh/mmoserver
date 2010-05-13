#include <list>
#include "QTRegion.h"
#include "QuadTree.h"
#include "ZoneTree.h"
#include "ForageManager.h"
#include "PlayerObject.h"
#include "MedicManager.h"
#include "ScoutManager.h"

#include "MessageLib/MessageLib.h"

ForageManager*	ForageManager::mSingleton = NULL;

ForageManager::ForageManager()
{
	mSI = gWorldManager->getSI();
	pHead = NULL;
}

//================================================================================
//FORAGING!
//================================================================================

class ForageAttempt
{
public:
	ForageAttempt::ForageAttempt(PlayerObject* player, uint64 time, forageClasses forageClass)
	{
		startTime = time;
		playerID = player->getId();
		completed = false;

		mForageClass = forageClass;

		orig_x = player->mPosition.x;
		orig_y = player->mPosition.y;
		orig_z = player->mPosition.z;
	}

	uint64 startTime;
	uint64 playerID;
	forageClasses mForageClass;
	bool completed;

	float orig_x;
	float orig_y;
	float orig_z;
};

class ForagePocket
{
public:
	ForagePocket::ForagePocket(PlayerObject* player, ZoneTree* mSI)
	{
		region = mSI->getQTRegion(player->mPosition.z,player->mPosition.z);

		innerRect = Anh_Math::Rectangle(player->mPosition.x - 10,player->mPosition.z - 10,20,20);
		outterRect = Anh_Math::Rectangle(player->mPosition.x - 30,player->mPosition.z - 30,60,60);

		pNext = NULL;
	}

	ForagePocket::~ForagePocket()
	{
		//This shouldn't be a problem, but it's here just in case.

		for(std::list<ForageAttempt*>::iterator it=attempts.begin(); it != attempts.end();)
		{
			delete (*it);
			it=attempts.erase(it);
		}
	}

	bool containsPlayer(PlayerObject* player)
	{
		if(region->mTree->ObjectContained(&outterRect, player))
			return true;
		else
			return false;
	}

	void addAttempt(ForageAttempt* attempt)
	{
		attempts.push_back(attempt);
	}

	bool updateAttempts(uint64 currentTime); //if True Delete this Pocket, if False don't

	ForagePocket* pNext;

private:
	std::list<ForageAttempt*> attempts;

	QTRegion* region;
	Anh_Math::Rectangle innerRect;
	Anh_Math::Rectangle outterRect;
};


void ForageManager::startForage(PlayerObject* player, forageClasses forageClass)
{
	//Check for Inside building
	if(player->getParentId() != 0)
	{
		gForageManager->failForage(player, NOT_OUTSIDE);
		return;
	}

	//Check for combat
	if(player->checkState(CreatureState_Combat))
	{
		gForageManager->failForage(player, IN_COMBAT);
		return;
	}

	//Check for action being too low
	if(player->getHam()->mAction.getCurrentHitPoints() < 101)
	{
		gForageManager->failForage(player, ACTION_LOW);
		return;
	}

	//Check for skill being too low
	if(forageClass == ForageClass_Scout && !player->checkSkill(45)) //Scout -> Survival 1
	{
		gForageManager->failForage(player, NO_SKILL);
		return;
	}
	else if(forageClass == ForageClass_Medic && !player->checkSkill(51))
	{
		gForageManager->failForage(player, NO_SKILL);
		return;
	}

	//Already foraging
	if(player->isForaging())
	{
		gForageManager->failForage(player, ALREADY_FORAGING);
		return;
	}

	player->setForaging(true);

	//Starts the Foraging Animation
	gMessageLib->sendCreatureAnimation(player, "forage");

	//Use up some action!
	player->getHam()->updatePropertyValue(HamBar_Action,HamProperty_CurrentHitpoints, -100);

	//Creates a ForageAttempt object for tracking the forage operation
	ForageAttempt* attempt = new ForageAttempt(player, gWorldManager->GetCurrentGlobalTick(), forageClass);

	//FIND THE APPROPRIATE FORAGEPocket
	ForagePocket* it = pHead;
	while(it != NULL)
	{
		if(it->containsPlayer(player))
		{
			it->addAttempt(attempt);
			return;
		}
		it = it->pNext;
	}

	//None of them contained the player. We need to make new one.

	ForagePocket* new_pocket = new ForagePocket(player, mSI);
	it = pHead;
	ForagePocket* previousHead = NULL;
	while(it != NULL)
	{
		previousHead = it;
		it = it->pNext;
	}

	if(previousHead == NULL)
		pHead = new_pocket;
	else
		previousHead->pNext = new_pocket;

	new_pocket->addAttempt(attempt);
	//gLogger->logMsg("NEW FORAGING ATTEMPT", FOREGROUND_RED);
}

void ForageManager::forageUpdate()
{
	ForagePocket* it = pHead;
	ForagePocket* previousHead = NULL;
	while(it != NULL)
	{
		if(it->updateAttempts(gWorldManager->GetCurrentGlobalTick())) //If true we delete this Pocket
		{
			//gLogger->logMsg("FORAGING TICK", FOREGROUND_RED);
			if(previousHead == NULL)
			{
				pHead = it->pNext;
			}
			else
			{
				previousHead->pNext = it->pNext;
			}
				delete it;
				it = previousHead;
		}
		if(it != NULL)
			it = it->pNext;
	}
}


void ForageManager::failForage(PlayerObject* player, forageFails fail)
{
	if(!player || !player->isConnected())
		return;

	switch(fail)
	{
	case NOT_OUTSIDE:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_inside");
		break;
	case PLAYER_MOVED:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_movefail");
		break;
	case ACTION_LOW:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_attrib");
		break;
	case IN_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_cant");
		break;
	case AREA_EMPTY:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_empty");
		break;
	case ENTERED_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_combatfail");
		break;
	case NO_SKILL:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_noskill");
		break;
	case ALREADY_FORAGING:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_already");
		return;
	}

	player->setForaging(false);
}

bool ForagePocket::updateAttempts(uint64 currentTime)
{	
	if(attempts.empty())
		return true;

	std::list<ForageAttempt*>::iterator it = attempts.begin();
	unsigned int AttemptCount = 0;
	while(it != attempts.end())
	{
		if((currentTime - (*it)->startTime) >= 300000) //5minutes until we reopen the pocket
		{
			delete (*it);
			it = attempts.erase(it);
			AttemptCount--;
			//gLogger->logMsg("POCKET REMOVED DUE TO TIMER", FOREGROUND_RED);
		}
		else if((currentTime - (*it)->startTime) >= 8000 && !(*it)->completed)
		{
			//gLogger->logMsg("ATTEMPT FINISHED", FOREGROUND_RED);
			PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
			if(player != NULL)
			{
				if(player->checkState(CreatureState_Combat))
				{
					ForageManager::failForage(player, ENTERED_COMBAT);
					(*it)->completed = true;
					it++; AttemptCount++;
					continue;
				}

				if(region->mTree->ObjectContained(&innerRect, player) && AttemptCount < 4)
				{
					//The player has a chance to get something
					ForageManager::successForage(player, (*it)->mForageClass);
					(*it)->completed = true;
					AttemptCount++;
				}
				else
				{
					(*it)->completed = true;
					ForageManager::failForage(player, AREA_EMPTY);
					AttemptCount++;
				}
			}
			it++;
		}
		else
		{
			PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
			if(!(*it)->completed && player)
			{
				if(player->checkState(CreatureState_Combat))
				{
					ForageManager::failForage(player, ENTERED_COMBAT);
					(*it)->completed = true;
					AttemptCount++;
				}

				if(!(*it)->completed)
				{
					if((*it)->orig_x != player->mPosition.x || (*it)->orig_y != player->mPosition.y || 
						(*it)->orig_z != player->mPosition.z)
					{
						ForageManager::failForage(player, PLAYER_MOVED);
						(*it)->completed = true;
						AttemptCount++;
					}
				}
			}
			it++;
		}
	}

	return false;
}


void ForageManager::successForage(PlayerObject* player, forageClasses forageClass)
{
	if(!player || player->isForaging() == false)
		return;

	switch(forageClass)
	{
	case ForageClass_Scout:
		gScoutManager->successForage(player);
		break;
	case ForageClass_Medic:
		gMedicManager->successForage(player);
		break;
	}
}