/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2009 The swgANH Team

---------------------------------------------------------------------------------------
*/

#include "MessageLib/MessageLib.h"
#include "AttackableCreature.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "WorldConfig.h"
#include "Weapon.h"
#include "ZoneServer/NonPersistentNpcFactory.h"
#include "LairObject.h"

// TODO: Implement by functionality.
static const int64 readyDefaultPeriodTime = 1000;
static const int64 activeDefaultPeriodTime = 500;

//=============================================================================


AttackableCreature::AttackableCreature() : NPCObject(), 
mReadyDelay(0),
mCombatTimer(0),
mHoming(false),
mCombatState(State_Unspawned),
mAttackTauntSent(false),
mWarningTauntSent(false),
mPrimaryWeapon(NULL),
mSecondaryWeapon(NULL),
mAssistedTargetId(0),
mAsssistanceNeededWithId(0),
mLairNeedAsssistanceWithId(0),
mIsAssistingLair(false),
mLootAllowedById(0)
{
	mNpcFamily	= NpcFamily_AttackableCreatures;

	mRadialMenu = RadialMenuPtr(new RadialMenu());
	mRadialMenu->addItem(1,0,radId_combatAttack,radAction_Default); 
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default); 
}

//=============================================================================

AttackableCreature::~AttackableCreature()
{
	mRadialMenu.reset();
	mDamageDealers.clear();

	delete mPrimaryWeapon;
	delete mSecondaryWeapon;
	// delete mSpawn;

	// gLogger->logMsgF("AttackableCreature::~AttackableCreature()", MSG_NORMAL);
}

//=============================================================================


void AttackableCreature::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	if (this->isDead())
	{
		// gLogger->logMsgF("AttackableCreature::prepareCustomRadialMenu Creature is dead", MSG_NORMAL);
		mRadialMenu.reset();
		mRadialMenu = RadialMenuPtr(new RadialMenu());

		if (!creatureObject->isDead() && !creatureObject->isIncapacitated())
		{
			// gLogger->logMsgF("Targeting Creature is alive", MSG_NORMAL);
			// mRadialMenu = RadialMenuPtr(new RadialMenu());
			mRadialMenu->addItem(1,0,radId_lootAll,radAction_ObjCallback, "@ui_radial:loot_all"); 
			mRadialMenu->addItem(2,1,radId_loot,radAction_ObjCallback, "@ui_radial:loot"); 
		}
		else
		{
			// gLogger->logMsgF("Targeting Creature is incapped or dead", MSG_NORMAL);
		}
	}
	else
	{
		// gLogger->logMsgF("AttackableCreature::prepareCustomRadialMenu Creature is alive", MSG_NORMAL);
	}
}

//=============================================================================

void AttackableCreature::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	// gLogger->logMsgF("AttackableCreature::handleObjectMenuSelect Entering", MSG_NORMAL);

	if (PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject))
	{	
		switch(messageType)
		{
			case radId_lootAll: 
			{
				// First, we have to have a connected player..
				// if (playerObject->isConnected() && !playerObject->isDead() && !playerObject->isIncapacitated() && this->isDead())
				{
					playerObject->getController()->lootAll(this->getId(),playerObject);
				}
			}
			break;

			case radId_loot: 
			{
				// gLogger->logMsgF("AttackableCreature::handleObjectMenuSelect Handle for loot, creature id = %llu", MSG_NORMAL, this->getId());

				// First, we have to have a connected player..
				if (playerObject->isConnected() && !playerObject->isDead() && !playerObject->isIncapacitated() && this->isDead())
				{
					// gLogger->logMsgF("AttackableCreature::handleObjectMenuSelect I am dead", MSG_NORMAL);
					if (this->allowedToLoot(playerObject->getId(), playerObject->getGroupId()))
					{
						Inventory* inventory = dynamic_cast<Inventory*>(this->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
						if (inventory)
						{
							// Open the Inventory.
							ObjectList*	objList = inventory->getObjects();
							ObjectList::iterator containerObjectIt = objList->begin();

							while (containerObjectIt != objList->end())
							{
								Object* object = (*containerObjectIt);
								
								if (TangibleObject* tangibleObject = dynamic_cast<TangibleObject*>(object))
								{
									// reminder: objects are owned by the global map, containers only keeps references
									// send the creates, if we are not owned by any player OR by exactly this player.
									if (playerObject)
									{
										if (!object->getPrivateOwner() || (object->isOwnedBy(playerObject))) 
										{
											// could be a resource container, need to check this first, since it inherits from tangible
											if (ResourceContainer* resCont = dynamic_cast<ResourceContainer*>(object))
											{
												gMessageLib->sendCreateResourceContainer(resCont,playerObject);
											}
											// or a tangible
											else
											{
												gMessageLib->sendCreateTangible(tangibleObject,playerObject);
											}
										}
									}
								}
								++containerObjectIt;
							}
							gMessageLib->sendOpenedContainer(this->getId()+1, playerObject);
							// gMessageLib->sendOpenedContainer(this->getId(), playerObject);

							int32 lootedCredits = inventory->getCredits();
							inventory->setCredits(0);
							// gLogger->logMsgF("AttackableCreature::handleObjectMenuSelect Found an Inventory with %u credits", MSG_NORMAL, lootedCredits);
							if (lootedCredits > 0)
							{
								if (playerObject->getGroupId() != 0)
								{
									// We are grouped. Split the credits with the group members in range.
									PlayerList inRangeMembers = playerObject->getInRangeGroupMembers(false);

									// Number of additional members.
									int32 noOfMembers = inRangeMembers.size();
									int32 splittedCredits = lootedCredits/(noOfMembers + 1);
													
									int8 str[64];
									sprintf(str,"%u", lootedCredits);
									string lootCreditsString(str);
									lootCreditsString.convert(BSTRType_Unicode16);

									if (splittedCredits == 0)
									{
										// To little to split.
										// "GROUP] You split %TU credits and receive %TT credits as your share."
										gMessageLib->sendSystemMessage(playerObject, L"", "group", "prose_split_coins_self", "", "", L"", 0, "", "", lootCreditsString, 0, 0, 0, "", "", lootCreditsString);
										// "There are insufficient group funds to split"
										gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "nsf_to_split");
									}
									else
									{
										int32 totalProse = lootedCredits;
										PlayerList::iterator it	= inRangeMembers.begin();
										while (it != inRangeMembers.end())
										{
											// "[GROUP] You receive %DI credits as your share."
											gMessageLib->sendSystemMessage((*it), L"", "group", "prose_split", "", "", L"", splittedCredits);
											
											// Now we need to add the credits to player inventory.
											Inventory* playerInventory = dynamic_cast<Inventory*>((*it)->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
											if (playerInventory)
											{
												playerInventory->updateCredits(splittedCredits);
											}
											totalProse -= splittedCredits;
											++it;
										}

										int8 str[64];
										sprintf(str,"%u", totalProse);
										string splitedLootCreditsString(str);
										splitedLootCreditsString.convert(BSTRType_Unicode16);

										// "GROUP] You split %TU credits and receive %TT credits as your share."
										gMessageLib->sendSystemMessage(playerObject, L"", "group", "prose_split_coins_self", "", "", L"", 0, "", "", splitedLootCreditsString, 0, 0, 0, "", "", lootCreditsString);

										// Now we need to add the credits to our own inventory.
										Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
										if (playerInventory)
										{
											playerInventory->updateCredits(totalProse);
										}
									}
								}
								else
								{
									// We looted some credits, always a start.
									// int8 str[128];
									// sprintf(str,"%u credits", lootedCredits);
									// string lootCreditsString(str);
									// gLogger->logMsgF("lootCreditsString = %s", MSG_NORMAL, lootCreditsString.getAnsi());
									// lootCreditsString.convert(BSTRType_Unicode16);
									// gMessageLib->sendSystemMessage(playerObject, L"", "spam", "loot_item_self", "", "", L"", 0, getSpeciesGroup(), getSpeciesString(), L"", 0, 0, 0, "", "", lootCreditsString);
									gMessageLib->sendSystemMessage(playerObject, L"", "base_player", "prose_coin_loot", "", "", L"", lootedCredits, getSpeciesGroup(), getSpeciesString());

									// Now we need to add the credits to our own inventory.
									Inventory* playerInventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
									if (playerInventory)
									{
										playerInventory->updateCredits(lootedCredits);
									}
								}
							}
							
							// @error_message:corpse_empty "You find nothing else of value on the selected corpse."
							if (objList->size() == 0)
							{
								if (lootedCredits == 0)
								{
									// There was no credits and no items in the inventory.
									gMessageLib->sendSystemMessage(playerObject, L"", "error_message", "corpse_empty");
								}

								// Put this creaure in the pool of delayed destruction and remove the corpse from scene.
								gWorldManager->addCreatureObjectForTimedDeletion(this->getId(), LootedCorpseTimeout);
							}
						}
					}
					else
					{
						// Player do not have permission to loot this corpse.
						gMessageLib->sendSystemMessage(playerObject,L"","error_message","no_corpse_permission");
					}
				}
			}
			break;

			default:
			{
				gLogger->logMsgF("AttackableCreature::handleObjectMenuSelect Unhandled messageType = %u\n", MSG_NORMAL, messageType);
			}
			break;

		}
	}
}

//=============================================================================
//
//	Routines realted to NPC AI
//
//=============================================================================

void AttackableCreature::addKnownObject(Object* object)
{
	if (object->getType() == ObjType_Player)
	{
		mKnownPlayers.insert(dynamic_cast<PlayerObject*>(object));

		if ((this->getAiState() == NpcIsDormant) && this->isAgressiveMode() && isSpawned())	// Do not wake up the not spawned.
		{
			// gLogger->logMsgF("AttackableCreature::addKnownObject() Wakie-wakie!", MSG_NORMAL);
			gWorldManager->forceHandlingOfDormantNpc(this->getId());
		}
	}
	else
	{
		mKnownObjects.insert(object);
	}
}


//=============================================================================
//
//	Set new active target, if any in range.
//  Object already in defender list is not set.
//
//	Taunt message or flytext "!" when attempting an attack, will only be shown once.
//  Peace will enable taunt/flytext again.
//	
// 
//	Return true if target set.
//

bool AttackableCreature::setTargetInAttackRange(void)
{
	bool targetSet = false;

	// Attack nearest target or the first target found within range?
	// if (mAgressiveMode)
	if (isAgressiveMode())
	{
		PlayerObjectSet* knownPlayers = this->getKnownPlayers();
		PlayerObjectSet::iterator it = knownPlayers->begin();
		
		// For now, we attack the first target we see, that have we enough aggro towards.
		while(it != knownPlayers->end())
		{
			if (!(*it)->isIncapacitated() && !(*it)->isDead())
			{
				// We only accepts new targets.
				ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
				bool newTarget = true;
				while (defenderIt != this->getDefenders()->end())
				{
					if ((*defenderIt) == (*it)->getId())
					{
						// Already in my list, try next.
						newTarget = false;
						break;
					}
					defenderIt++;
				}

				if (newTarget && gWorldManager->objectsInRange(this->getId(), (*it)->getId(), this->getAttackRange()))
				{
					// gLogger->logMsgF("AttackableCreature::setTargetInAttackRange() attacking player = %s", MSG_NORMAL, (*it)->getFirstName().getAnsi());
					if (gWorldConfig->isInstance())
					{
						if (this->getPrivateOwner() == (*it)->getId())
						{
							// gLogger->logMsgF("Attacking my owner = %s", MSG_NORMAL,(*it)->getFirstName().getAnsi());
						}
						else
						{
							gLogger->logMsgF("Attacking WRONG TARGET = %s\n", MSG_NORMAL,(*it)->getFirstName().getAnsi());
						}
					}
					// We have a new target in range etc.. But we may need him to be visible for a while before we attack.
					this->updateAggro((*it)->getId(), (*it)->getGroupId(), (*it)->getPosture());

					if (this->attackerHaveAggro((*it)->getId()))
					{
						// gLogger->logMsgF("AttackableCreature::setTargetInAttackRange: Target have aggro!", MSG_NORMAL);
						this->setTarget((*it)->getId());
						targetSet = true;
						gMessageLib->sendTargetUpdateDeltasCreo6(this);
						break;
					}
					else
					{
						// gLogger->logMsgF("AttackableCreature::setTargetInAttackRange: Target have NO aggro!", MSG_NORMAL);
					}
				}
			}
			else
			{
				// Handle incapped targets, for now we remove any aggro to them.
				this->updateAggro((*it)->getId(), (*it)->getGroupId(), (*it)->getPosture());
			}
			++it;
		}
	}

	if (targetSet && !this->isAttackTauntSent())
	{
		this->setAttackTauntSent();

		if (this->getAttackStartMessage().getLength())
		{
			// gLogger->logMsgF("AttackableCreature::setTargetInAttackRange() Attack taunt = %s", MSG_NORMAL, this->getAttackStartMessage().getAnsi());
			// for now, let's just taunt him.
			string msg(this->getAttackStartMessage());
			msg.convert(BSTRType_Unicode16);
			char quack[5][32];
			memset(quack, 0, sizeof(quack));

			if (!gWorldConfig->isInstance())
			{
				gMessageLib->sendSpatialChat(this, msg, quack);
				// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27));	// poke
			}
			else
			{
				PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->getTarget());
				if (playerObject)
				{
					gMessageLib->sendSpatialChat(this, msg, quack, playerObject);
					// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27), playerObject);
				}
			}
		}
		else
		{
			// Let's play with the flytexts.
			// gLogger->logMsgF("AttackableCreature::setTargetInAttackRange() Testing flytexts", MSG_NORMAL);
			gMessageLib->sendFlyText(this,"npc_reaction/flytext","threaten",255,0,0);
		}
	}
	return targetSet;
}

//=============================================================================
//
//	Show warning message when target gets near this npc.
//	For now a spatial chat used in the tutorial, but it can easily be customised to other visuals,
//	like the red "?" you see when you approach a creature.

/*
bool AttackableCreature::showWarningInRange(void)
{
	bool targetSet = false;


	if (isAgressiveMode())
	{
		PlayerObjectSet* knownPlayers = this->getKnownPlayers();
		PlayerObjectSet::iterator it = knownPlayers->begin();
		
		// For now, we attack the first target we see.
		while(it != knownPlayers->end())
		{
			if (!(*it)->isIncapacitated() && !(*it)->isDead())
			{
				// Only show this for new targets. Could possible be connected to the defenders list.
				if (!this->getTarget() || ((*it)->getId() != this->getTarget()->getId()))
				{
					if (gWorldManager->objectsInRange(this->getId(), (*it)->getId(), this->getAttackWarningRange()))
					{
						this->setTarget(*it);
						targetSet = true;
						if (getAttackWarningMessage().getLength())
						{
							string msg(getAttackWarningMessage());
							msg.convert(BSTRType_Unicode16);
							char quack[5][32];
							memset(quack, 0, sizeof(quack));

							if (!gWorldConfig->isInstance())
							{
								gMessageLib->sendSpatialChat(this, msg, quack);
								// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27));	// poke
							}
							else
							{
								PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->getTarget());
								if (playerObject)
								{
									gMessageLib->sendSpatialChat(this, msg, quack, playerObject);
									// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27), playerObject);
								}
							}
						}
						break;
					}
				}
			}
			++it;
		}
	}
	return targetSet;
}
*/

//=============================================================================
//
//	Show warning message when target gets near this npc.
//	For now a spatial chat used in the tutorial, but it can easily be customised to other visuals,
//	like the red "?" you see when you approach a creature.

bool AttackableCreature::showWarningInRange(void)
{
	bool targetSet = false;

	// Attack nearest target or the first target found within range?
	if (isAgressiveMode())
	{
		PlayerObjectSet* knownPlayers = this->getKnownPlayers();
		PlayerObjectSet::iterator it = knownPlayers->begin();
		
		// For now, we attack the first target we see.
		bool isOldTarget = false;
		while(it != knownPlayers->end())
		{
			if (!(*it)->isIncapacitated() && !(*it)->isDead())
			{
				ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
				bool newTarget = true;
				while (defenderIt != this->getDefenders()->end())
				{
					if ((*defenderIt) == (*it)->getId())
					{
						// Already in my list, try next.
						newTarget = false;
						break;
					}
					defenderIt++;
				}

				// We only accepts new targets.
				// if ((!this->getTarget() || ((*it) != this->getTarget())) && 
				// 	(newTarget && gWorldManager->objectsInRange(this->getId(), (*it)->getId(), this->getAttackWarningRange())))
				if (newTarget && gWorldManager->objectsInRange(this->getId(), (*it)->getId(), this->getAttackWarningRange()))
				{
					if (!this->getTarget() || ((*it) != this->getTarget()))
					{
						this->setTarget((*it)->getId());
						targetSet = true;
						gMessageLib->sendTargetUpdateDeltasCreo6(this);
						break;
					}
					else
					{
						// same old fellow targeted.
						isOldTarget = true;
					}
				}
			}
			++it;
		}
		if (!targetSet && !isOldTarget)
		{
			// We have a no targets in range, enable taunts.
			this->clearWarningTauntSent();
		}
	}

	if (targetSet && !this->isWarningTauntSent())
	{
		this->setWarningTauntSent();

		if (getAttackWarningMessage().getLength())
		{
			string msg(getAttackWarningMessage());
			msg.convert(BSTRType_Unicode16);
			char quack[5][32];
			memset(quack, 0, sizeof(quack));

			if (!gWorldConfig->isInstance())
			{
				gMessageLib->sendSpatialChat(this, msg, quack);
				// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27));	// poke
			}
			else
			{
				PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->getTarget());
				if (playerObject)
				{
					gMessageLib->sendSpatialChat(this, msg, quack, playerObject);
					// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27), playerObject);
				}
			}
		}
		else
		{
			// Let's play with the flytexts.
			// gLogger->logMsgF("AttackableCreature::showWarningInRange() Testing flytexts", MSG_NORMAL);
			gMessageLib->sendFlyText(this,"npc_reaction/flytext","alert",255,0,0);
		}
	}
	return targetSet;
}






//=============================================================================
//
//	Target defender if within max weapon range.
//	Return true if target set.
//

bool AttackableCreature::setTargetDefenderWithinWeaponRange(void)
{
	bool foundTarget = false;

	ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
	while (defenderIt != this->getDefenders()->end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			if (!defenderCreature->isIncapacitated() && !defenderCreature->isDead())
			{
				if (gWorldManager->objectsInRange(this->getId(), *defenderIt, this->getWeaponMaxRange()))
				{
					// Do only attack objects that have build up enough aggro.
					if (this->attackerHaveAggro(defenderCreature->getId()))
					{
						// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinWeaponRange() have aggro!", MSG_NORMAL);
						if (this->getTargetId() != *defenderIt)
						{
							this->setTarget(*defenderIt);
							gMessageLib->sendTargetUpdateDeltasCreo6(this);
						}
						foundTarget = true;
						break;
					}
					else
					{
						// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinWeaponRange() have NO aggro yet!", MSG_NORMAL);
					}
				}
			}
		}
		++defenderIt;
	}

	if (foundTarget)
	{
		if (getAttackedMessage().getLength())
		{
			// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinWeaponRange() Attack taunt = %s", MSG_NORMAL, getAttackedMessage().getAnsi());
			// for now, let's just taunt him.
			string msg(getAttackedMessage());
			msg.convert(BSTRType_Unicode16);
			char quack[5][32];
			memset(quack, 0, sizeof(quack));

			if (!gWorldConfig->isInstance())
			{
				gMessageLib->sendSpatialChat(this, msg, quack);
				// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27));	// poke
			}
			else
			{
				PlayerObject* playerObject = dynamic_cast<PlayerObject*>(this->getTarget());
				if (playerObject)
				{
					gMessageLib->sendSpatialChat(this, msg, quack, playerObject);
					// gMessageLib->sendCreatureAnimation(this,gWorldManager->getNpcConverseAnimation(27), playerObject);
				}
			}
		}
	}
	return foundTarget;
}



//=============================================================================
//
//	Target defender if within max range.
//  Range is defined as what we can fight in worst case. (stalkerDistanceMax + weaponMaxRange)
//	Return true if target set.
//

bool AttackableCreature::setTargetDefenderWithinMaxRange(void)
{
	bool foundTarget = false;

	ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
	while (defenderIt != this->getDefenders()->end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			if (!defenderCreature->isIncapacitated() && !defenderCreature->isDead())
			{
				if (gWorldManager->objectsInRange(this->getHomePosition(), this->getCellForSpawn(), *defenderIt, 
												  this->getStalkerDistanceMax() + this->getWeaponMaxRange()))		
				{
					// Do only attack objects that have build up enough aggro.
					if (this->attackerHaveAggro(defenderCreature->getId()))
					{
						// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinMaxRange() have aggro!", MSG_NORMAL);

						// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinMaxRange()", MSG_NORMAL);
						if (this->getTargetId() != (*defenderIt))
						{
							this->setTarget(*defenderIt);
							gMessageLib->sendTargetUpdateDeltasCreo6(this);
						}
						foundTarget = true;
						break;
					}
					else
					{
						// gLogger->logMsgF("AttackableCreature::setTargetDefenderWithinMaxRange() have NO aggro!", MSG_NORMAL);
					}
				}
			}
		}
		++defenderIt;
	}
	return foundTarget;
}

//=============================================================================
//
//	Return true if object is within our max range.
//

bool AttackableCreature::isTargetWithinMaxRange(uint64 targetId)
{
	bool foundTarget = false;

	if (CreatureObject* creature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(targetId)))
	{
		if (!creature->isIncapacitated() && !creature->isDead())
		{
			if (gWorldManager->objectsInRange(this->getHomePosition(), this->getCellForSpawn(), targetId, 
											  this->getStalkerDistanceMax() + this->getWeaponMaxRange()))		
			{
				// gLogger->logMsgF("AttackableCreature::isTargetWithinMaxRange()", MSG_NORMAL);
				foundTarget = true;
			}
		}
	}
	// For debug.
	if (foundTarget)
	{
		// gLogger->logMsgF("AttackableCreature::isTargetWithinMaxRange() TRUE", MSG_NORMAL);
	}
	else
	{
		// gLogger->logMsgF("AttackableCreature::isTargetWithinMaxRange() FALSE", MSG_NORMAL);
	}
	return foundTarget;
}



//=============================================================================
//
//	Verfify current target.
//	Return true if target exist.
//
/*
bool AttackableCreature::isTargetValid(void)
{
	bool foundTarget = false;

	ObjectList::iterator defenderIt = this->getDefenders()->begin();
	while (defenderIt != this->getDefenders()->end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(*defenderIt))
		{
			// if (!defenderCreature->isIncapacitated() && !defenderCreature->isDead())
			{
				PlayerObjectSet* knownPlayers = this->getKnownPlayers();
				PlayerObjectSet::iterator it = knownPlayers->begin();
				while (it != knownPlayers->end())
				{
					// if (!(*it)->isIncapacitated() && !(*it)->isDead())
					if ((*defenderIt) == (*it))	// Is the defender in the known list of objects?
					{
						// Is this the defender I have targeted?
						if (this->getTarget() && (*defenderIt) == this->getTarget())
						{
							// NOTE: STILL UNSAFE CODE with dangling pointers...
							if ((*defenderIt)->getId() == this->getTargetId())
							{
								// gLogger->logMsgF("AttackableCreature::checkCurrentTarget() Npc target validated OK.", MSG_NORMAL);
								foundTarget = true;
								break;
							}
						}
					}
					++it;
				}
				if (foundTarget)
				{
					break;
				}
			}
		}
		++defenderIt;
	}

	if (!foundTarget)
	{
		// We lost our target.
		// this->setTarget(NULL);
		// gMessageLib->sendTargetUpdateDeltasCreo6(this);

		// gLogger->logMsgF("AttackableCreature::checkCurrentTarget() Npc have lost it's target.", MSG_NORMAL);
	}
	return foundTarget;
}
*/

bool AttackableCreature::isTargetValid(void)
{
	bool foundTarget = false;

	if (this->getTarget())
	{
		// Is this target a valid player?
		PlayerObjectSet* knownPlayers = this->getKnownPlayers();
		PlayerObjectSet::iterator it = knownPlayers->begin();
		while (it != knownPlayers->end())
		{
			if (this->getTarget() == (*it))	// Is the target in the known list of objects?
			{
				if (!(*it)->isDead() && !(*it)->isIncapacitated())
				{
					// gLogger->logMsgF("AttackableCreature::checkCurrentTarget() Npc target validated OK.", MSG_NORMAL);
					foundTarget = true;
				}
				break;
			}
			++it;
		}
	}
	return foundTarget;
}




//=============================================================================
//
//	Get defender out of range.
//	Return untargeted defender Id, or 0.
//


uint64 AttackableCreature::getDefenderOutOfAggroRange(void)
{
	uint64 targetOutOfRange = 0;

	ObjectIDList::iterator defenderIt = this->getDefenders()->begin();
	while (defenderIt != this->getDefenders()->end())
	{
		if (CreatureObject* defenderCreature = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById((*defenderIt))))
		{
			// if (!defenderCreature->isIncapacitated() && !defenderCreature->isDead())
			if (!defenderCreature->isDead())
			{
				if (defenderCreature->isIncapacitated() || 
				   (!gWorldManager->objectsInRange(this->getId(), defenderCreature->getId(), this->getMaxAggroRange())))
				{
					// gLogger->logMsgF("AttackableCreature::getDefenderOutOfAggroRange()", MSG_NORMAL);
					targetOutOfRange = defenderCreature->getId();
					break;
				}
			}
		}
		++defenderIt;
	}
	return targetOutOfRange;
}


bool AttackableCreature::isTargetWithinWeaponRange(void) const
{
	bool inRange = false;
	if (CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(this->getTarget()))
	{
		inRange = gWorldManager->objectsInRange(this->getId(), targetCreature->getId(), this->getWeaponMaxRange());	
	}
	/*
	if (inRange)
	{
		gLogger->logMsgF("AttackableCreature::isTargetWithinWeaponRange() Target is within range", MSG_NORMAL);
	}
	else
	{
		gLogger->logMsgF("AttackableCreature::isTargetWithinWeaponRange() Target out of range", MSG_NORMAL);
	}
	*/
	return inRange;
}



//=============================================================================
//
//	Track all that do damage to me.
//

void AttackableCreature::updateDamage(uint64 playerId, uint64 groupId, uint32 weaponGroup, int32 damage, uint8 attackerPosture, float attackerDistance)
{
	// gLogger->logMsgF("AttackableCreature::updateDamage() %d", MSG_NORMAL, damage);

	// Players alone, player in group A, same player in group B, another player in group A...
	DamageDealer* damageDealer = NULL;
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (((*it)->mPlayerId == playerId) && ((*it)->mGroupId == groupId))
		{
			// Already have this attacker in list. Let's update damage and weapon usage.
			(*it)->mDamage += damage;
			(*it)->mWeaponGroupMasks |= weaponGroup;
			damageDealer = (*it);
			break;
		}
		it++;
	}
	
	if (it == mDamageDealers.end())
	{
		// Add this attacker.
		// gLogger->logMsgF("New damage dealer, id = %llu, group %llu", MSG_NORMAL, playerId, groupId);
		damageDealer = new DamageDealer();
		damageDealer->mPlayerId = playerId;
		damageDealer->mGroupId = groupId;
		damageDealer->mWeaponGroupMasks = weaponGroup;
		damageDealer->mDamage = damage;
		damageDealer->mAggroPoints = 0;
		mDamageDealers.push_back(damageDealer);
	}

	// TODO: A real handler that takes care of player posture (and maybe target posture too) and distance to target.
	// If attacker are within creature aggro range, give max aggro whatever attacker posture.
	assert(damageDealer);
	if (damageDealer->mAggroPoints < 10.1)
	{
		float aggroPoints = 4;		// This is max aggro you can get at one single occation when doing damage and be in range of the creature aggro.

		if (attackerDistance > this->getAttackRange())
		{
			if (attackerPosture == CreaturePosture_Prone)
			{
				aggroPoints = 2.5;
			}
			if (attackerDistance < 64.0)
			{
				if (attackerDistance < 32.0)
				{
					aggroPoints *= 2;
				}
				else
				{
					assert(attackerDistance != 0);
					aggroPoints *= (64.0 / attackerDistance);
				}
			}
		}
		damageDealer->mAggroPoints += aggroPoints;
		// gLogger->logMsgF("AttackableCreature::updateDamage() Damage=%d aggro = %.0f total aggro = %.0f", MSG_NORMAL, damage, aggroPoints, damageDealer->mAggroPoints);
	}
}

//=============================================================================
//
//	Track and aggro made.
//

void AttackableCreature::updateAggro(uint64 playerId, uint64 groupId, uint8 attackerPosture)
{
	// gLogger->logMsgF("AttackableCreature::updateAggro()", MSG_NORMAL);

	DamageDealer* damageDealer = NULL;
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (((*it)->mPlayerId == playerId) && ((*it)->mGroupId == groupId))
		{
			// Already have this attacker in list. Let's update damage and weapon usage.
			// gLogger->logMsgF("Already have this attacker in list.", MSG_NORMAL);
			damageDealer = (*it);
			break;
		}
		it++;
	}
	
	if (it == mDamageDealers.end())
	{
		// Add aggro for this attacker.
		// gLogger->logMsgF("New aggro, id = %llu, group %llu", MSG_NORMAL, playerId, groupId);
		damageDealer = new DamageDealer();
		damageDealer->mPlayerId = playerId;
		damageDealer->mGroupId = groupId;
		damageDealer->mWeaponGroupMasks = 0;
		damageDealer->mDamage = 0;
		damageDealer->mAggroPoints = 0;
		mDamageDealers.push_back(damageDealer);
	}

	assert(damageDealer);
	if ((attackerPosture == CreaturePosture_Incapacitated) || (attackerPosture == CreaturePosture_Dead))
	{
		// gLogger->logMsgF("Removed aggro, targer is incapped or dead.", MSG_NORMAL);
		damageDealer->mAggroPoints = 0;

		// Let's this player rebuild is aggro before we attack him again.
		if (this->mLairNeedAsssistanceWithId == playerId)
		{
			this->mAsssistanceNeededWithId = 0;
			this->mLairNeedAsssistanceWithId = 0;
			this->mIsAssistingLair = false;
		}
	}
	else
	{
		if (damageDealer->mAggroPoints < 10.1)
		{
			float aggroPoints = 3.0;		// This is max aggro you can get at one single occation by just be in range of the creature aggro.
			if (attackerPosture == CreaturePosture_Prone)
			{
				aggroPoints = 1.5;
			}
			damageDealer->mAggroPoints += aggroPoints;
		}
	}
}

//=============================================================================
//
//	Attacker got aggro?
//

bool AttackableCreature::attackerHaveAggro(uint64 attackerId)
{
	bool aggro = false;
	float aggroPoints = 0;

	// Check all occurnaces, since switching between groups does not matter here.
	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		if (attackerId == (*it)->mPlayerId)
		{
			aggroPoints += (*it)->mAggroPoints;
		}
		if (aggroPoints > 10)
		{
			aggro = true;
			break;
		}
		it++;
	}
	/*
	if (aggro)
	{
		gLogger->logMsgF("AttackableCreature::attackerHaveAggro = TRUE", MSG_NORMAL);
	}
	else
	{
		gLogger->logMsgF("AttackableCreature::attackerHaveAggro = FALSE", MSG_NORMAL);
	}
	*/
	return aggro;
}



//=============================================================================
//
//	Grant XP to the ones that deserve it.
//

void AttackableCreature::updateAttackersXp(void)
{
	// gLogger->logMsgF("AttackableCreature::updateAttackersXp() Entering", MSG_NORMAL);

	// First we need to figure out who has done most damage. 
	
	// It can be an individual player or a group of players.
	// If a player has made damage before or after he was a group member, that damage will not count into the group damage.
	// It will be added to his damage as an individual player.

	// Weapons used by a player before or after he was a group member, does not give XP if a group has done the most damage.
	// It will give him XP if he as an individual player has done the most damage.

	// You will only be given XP for the weapon types you used in the group, given that the group has done the most damage.

	// Start with the easy part, get the individual players that have done the most damage.
	int32 topDamageByPlayer = 0;
	uint64 topPlayerId = 0;
	int32 topPlayerWeaponMasks = 0;

	DamageDealers::iterator it = mDamageDealers.begin();
	while (it != mDamageDealers.end())
	{
		// gLogger->logMsgF("Handling Player id %llu, group %llu, damage %u", MSG_NORMAL, (*it)->mPlayerId, (*it)->mGroupId, (*it)->mDamage);
		if ((*it)->mGroupId == 0)
		{
			// gLogger->logMsgF("Player %llu hit with %u", MSG_NORMAL, (*it)->mPlayerId, (*it)->mDamage);
			if ((*it)->mDamage >= topDamageByPlayer)
			{
				topDamageByPlayer = (*it)->mDamage;
				topPlayerId = (*it)->mPlayerId;
				topPlayerWeaponMasks = (*it)->mWeaponGroupMasks;
			}
		}
		else
		{
			// This was damage dealt with someone grouped.
			updateGroupDamage(*it);
		}
		it++;
	}

	int32 topDamageByGroup = 0;
	uint64 groupTopId = 0;

	// We have to go through the group list now, and find out the group doing the most damage.
	DamageDealers::iterator groupIt = mDamageByGroups.begin();
	while (groupIt != mDamageByGroups.end())
	{
		// gLogger->logMsgF("Group %llu hit with %u", MSG_NORMAL, (*groupIt)->mGroupId, (*groupIt)->mDamage);
		if ((*groupIt)->mDamage >= topDamageByGroup)
		{
			topDamageByGroup = (*groupIt)->mDamage;
			groupTopId = (*groupIt)->mGroupId;
		}
		groupIt++;
	}

	// The weapon xp will be dived equally between your weapons used, 
	// (and the rest is a comlete made up rule... )
	// mapped to the average HAM value of this creature, lol.

	// TODO: Use real data for the experiance gained.
	// int32 damageDone = this->getHam()->mHealth.getMaxHitPoints();
	// damageDone += this->getHam()->mAction.getMaxHitPoints();
	// damageDone += this->getHam()->mMind.getMaxHitPoints();
	// damageDone = damageDone/6;	// Use half of average HAM.

	int32 weaponXp = this->getWeaponXp();

	// And the winner is....
	if (topDamageByPlayer >= topDamageByGroup)
	{
		// gLogger->logMsgF("Player %llu did the most damage", MSG_NORMAL, topPlayerId);
		// An individual player did the most damage.
		this->updateAttackersWeaponAndCombatXp(topPlayerId, 0, weaponXp, topPlayerWeaponMasks);
		this->mLootAllowedById = topPlayerId;
	}
	else
	{
		// gLogger->logMsgF("Group %llu did the most damage", MSG_NORMAL, groupTopId);
		// The players in this group have to divide the XP, any group bonuses are not implemented yet.
		this->mLootAllowedById = groupTopId;

		// But first, we need to find out how many players we are (have been) in this group during the fight that have done any damage.
		uint32 membersInGroup = 0;
		it = mDamageDealers.begin();
		while (it != mDamageDealers.end())
		{
			if ((*it)->mGroupId == groupTopId)
			{
				// We have found a player belonging to this winning group.
				membersInGroup++;
			}
			it++;
		}

		// Now let's update the players beloning to this group, if still exist, and if players are in range.
		if (membersInGroup != 0)
		{
			it = mDamageDealers.begin();
			while (it != mDamageDealers.end())
			{
				if ((*it)->mGroupId == groupTopId)
				{
					// We have found a player belonging to this winning group.
					this->updateAttackersWeaponAndCombatXp((*it)->mPlayerId, groupTopId, weaponXp/membersInGroup, (*it)->mWeaponGroupMasks);
				}
				it++;
			}
		}
	}
}

//=============================================================================
//
//	Track all that do damage to me.
//

void AttackableCreature::updateGroupDamage(DamageDealer* damageDealer)
{
	DamageDealers::iterator it = mDamageByGroups.begin();
	while (it != mDamageByGroups.end())
	{
		// gLogger->logMsgF("Comparing group %llu with existing group %llu", MSG_NORMAL, (*it)->mGroupId, damageDealer->mGroupId);
		if ((*it)->mGroupId == damageDealer->mGroupId)
		{
			// gLogger->logMsgF("Adding group %llu with damage %u", MSG_NORMAL, damageDealer->mGroupId, damageDealer->mDamage);
			// Already have this group as an attacker in list. Let's update damage.
			(*it)->mDamage += damageDealer->mDamage;
			break;
		}
		it++;
	}
	
	if (it == mDamageByGroups.end())
	{
		// gLogger->logMsgF("New group as damage dealer, group = %llu, damage = %u", MSG_NORMAL, damageDealer->mGroupId, damageDealer->mDamage);
		// Add this attacker.
		DamageDealer* groupDamageDealer = new DamageDealer();
		groupDamageDealer->mGroupId = damageDealer->mGroupId;
		groupDamageDealer->mDamage = damageDealer->mDamage;
		mDamageByGroups.push_back(groupDamageDealer);
	}
}


//=============================================================================
//
//	Update attacker
//
void AttackableCreature::updateAttackersWeaponAndCombatXp(uint64 playerId, uint64 groupId, int32 damageDone, int32 weaponUsedMask)
{
	// Combat XP is one tenth of the total weapon XP.
	int32 combatXp = damageDone/10;
	uint32 noOfWeaponsUsed = 0;

	// Now we need to figure out how many weapons that was involved in this mess.
	for (uint32 i = WeaponGroup_Unarmed; i < WeaponGroup_Flamethrower; i = i << 1)
	{
		if ((i & weaponUsedMask) ==  i)
		{
			// We got a weapon type.
			noOfWeaponsUsed++;
		}
	}

	// Here is the XP to grant for each type of damage done.
	if (noOfWeaponsUsed)	
	{
		int32 weaponXp = damageDone/(noOfWeaponsUsed);

		if (PlayerObject* playerObject = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(playerId)))
		{
			// Only update XP for players in range and still in group, if specified.
			if (playerObject && this->checkKnownPlayer(playerObject))
			{
				bool valid = true;
				if (groupId)
				{
					valid = (playerObject->getGroupId() == (groupId));
				}
				if (valid)
				{
					this->sendAttackersWeaponXp(playerObject, weaponUsedMask, weaponXp);
					gSkillManager->addExperience(XpType_combat_general, combatXp, playerObject);
				}
			}
		}
	}
}


//=============================================================================
//
//	Update attackers XP.
//

void AttackableCreature::sendAttackersWeaponXp(PlayerObject* playerObject, uint32 weaponMask, int32 xp)
{
	// Valid input?
	if (playerObject && weaponMask && xp)
	{
		// Then, we have to have a connected player..still alive.
		if (playerObject->isConnected() && !playerObject->isDead())
		{
			// Now send the XP update messages to player.
			for (uint32 i = WeaponGroup_Unarmed; i < WeaponGroup_Flamethrower; i = i << 1)
			{
				switch (i & weaponMask)
				{
					case WeaponGroup_Unarmed:
						gSkillManager->addExperience(XpType_combat_meleespecialize_unarmed, xp, playerObject);
						break;

					case WeaponGroup_1h:
						gSkillManager->addExperience(XpType_combat_meleespecialize_onehand, xp, playerObject);
						break;

					case WeaponGroup_2h:
						gSkillManager->addExperience(XpType_combat_meleespecialize_twohand, xp, playerObject);
						break;

					case WeaponGroup_Polearm:
						gSkillManager->addExperience(XpType_combat_meleespecialize_polearm, xp, playerObject);
						break;

					case WeaponGroup_Rifle:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_rifle, xp, playerObject);
						break;

					case WeaponGroup_Pistol:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_pistol, xp, playerObject);
						break;

					case WeaponGroup_Carbine:
						gSkillManager->addExperience(XpType_combat_rangedspecialize_carbine, xp, playerObject);
						break;

					default:
						break;
				}
			}
		}
	}
}

//=============================================================================
//
//	Returns true if target is allowed to loot.
//

bool AttackableCreature::allowedToLoot(uint64 targetId, uint64 groupId)
{
	return ((targetId == mLootAllowedById) || (groupId == mLootAllowedById));
}


void AttackableCreature::equipPrimaryWeapon(void)
{
	if (mPrimaryWeapon)
	{
		this->mEquipManager.removeEquippedObject(CreatureEquipSlot_Weapon);

		// if (!this->getEquipManager()->addEquippedObject(mPrimaryWeapon))
		if (this->mEquipManager.addEquippedObject(CreatureEquipSlot_Weapon, mPrimaryWeapon))
		{
			mPrimaryWeapon->setParentId(this->getId());
			gMessageLib->sendEquippedListUpdate_InRange(this);
			gMessageLib->sendWeaponIdUpdate(this);

			// gLogger->logMsgF("AttackableCreature::equipPrimaryWeapon() Equipping the primary weapon", MSG_NORMAL);

		/*
			mPrimaryWeapon->setInternalAttribute("equipped","1");
			gMessageLib->sendEquippedListUpdate_InRange(this);

			PlayerObjectSet* inRangePlayers = this->getKnownPlayers();
			PlayerObjectSet::iterator it = inRangePlayers->begin();

			while(it != inRangePlayers->end())
			{
				PlayerObject* targetObject = (*it);
				gMessageLib->sendBaselinesCREO_6(this,targetObject);
				// gMessageLib->sendCreateTangible(mPrimaryWeapon,targetObject);
				++it;
			}

			// weapon update
			gMessageLib->sendWeaponIdUpdate(this);
			gMessageLib->sendEquippedListUpdate_InRange(this);
			*/
		}
		else
		{
			gLogger->logMsgF("AttackableCreature::equipPrimaryWeapon() Can't equip primary weapon\n", MSG_NORMAL);
		}
	}
}	


void AttackableCreature::equipSecondaryWeapon(void)
{
	if (mSecondaryWeapon)
	{
		if (this->mEquipManager.addEquippedObject(CreatureEquipSlot_Weapon, mSecondaryWeapon))
		{
			gMessageLib->sendEquippedListUpdate_InRange(this);
			gMessageLib->sendWeaponIdUpdate(this);
		}
		else
		{
			gLogger->logMsgF("AttackableCreature::equipWeapon() Can't equip secondary weapon\n", MSG_NORMAL);
		}
	}
}	

void AttackableCreature::unequipWeapon(void)
{
	Weapon* weapon = dynamic_cast<Weapon*>(this->getEquipManager()->getEquippedObject(CreatureEquipSlot_Weapon));
	if (weapon)
	{
		this->mEquipManager.removeEquippedObject(CreatureEquipSlot_Weapon);

		gMessageLib->sendContainmentMessage_InRange(weapon->getId(), this->getId(), 0xffffffff, this);
		gMessageLib->sendDestroyObject(weapon->getId(), this);
		gMessageLib->sendEquippedListUpdate_InRange(this);

		// The weapon is now owned by the npc inventory. But we have not put it there, yet.
		// In fact, we keep these npc-weapons outside inventory, until we need to loot the them, 
		// they are setup as "default weopons" during npc creation.
		/*
		Inventory* inventory = dynamic_cast<Inventory*>(this->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
		if (inventory)
		{
			weapon->setParentId(inventory->getId());
		}
		*/
	}

	// if weapon slot is empty, equip the unarmed default weapon
	if (!this->mEquipManager.getEquippedObject(CreatureEquipSlot_Weapon))
	{
		// gLogger->logMsg("equip default weapon");
		this->mEquipManager.equipDefaultWeapon();
		gMessageLib->sendEquippedListUpdate_InRange(this);
		gMessageLib->sendWeaponIdUpdate(this);

		// TEST
		// gMessageLib->sendCreateCreature(this,defenderPlayer);
	}
}


//=============================================================================
//
//	Get respawn data.
//

const SpawnData& AttackableCreature::getSpawnData(void) const
{
	return mSpawn;
}

//=============================================================================
//
//	Set respawn data.
//

void AttackableCreature::setSpawnData(const SpawnData *spawn)
{
	mSpawn = spawn;
}

void AttackableCreature::setSpawnData(const SpawnData &spawn)
{
	mSpawn = spawn;
}

void AttackableCreature::handleEvents(void)
{
	// General issues like life and death first.
	if (this->isDead())
	{
		mCombatState = State_Dead;

		if (!gWorldConfig->isTutorial())
		{
			// make a final position update, reading the heightmap, since we may have been "on the move" and y-axis is not correct.
			Anh_Math::Vector3 newPosition(this->mPosition);

			newPosition.mY = getHeightAt2DPosition(newPosition.mX, newPosition.mZ);
			this->updatePosition(this->getParentId(), newPosition);
		}
		return;
	}

	switch (mCombatState)
	{
		case State_Unspawned:
		{
			// Let's get this object into the world.
			if (!this->isSpawned())
			{
				this->spawn();
			}

			if (this->getKnownPlayers()->empty() || !this->isAgressiveMode())
			{
				// We will not start roaming or do any action if there are no one near us watching. Creatures here have a big ego. :)
				mCombatState = State_Idle;
				this->setAiState(NpcIsDormant);
			}
			else
			{
				// mCombatState = State_Idle;
				// this->setAiState(NpcIsReady);

				// We have got some company.
				mCombatState = State_Alerted;
				this->setAiState(NpcIsReady);

				// Setup roaming, if any.
				if (this->isRoaming())
				{
					this->setupRoaming(15, 15);
				}
			}
		}
		break;

		case State_Idle:
		{	
			// gLogger->logMsgF("State_Idle ", MSG_NORMAL);

			if (!this->getKnownPlayers()->empty())
			{
				// We have got some company.
				mCombatState = State_Alerted;
				this->setAiState(NpcIsReady);

				// Setup roaming, if any.
				if (this->isRoaming())
				{
					// Setup a delay, we do not want all npc to start raom at the same time when a player enters a dormant area.
					// Make the base delay time shorter than normal case, since we can assume we have been dormant.
					uint64 roamingPeriods = this->getRoamingPeriodTime() / ((uint32)readyDefaultPeriodTime);
					// roamingReadyTicksDelay = (int64)((int64)roamingPeriods + gRandom->getRand() % (int32) (roamingPeriods/2));
					int64 roamingReadyTicksDelay = (int64)((int64)(roamingPeriods/2) + gRandom->getRand() % (int32) (roamingPeriods));
					// gLogger->logMsgF("Will wait for %llu seconds", MSG_NORMAL, (uint64)((readyDefaultPeriodTime * roamingReadyTicksDelay)/1000));
					this->SetReadyDelay(roamingReadyTicksDelay);

					// this->setupRoaming(15, 15);
				}
			}
		}
		break;

		case State_Alerted:
		{
			// gLogger->logMsgF("State_Alerted", MSG_NORMAL);

			if (this->getKnownPlayers()->empty())
			{
				mCombatState = State_Idle;
				this->setAiState(NpcIsDormant);
			}
			else
			{
				// Any attacked us?
				if (this->setTargetDefenderWithinWeaponRange())
				{
					// Yes.
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				// Any new target in range we can aggro??
				else if (this->setTargetInAttackRange())		// Any player within attack range when in aggressiveMode.
				{
					// Yes.
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				// Anyone need our help?
				if (this->needToAssistLair())
				{
					this->executeLairAssist(); 
				}
				else if (this->needAssist())
				{
					this->executeAssist();
				}
				// Any target we can rush into?
				else if (this->setTargetDefenderWithinMaxRange())	// Defenders within max range (stalkerDistanceMax + weaponMaxRange)
				{
					// Yes.
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				else if (this->getDefenders()->size() != 0)
				{
					mCombatState = State_CombatReady;
					this->setAiState(NpcIsReady);
				}
			}
		}
		break;

		case State_CombatReady:
		{	
			// gLogger->logMsgF("State_CombatReady", MSG_NORMAL);
			// 
			if (this->getKnownPlayers()->empty())
			{
				if (this->insideRoamingLimit())
				{
					mCombatState = State_Idle;
					this->setAiState(NpcIsDormant);
				}
				else
				{
					this->enableHoming();
					this->SetReadyDelay(1);	// Want to start the homing asap.
					this->setupRoaming(15, 15);
				}
				// End all combat.
			}
			else if (!this->isTargetValid())
			{
				// We lost our target.
				this->setTarget(NULL);
				gMessageLib->sendTargetUpdateDeltasCreo6(this);

				// gLogger->logMsgF("AttackableCreature::handleEvents Lost target", MSG_HIGH);
				mCombatState = State_Alerted;
				this->setAiState(NpcIsReady);

				this->mAsssistanceNeededWithId = 0;
				this->mLairNeedAsssistanceWithId = 0;
				this->mIsAssistingLair = false;
			}

			if (this->needToAssistLair())
			{
				this->executeLairAssist(); 
			}
			else if (this->isTargetWithinWeaponRange())
			{
				mCombatState = State_Combat;
				this->setAiState(NpcIsActive);

				// We may need to chase the target.
				this->setupStalking(activeDefaultPeriodTime);
			}
			else if (this->isHoming())
			{
				if (this->needToAssistLair())
				{
					this->executeLairAssist(); 
				}
				// Continue until I'm home.
				// Anyone need our help?
				else if (this->needAssist())
				{
					this->executeAssist();
				}
			}
			else
			{
				// Any attacked us?
				if (this->setTargetDefenderWithinWeaponRange())
				{
					// Yes.
					// gLogger->logMsgF("State_CombatReady: setTargetDefenderWithinWeaponRange", MSG_NORMAL);
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				// Any new target in range?
				else if (this->setTargetInAttackRange())
				{
					// gLogger->logMsgF("State_CombatReady: setTargetInAttackRange", MSG_NORMAL);
					// Yes.
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				// Any active target we can rush into?
				else if (this->setTargetDefenderWithinMaxRange())	// Defenders within max range (stalkerDistanceMax + weaponMaxRange)
				{
					// gLogger->logMsgF("State_CombatReady: setTargetDefenderWithinMaxRange", MSG_NORMAL);
					// Yes.
					mCombatState = State_Combat;
					this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				else if (this->getDefenders()->size() == 0)
				{
					mCombatState = State_Alerted;
					this->setAiState(NpcIsReady);
				}
				else if (!this->isHoming())
				{
					if (isTargetValid())
					{
						// gLogger->logMsgF("State_CombatReady We have a target", MSG_NORMAL);
						if (this->targetOutsideRoamingLimit())
						{
							// gLogger->logMsgF("State_CombatReady Target outside roaming limit", MSG_NORMAL);
							// Only attempt to "walk away" if we are a stalker.
							if (this->isStalker())
							{
								this->enableHoming();
								this->SetReadyDelay(1);	// Want to start the homing asap.
								this->setupRoaming(15, 15);
							}
						}
						else
						{
							// gLogger->logMsgF("State_CombatReady Target inside roaming limit", MSG_NORMAL);
						}
					}
					else
					{
						// We lost our target.
						this->setTarget(NULL);
						gMessageLib->sendTargetUpdateDeltasCreo6(this);
					}
				}
				else
				{
					// gLogger->logMsgF("State_CombatReady Already homing", MSG_NORMAL);
				}
			}
		}
		break;

		case State_Combat:
		{
			// gLogger->logMsgF("AttackableCreature::handleEvents: State_Combat", MSG_NORMAL);

			if (!this->isTargetValid())
			{
				// We lost our target.
				this->setTarget(NULL);
				gMessageLib->sendTargetUpdateDeltasCreo6(this);

				// gLogger->logMsgF("AttackableCreature::handleEvents Lost target", MSG_HIGH);
				mCombatState = State_CombatReady;
				this->setAiState(NpcIsReady);

				this->mAsssistanceNeededWithId = 0;
				this->mLairNeedAsssistanceWithId = 0;
				this->mIsAssistingLair = false;

				this->SetReadyDelay(1);	// Want to start the homing asap, if nothing better to do.
				this->setupRoaming(15, 15);
			}
			else if (this->needToAssistLair())
			{
				this->executeLairAssist(); 
			}
			else if (!this->checkState((CreatureState)(CreatureState_Combat + CreatureState_CombatAttitudeNormal)))
			{
				// We are not in combat.
				// We may be stalking a target....
				// But.. if someone attack us, we prio to defence ourself.
				// Anyone attacked us?
				if (this->setTargetDefenderWithinWeaponRange())
				{
					// gLogger->logMsgF("State_Combat: I am attacked, going to get that bastard.", MSG_NORMAL);
					// Yes.
					// mCombatState = State_Combat;
					// this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				// Any new target in range we can aggro??
				else if (this->setTargetInAttackRange())		// Any player within attack range when in aggressiveMode.
				{
					// gLogger->logMsgF("State_Combat: Found a new target", MSG_NORMAL);
					// Yes.
					// mCombatState = State_Combat;
					// this->setAiState(NpcIsActive);

					// We may need to chase the target.
					this->setupStalking(activeDefaultPeriodTime);
				}
				else if (this->atStalkLimit())
				{
					// gLogger->logMsgF("State_Combat: Start Homing", MSG_NORMAL);
					// We are at max limit and target is out of range. Let's go home.
					this->enableHoming();
					this->SetReadyDelay(1);	// Want to start the homing asap.
					this->setupRoaming(15, 15);

					// We drop the target, it's out of range.
					this->setTarget(NULL);
					gMessageLib->sendTargetUpdateDeltasCreo6(this);

					mCombatState = State_CombatReady;
					this->setAiState(NpcIsReady);

					// Clear the current assist target.
					this->mAsssistanceNeededWithId = 0;
					this->mLairNeedAsssistanceWithId = 0;
					this->mIsAssistingLair = false;

				}
				// else
				//{
				//	// gLogger->logMsgF("State_Combat: Hunt him down", MSG_NORMAL);
				//	// Hunt him down.
				//	this->setupStalking(activeDefaultPeriodTime);
				//}

				// gLogger->logMsgF("AttackableCreature::handleEvents: State_Combat: We are not in combat", MSG_NORMAL);
			}
			else
			{
				// Clear the current assist target.
				this->mAsssistanceNeededWithId = 0;
				// this->mLairNeedAsssistanceWithId = 0;

				CreatureObject* targetCreature = dynamic_cast<CreatureObject*>(this->getTarget());
				if (targetCreature)
				{
					if (targetCreature->isIncapacitated())
					{
						// Target is incapped, leave him alone, try to find a new target.
						// gLogger->logMsgF("AttackableCreature::handleEvents Target incapped", MSG_HIGH);
						mCombatState = State_CombatReady;
						this->setAiState(NpcIsReady);
					}
					else if (targetCreature->isDead())
					{
						// Target is dead, try to find a new target.
						// gLogger->logMsgF("AttackableCreature::handleEvents Target dead", MSG_HIGH);
						mCombatState = State_CombatReady;
						this->setAiState(NpcIsReady);
					}
					else if (!this->isTargetWithinWeaponRange() && (!this->isStalker()))
					{
						// Target is out of range, try to find a new target.
						// gLogger->logMsgF("AttackableCreature::handleEvents Target out of range.", MSG_HIGH);
						mCombatState = State_CombatReady;
						this->setAiState(NpcIsReady);
					}
					else if (this->isTargetWithinWeaponRange())
					{
						// gLogger->logMsgF("State_Combat: Target within weapon range.", MSG_NORMAL);
					}
					else
					{
						// gLogger->logMsgF("State_Combat Stalking", MSG_NORMAL);
						if (this->atStalkLimit())
						{
							// gLogger->logMsgF("State_Combat: Start Homing", MSG_NORMAL);
							// We are at max limit and target is out of range. Let's go home.
							this->enableHoming();
							this->SetReadyDelay(1);	// Want to start the homing asap.
							this->setupRoaming(15, 15);
							mCombatState = State_CombatReady;
							this->setAiState(NpcIsReady);
						}
						else
						{
							// gLogger->logMsgF("State_Combat: Hunt him down", MSG_NORMAL);
							// Hunt him down.
							this->setupStalking(activeDefaultPeriodTime);
						}
					}
				}
				else
				{
					// Invalid target.
					mCombatState = State_CombatReady;
					this->setAiState(NpcIsReady);

					// gLogger->logMsgF("AttackableCreature::handleEvents Invalid target", MSG_HIGH);
					assert(false);
				}
			}
		}
		break;

		case State_Halted:
		{
			// Do nothing. Mainly used by tutorial when getting the "bandit" to hold it's fighting animations.
		}
		break;

		default:
		break;
	}
}


uint64 AttackableCreature::handleState(uint64 timeOverdue)
{
	uint64 waitTime = 0;

	switch (mCombatState)
	{
		case State_Dead:
		{
			// Remove us from all timers etc...
			// NpcManager::Instance()->removeNpc(this->getId());
		}
		break;

		case State_Unspawned:
		{
			// This is not a valid state.
			gLogger->logMsgF("AttackableCreature::handleState Invalid state State_Unspawned.\n", MSG_NORMAL);
			// It's a serious isse that we need to investigate.
			assert(false);

			// Remove us from all timers etc...
			// NpcManager::Instance()->removeNpc(this->getId());
		}
		break;

		case State_Idle:
		{
			// Set me for a long wait...

			// Start roaming timer, then we will have them all running when we get players in range.
			waitTime = this->getRoamingPeriodTime() + gRandom->getRand() % (int32) (this->getRoamingPeriodTime()/2);
			// gLogger->logMsgF("State_Idle, will wait for %llu seconds", MSG_NORMAL, waitTime/1000);
		}
		break;

		case State_Alerted:
		{	
			// Hm... check this state fairly often.
			waitTime = (uint64)readyDefaultPeriodTime;

			// Any player that we should give a last warning?
			this->showWarningInRange();

			// Are we supposed to do any roaming?
			if (this->isRoaming())
			{
				// Yes, continue roaming 
				int32 movementCounter = this->getRoamingSteps();
				if (movementCounter > 0)
				{
					// gLogger->logMsgF("State_Alerted Is roaming", MSG_NORMAL);

					// Incremental moves
					movementCounter--;
					if (movementCounter == 0)
					{
						// Do the final move
						// this->mPosition = this->getDestination();
						this->updatePosition(this->getParentId(), this->getDestination());

						// gLogger->logMsgF("State_Alerted Roaming is done, have players present", MSG_NORMAL);
					}
					else
					{
						this->moveAndUpdatePosition();
						// waitTime = 1000;
						if (readyDefaultPeriodTime > (int64)timeOverdue)
						{
							waitTime = (readyDefaultPeriodTime - (int64)timeOverdue);
						}
						else
						{
							waitTime = readyDefaultPeriodTime;
						}
					}
					this->setRoamingSteps(movementCounter);
				}
				else 
				{
					// We are set to roaming, but ends up here when the roaming sequence is done and we have players nearby.
					// gLogger->logMsgF("State_Alerted Attempting to roam", MSG_NORMAL);

					// use a delay before we roam next time. We can't sleep because we have players present.
					int64 roamingReadyTicksDelay = this->getReadyDelay();
					if (roamingReadyTicksDelay > 0)
					{
						// gLogger->logMsgF("State_Alerted roamingReadyTicksDelay = %lld", MSG_NORMAL, roamingReadyTicksDelay);
						roamingReadyTicksDelay--;
						if (roamingReadyTicksDelay == 0)
						{
							// gLogger->logMsgF("State_Alerted Started roaming", MSG_NORMAL);
							
							// Start roaming again.
							setupRoaming(15, 15);
						}
					}
					else
					{
						// Set up a new roaming delay to be used here in this state.
						// gLogger->logMsgF("State_Alerted Setting up new roaming.", MSG_NORMAL);
						uint64 roamingPeriods = this->getRoamingPeriodTime() / ((uint32)readyDefaultPeriodTime);
						roamingReadyTicksDelay = (int64)((int64)roamingPeriods + gRandom->getRand() % (int32) (roamingPeriods/2));
						// gLogger->logMsgF("Will wait for %llu seconds", MSG_NORMAL, (uint64)((readyDefaultPeriodTime * roamingReadyTicksDelay)/1000));
					}
					this->SetReadyDelay(roamingReadyTicksDelay);
				}
			}
		}
		break;

		case State_CombatReady:
		{	
			// Hm... check this state fairly often.
			waitTime = (uint64)readyDefaultPeriodTime;

			// Any player that we should give a last warning?
			this->showWarningInRange();

			// Make peace with players out of range.
			uint64 untargetId;
			if ((untargetId = this->getDefenderOutOfAggroRange()) != 0)
			{
				// Make peace with him.
				this->makePeaceWithDefender(untargetId);
			}

			if (this->isHoming())
			{
				// gLogger->logMsgF("AttackableCreature::handleState State_CombatReady isHoming().", MSG_NORMAL);
				// We are moving home, may still have defenders, but current target was out of range when we hit our max stalking range.
				int32 movementCounter = this->getRoamingSteps();
				if (movementCounter > 0)
				{
					// gLogger->logMsgF("State_CombatReady Homing", MSG_NORMAL);

					// Incremental moves
					movementCounter--;
					if (movementCounter == 0)
					{
						this->disableHoming();

						// Do the final move
						// this->mPosition = this->getDestination();
						this->updatePosition(this->getParentId(), this->getDestination());
					}
					else
					{
						this->moveAndUpdatePosition();
						if (readyDefaultPeriodTime > (int64)timeOverdue)
						{
							waitTime = (readyDefaultPeriodTime - (int64)timeOverdue);
						}
						else
						{
							waitTime = readyDefaultPeriodTime;
						}
					}
					this->setRoamingSteps(movementCounter);
				}
				else
				{
					this->disableHoming();
				}
			}
		}
		break;

		case State_Combat:
		{
			waitTime = (uint64)activeDefaultPeriodTime;

			int64 activation = this->getCombatTimer();
			if (activation < 0)
			{
				// We are ready to strike. If we are/has been outside range, we just arm the attack.
			}
			else
			{
				activation -= (int64)activeDefaultPeriodTime;		// The average period time.
			}
			if (activation < 0)
			{
				// ask for assistance, somebody my show up and help.
				if (LairObject* lair = dynamic_cast<LairObject*>(gWorldManager->getObjectById(this->getLairId())))
				{
					if (this->getTarget()->getId() != this->mAssistedTargetId)
					{
						if (lair->requestAssistance(this->getTarget()->getId(), this->getId()))
						{
							this->mAssistedTargetId = this->getTarget()->getId();
						}
					}
				}

				if (this->isTargetWithinWeaponRange())
				{
					// gLogger->logMsgF("State_Combat: Attacking", MSG_NORMAL);
					activation += this->getAttackSpeed();
					NpcManager::Instance()->handleAttack(this, this->getTarget()->getId());
				}
				else
				{
					// gLogger->logMsgF("State_Combat: Target out of weapon range.", MSG_NORMAL);
				}
			}
			this->setCombatTimer(activation);
		
			if (this->isStalker())
			{
				// gLogger->logMsgF("State_Combat Stalking", MSG_NORMAL);
				this->stalk();
			}
		}
		break;

		case State_Halted:
		{
			// Do nothing. Mainly used by tutorial when getting the "bandit" to hold it's fighting animations.
		}
		break;


		default:
			gLogger->logMsgF("AttackableCreature::handleState Invalid state\n", MSG_NORMAL);
			// waitTime = 0;
		break;
	}

	return waitTime;

}


//=============================================================================
//
//	Spawn.
//

void AttackableCreature::spawn(void)
{
	// gLogger->logMsgF("AttackableCreature::spawnCreature: Attemtp to spawn id = %llu", MSG_NORMAL, this->getId());

	// Update the world about my presence.
	
	this->setSpawned();
	if (this->getParentId())
	{
		// insert into cell
		this->setSubZoneId(0);

		if (CellObject* cell = dynamic_cast<CellObject*>(gWorldManager->getObjectById(this->getParentId())))
		{
			cell->addChild(this);
		}
		else
		{
			gLogger->logMsgF("AttackableCreature::spawnCreature: couldn't find cell %llu\n", MSG_HIGH, this->getParentId());
			
			// It's a serious isse that we need to investigate.
			assert(cell);
		}
	}
	else
	{
		if (QTRegion* region = gWorldManager->getSI()->getQTRegion(this->mPosition.mX, this->mPosition.mZ))
		{
			this->setSubZoneId((uint32)region->getId());
			region->mTree->addObject(this);
		}
	}
	// Sleeping NPC's should be put in lower prio queue.

	if (this->getKnownPlayers()->empty())
		return;

	// Add us to the world.
	gMessageLib->broadcastContainmentMessage(this->getId(),this->getParentId(),4,this);

	// send out position updates to known players
	this->setInMoveCount(this->getInMoveCount() + 1);

	if (this->getParentId())
	{
		// We are inside a cell.
		gMessageLib->sendDataTransformWithParent(this);
		gMessageLib->sendUpdateTransformMessageWithParent(this);
	}
	else
	{
		gMessageLib->sendDataTransform(this);
		gMessageLib->sendUpdateTransformMessage(this);
	}
}

//=============================================================================
// 
//	Setup stalking target sequence.
//

void AttackableCreature::setupStalking(uint64 updatePeriodTime)
{
	// Even if we do not stalk, we need to face the enemy.
	// Get target position.
	if (this->getTarget())
	{
		CreatureObject* attacker = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getTarget()->getId()));
		if (attacker)
		{
			// Update the direction of the npc in the world.
			this->setDirection(attacker->mPosition.mX - this->mPosition.mX, attacker->mPosition.mZ - this->mPosition.mZ);
			if (this->isStalker())
			{
				// gLogger->logMsgF("Npc stalking target %s at %.0f, %.0f, %.0f", MSG_NORMAL, attacker->getFirstName().getAnsi(), destination.mX, destination.mY, destination.mZ);
				// gLogger->logMsgF("AttackableCreature::setupStalking of target %s", MSG_NORMAL, attacker->getFirstName().getAnsi());
				
				this->disableHoming();

				float distanceToMove = this->mPosition.distance2D(attacker->mPosition);
				Anh_Math::Vector3 destination = attacker->mPosition;

				// Save attackers pos, since we may have to redefine the destination in case of out of allowed distance.
				this->mStalkingTargetDestination = attacker->mPosition;

				// float steps = (distanceToMove * 1000)/(this->getStalkerSpeed() * activeDefaultPeriodTime);
				float steps = (distanceToMove * 1000)/(this->getStalkerSpeed() * updatePeriodTime);

				float xOffset = (destination.mX - this->mPosition.mX) / steps;
				float yOffset = (destination.mY - this->mPosition.mY) / steps;
				float zOffset = (destination.mZ - this->mPosition.mZ) / steps;

				// Check if we will end up outside given area.
				// We have to be able to move towards a target outside our stalking range.
				if (attacker->mPosition.distance2D(this->getHomePosition()) >= this->getStalkerDistanceMax())
				{
					// Yes, we have to change the destination.
					// The total distance we may move is from here to the stalker max distance.
					distanceToMove = this->getStalkerDistanceMax() - (this->mPosition.distance2D(this->getHomePosition()));
					// steps = distanceToMove/npc->getStalkerSpeed();
					steps = (distanceToMove * 1000)/(this->getStalkerSpeed() * updatePeriodTime);

					destination = this->mPosition;
					destination.mX = steps * xOffset;
					destination.mY = steps * yOffset;
					destination.mZ = steps * zOffset;
					// gLogger->logMsgF("Npc stalking target %s at LIMIT %.0f, %.0f, %.0f", MSG_NORMAL, attacker->getFirstName().getAnsi(), destination.mX, destination.mY, destination.mZ);
				}
				else
				{
					// gLogger->logMsgF("Npc stalking target %s at %.0f, %.0f, %.0f", MSG_NORMAL, attacker->getFirstName().getAnsi(), destination.mX, destination.mY, destination.mZ);
				}

				this->setDestination(destination);	
				this->setStalkerSteps((int32)steps);

				// Save the offset for each movement request.
				Anh_Math::Vector3 positionOffset(xOffset, yOffset, zOffset);
				this->setPositionOffset(positionOffset);
			}
		}
	}
}

//=============================================================================
// 
//	Setup a new roaming sequence.
//

void AttackableCreature::setupRoaming(int32 maxRangeX, int32 maxRangeZ)
{
	// Get a target position
	Anh_Math::Vector3 destination;

	// If we already outside roaming area, get back home. We may get here when we stalk other objects.
	if (this->mPosition.distance2D(this->getHomePosition()) >= this->getRoamingDistanceMax())
	{
		// gLogger->logMsgF("NPC is outside roaming area, going home.", MSG_NORMAL);
		this->enableHoming();
		this->SetReadyDelay(1);	// Want to start the homing asap.
		destination = this->getRandomPosition(this->getHomePosition(), 2*maxRangeX, 2*maxRangeZ);
	}
	else
	{
		// Verify that we don't roam outside given area.
		destination = this->getRandomPosition(this->mPosition, 2*maxRangeX, 2*maxRangeZ);
		while (this->getHomePosition().distance2D(destination) > this->getRoamingDistanceMax())
		{
			// gLogger->logMsgF("Trying to find a position in range", MSG_NORMAL);
			destination = this->getRandomPosition(this->mPosition, 2*maxRangeX, 2*maxRangeZ);
		}
	}
	// gLogger->logMsgF("Npc going to %.0f, %.0f, %.0f", MSG_NORMAL, destination.mX, destination.mY, destination.mZ);

	this->setDestination(destination);

	// Update the direction of the npc in the world.
	this->setDirection(destination.mX - this->mPosition.mX, destination.mZ - this->mPosition.mZ);

	// How many updates do we have to do before npc is at new target position?
	float distanceToMove = this->mPosition.distance2D(destination);

	// Dirty hack
	float steps;
	if (this->isHoming())
	{
		steps = distanceToMove/(this->getRoamingSpeed() * 3);
	}
	else
	{
		steps = distanceToMove/this->getRoamingSpeed();
	}

	float xOffset = (destination.mX - this->mPosition.mX) / steps;
	float yOffset = (destination.mY - this->mPosition.mY) / steps;
	float zOffset = (destination.mZ - this->mPosition.mZ) / steps;
	
	this->setRoamingSteps((int32)steps);

	// Calculate and save the offset for each movement request.
	Anh_Math::Vector3 positionOffset(xOffset, yOffset, zOffset);
	this->setPositionOffset(positionOffset);
}


bool AttackableCreature::atStalkLimit() const
{
	bool atLimit = false;

	// Are we supposed to do any stalking?
	if (this->isStalker())
	{
		float distanceToStalkerLimit = this->getStalkerDistanceMax() - (this->mPosition.distance2D(this->getHomePosition()));
		if (distanceToStalkerLimit <= 2.0)
		{
			atLimit = true;
		}
	}
	return atLimit;
}


bool AttackableCreature::insideRoamingLimit() const
{
	bool atLimit = false;

	float distance = this->getRoamingDistanceMax() - (this->mPosition.distance2D(this->getHomePosition()));
	if (distance < 0.0)
	{
		atLimit = true;
	}
	return atLimit;
}

bool AttackableCreature::targetOutsideRoamingLimit(void) const
{
	bool atLimit = false;

	float distance = this->getRoamingDistanceMax() - (this->getTarget()->mPosition.distance2D(this->getHomePosition()));
	if (distance < 0)
	{
		atLimit = true;
	}
	return atLimit;
}


//=============================================================================
// 
//	Stalk (follow your target).
//

void AttackableCreature::stalk()
{
	// Are we supposed to do any stalking?
	if (this->isStalker())
	{
		// gLogger->logMsgF("NpcManager::stalk() Stalking.", MSG_NORMAL);
		// Yes.
		CreatureObject* attacker = dynamic_cast<CreatureObject*>(gWorldManager->getObjectById(this->getTarget()->getId()));
		if (attacker)
		{
			// TODO: Only if we have changed more than x meters.

			// This will never be true when we are reaching the stalk limit, since the npc->position is not the attackers position.

			// if ((attacker->mPosition == this->getDestination()) == false)
			if ((attacker->mPosition == this->mStalkingTargetDestination) == false)
			{
				// Our target may have moved...
				// gLogger->logMsgF("NpcManager::stalk() Attacker has moved.", MSG_NORMAL);
				setupStalking(activeDefaultPeriodTime);
			}

			int32 movementCounter = this->getStalkerSteps();
			if (movementCounter > 0)
			{
				// gLogger->logMsgF("NpcManager::stalk() Stalking", MSG_NORMAL);

				// Incremental moves
				movementCounter--;

				if (movementCounter == 0)
				{
					// This our final move.
					// When we attack, we want to stay a little bit from our target. (true until we start using ranged)
					Anh_Math::Vector3 positionOffset = this->getPositionOffset();
					positionOffset.mX /= 1.50;
					positionOffset.mY /= 1.50;
					positionOffset.mZ /= 1.50;

					// Save the offset for each movement request.
					this->setPositionOffset(positionOffset);
				}
				this->moveAndUpdatePosition();
				this->setStalkerSteps(movementCounter);
			}
		}
	}
}

//=============================================================================
// 
//	We have no more defenders.
//	This method should be called as an "event" when we gets compleatly out of combat.
//

void AttackableCreature::inPeace(void)
{
	mWarningTauntSent = false;
	mAttackTauntSent = false;
	this->mAssistedTargetId = 0;
}


bool AttackableCreature::isAttackTauntSent(void) const
{
	return mAttackTauntSent;
}

void AttackableCreature::setAttackTauntSent(void)
{
	mAttackTauntSent = true;
}

bool AttackableCreature::isWarningTauntSent(void) const
{
	return mWarningTauntSent;
}

void AttackableCreature::setWarningTauntSent(void)
{
	mWarningTauntSent = true;
}

void AttackableCreature::clearWarningTauntSent(void)
{
	mWarningTauntSent = false;
}

void AttackableCreature::killEvent(void)
{
	// Are we handled by a lair or someone?
	if (this->getLairId())
	{
		// Yes. Report in as dead!
		if (LairObject* lair = dynamic_cast<LairObject*>(gWorldManager->getObjectById(this->getLairId())))
		{
			// gLogger->logMsgF("AttackableCreature::killEvent: Reporting in as dead %llu", MSG_NORMAL, this->getId());
			lair->reportedDead(this->getId());
		}
		else
		{
			// gLogger->logMsgF("AttackableCreature::killEvent: Lair is gone", MSG_NORMAL);
		}
	}
	else
	{
		if (this->getRespawnPeriod() != 0)
		{
			// gLogger->logMsgF("AttackableCreature::killEvent: Creating a new creature with template = %llu", MSG_NORMAL, this->getTemplateId());

			uint64 npcNewId = gWorldManager->getRandomNpNpcIdSequence();	
			if (npcNewId != 0)
			{
				// Let's put this sucker into play again.
				this->mSpawn.mBasic.timeToFirstSpawn = (uint64)(gRandom->getRand() % (int32)this->mSpawn.mBasic.respawnPeriod);
				NonPersistentNpcFactory::Instance()->requestObject(NpcManager::Instance(), this->getTemplateId(), npcNewId, this->getSpawnData());
			}
		}
	}
}

void AttackableCreature::respawn(void)
{
	// gLogger->logMsgF("AttackableCreature::respawn: Added new creature for spawn, with id = %llu", MSG_NORMAL, this->getId());

	const SpawnData	spawnData(this->getSpawnData());

	this->setParentId(spawnData.mBasic.cellForSpawn);	// The cell we will spawn in.
	// TODO creature->setFirstName((int8*)firstname.c_str());
	// TODO creature->setLastName((int8*)lastname.c_str());
	this->mDirection = spawnData.mBasic.spawnDirection;
	this->mPosition = spawnData.mBasic.spawnPosition;	// Todo: add some random if we like.
	this->setPrivateOwner(0);

	// Let WorldManager own the object.
	gWorldManager->addObject(this, true);

	// All init is done, just the spawn in the world is left.
	// Put this sucker in the Dormant queue.
	this->clearSpawned();

	// gLogger->logMsgF("NpcManager::handleObjectReady: RespawnPeriod = %llu", MSG_NORMAL, creature->getRespawnPeriod());
	// gLogger->logMsgF("Npc will RE-spawn at %.0f, %.0f, %.0f in %lld seconds", MSG_NORMAL, this->mPosition.mX, this->mPosition.mY, this->mPosition.mZ, mSpawn.mBasic.timeToFirstSpawn/1000);
	
	// Add some credits for loot.
	// Let's put some credits in the inventory.
	Inventory* inventory = dynamic_cast<Inventory*>(this->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
	if (inventory)
	{
		if (spawnData.mBasic.templateId == 47513085693) // a Rill.
		{
			inventory->setCredits((gRandom->getRand()%25) + 10);
		}
		else if (spawnData.mBasic.templateId == 47513085725) // a Womp Rat hue.
		{
			inventory->setCredits((gRandom->getRand()%50) + 30);
			this->mHam.mHealth.setCurrentHitPoints(1500);
			this->mHam.mHealth.setMaxHitPoints(1500);
			this->mHam.mHealth.setBaseHitPoints(1500);

			this->mHam.mAction.setCurrentHitPoints(1500);
			this->mHam.mAction.setMaxHitPoints(1500);
			this->mHam.mAction.setBaseHitPoints(1500);

			this->mHam.mMind.setCurrentHitPoints(1500);
			this->mHam.mMind.setMaxHitPoints(1500);
			this->mHam.mMind.setBaseHitPoints(1500);

			this->mHam.calcAllModifiedHitPoints();
		}
		else
		{
			inventory->setCredits((gRandom->getRand()%50) + 20);	// all other Womp Rat's.
			this->mHam.mHealth.setCurrentHitPoints(1000);
			this->mHam.mHealth.setMaxHitPoints(1000);
			this->mHam.mHealth.setBaseHitPoints(1000);

			this->mHam.mAction.setCurrentHitPoints(1000);
			this->mHam.mAction.setMaxHitPoints(1000);
			this->mHam.mAction.setBaseHitPoints(1000);

			this->mHam.mMind.setCurrentHitPoints(1000);
			this->mHam.mMind.setMaxHitPoints(1000);
			this->mHam.mMind.setBaseHitPoints(1000);

			this->mHam.calcAllModifiedHitPoints();
		}
	}

	// gWorldManager->addDormantNpc(this->getId(), this->getRespawnPeriod());
	gWorldManager->addDormantNpc(this->getId(), mSpawn.mBasic.timeToFirstSpawn);
}

//=============================================================================
// 
//	Assist some from my "group"
//

void AttackableCreature::assistCreature(uint64 targetId) 
{
	// if (this->isGroupAssist())
	{
		if (this->mAsssistanceNeededWithId == 0)
		{
			this->mAsssistanceNeededWithId = targetId;
		}
	}
}

//=============================================================================
// 
//	Assist my lair.
//

void AttackableCreature::assistLair(uint64 targetId) 
{
	// if (this->mLairNeedAsssistanceWithId == 0)
	{
		this->mLairNeedAsssistanceWithId = targetId;
		this->mIsAssistingLair = false;
	}
}

//=============================================================================
// 
//	Anyone need assistance?
//

bool AttackableCreature::needAssist(void) 
{
	bool assistNeeded = false;
	if (this->mAsssistanceNeededWithId != 0)
	{
		// Will not assist if target is outside stalking limit.
		assistNeeded = isTargetWithinMaxRange(this->mAsssistanceNeededWithId);
		
		if (!assistNeeded)
		{
			// Clear the current assist target.
			this->mAsssistanceNeededWithId = 0;
		}
	}
	return assistNeeded;
}

//=============================================================================
// 
//	Lair need assistance?
//

bool AttackableCreature::needToAssistLair(void) 
{
	bool status = ((this->mLairNeedAsssistanceWithId != 0) && !this->mIsAssistingLair);
	if (status)
	{
		this->mIsAssistingLair = true;
	}
	return status;
}

void AttackableCreature::executeAssist(void) 
{
	// Are we interested to help?
	// gLogger->logMsgF("AttackableCreature::executeAssist Entering", MSG_NORMAL);

	// Yes. Go and get that sucker.
	Object* object = gWorldManager->getObjectById(this->mAsssistanceNeededWithId );
	if (object)
	{
		// gLogger->logMsgF("AttackableCreature::assist Assisting", MSG_NORMAL);

		this->setTarget(object->getId());
		gMessageLib->sendTargetUpdateDeltasCreo6(this);

		mCombatState = State_Combat;
		this->setAiState(NpcIsActive);

		// We may need to chase the target.
		this->setupStalking(activeDefaultPeriodTime);
	}
}

void AttackableCreature::executeLairAssist(void) 
{
	// Are we interested to help?
	// gLogger->logMsgF("AttackableCreature::executeLairAssist Entering", MSG_NORMAL);

	// Yes. Go and get that sucker.
	Object* object = gWorldManager->getObjectById(this->mLairNeedAsssistanceWithId );
	if (object)
	{
		// gLogger->logMsgF("AttackableCreature::executeLairAssist Assisting", MSG_NORMAL);

		this->setTarget(object->getId());
		gMessageLib->sendTargetUpdateDeltasCreo6(this);

		mCombatState = State_Combat;
		this->setAiState(NpcIsActive);

		// We may need to chase the target.
		this->setupStalking(activeDefaultPeriodTime);
	}
}