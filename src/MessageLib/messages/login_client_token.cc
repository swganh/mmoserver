// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE

#include "login_client_token.h"

//#include "swganh_core/login/login_client_interface.h"
//#include "swganh_core/login/account.h"

using namespace swganh::login;
using namespace swganh::messages;
using namespace std;

using swganh::ByteBuffer;

LoginClientToken swganh::messages::BuildLoginClientToken(
    shared_ptr<LoginClientInterface> login_client,
    const string& session_key)
{
    LoginClientToken message;
	/*
    ByteBuffer session_buffer;
    session_buffer.write(session_key);

    message.session_key = session_buffer;
    message.station_id = login_client->GetAccount()->account_id();
    message.station_username = login_client->GetUsername();
	*/
    return message;
	
}
