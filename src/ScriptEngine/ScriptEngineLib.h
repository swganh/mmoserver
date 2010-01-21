/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
*/

#ifndef ANH_SCRIPTENGINE_SCRIPTENGINE_LIB_H
#define ANH_SCRIPTENGINE_SCRIPTENGINE_LIB_H

#include "Utils/typedefs.h"


//======================================================================================================================

typedef struct lua_State lua_State;

//======================================================================================================================

void LuaOpenScriptEngineLib(lua_State* l);

//======================================================================================================================

#endif

