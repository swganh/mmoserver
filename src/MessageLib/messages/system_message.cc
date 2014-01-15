// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#include "system_message.h"
#include "out_of_band.h"
#include "chat/chat_system_message.h"
#include "play_client_effect_loc_message.h"
#include "play_client_effect_object_message.h"
#include "play_client_effect_object_with_transform_message.h"
#include "play_client_event_loc_message.h"
#include "play_client_event_object_message.h"
#include "play_music_message.h"
#include <MessageLib\MessageLib.h>

#include <ZoneServer\Objects\Player Object/PlayerObject.h>

//#include "swganh_core/simulation/simulation_service_interface.h"

#ifndef WIN32
#include <boost/regex.hpp>
using boost::regex;
using boost::regex_search;
using boost::smatch;
#else
#include <regex>
#endif

using namespace std;
using namespace swganh::messages;
using namespace swganh::messages::controllers;


void SystemMessage::Send(
    std::shared_ptr<PlayerObject> sender,
    std::string filename, std::string label)
{
    swganh::messages::OutOfBand prose(filename, label);
    Send(sender, L"", prose, false, false);
}

void SystemMessage::Send(
    std::shared_ptr<PlayerObject> sender,
    const std::wstring& custom_message,
    bool chatbox_only, bool send_to_inrange)
{
    // Use regex to check if the chat string matches the stf string format.
    static const regex pattern("@([a-zA-Z0-9/_]+):([a-zA-Z0-9_]+)");
    smatch result;

    string stf_string(custom_message.begin(), custom_message.end());

    // If it's an exact match (2 sub-patterns + the full string = 3 elements) it's an stf string.
    // Reroute the call to the appropriate overload.
    if (regex_search(stf_string, result, pattern))
    {
        string file(result[1].str());
        string string(result[2].str());

        Send(sender, L"", OutOfBand(file, string), chatbox_only, send_to_inrange);
    }

    Send(sender, custom_message, OutOfBand(),chatbox_only, send_to_inrange);
}
void SystemMessage::Send(
    std::shared_ptr<PlayerObject> sender,
    const swganh::messages::OutOfBand& out_of_band,
    bool chatbox_only, bool send_in_range)
{
    Send(sender, L"", out_of_band, chatbox_only, send_in_range);
}
void SystemMessage::Send(
    std::shared_ptr<PlayerObject> sender,
    const std::wstring& custom_message,
    const swganh::messages::OutOfBand& out_of_band,
    bool chatbox_only, bool send_in_range)
{
    uint8_t chatbox = chatbox_only == true ? 2 : 0;
    ChatSystemMessage SystemMessage;

    SystemMessage.message = custom_message;
    SystemMessage.display_type = chatbox;
    if (out_of_band.Length() > 0)
    {
        SystemMessage.AddProsePackage(out_of_band.Pack());
    }

    //if (send_in_range)
        //sender->NotifyObservers(&SystemMessage);
    //else
        //sender->GetController()->Notify(&SystemMessage);
}

void SystemMessage::FlyText(
    std::shared_ptr<PlayerObject> sender,
    const std::string& fly_text, controllers::FlyTextColor color,
    bool display_flag, uint8_t red, uint8_t green, uint8_t blue)
{
    // Use regex to check if the chat string matches the stf string format.
    static const regex pattern("@([a-zA-Z0-9/_]+):([a-zA-Z0-9_]+)");
    smatch result;

    // If it's an exact match (2 sub-patterns + the full string = 3 elements) it's an stf string.
    // Reroute the call to the appropriate overload.
    if (regex_search(fly_text, result, pattern))
    {
        string file(result[1].str());
        string string(result[2].str());

        controllers::ShowFlyText fly_text;
        fly_text.object_id = sender->getId();
        fly_text.stf_location = file;
        fly_text.text = string;
        switch (color)
        {
        case RED:
            fly_text.red = 0xFF;
            break;
        case GREEN:
            fly_text.green = 0xFF;
            break;
        case BLUE:
            fly_text.blue = 0xFF;
            break;
        case MIX:
            fly_text.red = red;
            fly_text.green = green;
            fly_text.blue = blue;
            break;
        case WHITE:
            fly_text.red = 0xFF;
            fly_text.green = 0xFF;
            fly_text.blue = 0xFF;
        default:
            break;
        }
        fly_text.display_flag = (display_flag == true) ? 0 : 1;
        //sender->NotifyObservers(&fly_text);
    }
}
/*
void SystemMessage::ClientEffect(std::shared_ptr<SimulationServiceInterface> simulation_service, std::string client_effect_file,
                                 std::string planet_name, glm::vec3 location_coordinates, float range, uint64_t cell_id)
{
    PlayClientEffectLocMessage msg;
    msg.client_effect_file = "clienteffect/"+client_effect_file+".cef";
    msg.planet_name = planet_name;
    msg.location_coordinates = location_coordinates;
    msg.cell_id = cell_id;
    simulation_service->SendToSceneInRange( &msg, planet_name, location_coordinates, range );
}

void SystemMessage::ClientEffect(std::shared_ptr<SimulationServiceInterface> simulation_service, std::string client_effect_file,
                                 uint32_t planet_id, glm::vec3 location_coordinates, float range, uint64_t cell_id)
{
    ClientEffect(simulation_service, client_effect_file, simulation_service->SceneNameById(planet_id), location_coordinates, range, cell_id);
}
*/
void SystemMessage::ClientEffect(std::shared_ptr<PlayerObject> object, const std::string& client_effect_file,
                                 const std::string& auxiliary_string, bool send_in_range)
{
    PlayClientEffectObjectMessage msg;
    msg.client_effect_file = "clienteffect/"+client_effect_file+".cef";
    msg.auxiliary_string = auxiliary_string;
	msg.object_id = object->getId();

    if(send_in_range)
    {
        //object->NotifyObservers(&msg);
    }
    else
    {
        //auto controller = object->GetController();
        //if(controller)
        //{
            //controller->Notify(&msg);
        //}
    }
}

void SystemMessage::ClientEffect(std::shared_ptr<PlayerObject> object, const std::string& client_effect_file,
                                 glm::quat orientation, glm::vec3 offset, bool send_in_range)
{
    PlayClientEffectObjectWithTransformMessage msg;
    msg.effect_string = "clienteffect/"+client_effect_file+".cef";
    msg.orientation = orientation;
    msg.offset = offset;
    msg.object_id = object->getId();

    if(send_in_range)
    {
        //object->NotifyObservers(&msg);
    }
    else
    {
        //auto controller = object->GetController();
        //if(controller)
        //{
          //  controller->Notify(&msg);
        //}
    }
}
/*
void SystemMessage::ClientEvent(std::shared_ptr<SimulationServiceInterface> simulation_service,
                                const std::string& event_group_string, const std::string& event_string,
                                const std::string& planet_name_string, glm::vec3 location_coordinates, float range,  uint64_t cell_id)
{
    PlayClientEventLocMessage msg;
    msg.event_group_string = event_group_string;
    msg.event_string = event_string;
    msg.planet_name_string = planet_name_string;
    msg.location_coordinates = location_coordinates;
    msg.cell_id = cell_id;
    simulation_service->SendToSceneInRange( &msg, planet_name_string, location_coordinates, range );
}

void SystemMessage::ClientEvent(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service,
                                const std::string& event_group_string, const std::string& event_string,
                                uint32_t planet_id, glm::vec3 location_coordinates, float range, uint64_t cell_id)
{
    ClientEvent(simulation_service, event_group_string, event_string, simulation_service->SceneNameById(planet_id),
                location_coordinates, range, cell_id);
}
*/
void SystemMessage::ClientEvent(std::shared_ptr<PlayerObject> object, std::string event_string,
                                std::string hardpoint_string, bool send_in_range)
{
    PlayClientEventObjectMessage msg;
    msg.event_string = event_string;
    msg.hardpoint_string = hardpoint_string;
    msg.object_id = object->getId();

    if(send_in_range)
    {
        //object->NotifyObservers(&msg);
    }
	/*
    else
    {
        auto controller = object->GetController();
        if(controller)
        {
            controller->Notify(&msg);
        }
    }*/
}
/*
void SystemMessage::PlayMusic(std::shared_ptr<SimulationServiceInterface> simulation_service, std::string planet_name,
                              glm::vec3 point, float range,const std::string& music_name, uint32_t channel)
{
    PlayMusicMessage msg;
    msg.music_file = "sound/"+music_name+".snd";
    msg.channel = channel;

    simulation_service->SendToSceneInRange( &msg, planet_name, point, range );
}

void SystemMessage::PlayMusic(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service, uint32_t planet_id,
                              glm::vec3 point, float range, const std::string& music_name, uint32_t channel)
{
    PlayMusic(simulation_service, simulation_service->SceneNameById(planet_id), point, range, music_name, channel);
}
*/
void SystemMessage::PlayMusic(std::shared_ptr<PlayerObject> object, const std::string& music_name,
                              uint32_t channel, bool send_in_range)
{
    PlayMusicMessage msg;
    msg.music_file = "sound/"+music_name+".snd";
    msg.channel = channel;
	/*
    if(send_in_range)
    {
        object->NotifyObservers(&msg);
    }
    else
    {
        auto controller = object->GetController();
        if(controller)
        {
            controller->Notify(&msg);
        }
    }
	*/
}
