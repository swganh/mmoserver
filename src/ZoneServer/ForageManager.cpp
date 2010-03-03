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
	}

	uint64 startTime;
	uint64 playerID;
	forageClasses mForageClass;
	bool completed;
};

class ForagePocket
{
public:
	ForagePocket::ForagePocket(PlayerObject* player, ZoneTree* mSI)
	{
		region = mSI->getQTRegion(player->mPosition.mX,player->mPosition.mZ);

		innerRect = Anh_Math::Rectangle(player->mPosition.mX - 10,player->mPosition.mZ - 10,20,20);
		outterRect = Anh_Math::Rectangle(player->mPosition.mX - 30,player->mPosition.mZ - 30,60,60);

		pNext = NULL;
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
	if(!player || player->isForaging() == false)
		return;

	switch(fail)
	{
	case NOT_OUTSIDE:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_inside");
		return;
	case PLAYER_MOVED:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_movefail");
		break;
	case ACTION_LOW:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_attrib");
		return;
	case IN_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_cant");
		return;
	case AREA_EMPTY:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_empty");
		break;
	case ENTERED_COMBAT:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_combatfail");
		break;
	case NO_SKILL:
		gMessageLib->sendSystemMessage(player, L"", "skl_use","sys_forage_noskill");
		return;
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
				}
				else
				{
					(*it)->completed = true;
					ForageManager::failForage(player, AREA_EMPTY);
				}
			}
			it++;
			AttemptCount++;
		}
		else
		{
			PlayerObject* player = (PlayerObject*)gWorldManager->getObjectById((*it)->playerID);
			if(!(*it)->completed && player && player->checkState(CreatureState_Combat))
			{
				ForageManager::failForage(player, ENTERED_COMBAT);
				(*it)->completed = true;
			}

			it++;
			AttemptCount++;
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