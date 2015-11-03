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


#include "EntertainerManager.h"
#include "Inventory.h"
#include "PlayerObject.h"
#include "UIManager.h"
#include "WorldManager.h"

#include "ContainerManager.h"
#include "SpatialIndexManager.h"
#include "MessageLib/MessageLib.h"




#include "Utils/logger.h"

#include "DatabaseManager/Database.h"

#include "NetworkManager/MessageFactory.h"

//======================================================================================================================
//gets the information on a holoemote from the loaded db data
//
HoloStruct* EntertainerManager::getHoloEmoteByCRC(uint32 crc)
{

    HoloEmoteEffects::iterator it = mHoloList.begin();
    //bool found = false;
    while(it != mHoloList.end())
    {
        if ((*it)->pCRC == crc)
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

//======================================================================================================================
//gets the information on a holoemote from the loaded db data
//
HoloStruct* EntertainerManager::getHoloEmoteByClientCRC(uint32 crc)
{

    HoloEmoteEffects::iterator it = mHoloList.begin();
    //bool found = false;
    while(it != mHoloList.end())
    {
        if ((*it)->pClientCRC == crc)
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

//======================================================================================================================

BString EntertainerManager::getHoloNames()
{
    int8 collection[512];
    bool isNew = true;

    HoloEmoteEffects::iterator it = mHoloList.begin();
    while(it != mHoloList.end())
    {
        if ((*it)->pCRC != BString("all").getCrc())
        {
            if(isNew)
            {
                sprintf(collection,"%s",(*it)->pEmoteName);
                isNew = false;
            }
            else
                sprintf(collection,"%s, %s",collection,(*it)->pEmoteName);
        }
        it++;
    }

    return collection;
}

//======================================================================================================================

HoloStruct* EntertainerManager::getHoloEmoteIdbyName(BString name)
{

    HoloEmoteEffects::iterator it = mHoloList.begin();
    //bool found = false;
    while(it != mHoloList.end())
    {
        if (BString((*it)->pEmoteName).getCrc() == name.getCrc())
        {
            return (*it);
        }
        it++;
    }

    return NULL;
}

//=============================================================================
//test and apply the returned scale
//max and min values might need to be DB based at some point
//however the client has his own max and min values in the iffs so hardcoding at this time should be viable

BString EntertainerManager::commitIdheight(PlayerObject* customer, float value)
{
    float addtoMinScale = 0.0F;
    float minScale = 1.0F;
    float totalScale = 0.0F;
    float maxScale = 2.5F;

    switch(customer->getRaceId())
    {
    case 0:
    {
        //human
        //min 0.89
        //max 1.11
        if (value > 0.0F)
            addtoMinScale = 0.22F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.89F;
        totalScale = minScale+addtoMinScale;
    }
    break;

    case 1:
    {
        //rodian
        //min 0.81
        //max 0.94
        if (value > 0.0F)
            addtoMinScale = 0.13F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.81F;
        totalScale = minScale + addtoMinScale;
    }
    break;

    case 2:
    {
        //trandoshan
        //min 1.01
        //max 1.25
        if (value > 0.0F)
            addtoMinScale = 0.24F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 1.01F;
        totalScale = minScale + addtoMinScale;
    }
    break;

    case 3:
    {
        //moncal
        //min 0.89
        //max 1.0
        if (value > 0.0F)
            addtoMinScale = 0.11F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.89F;
        totalScale = minScale + addtoMinScale;
    }
    break;

    case 4:
    {
        //wookiee
        //min 1.08
        //max 1.25
        if (value > 0.0F)
            addtoMinScale = 0.17F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 1.08F;
        totalScale = minScale + addtoMinScale;
    }
    break;

    case 5:
    {
        //bothan
        //min 0.75
        //max 0.83
        float newVal = static_cast<float>(0.08*value);

        minScale = 0.75F;
        maxScale = 0.83F;
        totalScale = minScale + newVal;

    }
    break;

    case 6:
    {
        //twilek
        //min 0.92
        //max 1.11
        if (value > 0.0F)
            addtoMinScale = 0.19F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.92F;
        totalScale = minScale + addtoMinScale;
    }
    break;
    case 7:
    {
        //zabrak
        //min 0.92
        //max 1.06
        if (value > 0.0F)
            addtoMinScale = 0.14F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.92F;
        totalScale = minScale + addtoMinScale;
    }
    break;
    case 33:
    {
        //ithorian
        //min 0.92
        //max 1.06
        if (value > 0.0F)
            addtoMinScale = static_cast<float>(0.14 * value);
        else
            addtoMinScale = 0.0F;

        minScale = 0.92F;
        totalScale = minScale + addtoMinScale;
    }
    break;
    case 49:
    {
        //sullustan
        //min 0.92
        //max 1.06
        if (value > 0.0F)
            addtoMinScale = 0.14F * value;
        else
            addtoMinScale = 0.0F;

        minScale = 0.92F;
        totalScale = minScale + addtoMinScale;
    }
    break;
    }
    if((totalScale)> maxScale)
        totalScale = maxScale;


    customer->setScale(totalScale);

    //update the db
    int8 sql[50];

    sprintf(sql,"scale = '%.2f'",(totalScale));
    return(BString(sql));
}

//=============================================================================
//returns the XP for a given attribute change
//

uint32 EntertainerManager::getIdXP(BString attribute, uint16 value)
{
    IDStruct*	iDContainer = getIDAttribute(attribute.getCrc());
    if(!iDContainer)
    {
        return 0;
    }
    return iDContainer->XP;
}
//=============================================================================
//looks up the corresponding index and indexstring for Colors
//updates the indexvalue and prepares the attribute string to be added to the sql string
//
BString EntertainerManager::commitIdColor(PlayerObject* customer, BString attribute, uint16 value)
{

    BString		genderrace;
    int8		mString[64];
    char		*Token;
    char		separation[] = ".";

    sprintf(mString,"%s",&customer->getModelString().getAnsi()[30]);



    Token = strtok(mString,separation);
    genderrace = Token;

    IDStruct*	iDContainer = getIDAttribute(attribute.getCrc(),genderrace.getCrc());
    if(!iDContainer)
    {
        return(BString(""));
    }

    if (value == 0)
        value = 511;

    if (value == 255)
        value = 767;



    //check whether we are modifying the hair object
    if(iDContainer->hair)
    {
        //if(TangibleObject* hair = dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair)))
        if(TangibleObject* hair = dynamic_cast<TangibleObject*>(customer->getHair()))
        {
            hair->setCustomization(static_cast<uint8>(iDContainer->Atr1ID),value,3);

            //update hair customization db side seperately
            int8 sql[300];
            sprintf(sql,"UPDATE %s.character_appearance set %s = %u where character_id = '%" PRIu64 "'",mDatabase->galaxy(),iDContainer->Atr1Name, value,customer->getId());
            mDatabase->executeSqlAsync(NULL,NULL,sql);
            

            //update object clientside
            // now update the hair customization string
            gMessageLib->sendUpdateCustomization_InRange(hair,customer);

            //Udate equiplist
            //the equiplist is what makes us have the right hair and clothes in the ID ui
            //additionally it updates the hair properly on the login ui after change

            //make sure were not wearing a helmet
            TangibleObject* hairSlot = dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
            if(hairSlot&&hairSlot->getTangibleType() == TanType_Hair)
            {
                customer->getEquipManager()->removeEquippedObject(CreatureEquipSlot_Hair);
                gMessageLib->sendEquippedListUpdate_InRange(customer);

                customer->getEquipManager()->addEquippedObject(CreatureEquipSlot_Hair,hair);
                gMessageLib->sendEquippedListUpdate_InRange(customer);
            }
            //sends to inRange by default
            //cant get the update to work as intended :(
            //gMessageLib->sendEquippedItemUpdate_InRange(customer,hair->getId());

            return BString("");
        }
        else
        {
            return BString("");
        }
    }
    else
    {
        customer->setCustomization(static_cast<uint8>(iDContainer->Atr1ID),value);
    }

    int8 sql[50];
    sprintf(sql,"%s = '%u'",iDContainer->Atr1Name,value);

    return(BString(sql));

    //changes to the hair object (different hair) are only shown on selection when we update the inventory list
    // in the creo 6 - in this case the later color change is shown too
    //when we only change the color there is no change to haircolo

}
//=============================================================================
//looks up the corresponding index and indexstring for Attributes
//updates the indexvalue and prepares the attribute string to be added to the sql string
//

BString EntertainerManager::commitIdAttribute(PlayerObject* customer, BString attribute, float value)
{
    uint32	crc = attribute.getCrc();
    int8	add[50],mString[64];
    uint16	uVal,sk;
    BString genderrace;

    //sprintf(attrName,"");
    //sprintf(add,"");

    // get our gender and race so we can get hold of our Information from the id_attribute table
    sprintf(mString,"%s",&customer->getModelString().getAnsi()[30]);

    char *Token;
    char separation[] = ".";

    Token = strtok(mString,separation); //
    genderrace = Token;

    IDStruct*	iDContainer = getIDAttribute(attribute.getCrc(),genderrace.getCrc());

    if(!iDContainer)
    {
        return(BString(""));
    }

    float fValue = (value*iDContainer->divider);
    uVal = (uint16) (fValue);

    if (uVal == 0)
        uVal = 511;

    if (uVal == 255)
        uVal = 767;

    //CAVE chest is handled over 2 attributes I cant find the 2nd apart from 2b in the list
    //however when you ahh 80h to the 2b (first attribute) you get to the so called 2 attribute version
    //in case of chests we have then the attribute ID ABh followed by TWO values as returned by the client on
    //char creation

    if ((crc ==523041409)&(customer->getGender())) //chest
    {

        //Slider goes from 1 to 100
        uint16 v1,v2;
        v1 = 0;
        v2 = 0;

        uint8 slider = (uint8)(value * 100);

        if(slider < 51)
        {
            v2 = 0;
            //v1 goes from 155 for 0 to 0 for 50
            v1 = (uint8)155-((155/50)*slider);
        }
        else
        {
            v2 = 255;
            //v1 goes from 253 for 51
            //to 101 for 100
            slider = slider-50;
            uint8 amount = ((155/50)*slider);
            v1 = 256 - amount;

        }

        customer->setCustomization (171,v1);

        customer->setCustomization (172,v2);

        //overwrites 2 value version so set to zero (so it gets ignored)
        customer->setCustomization (0x2b,0);

        int8 sql[250];
        sprintf(sql,"ABFF = '%u', AB2FF = '%u'",v1,v2);
        return(BString(sql));
    }

    customer->setCustomization ((uint8)iDContainer->Atr1ID,uVal);

    //sometimes we have 2 attributes corresponding to one feature - like weight
    //it has the attributes skinny and fat which need to be set to corresponding values
    //that means the more fat the less skinny
    if(iDContainer->Atr2ID > 0)
    {
        if(uVal == 511)
            sk = 767;
        else if(uVal == 767)
            sk = 511;
        else
        {
            sk = 255 - uVal;
        }

        sprintf(add,",%s = '%u'",iDContainer->Atr2Name,sk);
        customer->setCustomization(static_cast<uint8>(iDContainer->Atr2ID),sk);
    }

    int8 sql[150];
    sprintf(sql,"%s = '%u'%s",iDContainer->Atr1Name,uVal,add);

    return(BString(sql));

}
//=============================================================================
//gets called by commitIdChanges to
//apply the Hair changes
//
void EntertainerManager::applyHair(PlayerObject* customer,BString newHairString)
{
	int8 sql[1024];

	//the reason the hair is unequipped when wearing a helmet is because otherwise parts of the hair looked out of the helmet, which plainly looks stupid

	//hark the equiplist might contain a helmet at this spot
	TangibleObject*				playerHair		= dynamic_cast<TangibleObject*>(customer->getHair());//dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));
	TangibleObject*				playerHairSlot	= dynamic_cast<TangibleObject*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Hair));

	bool hairEquipped;
	//do we need to equip the hair or are we wearing a helmet????
	if(!playerHairSlot)
		hairEquipped = true;//not technically equipped BUT were NOT wearing a helmet
	else
		hairEquipped = (playerHairSlot->getTangibleType() == TanType_Hair);

	BString						customization	= "";

	if(playerHair)
	{
		//are we wearing a helmet ? if not we need to update the world
		if(hairEquipped)
		{
			customization = playerHair->getCustomizationStr();

			//Udate equiplist
			customer->getEquipManager()->removeEquippedObject(CreatureEquipSlot_Hair);
			
			gContainerManager->updateEquipListToRegisteredPlayers(customer);
		
			gContainerManager->destroyObjectToRegisteredPlayers(customer,playerHair->getId(), true);


			//Update the db only if we remain bald
			if(!newHairString.getLength())
			{
				// update the db
				sprintf(sql,"UPDATE %s.character_appearance set hair = '' where character_id = '%" PRIu64 "'",mDatabase->galaxy(),customer->getId());
				mDatabase->executeSqlAsync(NULL,NULL,sql);
			}
		}
		//destroy serverside
		delete(playerHair);
		customer->setHair(NULL);

	}

	//do we have new hair ??
	if(newHairString.getLength())
	{
		playerHair		= new TangibleObject();
		customer->setHair(playerHair);

		int8 tmpHair[128];
		sprintf(tmpHair,"object/tangible/hair/%s/shared_%s",customer->getSpeciesString().getAnsi(),&newHairString.getAnsi()[22 + customer->getSpeciesString().getLength()]);
		playerHair->setId(customer->getId() + 8);
		playerHair->setParentId(customer->getId());
		playerHair->setModelString(tmpHair);
		playerHair->setTangibleGroup(TanGroup_Hair);
		playerHair->setTangibleType(TanType_Hair);
		playerHair->setName("hair");
		playerHair->setNameFile("hair_name");
		playerHair->setCustomizationStr((uint8*)customization.getAnsi());



		// update the db
		sprintf(sql,"UPDATE %s.character_appearance set hair = '%s' where character_id = '%" PRIu64 "'",mDatabase->galaxy(),newHairString.getAnsi(),customer->getId());
		mDatabase->executeSqlAsync(NULL,NULL,sql);

		// now update the modelstring in the creo6 equipped list and the corresponding tano
		//are we wearing a helmet ? if not we need to update the world
		if(hairEquipped)
		{
			customer->getEquipManager()->addEquippedObject(CreatureEquipSlot_Hair,playerHair);
			
			gContainerManager->updateEquipListToRegisteredPlayers(customer);
			gContainerManager->createObjectToRegisteredPlayers(customer,playerHair);

		}
	}
}

//=============================================================================
//handles the financial side
//
void EntertainerManager::applyMoney(PlayerObject* customer,PlayerObject* designer,int32 amount)
{
    int32 amountcash;
    int32 amountbank;
    int8 sql[1024];
    amountcash = amount;
    amountbank = 0;
    Inventory* inventory = dynamic_cast<Inventory*>(customer->getEquipManager()->getEquippedObject(CreatureEquipSlot_Inventory));

    if(inventory && inventory->getCredits() < amount)
    {
        // cash alone isnt sufficient
        amountcash = inventory->getCredits();
        amountbank = (amount - amountcash);
    }

    EntertainerManagerAsyncContainer* asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_IDFinances,0);
    Transaction* mTransaction = mDatabase->startTransaction(this,asyncContainer);

    asyncContainer->customer = customer;
    asyncContainer->performer = designer;
    asyncContainer->amountcash = amountcash;
    asyncContainer->amountbank = amountbank;


    sprintf(sql,"UPDATE %s.inventories SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),amountcash, customer->getId()+1);
    mTransaction->addQuery(sql);
    sprintf(sql,"UPDATE %s.banks SET credits=credits-%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),amountbank, customer->getId()+4);
    mTransaction->addQuery(sql);
    sprintf(sql,"UPDATE %s.banks SET credits=credits+%i WHERE id=%" PRIu64 "",mDatabase->galaxy(),amount, designer->getId()+4);
    mTransaction->addQuery(sql);

    mTransaction->execute();
    
}

//=============================================================================
//forcibly ends the design session should the customer refuse to
//
bool EntertainerManager::handleImagedesignTimeOut(CreatureObject* designer)
{
    // Was a valid object passed in?
    if (!designer) {
        return false;
    }

    PlayerObject* imageDesigner;

    //check whether (the old) ID Session is still active
    //if the session has been ended, we have been removed, anyway
    try {
        imageDesigner = dynamic_cast<PlayerObject*>(designer);
    } catch(...) {
        // The player must have disconnected or otherwise been removed and all
        // we have now is a dangling pointer. Bail out.
        return false;
    }

    // The image designer isn't a player object? bail out.
    if(!imageDesigner) {
        return false;
    }

    if (imageDesigner->getImageDesignSession() != IDSessionID) {
        //Panik!!!!!!!
        return false;
    }

    PlayerObject* customer = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(imageDesigner->getIDPartner()));

    if(customer)
    {
        gMessageLib->sendIDEndMessage(customer,customer,imageDesigner,"", 0,0, 0,0,0,0,0);
        customer->setIDPartner(0);
        customer->SetImageDesignSession(IDSessionNONE);
    }

    imageDesigner->setIDPartner(0);
    imageDesigner->SetImageDesignSession(IDSessionNONE);
    return false;

}


//=============================================================================
//commits the changes of the ID session
//
void EntertainerManager::commitIdChanges(PlayerObject* customer,PlayerObject* designer, BString hair, uint32 amount,uint8 statMigration, BString holoEmote,uint8 flagHair)
{
    Ham* ham = designer->getHam();
    if(ham->checkMainPools(0,0,66))
    {
        //int32 mindCost = 66;
        designer->getHam()->updatePropertyValue(HamBar_Mind,HamProperty_CurrentHitpoints,-(66));
    }

    int8 sql[1024];
    EntertainerManagerAsyncContainer* asyncContainer;

    //money
    if(amount >0)
        applyMoney(customer,designer,amount);


    ////////
    //Hair//
    ///////
    if(flagHair)
        applyHair(customer,hair);



    ////////////
    //attributes
    ///////////
    int8						mySQL[2048];
    AttributesList*				aList			 = customer->getIdAttributesList();
    AttributesList::iterator	it				 = aList->begin();
    BString						data;
    bool						firstUpdate		 = true;

    sprintf(mySQL,"UPDATE %s.character_appearance set ",mDatabase->galaxy());

    while(it != aList->end())
    {
        //apply the attributes and retrieve the data to update the db
        if(it->first.getCrc() != BString("height").getCrc())
        {
            data = commitIdAttribute(customer, it->first, it->second);
        }
        else
        {
            data = commitIdheight(customer, it->second);
        }


        if (firstUpdate)
        {
            sprintf(mySQL,"%s %s",mySQL,data.getAnsi());
            firstUpdate = false;
        }
        else
            sprintf(mySQL,"%s, %s",mySQL,data.getAnsi());

        ++it;
    }

    //colors
    ColorList* cList = customer->getIdColorList();
    ColorList::iterator cIt = cList->begin();
    while(cIt != cList->end())
    {
        data = commitIdColor(customer, cIt->first, cIt->second);
        if(data.getLength())
        {
            if (firstUpdate)
            {
                sprintf(mySQL,"%s %s",mySQL,data.getAnsi());
                firstUpdate = false;
            }
            else
                sprintf(mySQL,"%s, %s",mySQL,data.getAnsi());
        }


        ++cIt;
    }

    //do we have actual data or only the primer ??? "UPDATE character_appearance set "
    if(strlen(mySQL) > 33)
    {
        sprintf(sql,"%s where character_id = '%" PRIu64 "'",mySQL,customer->getId());
        asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
        mDatabase->executeSqlAsync(this,asyncContainer,sql);
        
    }

    //build plus send customization
    //please note that hair object customizatio is send updated and maintained b ycommitIdColor
    customer->buildCustomization(customer->getCustomization());

    gMessageLib->sendCustomizationUpdateCreo3(customer);
    gMessageLib->sendScaleUpdateCreo3(customer);

    //Holoemotes
    if(holoEmote.getLength())
        applyHoloEmote(customer,holoEmote);

    //statmigration
    if(statMigration)
    {
        //xp in case of statmigration will be handled by the callback
        int8 sql[256];
        asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_IDMigrateStats,0);
        asyncContainer->customer = customer;
        asyncContainer->performer = designer;

        sprintf(sql,"SELECT target_health, target_strength, target_constitution, target_action, target_quickness, target_stamina, target_mind, target_focus, target_willpower FROM %s.character_stat_migration where character_id = %" PRIu64 "",mDatabase->galaxy(), customer->getId());
        mDatabase->executeSqlAsync(this,asyncContainer,sql);
        
    }
    else
    {
        //XP does not stack. So in the case of statmigration only 2000xp will be given
        //otherwise xp will be determined here
        //
        uint32		xP		= 0;
        uint32		tempXP	= 0;

        it = aList->begin();
        while(it != aList->end())
        {
            tempXP = getIdXP(it->first, static_cast<uint16>(it->second));
            if(tempXP > xP)
                xP = tempXP;

            ++it;
        }

        cIt = cList->begin();
        while(cIt != cList->end())
        {
            tempXP = getIdXP(cIt->first, cIt->second);
            if(tempXP > xP)
                xP = tempXP;

            ++cIt;
        }

        if(flagHair)
        {
            tempXP = 50;
            if(tempXP > xP)
                xP = tempXP;
        }

        //only half XP if you design yourself
        if(designer == customer)
            gSkillManager->addExperience(XpType_imagedesigner,(uint32)(xP/2),designer);
        else
            gSkillManager->addExperience(XpType_imagedesigner,(uint32)xP,designer);
    }

    //empty the attribute lists
    aList->clear();
    cList->clear();
}

//=============================================================================
//Holoemot data is retrieved and the HoloEmote applied to the player Object

void EntertainerManager::applyHoloEmote(PlayerObject* customer,BString holoEmote)
{
    //get the Data
    HoloStruct* myEmote = getHoloEmoteByClientCRC(holoEmote.getCrc());
    if(!myEmote)
    {
        return;
    }

    //update the PlayerObject
    customer->setHoloEmote(myEmote->pCRC);
    customer->setHoloCharge(20);

    //update the db
    int8 sql[512];
    EntertainerManagerAsyncContainer* asyncContainer;
    asyncContainer = new EntertainerManagerAsyncContainer(EMQuery_NULL,0);
    asyncContainer->customer = customer;

    sprintf(sql,"call %s.sp_CharacterHoloEmoteCreate(%" PRIu64 ",%u,%u)",mDatabase->galaxy(), customer->getId(),myEmote->pCRC,20);
    mDatabase->executeProcedureAsync(this,asyncContainer,sql);


    //send message box holoemote bought

    bool lotsOfStuff = false;

    if(!strcmp(myEmote->pEmoteName,"all"))
    {
        lotsOfStuff = true;
    }

    //just give help
    int8 sql1[512];

    if(lotsOfStuff)
    {
        sprintf(sql1,"Your Holo-Emote generator can play all Holo-Emotes available. You have %u charges remaining."
                "\xa To play your Holo-Emote type \x2fholoemote \x3cname\x3e.\xa To delete your Holo-Emote type \x2fholoemote delete. "
                "\xa Purchasing a new Holo-Emote will automatically delete your current Holo-Emote.",customer->getHoloCharge());

        sprintf(sql,"%s \xa \xa The available Holo-Emote names are: \xa \xa"
                "Beehive \x9 \x9 Blossom \x9 Brainstorm \xa"
                "Bubblehead \x9 Bullhorns \x9 Butterflies \xa"
                "Champagne \x9 Haunted \x9 Hearts \xa"
                "Hologlitter \x9 \x9 Holonotes \x9 Imperial \xa"
                "Kitty \x9 \x9 \x9 Phonytail \x9 Rebel \xa"
                "Sparky",sql1);
    }
    else
    {
        sprintf(sql,"Your current Holo Emote is %s.\xa You have %u charges remaining."
                "\xa To play your Holo-Emote type \x2fholoemote %s.\xa To delete your Holo-Emote type \x2fholoemote delete. "
                "\xa Purchasing a new Holo-Emote will automatically delete your current Holo-Emote.",myEmote->pEmoteName,customer->getHoloCharge(),myEmote->pEmoteName);
    }



    gUIManager->createNewMessageBox(NULL,"holoHelpOff","Holo-Emote Help",sql,customer);
}

//=============================================================================


