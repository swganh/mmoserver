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

// The LoginServer's entry point, apart from the rest so the unit tests can
// link the server's code.

#include <iostream>
#include <string>

#include <boost/thread/thread.hpp>

#include "LoginServer/LoginServer.h"
#include "LoginServer/Password.h"
#include "Utils/utils.h"

extern LoginServer* gLoginServer;

//======================================================================================================================
int main(int argc, char* argv[])
{
    bool exit = false;

    if (!password::init())
    {
        std::cerr << "libsodium could not be initialised: passwords cannot be checked" << std::endl;
        return 1;
    }

    // LoginServer --hash-password: prints the Argon2id hash of the password
    // read from standard input (setup.sh add-account uses it).
    if (argc == 2 && std::string(argv[1]) == "--hash-password")
    {
        std::string text;
        std::getline(std::cin, text);
        std::string hashed = password::hash(text);
        if (hashed.empty())
        {
            std::cerr << "could not hash the password" << std::endl;
            return 1;
        }
        std::cout << hashed << std::endl;
        return 0;
    }

    try {
		gLoginServer = new LoginServer(argc, argv);

		// Since startup completed successfully, now set the atexit().  Otherwise we try to gracefully shutdown a failed startup, which usually fails anyway.
		//atexit(handleExit);

		// Main loop
		while (!exit)
		{
			gLoginServer->Process();

			if(Anh_Utils::kbhit())
				if(std::cin.get() == 'q')
					break;

			boost::this_thread::sleep(boost::posix_time::milliseconds(10));
		}

		// Shutdown things
		delete gLoginServer;
	} catch( std::exception& e ) {
		std::cout << e.what() << std::endl;
		std::cin.get();
		return 0;
	}

	return 0;
}




