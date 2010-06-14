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
#include "CharacterBuilderTerminal.h"
#include "Bank.h"

#include "PlayerObject.h"
#include "WorldManager.h"
#include "LogManager/LogManager.h"
#include "MessageLib/MessageLib.h"
#include "UIManager.h"
#include "UIResourceSelectListBox.h"
#include "ResourceManager.h"
#include "TreasuryManager.h"
#include "TradeManager.h"
#include "SkillManager.h"
#include "Buff.h"
#include "ResourceType.h"
#include "ResourceCategory.h"
#include "Inventory.h"
#include "ObjectFactory.h"

#include "Utils/utils.h"

#include "utils/rand.h"
//=============================================================================

CharacterBuilderTerminal::CharacterBuilderTerminal() : Terminal(), mSortedList(NULL)
{

	InitMenus();
	//mMainMenu.push_back("Manage Experience");
	//mMainMenu.push_back("Manage Credits");
	//mMainMenu.push_back("Manage Attributes");
	//mMainMenu.push_back("Manage Items");
	//mMainMenu.push_back("Manage Professions");
	//mMainMenu.push_back("Manage Resources");

	//mCreditsMenu.push_back("Inventory credits");
	//mCreditsMenu.push_back("Bank credits");


	//mAttributesMenu.push_back("Battle fatigue   (50)");
	//mAttributesMenu.push_back("Battle fatigue  (250)");
	//mAttributesMenu.push_back("Mind Wounds      (50)");
	//mAttributesMenu.push_back("Mind Wounds     (250)");
	//mAttributesMenu.push_back("Heal Fatigue    (123)");
	//mAttributesMenu.push_back("Heal Wounds     (123)");
	//mAttributesMenu.push_back("Mugly's cocktail");
	//mAttributesMenu.push_back("Lloyd's Health Buffs");
	//mAttributesMenu.push_back("Lloyd's Action Buffs");
	//mAttributesMenu.push_back("Lloyd's Mind Buffs");
	//mAttributesMenu.push_back("Jawa beer (Mask Scent)");
	//mAttributesMenu.push_back("Damage Health 200");
	//mAttributesMenu.push_back("Damage Action 200");

}

//=============================================================================
void CharacterBuilderTerminal::InitMenus()
{
	mMainMenu.push_back("Manage Experience");
	mMainMenu.push_back("Manage Credits");
	mMainMenu.push_back("Manage Buffs");
	mMainMenu.push_back("Manage Items");
	mMainMenu.push_back("Manage Resources");
	mMainMenu.push_back("Manage Professions");
	
	mMainCsrMenu.push_back("Manage Experience");
	mMainCsrMenu.push_back("Manage Credits");
	mMainCsrMenu.push_back("Manage Buffs");
	mMainCsrMenu.push_back("Manage Items");
	mMainCsrMenu.push_back("Manage Resources");
	mMainCsrMenu.push_back("Get Item by ID");
	mMainCsrMenu.push_back("Manage Professions");
	mMainCsrMenu.push_back("Manage Wounds");

	InitExperience();
	InitProfessions();
	InitCredits();
	InitBuffs();
	InitItems();
	InitWounds();
}
void CharacterBuilderTerminal::InitProfessions()
{
	mProfessionMenu.push_back("Drop All Skills.");

	SkillList*			skillList	= gSkillManager->getMasterProfessionList();
	SkillList::iterator skillIt		= skillList->begin();

	while(skillIt != skillList->end())
	{
		//// skip jedi professions, if flag isn't set
		//if(!playerObject->getJediState() && strstr((*skillIt)->mName.getAnsi(),"discipline"))
		//{
		//	++skillIt;

		//	continue;
		//}

		char profession_nam[64];

		snprintf(profession_nam, 64, "@skl_n:%s", (*skillIt)->mName);
		mProfessionMenu.push_back(profession_nam);

		++skillIt;
	}
}
void CharacterBuilderTerminal::InitExperience()
{
}
void CharacterBuilderTerminal::InitCredits()
{
	mCreditMenu.push_back("Inventory credits");
	mCreditMenu.push_back("Bank credits");
}
void CharacterBuilderTerminal::InitBuffs()
{
	mBuffMenu.push_back("+2400 Health Buffs (60secs)");
	mBuffMenu.push_back("+2400 Action Buffs (60secs)");
	mBuffMenu.push_back("+ 600 Mind   Buffs (60secs)");
	mBuffMenu.push_back("+2400 Health Buffs (3hour)");
	mBuffMenu.push_back("+2400 Action Buffs (3hour)");
	mBuffMenu.push_back("+ 600 Mind   Buffs (3hour)");
	mBuffMenu.push_back("Clear All Buffs");
}
void CharacterBuilderTerminal::InitWounds()
{
	mWoundMenu.push_back("+100 Health Wound");
	mWoundMenu.push_back("+100 Strength Wound");
	mWoundMenu.push_back("+100 Stamina Wound");
	mWoundMenu.push_back("+100 Action Wound");
	mWoundMenu.push_back("+100 Constitution Wound");
	mWoundMenu.push_back("+100 Quickness Wound");
	mWoundMenu.push_back("+100 Mind Wound");
	mWoundMenu.push_back("+100 Focus Wound");
	mWoundMenu.push_back("+100 willpower Wound");
	mWoundMenu.push_back("+100 Battle Fatigue");

	mWoundMenu.push_back("-100 Health Wound");
	mWoundMenu.push_back("-100 Strength Wound");
	mWoundMenu.push_back("-100 Stamina Wound");
	mWoundMenu.push_back("-100 Action Wound");
	mWoundMenu.push_back("-100 Quickness Wound");
	mWoundMenu.push_back("-100 Constitution Wound");
	mWoundMenu.push_back("-100 Mind Wound");
	mWoundMenu.push_back("-100 Focus Wound");
	mWoundMenu.push_back("-100 willpower Wound");
	mWoundMenu.push_back("-100 Battle Fatigue");
}
void CharacterBuilderTerminal::InitItems()
{
	mItemMenu.push_back("Structures");
	mItemMenu.push_back("Furniture");
	mItemMenu.push_back("Vehicles");
	mItemMenu.push_back("Instruments");
	mItemMenu.push_back("Tools");
	mItemMenu.push_back("Weapons");
	mItemMenu.push_back("Armor");

	InitStructures();
	InitFurniture();
	InitVehicles();
	InitInstruments();
	InitTools();
	InitWeapons();
	InitArmor();
}
void CharacterBuilderTerminal::InitStructures()
{
	mStructureMenu.push_back("Factories");
	mStructureMenu.push_back("Harvesters");
	mStructureMenu.push_back("Camps");
	mStructureMenu.push_back("Houses");
	mStructureMenu.push_back("Civic");

	//BStringVector			mFactoryMenu;
	mFactoryMenu.push_back("Wearables Factory");
	mFactoryMenu.push_back("Food Factory");
	mFactoryMenu.push_back("Equipment Factory");
	mFactoryMenu.push_back("Structure Factory");
	
	//BStringVector			mHarvesterMenu;
	mHarvesterMenu.push_back("Flora Harvester");
	mHarvesterMenu.push_back("Gas Harvester");
	mHarvesterMenu.push_back("Chemical Harvester");
	mHarvesterMenu.push_back("Water Harvester");
	mHarvesterMenu.push_back("Mineral Harvester");

	//BStringVector			mCampMenu;
	mCampMenu.push_back("Basic Camp");
	mCampMenu.push_back("Field Base Camp");
	mCampMenu.push_back("Improved Camp");
	mCampMenu.push_back("HiTech Field Base Camp");
	mCampMenu.push_back("Multiperson Camp");
	mCampMenu.push_back("High Quality Camp");

	mCivicMenu.push_back("Guild Halls");
	mCivicMenu.push_back("City Halls");

	//Guild Halls
	mGuildHallMenu.push_back("Corellia Guild Hall");
	mGuildHallMenu.push_back("Generic Guild Hall");
	mGuildHallMenu.push_back("Naboo Guild Hall");
	mGuildHallMenu.push_back("Tatooine Guild Hall");
	
	//City Halls
	mCityHallMenu.push_back("Corellian City Hall");
	mCityHallMenu.push_back("Nabooian City Hall");
	mCityHallMenu.push_back("Tatooine City Hall");

	//BStringVector			mHouseMenu;
	mHouseMenu.push_back("Generic Houses");
	mHouseMenu.push_back("Tatooine Houses");

	////Harvesters
	//BStringVector			mFloraMenu;
	mFloraMenu.push_back("Small");
	mFloraMenu.push_back("Heavy");
	mFloraMenu.push_back("Medium");
	//BStringVector			mGasMenu;
	mGasMenu.push_back("Small");
	mGasMenu.push_back("Heavy");
	mGasMenu.push_back("Medium");
	
	//BStringVector			mChemicalMenu;
	mChemicalMenu.push_back("Small");
	mChemicalMenu.push_back("Heavy");
	mChemicalMenu.push_back("Medium");
	
	//BStringVector			mWaterMenu;
	mWaterMenu.push_back("Small");
	mWaterMenu.push_back("Heavy");
	mWaterMenu.push_back("Medium");
	
	//BStringVector			mMineralMenu;
	mMineralMenu.push_back("Heavy");
	mMineralMenu.push_back("Small");
	mMineralMenu.push_back("Medium");
	

	////Houses
	//BStringVector			mGenericMenu;
	mGenericMenu.push_back("Generic Large  (Style 1)");
	mGenericMenu.push_back("Generic Large  (Style 2)");
	mGenericMenu.push_back("Generic Medium (Style 1)");
	mGenericMenu.push_back("Generic Medium (Style 2)");
	mGenericMenu.push_back("Generic Small  (Style 1, Floor 1)");
	mGenericMenu.push_back("Generic Small  (Style 1, Floor 2)");
	mGenericMenu.push_back("Generic Small  (Style 2, Floor 1)");
	mGenericMenu.push_back("Generic Small  (Style 2, Floor 2)");
	//BStringVector			mTatooineMenu;
	mTatooineMenu.push_back("Tatooine Large");
	mTatooineMenu.push_back("Tatooine Medium");
	mTatooineMenu.push_back("Tatooine Small (Style 1)");
	mTatooineMenu.push_back("Tatooine Small (Style 2)");
}
void CharacterBuilderTerminal::InitFurniture()
{
	mFurnitureMenu.push_back("Rugs");
	mFurnitureMenu.push_back("Plants");
	mFurnitureMenu.push_back("Elegant");
	mFurnitureMenu.push_back("Modern");
	mFurnitureMenu.push_back("Plain");
	mFurnitureMenu.push_back("Cheap");
	////Furniture
	//BStringVector			mRugMenu;
	mRugMenu.push_back("Large Style 1");
	mRugMenu.push_back("Large Style 2");
	mRugMenu.push_back("Large Style 3");
	//BStringVector			mPlantMenu;
	mPlantMenu.push_back("Large Style 1");
	mPlantMenu.push_back("Large Style 2");
	//BStringVector			mElegantMenu;
	mElegantMenu.push_back("Armoire");
	mElegantMenu.push_back("Bookcase");
	mElegantMenu.push_back("Cabinet");
	mElegantMenu.push_back("Chair");
	mElegantMenu.push_back("Chest");
	mElegantMenu.push_back("Coffee Table");
	mElegantMenu.push_back("Couch");
	mElegantMenu.push_back("End Table");
	mElegantMenu.push_back("Love Seat");
	//BStringVector			mModernMenu;
	mModernMenu.push_back("Armoire");
	mModernMenu.push_back("Bookcase");
	mModernMenu.push_back("Cabinet");
	mModernMenu.push_back("Chair");
	mModernMenu.push_back("Chest");
	mModernMenu.push_back("Coffee Table");
	mModernMenu.push_back("Couch Style 1");
	mModernMenu.push_back("Couch Style 2");
	mModernMenu.push_back("Dining Table");
	mModernMenu.push_back("End Table");
	mModernMenu.push_back("Love Seat");
	//BStringVector			mPlainMenu;
	mPlainMenu.push_back("Armoire");
	mPlainMenu.push_back("Bookcase");
	mPlainMenu.push_back("Cabinet");
	mPlainMenu.push_back("Chair");
	mPlainMenu.push_back("Chest");
	mPlainMenu.push_back("Coffee Table");
	mPlainMenu.push_back("Couch");
	mPlainMenu.push_back("End Table");
	mPlainMenu.push_back("Love Seat");
	//BStringVector			mCheapMenu;
	mCheapMenu.push_back("Armoire");
	mCheapMenu.push_back("Bookcase");
	mCheapMenu.push_back("Cabinet");
	mCheapMenu.push_back("Chair");
	mCheapMenu.push_back("Chest");
	mCheapMenu.push_back("Coffee Table");
	mCheapMenu.push_back("Couch");
	mCheapMenu.push_back("End Table");
	mCheapMenu.push_back("Love Seat");
}
void CharacterBuilderTerminal::InitVehicles()
{
	mVehicleMenu.push_back("x34 Landspeeder");
	mVehicleMenu.push_back("Speederbike");
	mVehicleMenu.push_back("Swoop");
}
void CharacterBuilderTerminal::InitInstruments()
{
	mInstrumentMenu.push_back("Bandfill");
	mInstrumentMenu.push_back("Fanfar");
	mInstrumentMenu.push_back("Fizzz");
	mInstrumentMenu.push_back("Chidinkalu Horn");
	mInstrumentMenu.push_back("Kloo Horn");
	mInstrumentMenu.push_back("Mandoviol");
	mInstrumentMenu.push_back("Nalargon");
	mInstrumentMenu.push_back("Omni Box");
	mInstrumentMenu.push_back("slitherhorn");
	mInstrumentMenu.push_back("Traz");
}
void CharacterBuilderTerminal::InitTools()
{
	//	//Tools
	mToolMenu.push_back("Survey Tool");
	mToolMenu.push_back("Crafting Tool");

	mSurveyToolMenu.push_back("Wind");
	mSurveyToolMenu.push_back("Gas");
	mSurveyToolMenu.push_back("Flora");
	mSurveyToolMenu.push_back("Moisture");
	mSurveyToolMenu.push_back("Chemical");
	mSurveyToolMenu.push_back("Solar");
	mSurveyToolMenu.push_back("Mineral");
	mSurveyToolMenu.push_back("Organic");
	mSurveyToolMenu.push_back("Inorganic");

	mCraftingToolMenu.push_back("Clothing & Armor");
	mCraftingToolMenu.push_back("Generic");
	mCraftingToolMenu.push_back("Weapon");
	mCraftingToolMenu.push_back("Food");
	mCraftingToolMenu.push_back("Structure");
}
void CharacterBuilderTerminal::InitWeapons()
{
	////Weapons
	mWeaponMenu.push_back("Melee");
	mWeaponMenu.push_back("Ranged");

	mMeleeMenu.push_back("One Handed Sword");
	mMeleeMenu.push_back("Two Handed Sword");
	mMeleeMenu.push_back("Axe");
	mMeleeMenu.push_back("Baton");
	mMeleeMenu.push_back("Polearm");
	mMeleeMenu.push_back("Knife");
	mMeleeMenu.push_back("Unarmed");

	mRangedMenu.push_back("Carbine");
	mRangedMenu.push_back("Thrown");
	mRangedMenu.push_back("Heavy");
	mRangedMenu.push_back("Pistol");
	mRangedMenu.push_back("Rifle");
	////Melee Weapon
	//BStringVector			mOneHandSwordMenu;
	mOneHandSwordMenu.push_back("Noob Sword");
	mOneHandSwordMenu.push_back("Sword 1");
	mOneHandSwordMenu.push_back("Sword 2");
	//BStringVector			mTwoHandSwordMenu;
	mTwoHandSwordMenu.push_back("Battleaxe");
	mTwoHandSwordMenu.push_back("Cleaver");
	mTwoHandSwordMenu.push_back("Maul");
	mTwoHandSwordMenu.push_back("Scythe");
	//BStringVector			mAxeMenu;
	mAxeMenu.push_back("Heavy Duty");
	mAxeMenu.push_back("Vibroaxe");
	//BStringVector			mBatonMenu;
	mBatonMenu.push_back("Gaderiffi");
	mBatonMenu.push_back("Stun Baton");
	mBatonMenu.push_back("Victor Gaderiffi");
	//BStringVector			mKnifeMenu;
	mKnifeMenu.push_back("Dagger");
	mKnifeMenu.push_back("Donkuwah");
	mKnifeMenu.push_back("Janta");
	mKnifeMenu.push_back("Stone");
	mKnifeMenu.push_back("Stone (Noob)");
	mKnifeMenu.push_back("Survival");
	mKnifeMenu.push_back("Vibroblade");
	//BStringVector			mPolearmMenu;
	mPolearmMenu.push_back("Controller");
	mPolearmMenu.push_back("Janta");
	mPolearmMenu.push_back("Metal");
	mPolearmMenu.push_back("Wood (Type 1)");
	mPolearmMenu.push_back("Wood (Type 2)");
	mPolearmMenu.push_back("Vibrolance");
	mPolearmMenu.push_back("Polearm Vibro Axe");
	//Unarmed
	mUnarmedMenu.push_back("Vibro Knuckler");

	////Ranged Weapon
	//BStringVector			mCarbineMenu;
	mCarbineMenu.push_back("CDEF Carbine");
	mCarbineMenu.push_back("CDEF CorSec Carbine");
	mCarbineMenu.push_back("DH17");
	mCarbineMenu.push_back("DH17 Black");
	mCarbineMenu.push_back("DH17 Snub");
	mCarbineMenu.push_back("DXR6");
	mCarbineMenu.push_back("E11");
	mCarbineMenu.push_back("EE3");
	mCarbineMenu.push_back("Elite Carbine");
	mCarbineMenu.push_back("Laser Carbine");
	mCarbineMenu.push_back("Nym's Slugthrower");
	//BStringVector			mThrownMenu;
	mThrownMenu.push_back("Bug Bomb");
	mThrownMenu.push_back("Cryoban");
	mThrownMenu.push_back("Fallback");
	mThrownMenu.push_back("Fragmentation");
	mThrownMenu.push_back("Fragmentation (Light)");
	mThrownMenu.push_back("Glop");
	mThrownMenu.push_back("Poison");
	mThrownMenu.push_back("Proton");
	mThrownMenu.push_back("Thermal Detonator");
	//BStringVector			mHeavyMenu;
	mHeavyMenu.push_back("Acid Beam");
	mHeavyMenu.push_back("Lightning Beam");
	mHeavyMenu.push_back("Particle Beam");
	mHeavyMenu.push_back("Heavy Rocket Launcher");
	mHeavyMenu.push_back("Launcher");
	//BStringVector			mPistolMenu;
	mPistolMenu.push_back("CDEF Pistol");
	mPistolMenu.push_back("CDEF CorSec Pistol");
	mPistolMenu.push_back("CDEF Noob Pistol");
	mPistolMenu.push_back("D18");
	mPistolMenu.push_back("DE10");
	mPistolMenu.push_back("DH17");
	mPistolMenu.push_back("DL44");
	mPistolMenu.push_back("DL44 Metal");
	mPistolMenu.push_back("DX2");
	mPistolMenu.push_back("FWG5");
	mPistolMenu.push_back("Launcher Pistol");
	mPistolMenu.push_back("Power5");
	mPistolMenu.push_back("Republic Blaster");
	mPistolMenu.push_back("Scatter Pistol");
	mPistolMenu.push_back("Scout Blaster");
	mPistolMenu.push_back("Scout CorSec Blaster");
	mPistolMenu.push_back("SR Combat");
	mPistolMenu.push_back("Striker");
	mPistolMenu.push_back("Striker (Noob)");
	mPistolMenu.push_back("Tangle Pistol");
	//BStringVector			mRifleMenu;
	mRifleMenu.push_back("Acid Beam");
	mRifleMenu.push_back("Beam Rifle");
	mRifleMenu.push_back("Beserker");
	mRifleMenu.push_back("Bowcaster");
	mRifleMenu.push_back("CDEF Rifle");
	mRifleMenu.push_back("DLT20");
	mRifleMenu.push_back("DLT20a");
	mRifleMenu.push_back("E11 Rifle");
	mRifleMenu.push_back("Ewok Crossbow");
	mRifleMenu.push_back("Flame Thrower");
	mRifleMenu.push_back("Jawa Ion");
	mRifleMenu.push_back("Laser Rifle");
	mRifleMenu.push_back("Laser Rifle (Noob)");
	mRifleMenu.push_back("Lightning Rifle");
	mRifleMenu.push_back("SG82");
	mRifleMenu.push_back("Spraystick");
	mRifleMenu.push_back("T21");
	mRifleMenu.push_back("Tusken");
	mRifleMenu.push_back("Tusken (Victor)");
}
void CharacterBuilderTerminal::InitArmor()
{
	mArmorMenu.push_back("Bone Armor");
	mArmorMenu.push_back("Composite Armor");
	mArmorMenu.push_back("Ubese Armor");
	//BStringVector			mBoneArmorMenu;
	mBoneArmorMenu.push_back("Full Set");
	mBoneArmorMenu.push_back("Left Bicep");
	mBoneArmorMenu.push_back("Right Bicep");
	mBoneArmorMenu.push_back("Boots");
	mBoneArmorMenu.push_back("Left Bracer");
	mBoneArmorMenu.push_back("Right Bracer");
	mBoneArmorMenu.push_back("Chest Plate");
	mBoneArmorMenu.push_back("Gloves");
	mBoneArmorMenu.push_back("Helmet");
	mBoneArmorMenu.push_back("Leggings");
	mBoneArmorMenu.push_back("Alternative Left Bicep");
	//BStringVector			mCompositeArmorMenu;
	mCompositeArmorMenu.push_back("Full Set");
	mCompositeArmorMenu.push_back("Left Bicep");
	mCompositeArmorMenu.push_back("Right Bicep");
	mCompositeArmorMenu.push_back("Boots");
	mCompositeArmorMenu.push_back("Left Bracer");
	mCompositeArmorMenu.push_back("Right Bracer");
	mCompositeArmorMenu.push_back("Chest Plate");
	mCompositeArmorMenu.push_back("Gloves");
	mCompositeArmorMenu.push_back("Helmet");
	mCompositeArmorMenu.push_back("Leggings");
	//BStringVector			mUbeseArmorMenu;
	mUbeseArmorMenu.push_back("Full Set");
	mUbeseArmorMenu.push_back("Bandolier");
	mUbeseArmorMenu.push_back("Boots");
	mUbeseArmorMenu.push_back("Left Bracer");
	mUbeseArmorMenu.push_back("Right Bracer");
	mUbeseArmorMenu.push_back("Gloves");
	mUbeseArmorMenu.push_back("Helmet");
	mUbeseArmorMenu.push_back("Jacket");
	mUbeseArmorMenu.push_back("Pants");
	
}
CharacterBuilderTerminal::~CharacterBuilderTerminal()
{
}

//=============================================================================


void CharacterBuilderTerminal::prepareCustomRadialMenu(CreatureObject* creatureObject, uint8 itemCount)
{
	mRadialMenu = RadialMenuPtr(new RadialMenu());

	// any object with callbacks needs to handle those (received with menuselect messages) !
	mRadialMenu->addItem(1,0,radId_itemUse,radAction_ObjCallback);
	mRadialMenu->addItem(2,0,radId_examine,radAction_Default);
}

//=============================================================================

void CharacterBuilderTerminal::GiveItem(PlayerObject* playerObject, uint32 inputId)
{
	ItemFrogItemClass* item = gTradeManager->mItemFrogClass.LookUpType(inputId);

	if(item)
	{
		Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
        gObjectFactory->requestNewDefaultItem(inventory,item->family,item->type,inventory->getId(),99, glm::vec3(),"");
		gMessageLib->sendSystemMessage(playerObject, L"The item has been placed in your inventory.");
	}
	else
	{
		gMessageLib->sendSystemMessage(playerObject, L"No such item.");
	}
}
void CharacterBuilderTerminal::SendXPMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	BStringVector availableXpTypes;
	XPList* xpList = playerObject->getXpList();
	XPList::iterator xpIt = xpList->begin();

	SortedList::iterator it;
	// Clear existing space, and allocate space where we can sort this stuff.
	if(mSortedList)
	{
		SAFE_DELETE(mSortedList);
	}
	mSortedList = new SortedList;
	

	while (xpIt != xpList->end())
	{
		it = mSortedList->begin();

		for (uint32 index = 0; index < mSortedList->size(); index++)
		{
			if (Anh_Utils::cmpistr(gSkillManager->getXPTypeExById((*xpIt).first).getAnsi(), (*it).first.getAnsi()) < 0)
			{
				break;
			}
			it++;
		}
		mSortedList->insert(it, std::make_pair(gSkillManager->getXPTypeExById((*xpIt).first),(*xpIt).first));

		++xpIt;
	}

	// Let's put up the sorted strings for the show.
	it = mSortedList->begin();
	while (it != mSortedList->end())
	{
		availableXpTypes.push_back((*it).first);
		it++;
	}

	if (availableXpTypes.size() == 0)
	{
		gMessageLib->sendSystemMessage(playerObject, L"You currently do not have any skills.");
		SAFE_DELETE(mSortedList);
	}
	else
	{
		gUIManager->createNewListBox(this,"handleGetXp","Select Xp Type","Select from the list below.", availableXpTypes, playerObject, SUI_Window_CharacterBuilder_ListBox_ExperienceMenu);
	}
}
void CharacterBuilderTerminal::SendResourcesMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if(playerObject->isConnected())
	{
		ResourceTypeMap*				rtMap				= gResourceManager->getResourceTypeMap();
		ResourceCategoryMap*			rcMap				= gResourceManager->getResourceCategoryMap();
		ResourceCategoryMap::iterator	rcIt				= rcMap->begin();
		BStringVector					resourceNameList;
		ResourceIdList					resourceIdList;
		//uint32							counter = 0;

		while(rcIt != rcMap->end())
		{
			ResourceTypeMap::iterator rtIt = rtMap->begin();

			while(rtIt != rtMap->end())
			{
				//we want only parent ==1 to begin with
				uint32 parentId = (*(*rcIt).second).getParentId();
				if(parentId == 1)
				{
					resourceIdList.push_back((*rcIt).first);
					resourceNameList.push_back((*(*rcIt).second).getName());
					break;
				}

				++rtIt;
			}

			++rcIt;
		}

		gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilder_ListBox_ResourceMenu);
	}
}

void CharacterBuilderTerminal::_handleMainMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
  // Check if the player is a csr and handle the menu appropriately.
  if (playerObject->getCsrTag())
		return _handleMainCsrMenu(playerObject, action, element, inputStr, window);

	switch(element)
	{
	case 0://Experience
		SendXPMenu(playerObject, action, element, inputStr, window);
		break;
	case 1://Credits
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCreditsMenu","Credits","Select a category.",mCreditMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CreditMenu);
		}
		break;
	case 2://Buffs
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleAttributesMenu","Attributes","Select a Buff.",mBuffMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_BuffMenu);
		}
		break;
	case 3://Items
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleItemsMenu","Items","Select a Category.",mItemMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ItemMenu);
		}
		break;
	case 4://Resources
		SendResourcesMenu(playerObject, action, element, inputStr, window);
		break;
	case 5: //Professions
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleGetProf","Select Profession to Master","Select from the list below.",mProfessionMenu,playerObject,SUI_Window_CharacterBuilderProfessionMastery_ListBox);
		}
		break;
	default:
		break;
	}
}
void CharacterBuilderTerminal::_handleMainCsrMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0://Experience
		SendXPMenu(playerObject, action, element, inputStr, window);
		break;
	case 1://Credits
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCreditsMenu","Credits","Select a category.",mCreditMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CreditMenu);
		}
		break;
	case 2://Buffs
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleAttributesMenu","Attributes","Select a Buff.",mBuffMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_BuffMenu);
		}
		break;
	case 3://Items
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleItemsMenu","Items","Select a Category.",mItemMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ItemMenu);
		}
		break;
	case 4://Resources
		SendResourcesMenu(playerObject, action, element, inputStr, window);
		break;
	case 5://Get Item by ID
		if(playerObject->isConnected())
		{
			BStringVector dropDowns;
			gUIManager->createNewInputBox(this, "handleInputItemId", "Get Item", "Enter the item ID", dropDowns, playerObject, SUI_IB_NODROPDOWN_OKCANCEL, SUI_Window_CharacterBuilderItemIdInputBox,8);
		}
		break;
	case 6: //Professions
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleGetProf","Select Profession to Master","Select from the list below.",mProfessionMenu,playerObject,SUI_Window_CharacterBuilderProfessionMastery_ListBox);
		}
		break;
	case 7: //Wounds
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleWoundMenu","Wounds","Select a Wound.",mWoundMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_WoundMenu);
		}
		break;
	default:
		break;
	}
}

void CharacterBuilderTerminal::_handleProfessionMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if(element < 0)
	{
		return;
	}

	if(element == 0)
	{
		//We want to drop all our skills! OH NOES!
		SkillList::iterator it = playerObject->getSkills()->begin();
		SkillList::iterator end = playerObject->getSkills()->end();

		std::vector<uint32> skills;

		while(it != end)
		{
			skills.push_back((*it)->mId);
			++it;
		}

		std::vector<uint32>::iterator jt = skills.begin();
		std::vector<uint32>::iterator jend = skills.end();

		while( jt != jend )
		{
			gSkillManager->dropSkill((*jt), playerObject, false);
			jt++;
		}

		gMessageLib->sendSystemMessage(playerObject,L"All skills surrendered successfully.");

		return;
	}

	SkillList* skillList = gSkillManager->getMasterProfessionList();
	SkillList::iterator newSkill = skillList->begin();
	newSkill+=element-1;
	if(!playerObject->getJediState() && strstr((*newSkill)->mName.getAnsi(),"discipline"))
	{
		gMessageLib->sendSystemMessage(playerObject, L"Sorry. That profession is only available to Jedi Enabled Accounts");
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleGetProf","Select Profession to Master","Select from the list below.",mProfessionMenu,playerObject,SUI_Window_CharacterBuilderProfessionMastery_ListBox);
		}
	} else {
		gSkillManager->learnSkillLine((*newSkill)->mId, playerObject, false);
	}
}
void CharacterBuilderTerminal::_handleExperienceMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if (element > (int32)playerObject->getXpList()->size() - 1 || element < 0)
	{
		gMessageLib->sendSystemMessage(playerObject, L"Error while giving Xp!");
	}
	else if (mSortedList)
	{	// Get the xp type for the selection.
		uint32 xpType = mSortedList->at(element).second;
		gSkillManager->addExperience(xpType,600000,playerObject);
	}
	SAFE_DELETE(mSortedList);
}
void CharacterBuilderTerminal::_handleCreditMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if(window->getWindowType() == SUI_Window_CharacterBuilder_ListBox_CreditMenu)
	{
		switch(element)
		{
			case 0: // inventory credits
			{
				BStringVector dropDowns;
				dropDowns.push_back("test");
				gUIManager->createNewInputBox(this,
					"handleInputInventoryCredits",
					"Inventory Credits",
					"Enter amount",
					dropDowns,
					player,
					SUI_IB_NODROPDOWN_OKCANCEL,
					SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox,
					8);
			}
			break;

			case 1: // bank credits
			{
				BStringVector dropDowns;
				dropDowns.push_back("test");
				gUIManager->createNewInputBox(this,
					"handleInputInventoryCredits",
					"Bank Credits",
					"Enter amount",
					dropDowns,
					player,
					SUI_IB_NODROPDOWN_OKCANCEL,
					SUI_Window_CharacterBuilderCreditsMenuBank_InputBox,
					8);
			}
			break;

			default:{}break;
		}
	} else {
		// parse the input value
		if(swscanf(inputStr.getUnicode16(),L"%i",&mInputBoxAmount) != 1)
		{
			mInputBoxAmount = -1;
		}

		if(mInputBoxAmount < 0)
		{
			gMessageLib->sendSystemMessage(player, L"Invalid amount.");
			return;
		}

		gLogger->log(LogManager::DEBUG,"input: %u", mInputBoxAmount);

		// bank or inv?
		if(window->getWindowType() == SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox)
		{
			dynamic_cast<Inventory*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory))->setCredits(mInputBoxAmount);
			gTreasuryManager->saveAndUpdateInventoryCredits(player);

		}
		else
		{
			dynamic_cast<Bank*>(player->getEquipManager()->getEquippedObject(CreatureEquipSlot_Bank))->setCredits(mInputBoxAmount);
			gTreasuryManager->saveAndUpdateBankCredits(player);
		}
	}	
}
void CharacterBuilderTerminal::_handleBuffMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_health, +2400,0,-2400);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_health, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_strength, +2400,0,-2400);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_strength, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_constitution, +2400,0,-2400);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_constitution, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 1: 
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_action, +2400,0,-2400);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_action, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_quickness, +2400,0,-2400);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_quickness, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_stamina, +2400,0,-2400);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 60000, medical_enhance_stamina, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 2: 
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_mind, +600,0,-600);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 60000, performance_enhance_dance_mind, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_focus, +600,0,-600);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 60000, performance_enhance_music_focus, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_willpower, +600,0,-600);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 60000, performance_enhance_music_willpower, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 3:
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_health, +2400,0,-2400);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_health, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_strength, +2400,0,-2400);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_strength, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_constitution, +2400,0,-2400);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_constitution, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 4: 
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_action, +2400,0,-2400);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_action, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_quickness, +2400,0,-2400);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_quickness, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_stamina, +2400,0,-2400);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 10800000, medical_enhance_stamina, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 5: 
		{
			BuffAttribute* tempAttribute1 = new BuffAttribute(attr_mind, +600,0,-600);
			Buff* tempBuff1 = Buff::SimpleBuff(playerObject, playerObject, 10800000, performance_enhance_dance_mind, gWorldManager->GetCurrentGlobalTick());
			tempBuff1->AddAttribute(tempAttribute1);
			playerObject->AddBuff(tempBuff1);

			BuffAttribute* tempAttribute2 = new BuffAttribute(attr_focus, +600,0,-600);
			Buff* tempBuff2 = Buff::SimpleBuff(playerObject, playerObject, 10800000, performance_enhance_music_focus, gWorldManager->GetCurrentGlobalTick());
			tempBuff2->AddAttribute(tempAttribute2);
			playerObject->AddBuff(tempBuff2);

			BuffAttribute* tempAttribute3 = new BuffAttribute(attr_willpower, +600,0,-600);
			Buff* tempBuff3 = Buff::SimpleBuff(playerObject, playerObject, 10800000, performance_enhance_music_willpower, gWorldManager->GetCurrentGlobalTick());
			tempBuff3->AddAttribute(tempAttribute3);
			playerObject->AddBuff(tempBuff3);

		}break;
	case 6:
		playerObject->ClearAllBuffs();
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleItemMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0: //structures
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleStructureMenu","Structure","Select a category.",mStructureMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_StructureMenu);
		}
		break;
	case 1://Furniture
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleFurnitureMenu","Furniture","Select a category.",mFurnitureMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_FurnitureMenu);
		}
		break;
	case 2://Vehicles
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleVehicleMenu","Vehicle","Select a category.",mVehicleMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_VehicleMenu);
		}
		break;
	case 3://Instruments
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleInstrumentMenu","Instrument","Select a category.",mInstrumentMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_InstrumentMenu);
		}
		break;
	case 4://Tools
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleToolMenu","Tool","Select a category.",mToolMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ToolMenu);
		}
		break;
	case 5://Weapons
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleWeaponMenu","Weapon","Select a category.",mWeaponMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_WeaponMenu);
		}
		break;
	case 6://Armor
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleArmorMenu","Armor","Select a category.",mArmorMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ArmorMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleResourceMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();
	ResourceTypeMap*				rtMap				= gResourceManager->getResourceTypeMap();

	if(element > (int32)resourceIdList.size()||element < 0)
	{
		return;
	}

	uint64							catId				= resourceIdList[element];
	ResourceCategory*				rParent				= gResourceManager->getResourceCategoryById((uint32)catId);

	resourceIdList.clear();
	if(rParent)
	{
		BStringVector					resourceNameList;
		ResourceIdList					resourceIdList;

		//do we have children categories or children resources?
		if(rParent->getChildren()->size())
		{
			//iterate through the children categories and display them
			ResourceCategoryList*			rcList				= rParent->getChildren();
			ResourceCategoryList::iterator	rcIt				= rcList->begin();

			//bool found = false;
			//iterate through the children and add them as necessary
			while(rcIt != rcList->end())
			{
				if((*(*rcIt)).getChildren()->size())
				{
					resourceIdList.push_back((*(*rcIt)).getId());
					resourceNameList.push_back((*(*rcIt)).getName());
					rcIt++;
					continue;
				}
				ResourceTypeMap::iterator rtIt = rtMap->begin();
				while(rtIt != rtMap->end())
				{
					if(rcIt == rcList->end())
						break;

					ResourceType *rt = (*rtIt).second;
					if(!rt)
					{
						rtIt++;
						continue;
					}

					// the resources category id is equal to the category id we are about to display!
					// ie it is not empty - add it
					if(rt->getCategoryId() == (*(*rcIt)).getId())
					{
						resourceIdList.push_back((*(*rcIt)).getId());
						resourceNameList.push_back((*(*rcIt)).getName());
						rtIt++;
						//only one of each
						break;

					}

					if((*(*rcIt)).getParentId() == rParent->getId())
					{
						resourceIdList.push_back((*(*rcIt)).getId());
						resourceNameList.push_back((*(*rcIt)).getName());
						rtIt++;
						//only one of each
						break;
						//}
					}

					++rtIt;
				}
				if(rcIt != rcList->end())
					rcIt++;
			}
			gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilder_ListBox_ResourceMenu);
		}
		else//if(rParent->getChildren()->size())
		{				//it was a resource - create

			ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();

			if(element > (int32)resourceIdList.size()|| element < 0)
			{
				resourceIdList.clear();
				return;
			}

			ResourceCRCNameMap*				rCRCMap				= gResourceManager->getResourceCRCNameMap();
			ResourceCRCNameMap::iterator	rCrcMapIt;

			ResourceTypeMap::iterator		rtIt				= rtMap->begin();
			uint32							catId				= static_cast<uint32>(resourceIdList[element]);
			//ResourceCategory*				rCategory			= gResourceManager->getResourceCategoryById(catId);
			BStringVector					resourceNameList;

			resourceIdList.clear();

			//no iterate through all the types and check for those containing resis
			while(rtIt != rtMap->end())
			{
				if((*(*rtIt).second).getCategoryId() == catId)
				{
					// check whether we have associated resources

					rCrcMapIt	= rCRCMap->begin();
					while(rCrcMapIt != rCRCMap->end())
					{
						if((*(*rCrcMapIt).second).getTypeId() == (*(*rtIt).second).getId())
						{
							resourceNameList.push_back((*(*rtIt).second).getName());
							resourceIdList.push_back((*rtIt).first);

							break;
						}

						++rCrcMapIt;
					}
				}

				++rtIt;
			}

			// is the list filled? if not we might have resources already on our hand!
			if(resourceIdList.size())
			{
				gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox);
				return;
			}
			else
			{
				//as no types are available anymore it might be resources!

				rCrcMapIt	= rCRCMap->begin();
				while(rCrcMapIt != rCRCMap->end())
				{
					if((*(*rCrcMapIt).second).getTypeId() == catId)
					{
						resourceNameList.push_back((*(*rCrcMapIt).second).getName());
						resourceIdList.push_back((*rCrcMapIt).first);
					}

					++rCrcMapIt;
				}

				gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox);
			}
		}
	}
}	
void CharacterBuilderTerminal::_handleResourcesCRC(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	if(element < 0)
	{
		return;
	}

	// now have the type we need the resource
	if(playerObject->isConnected())
	{
		ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();

		if(element > (int32)resourceIdList.size())
		{
			return;
		}

		uint32		crc			= static_cast<uint32>(resourceIdList[element]);
		Resource*	resource	= gResourceManager->getResourceByNameCRC(crc);

		if(resource)
		{
			Inventory* inventory = dynamic_cast<Inventory*>(playerObject->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));
			gObjectFactory->requestNewResourceContainer(inventory ,resource->getId(),inventory ->getId(),99,100000);
		}
	}
}
void CharacterBuilderTerminal::_handleResourcesTypes(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	ResourceIdList resourceIdList = dynamic_cast<UIResourceSelectListBox*>(window)->getResourceIdList();
	if(element < 0)
	{
		resourceIdList.clear();
		return;
	}

	// now have the category we need the type
	if(playerObject->isConnected())
	{

		if(element > (int32)resourceIdList.size())
		{
			resourceIdList.clear();
			return;
		}

		uint32							typeId		= static_cast<uint32>(resourceIdList[element]);
		//ResourceType*					rType		= gResourceManager->getResourceTypeById(typeId);
		//ResourceTypeMap*				rtMap		= gResourceManager->getResourceTypeMap();
		ResourceCRCNameMap*				rCRCMap		= gResourceManager->getResourceCRCNameMap();
		ResourceCRCNameMap::iterator	rCrcNameIt	= rCRCMap->begin();
		BStringVector					resourceNameList;

		resourceIdList.clear();

		while(rCrcNameIt != rCRCMap->end())
		{
			if((*(*rCrcNameIt).second).getTypeId() == typeId)
			{
				resourceNameList.push_back((*(*rCrcNameIt).second).getName());
				resourceIdList.push_back((*rCrcNameIt).first);
			}

			++rCrcNameIt;
		}

		gUIManager->createNewResourceSelectListBox(this,"handleResourcesMenu","Resources","Select",resourceNameList,resourceIdList,playerObject,SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox);
	}	
}
void CharacterBuilderTerminal::_handleWoundMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0: //Health Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Wounds, 100);
		break;
	case 1: //Strength Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Strength, HamProperty_Wounds, 100);
		break;
	case 2: //Constitution Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Constitution, HamProperty_Wounds, 100);
		break;
	case 3: //Action Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Wounds, 100);
		break;
	case 4: //Stamina Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Stamina, HamProperty_Wounds, 100);
		break;
	case 5: //Quickness Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Quickness, HamProperty_Wounds, 100);
		break;
	case 6: //Mind Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, 100);
		break;
	case 7: //Focus Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Focus, HamProperty_Wounds, 100);
		break;
	case 8: //Willpower Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Willpower, HamProperty_Wounds, 100);
		break;
	case 9: //BattleFatigue
		playerObject->getHam()->updateBattleFatigue(100);
		break;
	case 10: //Health Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Health, HamProperty_Wounds, -100);
		break;
	case 11: //Strength Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Strength, HamProperty_Wounds, -100);
		break;
	case 12: //Constitution Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Constitution, HamProperty_Wounds, -100);
		break;
	case 13: //Action Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Action, HamProperty_Wounds, -100);
		break;
	case 14: //Stamina Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Stamina, HamProperty_Wounds, -100);
		break;
	case 15: //Quickness Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Quickness, HamProperty_Wounds, -100);
		break;
	case 16: //Mind Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Mind, HamProperty_Wounds, -100);
		break;
	case 17: //Focus Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Focus, HamProperty_Wounds, -100);
		break;
	case 18: //Willpower Wound
		playerObject->getHam()->updatePropertyValue(HamBar_Willpower, HamProperty_Wounds, -100);
		break;
	case 19: //BattleFatigue
		playerObject->getHam()->updateBattleFatigue(-100);
		break;
	default:
		break;
	}	
}

void CharacterBuilderTerminal::_handleStructureMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0: //Factories
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleFactoryMenu","Factory","Select a category.",mFactoryMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_FactoryMenu);
		}
		break;
	case 1: //Harvesters
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleHarvesterMenu","Harvester","Select a category.",mHarvesterMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_HarvesterMenu);
		}
		break;
	case 2: //Camps
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCampMenu","Camp","Select a category.",mCampMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CampMenu);
		}
		break;
	case 3: //Houses
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleHouseMenu","House","Select a category.",mHouseMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_HouseMenu);
		}
		break;
	case 4:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this, "handleCivicMenu", "Civic", "Select a category.", mCivicMenu, playerObject, SUI_Window_CharacterBuilder_ListBox_CivicMenu);
		}
	default:break;
	}
}
void CharacterBuilderTerminal::_handleFurnitureMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0: //rugs
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleRugsMenu","Rugs","Select a category.",mRugMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_RugMenu);
		}
		break;
	case 1: //Plant
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handlePlantMenu","Plant","Select a category.",mPlantMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_PlantMenu);
		}
		break;
	case 2: //Elegant
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleElegantMenu","Elegant","Select a category.",mElegantMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ElegantMenu);
		}
		break;
	case 3: //Modern
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleModernMenu","Modern","Select a category.",mModernMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ModernMenu);
		}
		break;
	case 4://Plain
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handlePlainMenu","Plain","Select a category.",mPlainMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_PlainMenu);
		}
		break;
	case 5://Cheap
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCheapMenu","Cheap","Select a category.",mCheapMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CheapMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleInstrumentMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,1316);
		break;
	case 1:
		GiveItem(player,1318);
		break;
	case 2:
		GiveItem(player,1315);
		break;
	case 3:
		GiveItem(player,1317);
		break;
	case 4:
		GiveItem(player,1314);
		break;
	case 5:
		GiveItem(player,1323);
		break;
	case 6:
		GiveItem(player,1320);
		break;
	case 7:
		GiveItem(player,1319);
		break;
	case 8:
		GiveItem(player,1313);
		break;
	case 9:
		GiveItem(player,1322);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleToolMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0://SurveyTool
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleSurveyToolMenu","Survey Tool","Select a category.",mSurveyToolMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_SurveyToolMenu);
		}
		break;
	case 1://CraftingTool
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCraftingToolMenu","Crafting Tool","Select a category.",mCraftingToolMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CraftingToolMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleWeaponMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0://Melee
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleMeleeMenu","Melee","Select a category.",mMeleeMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_MeleeMenu);
		}
		break;
	case 1://Ranged
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleRangedMenu","Ranged","Select a category.",mRangedMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_RangedMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleArmorMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
		switch(element)
	{
	case 0://Bone
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleBoneArmorMenu","Bone Armor","Select a category.",mBoneArmorMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_BoneArmorMenu);
		}
		break;
	case 1://CompositeArmor
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCompositeArmorMenu","Composite Armor","Select a category.",mCompositeArmorMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CompositeArmorMenu);
		}
		break;
	case 2://UbeseArmor
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleUbeseArmorMenu","Ubese Armor","Select a category.",mUbeseArmorMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_UbeseArmorMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleHarvesterMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{	
	switch(element)
	{
	case 0:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleFloraMenu","Flora","Select a category.",mFloraMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_FloraMenu);
		}
		break;
	case 1:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleGasMenu","Gas","Select a category.",mGasMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_GasMenu);
		}
		break;
	case 2:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleChemicalMenu","Chemical","Select a category.",mChemicalMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ChemicalMenu);
		}
		break;
	case 3:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleWaterMenu","Water","Select a category.",mWaterMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_WaterMenu);
		}
		break;
	case 4:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleMineralMenu","Mineral","Select a category.",mMineralMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_MineralMenu);
		}
		break;
	default:
		break;
	}

}
void CharacterBuilderTerminal::_handleHouseMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleGenericMenu","Generic","Select a category.",mGenericMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_GenericMenu);
		}
		break;
	case 1:
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleTatooineMenu","Tatooine","Select a category.",mTatooineMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_TatooineMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleRugMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,378);
		break;
	case 1:
		GiveItem(player,285);
		break;
	case 2:
		GiveItem(player,472);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handlePlantMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
		switch(element)
	{
	case 0:
		GiveItem(player,364);
		break;
	case 1:
		GiveItem(player,312);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handleElegantMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,260);
		break;
	case 1:
		GiveItem(player,328);
		break;
	case 2:
		GiveItem(player,403);
		break;
	case 3:
		GiveItem(player,254);
		break;
	case 4:
		GiveItem(player,433);
		break;
	case 5:
		GiveItem(player,322);
		break;
	case 6:
		GiveItem(player,408);
		break;
	case 7:
		GiveItem(player,229);
		break;
	case 8:
		GiveItem(player,217);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handleModernMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,330);
		break;
	case 1:
		GiveItem(player,263);
		break;
	case 2:
		GiveItem(player,302);
		break;
	case 3:
		GiveItem(player,296);
		break;
	case 4:
		GiveItem(player,425);
		break;
	case 5:
		GiveItem(player,332);
		break;
	case 6:
		GiveItem(player,264);
		break;
	case 7:
		GiveItem(player,464);
		break;
	case 8:
		GiveItem(player,457);
		break;
	case 9:
		GiveItem(player,232);
		break;
	case 10:
		GiveItem(player,334);
		break;

	default:
			break;
	}
}
void CharacterBuilderTerminal::_handlePlainMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,424);
		break;
	case 1:
		GiveItem(player,434);
		break;
	case 2:
		GiveItem(player,456);
		break;
	case 3:
		GiveItem(player,344);
		break;
	case 4:
		GiveItem(player,359);
		break;
	case 5:
		GiveItem(player,438);
		break;
	case 6:
		GiveItem(player,289);
		break;
	case 7:
		GiveItem(player,376);
		break;
	case 8:
		GiveItem(player,419);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handleCheapMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,252);
		break;
	case 1:
		GiveItem(player,227);
		break;
	case 2:
		GiveItem(player,309);
		break;
	case 3:
		GiveItem(player,213);
		break;
	case 4:
		GiveItem(player,467);
		break;
	case 5:
		GiveItem(player,447);
		break;
	case 6:
		GiveItem(player,248);
		break;
	case 7:
		GiveItem(player,223);
		break;
	case 8:
		GiveItem(player,237);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handleMeleeMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0://OneHandSword
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleOneHandSwordMenu","One Hand Sword","Select a category.",mOneHandSwordMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_OneHandSwordMenu);
		}
		break;
	case 1://TwoHandSword
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleTwoHandSwordMenu","Two Hand Sword","Select a category.",mTwoHandSwordMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_TwoHandSwordMenu);
		}
		break;
	case 2://Axe
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleAxeMenu","Axe","Select a category.",mAxeMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_AxeMenu);
		}
		break;
	case 3://Baton
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleBatonMenu","Baton","Select a category.",mBatonMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_BatonMenu);
		}
		break;
	case 4://Polearm
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handlePolearmMenu","Polearm","Select a category.",mPolearmMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_PolearmMenu);
		}
		break;
	case 5://Knife
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleKnifeMenu","Knife","Select a category.",mKnifeMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_KnifeMenu);
		}
		break;
	case 6://Unarmed
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"UnarmedMenu","Unarmed","Select a category.",mUnarmedMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_UnarmedMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleRangedMenu(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0://Carbine
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleCarbineMenu","Carbine","Select a category.",mCarbineMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_CarbineMenu);
		}
		break;
	case 1://Thrown
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleThrownMenu","Thrown","Select a category.",mThrownMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_ThrownMenu);
		}
		break;
	case 2://Heavy
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleHeavyMenu","Heavy","Select a category.",mHeavyMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_HeavyMenu);
		}
		break;
	case 3://Pistol
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handlePistolMenu","Pistol","Select a category.",mPistolMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_PistolMenu);
		}
		break;
	case 4://Rifle
		if(playerObject->isConnected())
		{
			gUIManager->createNewListBox(this,"handleRifleMenu","Rifle","Select a category.",mRifleMenu,playerObject,SUI_Window_CharacterBuilder_ListBox_RifleMenu);
		}
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleBoneArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		{
		GiveItem(player,1131);
		GiveItem(player,615);
		GiveItem(player,608);
		GiveItem(player,769);
		GiveItem(player,1177);
		GiveItem(player,1257);
		GiveItem(player,705);
		GiveItem(player,1001);
		GiveItem(player,870);
		}
		break;
	case 1:
		GiveItem(player,870);
		break;
	case 2:
		GiveItem(player,1131);
		break;
	case 3:
		GiveItem(player,615);
		break;
	case 4:
		GiveItem(player,608);
		break;
	case 5:
		GiveItem(player,769);
		break;
	case 6:
		GiveItem(player,1177);
		break;
	case 7:
		GiveItem(player,1257);
		break;
	case 8:
		GiveItem(player,705);
		break;
	case 9:
		GiveItem(player,1001);
		break;
	case 10:
		GiveItem(player,870);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleCompositeArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		{
		GiveItem(player,518);
		GiveItem(player,784);
		GiveItem(player,1195);
		GiveItem(player,727);
		GiveItem(player,666);
		GiveItem(player,864);
		GiveItem(player,601);
		GiveItem(player,978);
		GiveItem(player,1107);
		}
		break;
	case 1:
		GiveItem(player,518);
		break;
	case 2:
		GiveItem(player,784);
		break;
	case 3:
		GiveItem(player,1195);
		break;
	case 4:
		GiveItem(player,727);
		break;
	case 5:
		GiveItem(player,666);
		break;
	case 6:
		GiveItem(player,864);
		break;
	case 7:
		GiveItem(player,601);
		break;
	case 8:
		GiveItem(player,978);
		break;
	case 9:
		GiveItem(player,1107);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleUbeseArmorMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		{
		GiveItem(player,1249);
		GiveItem(player,711);
		GiveItem(player,1281);
		GiveItem(player,902);
		GiveItem(player,772);
		GiveItem(player,1232);
		GiveItem(player,1196);
		GiveItem(player,1296);
		}
		break;
	case 1:
		GiveItem(player,1249);
		break;
	case 2:
		GiveItem(player,711);
		break;
	case 3:
		GiveItem(player,1281);
		break;
	case 4:
		GiveItem(player,902);
		break;
	case 5:
		GiveItem(player,772);
		break;
	case 6:
		GiveItem(player,1232);
		break;
	case 7:
		GiveItem(player,1196);
		break;
	case 8:
		GiveItem(player,1296);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleOneHandSwordMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2301);
		break;
	case 1:
		GiveItem(player,2651);
		break;
	case 2:
		GiveItem(player,2302);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleTwoHandSwordMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
		switch(element)
	{
	case 0:
		GiveItem(player,2272);
		break;
	case 1:
		GiveItem(player,2273);
		break;
	case 2:
		GiveItem(player,2274);
		break;
	case 3:
		GiveItem(player,2275);
		break;
	case 4:
		GiveItem(player,2561);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleBatonMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2278);
		break;
	case 1:
		GiveItem(player,2279);
		break;
	case 2:
		GiveItem(player,2627);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handlePolearmMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2288);
		break;
	case 1:
		GiveItem(player,2638);
		break;
	case 2:
		GiveItem(player,2289);
		break;
	case 3:
		GiveItem(player,2290);
		break;
	case 4:
		GiveItem(player,2291);
		break;
	case 5:
		GiveItem(player,2292);
		break;
	case 6:
		GiveItem(player,2293);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleKnifeMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2280);
		break;
	case 1:
		GiveItem(player,2628);
		break;
	case 2:
		GiveItem(player,2629);
		break;
	case 3:
		GiveItem(player,2630);
		break;
	case 4:
		GiveItem(player,2631);
		break;
	case 5:
		GiveItem(player,2281);
		break;
	case 6:
		GiveItem(player,2282);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleCarbineMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2307);
		break;
	case 1:
		GiveItem(player,2745);
		break;
	case 2:
		GiveItem(player,2308);
		break;
	case 3:
		GiveItem(player,2746);
		break;
	case 4:
		GiveItem(player,2309);
		break;
	case 5:
		GiveItem(player,2310);
		break;
	case 6:
		GiveItem(player,2311);
		break;
	case 7:
		GiveItem(player,2312);
		break;
	case 8:
		GiveItem(player,2313);
		break;
	case 9:
		GiveItem(player,2314);
		break;
	case 10:
		GiveItem(player,2749);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleThrownMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2750);
		break;
	case 1:
		GiveItem(player,2315);
		break;
	case 2:
		GiveItem(player,2751);
		break;
	case 3:
		GiveItem(player,2316);
		break;
	case 4:
		GiveItem(player,2317);
		break;
	case 5:
		GiveItem(player,2318);
		break;
	case 6:
		GiveItem(player,2319);
		break;
	case 7:
		GiveItem(player,2752);
		break;
	case 8:
		GiveItem(player,2320);
		break;
	case 9:
		GiveItem(player,2321);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handleHeavyMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2322);
		break;
	case 1:
		GiveItem(player,2323);
		break;
	case 2:
		GiveItem(player,2324);
		break;
	case 3:
		GiveItem(player,2325);
		break;
	case 4:
		GiveItem(player,2753);
		break;
	default:break;
	}
}
void CharacterBuilderTerminal::_handlePistolMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2326);
		break;
	case 1:
		GiveItem(player,2754);
		break;
	case 2:
		GiveItem(player,2755);
		break;
	case 3:
		GiveItem(player,2327);
		break;
	case 4:
		GiveItem(player,2756);
		break;
	case 5:
		GiveItem(player,2328);
		break;
	case 6:
		GiveItem(player,2329);
		break;
	case 7:
		GiveItem(player,2330);
		break;
	case 8:
		GiveItem(player,2331);
		break;
	case 9:
		GiveItem(player,2332);
		break;
	case 10:
		GiveItem(player,2333);
		break;
	case 11:
		GiveItem(player,2334);
		break;
	case 12:
		GiveItem(player,2335);
		break;
	case 13:
		GiveItem(player,2336);
		break;
	case 14:
		GiveItem(player,2337);
		break;
	case 15:
		GiveItem(player,2760);
		break;
	case 16:
		GiveItem(player,2761);
		break;
	case 17:
		GiveItem(player,2338);
		break;
	case 18:
		GiveItem(player,2339);
		break;
	case 19:
		GiveItem(player,2762);
		break;
	case 20:
		GiveItem(player,2340);
		break;
	default:
			break;
	}
}
void CharacterBuilderTerminal::_handleRifleMenu(PlayerObject* player, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,2341);
		break;
	case 1:
		GiveItem(player,2342);
		break;
	case 2:
		GiveItem(player,2763);
		break;
	case 3:
		GiveItem(player,2343);
		break;
	case 4:
		GiveItem(player,2344);
		break;
	case 5:
		GiveItem(player,2345);
		break;
	case 6:
		GiveItem(player,2346);
		break;
	case 7:
		GiveItem(player,2347);
		break;
	case 8:
		GiveItem(player,2764);
		break;
	case 9:
		GiveItem(player,2348);
		break;
	case 10:
		GiveItem(player,2349);
		break;
	case 11:
		GiveItem(player,2350);
		break;
	case 12:
		GiveItem(player,2765);
		break;
	case 13:
		GiveItem(player,2351);
		break;
	case 14:
		GiveItem(player,2352);
		break;
	case 15:
		GiveItem(player,2353);
		break;
	case 16:
		GiveItem(player,2354);
		break;
	case 17:
		GiveItem(player,2355);
		break;
	case 18:
		GiveItem(player,2767);
		break;
	default:
			break;
	}
}

void CharacterBuilderTerminal::_handleCSRItemSelect(PlayerObject* playerObject, uint32 action,int32 element,string inputStr,UIWindow* window)
{
	uint32 inputId = 0;
	
	if(swscanf(inputStr.getUnicode16(),L"%u",&inputId) == 1)
	{
		GiveItem(playerObject,inputId);
	}

	BStringVector dropDowns;
	gUIManager->createNewInputBox(this, "handleInputItemId", "Get Item", "Enter the item ID", dropDowns, playerObject, SUI_IB_NODROPDOWN_OKCANCEL, SUI_Window_CharacterBuilderItemIdInputBox,8);
}
void CharacterBuilderTerminal::handleObjectMenuSelect(uint8 messageType,Object* srcObject)
{
	if(messageType == radId_itemUse)
	{
		PlayerObject* playerObject = dynamic_cast<PlayerObject*>(srcObject);

		// bring up the terminal window
		if(playerObject && playerObject->isConnected())
		{
			if(playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead() || playerObject->checkState(CreatureState_Combat))
			{
				return;
			}

			if(playerObject->getCsrTag() > 0)
			{
				gUIManager->createNewListBox(this,"handleMainMenu","Main menu","Select a category.", mMainCsrMenu,playerObject,SUI_Window_CharacterBuilderMainMenu_ListBox,SUI_LB_OK,this->getId());
			} else {
				gUIManager->createNewListBox(this,"handleMainMenu","Main menu","Select a category.", mMainMenu,playerObject,SUI_Window_CharacterBuilderMainMenu_ListBox,SUI_LB_OK,this->getId());
			}
		}
	}
	else
	{
		gLogger->log(LogManager::NOTICE,"TravelTerminal: Unhandled MenuSelect: %u",messageType);
	}
}

//=============================================================================
void  CharacterBuilderTerminal::handleUIEvent(uint32 action,int32 element,string inputStr,UIWindow* window)
{
		PlayerObject* playerObject = window->getOwner();

	if(!playerObject || action || playerObject->getSurveyState() || playerObject->getSamplingState() || playerObject->isIncapacitated() || playerObject->isDead())
	{
		return;
	}

	switch(window->getWindowType())
	{
		case SUI_Window_CharacterBuilderMainMenu_ListBox:
			_handleMainMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ExperienceMenu:
			_handleExperienceMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CreditMenu:
		case SUI_Window_CharacterBuilderCreditsMenuInventory_InputBox:
		case SUI_Window_CharacterBuilderCreditsMenuBank_InputBox:
			_handleCreditMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_BuffMenu:
			_handleBuffMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ItemMenu:
			_handleItemMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilderResourcesTypesMenu_ListBox:
			_handleResourcesTypes(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ResourceMenu:
			_handleResourceMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilderResourcesCRCMenu_ListBox:
			_handleResourcesCRC(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_StructureMenu:
			_handleStructureMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_FurnitureMenu:
			_handleFurnitureMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_VehicleMenu:
			GiveItem(playerObject, 1736+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_InstrumentMenu:
			_handleInstrumentMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ToolMenu:
			_handleToolMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_WeaponMenu:
			_handleWeaponMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ArmorMenu:
			_handleArmorMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_FactoryMenu:
			GiveItem(playerObject, 1590+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_HarvesterMenu:
			_handleHarvesterMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CampMenu:
			GiveItem(playerObject,1970+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_HouseMenu:
			_handleHouseMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_RugMenu:
			_handleRugMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_PlantMenu:
			_handlePlantMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ElegantMenu:
			_handleElegantMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ModernMenu:
			_handleModernMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_PlainMenu:
			_handlePlainMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CheapMenu:
			_handleCheapMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_SurveyToolMenu:
			GiveItem(playerObject, 1+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CraftingToolMenu:
			GiveItem(playerObject, 11+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_MeleeMenu:
			_handleMeleeMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_RangedMenu:
			_handleRangedMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_BoneArmorMenu:
			_handleBoneArmorMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CompositeArmorMenu:
			_handleCompositeArmorMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_UbeseArmorMenu:
			_handleUbeseArmorMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_FloraMenu:
			GiveItem(playerObject,1601+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_GasMenu:
			GiveItem(playerObject,1604+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ChemicalMenu:
			GiveItem(playerObject,1607+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_WaterMenu:
			GiveItem(playerObject,1610+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_MineralMenu:
			GiveItem(playerObject,1613+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_GenericMenu:
			GiveItem(playerObject, 1720+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_TatooineMenu:
			GiveItem(playerObject, 1732+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_OneHandSwordMenu:
			_handleOneHandSwordMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_TwoHandSwordMenu:
			_handleTwoHandSwordMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_AxeMenu:
			GiveItem(playerObject,2276+element);
			break;
		case SUI_Window_CharacterBuilder_ListBox_BatonMenu:
			_handleBatonMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_PolearmMenu:
			_handlePolearmMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_KnifeMenu:
			_handleKnifeMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_UnarmedMenu:
			GiveItem(playerObject,2294);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CarbineMenu:
			_handleCarbineMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_ThrownMenu:
			_handleThrownMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_HeavyMenu:
			_handleHeavyMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_PistolMenu:
			_handlePistolMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_RifleMenu:
			_handleRifleMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilderItemIdInputBox:
			_handleCSRItemSelect(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilderProfessionMastery_ListBox:
			_handleProfessionMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_WoundMenu:
			_handleWoundMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CivicMenu:
			_handleCivicMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_GuildHallMenu:
			_handleGuildMenu(playerObject, action, element, inputStr, window);
			break;
		case SUI_Window_CharacterBuilder_ListBox_CityHallMenu:
			_handleCityMenu(playerObject, action, element, inputStr, window);
			break;
		default:
			break;
	}
}

//=============================================================================


void CharacterBuilderTerminal::_handleCivicMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window)
{
	switch(element)
	{
	case 0:
		gUIManager->createNewListBox(this,"handleGuildHalls","Guild Halls","Select a category.", mGuildHallMenu,player,SUI_Window_CharacterBuilder_ListBox_GuildHallMenu);
		break;
	case 1:
		gUIManager->createNewListBox(this,"handleCityHalls","City Halls","Select a category.", mCityHallMenu,player,SUI_Window_CharacterBuilder_ListBox_CityHallMenu);
		break;
	default:
		break;
	}
}

void CharacterBuilderTerminal::_handleGuildMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,1597);
		break;
	case 1:
		GiveItem(player,1598);
		break;
	case 2:
		GiveItem(player,1599);
		break;
	case 3:
		GiveItem(player,1600);
		break;
	default:
		break;
	}	
}

void CharacterBuilderTerminal::_handleCityMenu(PlayerObject* player, uint32 action, int32 element, string inputStr, UIWindow* window)
{
	switch(element)
	{
	case 0:
		GiveItem(player,1566);
		break;
	case 1:
		GiveItem(player,1567);
		break;
	case 2:
		GiveItem(player,1568);
		break;
	default:
		break;
	}
}
