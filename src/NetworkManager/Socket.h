/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_NETWORKMANAGER_SOCKET_H
#define ANH_NETWORKMANAGER_SOCKET_H

// Just an abstraction layer.  Should be put in the OS library at some point.

#ifdef WIN32
//#define socklen_t int
#else
#define SOCKET unsigned int

#endif //WIN32



#endif //ANH_NETWORKMANAGER_SOCKET_H


