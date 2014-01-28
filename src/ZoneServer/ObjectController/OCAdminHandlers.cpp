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
#include "Zoneserver/GameSystemManagers/AdminManager.h"
#include "Zoneserver/ObjectController/ObjectController.h"
#include "ZoneServer/ObjectController/ObjectControllerCommandMap.h"
#include "ZoneServer/Objects/Player Object/PlayerObject.h"
#include "ZoneServer/WorldManager.h"
#include "ZoneServer/ZoneOpcodes.h"
#include "ZoneServer/ZoneServer.h"
#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "Utils/utils.h"

#include "DatabaseManager/Database.h"
//#include "DatabaseManager/DatabaseResult.h"
//#include "DatabaseManager/DataBinding.h"

#include <boost/regex.hpp>  // NOLINT

using boost::regex;
using boost::smatch;
using boost::regex_search;
using boost::sregex_token_iterator;

//=============================================================================================================================
//
// system message
//
/*
void ObjectController::_handleAdminSysMsg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		playerObject	= dynamic_cast<PlayerObject*>(mObject);
    BString				dataStr;

    message->getStringUnicode16(dataStr);

    if(dataStr.getLength())
    {
        PlayerAccMap::const_iterator it = gWorldManager->getPlayerAccMap()->begin();

        while(it != gWorldManager->getPlayerAccMap()->end())
        {
            const PlayerObject* const player = (*it).second;

            if(player->isConnected())
            {
                gMessageLib->sendSystemMessage((PlayerObject*)player,dataStr);
            }

            ++it;
        }
    }
}
*/

//======================================================================================================================
//
// warp self
//

void ObjectController::_handleAdminWarpSelf(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{
    PlayerObject*		player		= dynamic_cast<PlayerObject*>(mObject);
    BString				dataStr;
    BString				planet;
    int32				planetId	= 0;
    int32				x,z;

    message->getStringUnicode16(dataStr);

    int32 elementCount = swscanf(dataStr.getUnicode16(),L"%i %i %s",&x,&z,planet.getAnsi());

    switch(elementCount)
    {
        // warp on current planet
    case 2:
    {
        // make sure we in bounds
        if(x < -8192 || x > 8192 || z < -8192 || z > 8192)
            break;

        gWorldManager->warpPlanet(player, glm::vec3(static_cast<float>(x),0.0f,static_cast<float>(z)),0);
    }
    return;

    // warp to other or current planet
    case 3:
    {
        // make sure we in bounds
        if(x < -8192 || x > 8192 || z < -8192 || z > 8192)
            break;

		planetId = gWorldManager->getPlanetIdByName(planet.getAnsi());

        if(planetId == -1)
            break;

        // warp on this planet
        if(static_cast<uint32>(planetId) == gWorldManager->getZoneId())
        {
            gWorldManager->warpPlanet(player, glm::vec3(static_cast<float>(x),0.0f,static_cast<float>(z)),0);
        }
        // zone transfer request
        else
        {
			std::string message_ansi("Requesting zone transfer...");
            gMessageLib->SendSystemMessage(std::u16string(message_ansi.begin(), message_ansi.end()), player);

            gMessageLib->sendClusterZoneTransferRequestByPosition(player, glm::vec3(static_cast<float>(x),0.0f,static_cast<float>(z)),planetId);
        }
    }
    return;

    default:
    {
        gMessageLib->SendSystemMessage(L"[SYNTAX] /admin_warp_self <x> <z> <planet>", player);
    }
    return;
    }

    gMessageLib->SendSystemMessage(L"Error parsing parameters.", player);
}


//=============================================================================================================================
//
//	This IS the entry point for the admin system message, but we will use it as an entry point for our admin handler.
//
//	That accepted, we can now create whatever commands we need.
//


// typedef string (ObjectController::*adminFuncPointer)(string);
typedef struct _AdminCommands
{
    BString command;
    const int32 testLength;
    // string (ObjectController::*adminFuncPointer)(string) const;
} AdminCommands;

#define noOfAdminCommands 5
static AdminCommands adminCommands[noOfAdminCommands] = {
    {"broadcast", 10}, // &ObjectController::handleBroadcast,
    {"broadcastPlanet", 10}, // &ObjectController::handleBroadcastPlanet,
    {"broadcastGalaxy", 10}, // &ObjectController::handleBroadcastGalaxy,
    {"shutdownGalaxy", 9}, // &ObjectController::handleShutdownGalaxy,
    {"cancelShutdownGalaxy", 15}, // &ObjectController::handleCancelShutdownGalaxy
};

void ObjectController::_handleAdminSysMsg(uint64 targetId,Message* message,ObjectControllerCmdProperties* cmdProperties)
{

	//todo use smatch regex
    bool status = false;

    BString dataStr;
    message->getStringUnicode16(dataStr);

    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    if ((player) && (player->isConnected()))
    {
        // gMessageLib->sendSystemMessage(player, dataStr, true);

        //dataStr.convert(BSTRType_ANSI);
        DLOG(info) << "Admin "<< player->getFirstName() <<":" << dataStr.getAnsi();
    }
    else
    {
        //dataStr.convert(BSTRType_ANSI);
        DLOG(info) << "Admin (anon): " <<  dataStr.getAnsi();
    }

    int8 rawData[128];
    rawData[0] = 0;

    if (dataStr.getLength())
    {
        int32 elementCount = sscanf(dataStr.getAnsi(), "%80s", rawData);
        BString adminCommand(rawData);
        if (elementCount > 0)
        {
            int32 commandIndex = this->getAdminCommandFunction(adminCommand);
            if (commandIndex >= 0)
            {
                // We have a valid command name, but strip it. It can be an abreviation.
                int32 index = indexOfNextField(dataStr);
                if (index < 0)
                {
                    // Not all commands have payload.
                    index = dataStr.getLength();
                }
                BString ansiData;
                ansiData.setLength(dataStr.getLength());
                dataStr.substring(ansiData,static_cast<uint16>(index), dataStr.getLength());

                // Now ADD a proper spelled command. It HAS to match the crc.
                char* newCommandString = new char[adminCommands[commandIndex].command.getLength() + ansiData.getLength() + 1];
                sprintf(newCommandString,"%s %s", adminCommands[commandIndex].command.getAnsi(), ansiData.getAnsi());

                // Execute the command.
                BString opcodeStr(adminCommands[commandIndex].command);
                opcodeStr.toLower();

				std::string stdnewCommandString(newCommandString);
				std::wstring wstdnewCommandString(stdnewCommandString.begin(), stdnewCommandString.end());
				delete[] newCommandString;

                // Now let's parse it BACK to objectcontroller, so we can continue to maintain access rights control as initial designed.
                gMessageFactory->StartMessage();
                gMessageFactory->addUint32(0);						// clientTicks, We do not have these, here.
                gMessageFactory->addUint32(0);						// sequence number, we really need this
                gMessageFactory->addUint32(opcodeStr.getCrc());		// opCode for new command.
                gMessageFactory->addUint64(targetId);
                gMessageFactory->addString(wstdnewCommandString);
                Message* newMessage = gMessageFactory->EndMessage();
                newMessage->ResetIndex();
                this->enqueueCommandMessage(newMessage);

                status = true;
            }
            else
            {
                // Invalid command.
                sprintf(rawData,"Admin: [%s No such command]", adminCommand.getAnsi());
            }
        }
        else
        {
            sprintf(rawData,"Admin: [Unexpected error]");
        }
    }
    else
    {
        sprintf(rawData,"Admin: [Missing command]");
    }

    // Send reply to caller.
    if (!status)
    {
        this->sendAdminFeedback(rawData);
    }
}

//=============================================================================================================================
//
//	Broadcast system message
//
//	Syntax Broadcast theBroadcast
//

void ObjectController::_handleBroadcast(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    int8 rawData[128];
    rawData[0] = 0;
    BString msgString;
    message->getStringUnicode16(msgString);
    msgString.convert(BSTRType_ANSI);

    // Remove current command and leadig white space.
    msgString = skipToNextField(msgString);

    BString feedback(this->handleBroadcast(msgString));
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleBroadcast(BString message) const
{
    int8 rawData[128];

    int8* replyStr = "OK";
    if (message.getLength())
    {
        // Any valid message?
        int32 elementCount = sscanf(message.getAnsi(), "%80s", rawData);
        if (elementCount > 0)
        {
            message.convert(BSTRType_Unicode16);

            PlayerAccMap::const_iterator it = gWorldManager->getPlayerAccMap()->begin();
            while(it != gWorldManager->getPlayerAccMap()->end())
            {
                const PlayerObject* const player = (*it).second;
                if (player->isConnected())
                {
                    gMessageLib->SendSystemMessage(message.getUnicode16(), player);
                }
                ++it;
            }
        }
        else
        {
            replyStr = "No broadcast supplied";
        }
    }
    else
    {
        replyStr = "No broadcast supplied";
    }
    return replyStr;
}


void ObjectController::_handleAdminSetName(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
}


void ObjectController::_handleAdminTeleportTo(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));

	if(!admin)	{	
		LOG(error) << "ObjectController::_handleAdminTeleportTo - No Admin ?? : " << targetId;
		return;
	}

	PlayerObject* target_player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
	if(!target_player)	{	
		LOG(error) << "ObjectController::_handleAdminTeleportTo - the target player couldnt be located : " << targetId;
		std::string result("the target player couldnt be located");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}

	uint32 x,y,z;
	x = target_player->mPosition.x;
	y = target_player->mPosition.y;
	z = target_player->mPosition.z;

	//this is hardcoded stuff we need a way to determine a zones size at some point
	/*if(x < -8192 || x > 8192 || z < -8192 || z > 8192)	{
        std::string result("Please make sure you enter valid coordinates. At current these are -8192 to 8192 in x and z for planets");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}*/

	
    gWorldManager->warpPlanet(admin, glm::vec3(static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)),0);

}

void ObjectController::_handleAdminTeleportTarget(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
	PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));
	if(!admin)	{	
		LOG(error) << "ObjectController::_handleAdminTeleportTo - No Admin ?? : " << targetId;
		return;
	}

	PlayerObject* target_player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
	if(!target_player)	{	
		LOG(error) << "ObjectController::_handleAdminTeleportTo - the target player couldnt be located : " << targetId;
		std::string result("the target player couldnt be located");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}

	float x,y,z;
	x = admin->mPosition.x;
	y = admin->mPosition.y;
	z = admin->mPosition.z;

	//this is hardcoded shit we need a way to determine a zones size at some point
	/*if(x < -8192 || x > 8192 || z < -8192 || z > 8192)	{
        std::string result("Please make sure you enter valid coordinates. At current these are -8192 to 8192 in x and z for planets");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}*/

    gWorldManager->warpPlanet(target_player, admin->mPosition, 0 );

}

void ObjectController::_handleAdminTeleport(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
	std::u16string raw_message_unicode = message->getStringUnicode16();
	std::string    raw_message_ansi(raw_message_unicode.begin(), raw_message_unicode.end());
	
	LOG(info) << "ObjectController::_handleAdminTeleport : " << raw_message_ansi;

	static const regex pattern("([a-zA-Z0-9/_]+)? ([0-9-]+) ([0-9-]+)");
    smatch result;

    std::string text(raw_message_ansi.begin(), raw_message_ansi.end());

	LOG (info) << raw_message_ansi;

	bool is_true = regex_search(text, result, pattern);

	PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));

	if(!is_true)	{	
		std::string result("Proper usage : /teleport [optional: planet] [x] [z]");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}

	LOG (info) << "Matched : " << is_true;
	LOG (info) << "string : " << result[0].str();
	LOG (info) << "1 : " << result[1].str();
	LOG (info) << "2 : " << result[2].str();
	LOG (info) << "3 : " << result[3].str();
	LOG (info) << "4 : " << result[4].str();
	LOG (info) << "5 : " << result[5].str();
	LOG (info) << "6 : " << result[6].str();
	LOG (info) << "7 : " << result[7].str();
	LOG (info) << "8 : " << result[8].str();
	LOG (info) << "9 : " << result[9].str();
	LOG (info) << "10 : " << result[10].str();

	int32 x,z, planetId;
	std::string zone(result[1].str());
	x = atoi(result[2].str().c_str());
	z = atoi(result[3].str().c_str());

	//this is hardcoded stuff we need a way to determine a zones size at some point
	if(x < -8192 || x > 8192 || z < -8192 || z > 8192)	{
        std::stringstream result_stream;
		result_stream << "Please make sure you enter valid coordinates. At current these are -8192 to 8192 in x and z for planets : " << x << " : " << z;
		std::string result(result_stream.str());
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}

	if(zone.length()){
		planetId = gWorldManager->getPlanetIdByName(zone);

			
		if(planetId == -1)	{
			std::string result("Please make sure you enter a valid Zone (Instance).");
			std::u16string message_u16(result.begin(), result.end());
			gMessageLib->SendSystemMessage(message_u16, admin);
			return;
		}
	}

    // teleport on this zone
    if(static_cast<uint32>(planetId) == gWorldManager->getZoneId())
    {
            gWorldManager->warpPlanet(admin, glm::vec3(static_cast<float>(x),0.0f,static_cast<float>(z)),0);
    }
    // zone transfer request
    else
    {
		std::string message_ansi("Requesting zone transfer...");
        gMessageLib->SendSystemMessage(std::u16string(message_ansi.begin(), message_ansi.end()), admin);

        gMessageLib->sendClusterZoneTransferRequestByPosition(admin, glm::vec3(static_cast<float>(x),0.0f,static_cast<float>(z)),planetId);
    }


}

void ObjectController::_handleAdminKick(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
	std::u16string raw_message_unicode = message->getStringUnicode16();
	std::string    raw_message_ansi(raw_message_unicode.begin(), raw_message_unicode.end());
	PlayerObject* player = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(targetId));
	if(!player)	{
		PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));
		std::string result("You must either target the offending player or provide his name in case he is nearby. You cannot /kick offline players or players that are not nearby.");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);
		return;
	}
	
	//(([a-zA-Z0-9/_]+) ((ban:) ((days:) ([0-9]+) )?((hours:) ([0-9]+) )?((minutes:) ([0-9]+))?)?)([a-zA-Z0-9/_]+)?

	//static const regex pattern("(([a-zA-Z0-9/_]+) ((ban:)([ ]+?(days:)([0-9]+))?([ ]+?(hours:)([0-9]+))?([ ]+?(minutes:)([0-9]+))?)?)([a-zA-Z0-9/_ :]+)?");
	static const regex pattern("(([a-zA-Z0-9/_]+)? (([ ]+)?(ban:)([ ]+?(days:)([0-9]+))?(([ ]+)?(hours:)([0-9]+))?(([ ]+)?(minutes:)([0-9]+))?)?)([a-zA-Z0-9/_ :]+)?");
    smatch result;

    std::string text(raw_message_ansi.begin(), raw_message_ansi.end());

    // If it's an exact match (2 sub-patterns + the full string = 3 elements) it's an stf string.
    // Reroute the call to the appropriate overload.
    bool is_true = regex_search(text, result, pattern);

	LOG (info) << "Matched : " << is_true;
	LOG (info) << "string : " << result[0].str();
	LOG (info) << "1 : " << result[1].str();
	LOG (info) << "2 : " << result[2].str();
	LOG (info) << "3 : " << result[3].str();
	LOG (info) << "4 : " << result[4].str();
	LOG (info) << "5 : " << result[5].str();
	LOG (info) << "6 : " << result[6].str();
	LOG (info) << "7 : " << result[7].str();
	LOG (info) << "8 : " << result[8].str();
	LOG (info) << "9 : " << result[9].str();
	LOG (info) << "10 : " << result[10].str();
	LOG (info) << "11 : " << result[11].str();
	LOG (info) << "12 : " << result[12].str();
	LOG (info) << "13 : " << result[13].str();
	LOG (info) << "14 : " << result[14].str();
	LOG (info) << "15 : " << result[13].str();
	LOG (info) << "16 : " << result[14].str();


	if(!is_true)	{
		PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));
		std::string result("Syntax: /kick <reason> [ban: days: [n] hours: [n] minutes: [n] ] ");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);

		result = "Only ban if it is absolutely necessary. You must include a specific reason for the ban.";
		std::u16string message2_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message2_u16, admin);
		return;
	}

	//if we ban him we need to get the duration
	if( result[5].str().length() == 4)	{

		std::string days(result[8].str());
		std::string hours(result[12].str());
		std::string minutes(result[16].str());

		bool time_provided = false;

		if(!days.length())	{
			days = "0";
		} else time_provided = true;
		
		if(!hours.length())	{
			hours = "0";
		} else time_provided = true;
		
		if(!minutes.length())	{
			minutes = "0";
		} else time_provided = true;

		if(!time_provided)	{
		PlayerObject* admin = dynamic_cast<PlayerObject*>(gWorldManager->getObjectById(this->getObject()->getId()));
		std::string result("Syntax: /kick <reason> [ban: days: [n] hours: [n] minutes: [n] ] ");
		std::u16string message_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message_u16, admin);

		result = "Only ban if it is absolutely necessary. You must include a specific reason for the ban.";
		std::u16string message2_u16(result.begin(), result.end());
		gMessageLib->SendSystemMessage(message2_u16, admin);
		return;
	}

		LOG (info) << "ObjectController::_handleAdminKick Player " << targetId << " received a ban for " << days << " days - " << hours << " hours - and " << minutes << "minutes";

		std::stringstream sql;
		sql <<"UPDATE " << this->mDatabase->galaxy() << ".account INNER JOIN " << this->mDatabase->galaxy() 
			<< ".characters ON account.account_id = characters.account_id SET account.account_banned = 1, account.banned_until = (NOW() + INTERVAL "
			<< minutes << " MINUTE + INTERVAL " << hours << " HOUR + INTERVAL " << days << " DAY) WHERE characters.id = " << targetId << ";";

		LOG(info) << "Query : " << sql.str();
		this->mDatabase->executeAsyncSql(sql.str());
	}
	LOG (info) << "ObjectController::_handleAdminKick Player " << targetId << " reason " << result[3].str();
	
	std::string result_ansi = result[3].str();
	std::u16string message_u16(result_ansi.begin(), result_ansi.end());
	gMessageLib->SendSystemMessage(message_u16, player);

	gMessageLib->sendLogout(player);
    player->togglePlayerCustomFlagOff(PlayerCustomFlag_LogOut);
    gWorldManager->addDisconnectedPlayer(player);
	
}

//=============================================================================================================================
//
//	Broadcast system message to named planet
//
//	Syntax BroadcastPlanet planetName theBroadcast
//
void ObjectController::_handleBroadcastPlanet(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    int8 rawData[128];
    rawData[0] = 0;
    
	BString msgString;
    message->getStringUnicode16(msgString);
    msgString.convert(BSTRType_ANSI);

    // Remove current command and leadig white space.
    // msgString = skipToNextField(msgString);
    msgString = skipToNextField(msgString);

    BString feedback = this->handleBroadcastPlanet(msgString);
	LOG(info) << "ObjectController::_handleBroadcastPlanet : " << msgString.getAnsi();
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleBroadcastPlanet(BString message) const
{
    // Get planet name
    int8 rawData[128];
    rawData[0] = 0;

    int32 elementCount = sscanf(message.getAnsi(), "%80s", rawData);
	std::string planet(rawData);
    //BString planet(rawData);
    if (elementCount > 0)
    {
        // Yes. Validate the planet name.
        int32 planetId = gWorldManager->getPlanetIdByNameLike(planet);
        if (planetId >= 0)
        {
            // We have no idea of how much white space are inserted in string...
            int32 index = indexOfNextField(message);
            if (index > 0)
            {
                // Now we can get the broadcaste from the message.
                // Remove planet name from message.
                BString ansiData;
                ansiData.setLength(message.getLength());
                message.substring(ansiData, static_cast<uint16>(index), message.getLength());

                // Any valid message?
                elementCount = sscanf(ansiData.getAnsi(), "%80s", rawData);
                if (elementCount > 0)
                {
                    // string planetName(gWorldManager->getPlanetNameById(planetId));

                    this->broadcastGalaxyMessage(ansiData, planetId);
                    sprintf(rawData,"OK");
                }
                else
                {
                    sprintf(rawData,"You must include a broadcast message. You did not supply a broadcast message.");
                }
            }
            else
            {
                sprintf(rawData,"Supplied broadcast was invalid.");
            }
        }
        else
        {
            sprintf(rawData,"%s is not valid planet name.", planet);
        }
    }
    else
    {
        sprintf(rawData,"You must include a planet name. You did not supply a planet name.");
    }
    return rawData;
}


//=============================================================================================================================
//
//	Broadcast system message to all players in this galaxy
//
//	Syntax BroadcastGalaxy theBroadcast
//

void ObjectController::_handleBroadcastGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    int8 rawData[128];
    rawData[0] = 0;
    BString msgString;
    message->getStringUnicode16(msgString);
    msgString.convert(BSTRType_ANSI);

    // Remove current command and leadig white space.
    msgString = skipToNextField(msgString);

    BString feedback = this->handleBroadcastGalaxy(msgString);
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleBroadcastGalaxy(BString message) const
{
    int8* replyStr = "OK";

    int8 rawData[128];
    // We have no idea of how much white space are inserted in string...
    int32 index = indexOfFirstField(message);
    if (index >= 0)
    {
        // Now we can get the broadcast from the message.

        // Remove white space from start of message.
        BString ansiData;
        ansiData.setLength(message.getLength());
        message.substring(ansiData, static_cast<uint16>(index), message.getLength());

        // Any valid message?
        int32 elementCount = sscanf(ansiData.getAnsi(), "%80s", rawData);
        if (elementCount > 0)
        {
            this->broadcastGalaxyMessage(ansiData, -1);
        }
        else
        {
            replyStr = "You must include a broadcast message. You did not supply a broadcast message.";
        }
    }
    else
    {
        replyStr = "Supplied broadcast was invalid.";
    }
    return replyStr;
}


//=============================================================================================================================
//
//	Schedule a shutdown of all zones in the galaxy.
//
//	Syntax shutdownGalaxy timeToShutdown <theBroadcast>
//	where	timeToShutdown is minutes
//			theBroadcast is an optional message.
//

void ObjectController::_handleShutdownGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    int8 rawData[128];
    rawData[0] = 0;
    BString msgString;
    message->getStringUnicode16(msgString);
    msgString.convert(BSTRType_ANSI);

    // Remove current command and leadig white space.
    msgString = skipToNextField(msgString);

    BString feedback = this->handleShutdownGalaxy(msgString);
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleShutdownGalaxy(BString message) const
{
    int8 replyData[128];

    if (AdminManager::Instance()->shutdownPending())
    {
        sprintf(replyData,"A Galaxy Shutdown is already in progress. To cancel this shutdown, please use /cancelShutdownGalaxy.");
    }
    else
    {
        sprintf(replyData,"OK");
        // Get time for shutdown
        //
        int32 minutesToShutdown;
        int32 elementCount = sscanf(message.getAnsi(), "%3d", &minutesToShutdown);
        // string planet(rawData);
        if (elementCount > 0)
        {
            // We have no idea of how much white space are inserted in string...
            BString ansiData;
            int32 index = indexOfNextField(message);
            if (index < 0)
            {
                // We may not have any "next field", it's optional here.
                index = 0;
                ansiData = "";
            }
            else
            {
                // Now we can get the broadcaste from the message.
                // Remove first field from message.
                ansiData.setLength(message.getLength());
                message.substring(ansiData, static_cast<uint16>(index), message.getLength());
            }

            // Validate the timeout.
            if (minutesToShutdown >= 0)
            {
                // Any valid message?
                int8 rawData[128];
                elementCount = sscanf(ansiData.getAnsi(), "%80s", rawData);
                // Send request to all zones, via chatserver.
                this->scheduleShutdown(minutesToShutdown*60, ansiData);
            }
            else
            {
                sprintf(replyData,"%d is not valid shutdown time.", minutesToShutdown);
            }
        }
        else
        {
            sprintf(replyData,"You did not supply a valid Shut Down time. A shut down time is required.");
        }
    }
    return replyData;
}


//=============================================================================================================================
//
//	Cancel a scheduled shutdown of all zones in the galaxy.
//
//	Syntax cancelShutdownGalaxy <theBroadcast>
//	where	theBroadcast is an optional message.
//

void ObjectController::_handleCancelShutdownGalaxy(uint64 targetId, Message* message, ObjectControllerCmdProperties* cmdProperties)
{
    int8 rawData[128];
    rawData[0] = 0;
    BString msgString;
    message->getStringUnicode16(msgString);
    msgString.convert(BSTRType_ANSI);

    // Remove current command and leadig white space.
    msgString = skipToNextField(msgString);

    BString feedback = this->handleCancelShutdownGalaxy(msgString);
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleCancelShutdownGalaxy(BString message) const
{
    int8 replyData[128];

    if (!AdminManager::Instance()->shutdownPending())
    {
        sprintf(replyData,"There is not currently a shut down in progress. To shutdown the server use /ShutdownGalaxy");
    }
    else
    {
        sprintf(replyData,"OK");

        // We have no idea of how much white space are inserted in string...
        BString ansiData;
        int32 index = indexOfFirstField(message);
        if (index < 0)
        {
            // We may not have any "next field", it's optional here.
            index = 0;
            ansiData = "";
        }
        else
        {
            // Now we can get the broadcaste from the message.
            // Remove first field from message.
            ansiData.setLength(message.getLength());
            message.substring(ansiData, static_cast<uint16>(index), message.getLength());
        }

        // Send request to all zones, via chatserver.
        this->cancelScheduledShutdown(ansiData);
    }
    return replyData;
}



//=============================================================================================================================
//
//	Send system message to galaxy
//
//

void ObjectController::broadcastGalaxyMessage(BString theBroadcast, int32 planetId) const
{
    if (theBroadcast.getLength())
    {
        PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
        if (player)
        {
            theBroadcast.convert(BSTRType_Unicode16);

            // let the chatserver handle this.
            Message* newMessage;
            gMessageFactory->StartMessage();
            gMessageFactory->addUint32(opIsmBroadcastGalaxy);
            gMessageFactory->addUint32(planetId);
            gMessageFactory->addString(theBroadcast);
            newMessage = gMessageFactory->EndMessage();
            player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
            //this should be fastpath as not being Mission critical and we want to prevent the communication protocol overhead with Acks and resends

            // Convert since we are going to print it.
            // theBroadcast.convert(BSTRType_ANSI);
        }
    }

}

//=============================================================================================================================
//
//	Schedule server shutdown.
//
//

void ObjectController::scheduleShutdown(int32 scheduledTime, BString shutdownReason) const
{

    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    if (player)
    {
        shutdownReason.convert(BSTRType_Unicode16);

        // let the chatserver handle this.
        Message* newMessage;
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opIsmScheduleShutdown);
        gMessageFactory->addUint32((uint32)scheduledTime);
        gMessageFactory->addString(shutdownReason);
        newMessage = gMessageFactory->EndMessage();
        player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
        //this should be fastpath as not being Mission critical and we want to prevent the communication protocol overhead with Acks and resends
    }
}

//=============================================================================================================================
//
//	Cancel scheduled server shutdown.
//
//

void ObjectController::cancelScheduledShutdown(BString cancelShutdownReason) const
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    if (player)
    {
        cancelShutdownReason.convert(BSTRType_Unicode16);

        // let the chatserver handle this.
        Message* newMessage;
        gMessageFactory->StartMessage();
        gMessageFactory->addUint32(opIsmCancelShutdown);
        gMessageFactory->addUint32(0);					// Can be used as an option in the future,
        gMessageFactory->addString(cancelShutdownReason);
        newMessage = gMessageFactory->EndMessage();
        player->getClient()->SendChannelA(newMessage,player->getAccountId(),CR_Chat,2);
        //this should be fastpath as not being Mission critical and we want to prevent the communication protocol overhead with Acks and resends
    }
}

//=============================================================================================================================
//
// Find the start of the first field within a text message.
//
//

int32 ObjectController::indexOfFirstField(const BString message) const
{
    int32 index = -1;
    int8 *ptr = message.getRawData();
    //bool foundStart = false;
    //bool foundEnd = false;
    for (int32 i = 0; i < (int32)message.getLength(); i++)
    {
        // Find the start of next field.
        if (!isspace(*ptr++))
        {
            index = i;
            break;
        }
    }
    return index;
}

//=============================================================================================================================
//
// Find the start of the second field within a text message.
//
//

int32 ObjectController::indexOfNextField(const BString message) const
{
    int32 index = -1;
    int8 *ptr = message.getRawData();
    bool foundStart = false;
    bool foundEnd = false;
    for (int32 i = 0; i < (int32)message.getLength(); i++)
    {
        if (!foundStart)
        {
            if (!isspace(*ptr++))
            {
                foundStart = true;
            }
        }
        else if (!foundEnd)
        {
            if (isspace(*ptr++))
            {
                // we are done with the first field
                foundEnd = true;
            }
        }
        else
        {
            // Find the start of next field.
            if (!isspace(*ptr++))
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

//======================================================================================================================
//
//	Echo command feedback to user.
//

void ObjectController::sendAdminFeedback(BString reply) const
{
    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    if ((player) && (player->isConnected()))
    {
        if (reply.getLength())
        {
            reply.convert(BSTRType_Unicode16);
            gMessageLib->SendSystemMessage(reply.getUnicode16(), player, true);
        }
        else
        {
            DLOG(info) << "Admin :" << player->getFirstName();
        }
    }
    else
    {
        if (reply.getDataLength())
        {
            DLOG(info) << "Admin (anon): " << reply.getAnsi();
        }
        else
        {
        }
    }
}

BString ObjectController::removeWhiteSpace(BString str) const
{
    int32 index = indexOfFirstField(str);
    if (index > 0)
    {
        // Remove white space from start of message.
        BString resultStr;
        resultStr.setLength(str.getLength());
        str.substring(resultStr, static_cast<uint16>(index), str.getLength());
        return resultStr;
    }
    // Nothing we can do about that string... good or bad.
    return str;
}



BString ObjectController::skipToNextField(BString str) const
{
    int32 index = indexOfNextField(str);
    if (index < 0)
    {
        // Not all commands have payload.
        index = str.getLength();
    }

    // Remove white space from start of message.
    BString resultStr;
    resultStr.setLength(str.getLength());
    str.substring(resultStr, static_cast<uint16>(index), str.getLength());
    return resultStr;
}


//======================================================================================================================

int32 ObjectController::getAdminCommandFunction(BString command) const
{
    for (int i = 0; i < noOfAdminCommands; i++)
    {
        if(Anh_Utils::cmpnistr(command.getAnsi(),adminCommands[i].command.getAnsi(), adminCommands[i].testLength) == 0)
        {
            return i;
        }
    }
    return -1;
}
