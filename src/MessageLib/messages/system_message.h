// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include "MessageLib/messages/controllers/show_fly_text.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

class PlayerObject;

namespace swganh
{

namespace messages
{
class OutOfBand;
/*
 * A helper class that builds up and sends a System Message
 */
class SystemMessage
{
public:
    static void Send(std::shared_ptr<PlayerObject> sender, const std::wstring& custom_message, bool chatbox_only=true, bool send_to_inrange=true);
    static void Send(std::shared_ptr<PlayerObject> sender, const swganh::messages::OutOfBand& out_of_band, bool chatbox_only=true, bool send_in_range=true);
    static void Send(std::shared_ptr<PlayerObject> sender, const std::wstring& custom_message,const swganh::messages::OutOfBand& out_of_band, bool chatbox_only=true, bool send_in_range=true);
    static void Send(std::shared_ptr<PlayerObject> sender, std::string filename, std::string label);
    static void FlyText(std::shared_ptr<PlayerObject> sender, const std::string& fly_text, controllers::FlyTextColor color, bool display_flag=false, uint8_t red=0, uint8_t green=0, uint8_t blue=0);

    //static void ClientEffect(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service,
    //                         std::string client_effect_file, std::string planet_name, glm::vec3 location_coordinates, float range, uint64_t cell_id=0);

    //static void ClientEffect(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service,
                             //std::string client_effect_file, uint32_t planet_id, glm::vec3 location_coordinates, float range, uint64_t cell_id=0);

    static void ClientEffect(std::shared_ptr<PlayerObject> object, const std::string& client_effect_file,
                             const std::string& auxiliary_string="head", bool send_in_range=true);

    static void ClientEffect(std::shared_ptr<PlayerObject> object, const std::string& client_effect_file,
                             glm::quat orientation, glm::vec3 offset, bool send_in_range=true);

    //static void ClientEvent(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service, const std::string& event_group_string,
      //                      const std::string& event_string, const std::string& planet_name_string, glm::vec3 location_coordinates, float range, uint64_t cell_id=0);

    //static void ClientEvent(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service, const std::string& event_group_string,
                            //const std::string& event_string, uint32_t planet_id, glm::vec3 location_coordinates, float range, uint64_t cell_id=0);

    static void ClientEvent(std::shared_ptr<PlayerObject> object, std::string event_string,
                            std::string hardpoint_string="head", bool send_in_range=true);

    //static void PlayMusic(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service, std::string planet_name,
      //                    glm::vec3 point, float range, const std::string& music_name, uint32_t channel);

    //static void PlayMusic(std::shared_ptr<swganh::simulation::SimulationServiceInterface> simulation_service, uint32_t planet_id,
      //                    glm::vec3 point, float range, const std::string& music_name, uint32_t channel);

    //Channel 0 = Background Music
    //Channel 1 = Sound Effect
    // NOTE: Only 1 sound at a time per channel.
    static void PlayMusic(std::shared_ptr<PlayerObject> object, const std::string& music_name,
                          uint32_t channel, bool send_in_range=false);
};


}
} // swganh::messages