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
#include "AdminManager.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"
#include "PlayerObject.h"
#include "WorldManager.h"
#include "ZoneOpcodes.h"
#include "ZoneServer.h"
#include "MessageLib/MessageLib.h"
#include "NetworkManager/Message.h"
#include "NetworkManager/MessageFactory.h"
#include "Utils/utils.h"

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

        planetId = gWorldManager->getPlanetIdByName(planet);

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
            gMessageLib->SendSystemMessage(L"Requesting zone transfer...", player);

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
    bool status = false;

    BString dataStr;
    message->getStringUnicode16(dataStr);

    PlayerObject* player = dynamic_cast<PlayerObject*>(mObject);
    if ((player) && (player->isConnected()))
    {
        // gMessageLib->sendSystemMessage(player, dataStr, true);

        dataStr.convert(BSTRType_ANSI);
        DLOG(INFO) << "Admin "<< player->getFirstName().getAnsi() <<":" << dataStr.getAnsi();
    }
    else
    {
        dataStr.convert(BSTRType_ANSI);
        DLOG(INFO) << "Admin (anon): " <<  dataStr.getAnsi();
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
    sprintf(rawData,"%s: [%s]", cmdProperties->mCommandStr.getAnsi(), feedback.getAnsi());
    this->sendAdminFeedback(rawData);
}


BString ObjectController::handleBroadcastPlanet(BString message) const
{
    // Get planet name
    int8 rawData[128];
    rawData[0] = 0;

    int32 elementCount = sscanf(message.getAnsi(), "%80s", rawData);
    BString planet(rawData);
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
                    sprintf(rawData,"No broadcast supplied");
                }
            }
            else
            {
                sprintf(rawData,"No valid broadcast supplied");
            }
        }
        else
        {
            sprintf(rawData,"%s is not valid planet name", planet.getAnsi());
        }
    }
    else
    {
        sprintf(rawData,"Missing planet name");
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
            replyStr = "No broadcast supplied";
        }
    }
    else
    {
        replyStr = "No valid broadcast supplied";
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
        sprintf(replyData,"Shutdown already in progress");
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
                sprintf(replyData,"%d is not valid shutdown time", minutesToShutdown);
            }
        }
        else
        {
            sprintf(replyData,"No shutdown time supplied");
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
        sprintf(replyData,"No shutdown in progress");
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
            DLOG(INFO) << "Admin :" << player->getFirstName().getAnsi();
        }
    }
    else
    {
        if (reply.getDataLength())
        {
            DLOG(INFO) << "Admin (anon): " << reply.getAnsi();
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
