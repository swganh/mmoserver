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

#include "CharacterAdminHandler.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>

#ifdef WIN32
#include <regex>
#else
#include <boost/regex.hpp>
#endif

#include <boost/lexical_cast.hpp>

#include "Utils/logger.h"

#include <cppconn/resultset.h>

#include "Utils/utils.h"

#include "NetworkManager/DispatchClient.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageDispatch.h"
#include "NetworkManager/MessageFactory.h"

#include "DatabaseManager/DataBinding.h"
#include "DatabaseManager/Database.h"
#include "DatabaseManager/DatabaseResult.h"

#include "ChatOpcodes.h"

#ifdef WIN32
using std::wregex;
using std::wsmatch;
using std::regex_match;
#else
using boost::wregex;
using boost::wsmatch;
using boost::regex_match;
#endif

CharacterAdminHandler::CharacterAdminHandler(Database* database, MessageDispatch* dispatch) 
    : database_(database)
    , message_dispatch_(dispatch)
{
    // Register our opcodes
    message_dispatch_->RegisterMessageCallback(opClientCreateCharacter, std::bind(&CharacterAdminHandler::_processCreateCharacter, this, std::placeholders::_1, std::placeholders::_2));
    message_dispatch_->RegisterMessageCallback(opClientRandomNameRequest, std::bind(&CharacterAdminHandler::_processRandomNameRequest, this, std::placeholders::_1, std::placeholders::_2));
}


CharacterAdminHandler::~CharacterAdminHandler() {
    // Unregister our callbacks
    message_dispatch_->UnregisterMessageCallback(opClientCreateCharacter);
    message_dispatch_->UnregisterMessageCallback(opLagRequest);
    message_dispatch_->UnregisterMessageCallback(opClientRandomNameRequest);
}


void CharacterAdminHandler::Process() {}


void CharacterAdminHandler::_processRandomNameRequest(Message* message, DispatchClient* client) {
    if(!client) {
        return;
    }

    // If it's not, validate it to the client.
    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opHeartBeat);
    Message* newMessage = gMessageFactory->EndMessage();

    // Send our message to the client.
    client->SendChannelAUnreliable(newMessage, client->getAccountId(), CR_Client, 1);

    // set our object type string.
    std::string object_type = message->getStringAnsi();

    std::stringstream ss;

    ss << "SELECT "<<database_->galaxy() << ".sf_CharacterNameCreate('" << object_type << "')";
    
    database_->executeAsyncSql(ss.str(), [this, client, object_type] (DatabaseResult* result) {
        // Vaalidate the input.
        if (! client || ! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
        
        if (!result_set->next()) {
            LOG(WARNING) << "Unable to generate random name for client [" << client->getAccountId() << "]";

			Message* newMessage = gMessageFactory->EndMessage();
			newMessage->setPendingDelete(true);

            return;
        }

        std::string random_name = result_set->getString(1);

        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opClientRandomNameResponse);
        gMessageFactory->addString(object_type);
        gMessageFactory->addString(std::wstring(random_name.begin(), random_name.end()));
        gMessageFactory->addString("ui");
        gMessageFactory->addUint32(0);
        gMessageFactory->addString("name_approved");
        Message* newMessage = gMessageFactory->EndMessage();

        client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 4);
    });
}


void CharacterAdminHandler::_processCreateCharacter(Message* message, DispatchClient* client)
{
    CharacterCreateInfo characterInfo;
    memset(&characterInfo, 0, sizeof(characterInfo));

    // Using a string as a buffer object for the appearance data.  The string class can parse the message format
    _parseAppearanceData(message, &characterInfo);

    // Get the u16string and convert it to wstring for processing.
    std::u16string raw_character_name = message->getStringUnicode16();
    std::wstring tmp(raw_character_name.begin(), raw_character_name.end());

    // A regular expression that searches for a first name and optional sirname.
    // Only letters, and the ' and - characters are allowed. Only 3 instances
    // of the ' and - characters may be in the entire name, which must be between
    // 3 and 16 characters long.
    const wregex p(L"(?!['-])(?!.*['-].*['-].*['-].*['-])([a-zA-Z][a-z'-]{3,16}?)(?: ([a-zA-Z][a-z'-]{3,16}?))?");
    wsmatch m;

    if (! regex_match(tmp, m, p)) {
        LOG(WARNING) << "Invalid character name [" << std::string(tmp.begin(), tmp.end()) << "]";
        _sendCreateCharacterFailed(15, client);
        return;
    }

    characterInfo.mFirstName = m[1].str().c_str();

    if (m[2].matched) {
        characterInfo.mLastName = m[2].str().c_str();
    }
    
    // Base character model
    message->getStringAnsi(characterInfo.mBaseModel);
    //uint32 crc = characterInfo.mBaseModel.getCrc();

    // Starting city
    message->getStringAnsi(characterInfo.mStartCity);

    // Hair model
    message->getStringAnsi(characterInfo.mHairModel);

    // hair customization
    _parseHairData(message, &characterInfo);

    // Character profession
    message->getStringAnsi(characterInfo.mProfession);

    // Unknown attribute
    /* uint8 unkown = */
    message->getUint8();

    // Character height
    characterInfo.mScale = message->getFloat();

    // Character biography (Optional);
    uint32 bioSize = message->peekUint32();
    if(bioSize)
    {
        message->getStringUnicode16(characterInfo.mBiography);
    }
    else
    {
        message->getUint32();
        characterInfo.mBiography.setType(BSTRType_ANSI);
    }

    uint8 tutorialFlag = message->getUint8();

    if(tutorialFlag)
        characterInfo.mStartCity = "tutorial";
    else
        characterInfo.mStartCity = "default_location";

    // Setup our statement
    int8 sql[4096];

    // Convert our name strings to UTF8 so they can be put in the DB.
    characterInfo.mFirstName.convert(BSTRType_ANSI);
    characterInfo.mBiography.convert(BSTRType_ANSI);
    
    if(characterInfo.mLastName.getType() != BSTRType_ANSI && characterInfo.mLastName.getLength()) {
        characterInfo.mLastName.convert(BSTRType_ANSI);
        
        // Build our procedure call
        sprintf(sql, "CALL %s.sp_CharacterCreate(%"PRIu32", 2,'%s','%s', '%s', '%s', %f",
                database_->galaxy(),
                client->getAccountId(),
                database_->escapeString(characterInfo.mFirstName.getAnsi()).c_str(),
                database_->escapeString(characterInfo.mLastName.getAnsi()).c_str(),
                characterInfo.mProfession.getAnsi(),
                characterInfo.mStartCity.getAnsi(),
                characterInfo.mScale);
    } else {
        sprintf(sql, "CALL %s.sp_CharacterCreate(%"PRIu32", 2, '%s', NULL , '%s', '%s', %f",
                database_->galaxy(),
                client->getAccountId(),
                database_->escapeString(characterInfo.mFirstName.getAnsi()).c_str(),
                characterInfo.mProfession.getAnsi(),
                characterInfo.mStartCity.getAnsi(),
                characterInfo.mScale);
    }

    int8 sql2[4096];

    if(bioSize)
    {
        characterInfo.mBiography = strRep(std::string(characterInfo.mBiography.getAnsi()),"'","''").c_str();
        characterInfo.mBiography = strRep(std::string(characterInfo.mBiography.getAnsi()),"\"","\"\"").c_str();
        characterInfo.mBiography = strRep(std::string(characterInfo.mBiography.getAnsi()),"\\","\\\\").c_str();

        sprintf(sql2,",'%s'",characterInfo.mBiography.getAnsi());
        strcat(sql,sql2);
    }
    else
        strcat(sql,",NULL");

    // Iterate our appearance and append as necessary.
    for (uint32 i = 0; i < 0x71; i++)
    {
        strcat(sql, ", ");
        strcat(sql, boost::lexical_cast<std::string>(characterInfo.mAppearance[i]).c_str());
    }
    strcat(sql, ", ");
    strcat(sql, boost::lexical_cast<std::string>(characterInfo.mAppearance[171]).c_str());
    strcat(sql, ", ");
    strcat(sql, boost::lexical_cast<std::string>(characterInfo.mAppearance[172]).c_str());

    if(characterInfo.mHairModel.getLength())
    {
        sprintf(sql2,",'%s',%u,%u", characterInfo.mHairModel.getAnsi(),characterInfo.mHairCustomization[1],characterInfo.mHairCustomization[2]);
    }

    else
    {
        sprintf(sql2,",NULL,0,0");

    }

    int8 sql3[128];

    sprintf(sql3,",'%s');",characterInfo.mBaseModel.getAnsi());


    strcat(sql2,sql3);
    strcat(sql, sql2);

    //Logging the character create sql for debugging purposes,beware this contains binary data
    database_->executeAsyncProcedure(sql, [this, client] (DatabaseResult* result) {       
        // Vaalidate the input.
        if (! client || ! result) {
            return;
        }

        std::unique_ptr<sql::ResultSet>& result_set = result->getResultSet();
        
        if (!result_set->next()) {
            LOG(WARNING) << "Unable to generate random name for client [" << client->getAccountId() << "]";
            return;
        }

        uint64 query_result = result_set->getUInt64(1);

        if(query_result >= 0x0000000200000000ULL) {
            _sendCreateCharacterSuccess(query_result, client);
        } else {
            _sendCreateCharacterFailed(static_cast<uint32>(query_result), client);
        }
    });
}


void CharacterAdminHandler::_parseHairData(Message* message, CharacterCreateInfo* info)
{
    info->mHairCustomization[1]= 0;
    info->mHairCustomization[2]= 0;

    // Get the size of the data block
    uint16 dataSize = message->getUint16();
    DLOG(INFO) << "datasize : " << dataSize;

    uint8 startindex = 0;
    uint8 endindex = 0;

    if(dataSize!= 0)
    {
        uint16  dataIndex = 0;
        if(dataSize!= 5)
        {
            //cave the trandoshan - there we have no index!!!

            startindex = message->getUint8();
            endindex = message->getUint8();
            DLOG(INFO) << "StartIndex : "<< startindex << " : EndIndex " << endindex;
            dataIndex = 2;
        }

        uint8 index =0;


        uint8   attributeIndex = 0, valueLowByte = 0, valueHighByte = 0;

        while (dataIndex != (dataSize -2))
        {
            index++;
            // reset our locals
            attributeIndex = 0;
            valueLowByte = 0;
            valueHighByte = 0;

            // get our attribute index
            attributeIndex = message->getUint8();
            dataIndex++;

            // get our first byte of the value
            valueLowByte = message->getUint8();
            dataIndex++;

            // If it's an 0xff, get the next byte.
            if (valueLowByte == 0xFF)
            {
                valueHighByte = message->getUint8();
                dataIndex++;
            }

            // Set our attribute value
            info->mHairCustomization[attributeIndex] = ((uint16)valueHighByte << 8) | valueLowByte;
            DLOG(INFO) << "Hair Customization Index : " << attributeIndex << "   : data " << info->mHairCustomization[attributeIndex];
        }

        /* uint16 end2  = */message->getUint16();
    }
}


void CharacterAdminHandler::_parseAppearanceData(Message* message, CharacterCreateInfo* info)
{
    // Get the size of the data block
    uint16 dataSize = message->getUint16();

    // Grab our first two bytes which are unknown at this point
    //byte 1 = start index; byte 2 = end index
    /*uint8		startindex  = */
    message->getUint8();
    /*uint8		endindex	= */
    message->getUint8();
    uint8		boobJob		= message->peekUint8();
    uint16	dataIndex	= 2;
    uint8		index		= 0;
    uint8		attributeIndex = 0, valueLowByte = 0, valueHighByte = 0;

    if(boobJob==171)
    {
        valueLowByte = 0;
        valueHighByte = 0;
        //field > 73 means 2 attributes after index in this case breasts
        boobJob	= message->getUint8();
        dataIndex++;

        valueLowByte = message->getUint8();
        dataIndex++;

        // If it's an 0xff, get the next byte.
        if (valueLowByte == 0xFF)
        {
            valueHighByte = message->getUint8();
            dataIndex++;
        }

        // Set our attribute value
        info->mAppearance[171] = ((uint16)valueHighByte << 8) | valueLowByte;

        valueLowByte = 0;
        valueHighByte = 0;

        valueLowByte = message->getUint8();
        dataIndex++;

        // If it's an 0xff, get the next byte.
        if (valueLowByte == 0xFF)
        {
            valueHighByte = message->getUint8();
            dataIndex++;
        }

        // Set our attribute value
        info->mAppearance[172] = ((uint16)valueHighByte << 8) | valueLowByte;

    }

    //dont interpret end header as data
    while (dataIndex != (dataSize-2))
    {
        index++;
        // reset our locals
        attributeIndex = 0;
        valueLowByte = 0;
        valueHighByte = 0;

        // get our attribute index
        attributeIndex = message->getUint8();
        dataIndex++;

        // get our first byte of the value
        valueLowByte = message->getUint8();
        dataIndex++;

        // If it's an 0xff, get the next byte.
        if (valueLowByte == 0xFF)
        {
            valueHighByte = message->getUint8();
            dataIndex++;
        }

        // Set our attribute value
        info->mAppearance[attributeIndex] = ((uint16)valueHighByte << 8) | valueLowByte;
    }

    //end is ff 03 - no data
    /* uint16 end  = */message->getUint16();
}


void CharacterAdminHandler::_sendCreateCharacterSuccess(uint64 characterId,DispatchClient* client)
{
    if(!client)
    {
        return;
    }

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opHeartBeat);
    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelAUnreliable(newMessage, client->getAccountId(), CR_Client, 1);

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClientCreateCharacterSuccess);
    gMessageFactory->addUint64(characterId);
    newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 2);
}


void CharacterAdminHandler::_sendCreateCharacterFailed(uint32 errorCode,DispatchClient* client)
{
    if(!client)
    {
        return;
    }

    BString unknown = L"o_O";
    BString stfFile = "ui";
    BString errorString;

    switch(errorCode)
    {
    case 0:
        errorString = "name_declined_developer";
        break;

    case 1:
        errorString = "name_declined_empty";
        break;

    case 2:
        errorString = "name_declined_fictionally_reserved";
        break;

    case 3:
        errorString = "name_declined_in_use";
        break;

    case 4:
        errorString = "name_declined_internal_error";
        break;

    case 5:
        errorString = "name_declined_no_name_generator";
        break;

    case 6:
        errorString = "name_declined_no_template";
        break;

    case 7:
        errorString = "name_declined_not_authorized_for_species";
        break;

    case 8:
        errorString = "name_declined_not_creature_template";
        break;

    case 9:
        errorString = "name_declined_not_authorized_for_species";
        break;

    case 10:
        errorString = "name_declined_number";
        break;

    case 11:
        errorString = "name_declined_profane";
        break;

    case 12:
        errorString = "name_declined_racially_inappropriate";
        break;

    case 13:
        errorString = "name_declined_reserved";
        break;

    case 14:
        errorString = "name_declined_retry";
        break;

    case 15:
        errorString = "name_declined_syntax";
        break;

    case 16:
        errorString = "name_declined_too_fast";
        break;

    case 17:
        errorString = "name_declined_cant_create_avatar";
        break;

    default:
        errorString = "name_declined_internal_error";
        DLOG(INFO) << "CharacterAdminHandler::_sendCreateCharacterFailed Unknown Errorcode in CharacterCreation: " << errorCode;
        break;
    }

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opHeartBeat);
    Message* newMessage = gMessageFactory->EndMessage();

    client->SendChannelAUnreliable(newMessage, client->getAccountId(), CR_Client, 1);

    gMessageFactory->StartMessage();
    gMessageFactory->addUint32(opClientCreateCharacterFailed);
    gMessageFactory->addString(unknown);
    gMessageFactory->addString(stfFile);
    gMessageFactory->addUint32(0);            // unknown
    gMessageFactory->addString(errorString);
    newMessage = gMessageFactory->EndMessage();

    client->SendChannelA(newMessage, client->getAccountId(), CR_Client, 3);
}
