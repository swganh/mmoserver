// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "login_cluster_status.h"

//using namespace swganh::login;
using namespace swganh::messages;
using namespace std;
/*
LoginClusterStatus swganh::messages::BuildLoginClusterStatus(
    const vector<GalaxyStatus>& galaxy_status)
{
    LoginClusterStatus message;

    std::for_each(
        galaxy_status.begin(),
        galaxy_status.end(),
        [&message] (const GalaxyStatus& status)
    {
        ClusterServer cluster_server;
        cluster_server.address = status.address;
        cluster_server.ping_port = status.ping_port;
        cluster_server.conn_port = status.connection_port;
        cluster_server.distance = status.distance;
        cluster_server.status = status.status;
        cluster_server.server_id = status.galaxy_id;
        cluster_server.not_recommended_flag = 0;
        cluster_server.max_chars = status.max_characters;
        cluster_server.max_pop = status.max_population;
        cluster_server.server_pop = status.server_population;

        message.servers.push_back(cluster_server);
    });

    return message;
}
*/