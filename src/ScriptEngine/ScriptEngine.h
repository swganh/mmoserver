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

#ifndef ANH_SCRIPTENGINE_SCRIPTENGINE_H
#define ANH_SCRIPTENGINE_SCRIPTENGINE_H

//======================================================================================================================

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "Script.h"
#include "Utils/typedefs.h"

#include <boost/pool/pool.hpp>
#include <boost/thread/mutex.hpp>
#include <list>

#define	 gScriptEngine	ScriptEngine::getSingletonPtr()

class Tutorial;

typedef std::list<Script*>	ScriptList;

//======================================================================================================================

class ScriptEngine
{
public:

    static ScriptEngine*	getSingletonPtr() {
        return mSingleton;
    }
    static ScriptEngine*	Init();

    lua_State*				getMasterState() {
        return mMasterState;
    }

    Script*					createScript();
    void					removeScript(Script* script);

    void 					shutdown();
    void 					process();

    ~ScriptEngine();
    Tutorial*				getTutorial(void* script);


private:

    ScriptEngine();

    static ScriptEngine*	mSingleton;
    static bool				mInsFlag;

    lua_State*				mMasterState;
    // Anh_Utils::Clock*		mClock;
    uint64					mLastProcessTime;
    boost::mutex			mScriptMutex;

    ScriptList				mScripts;
    boost::pool<boost::default_user_allocator_malloc_free>	mScriptPool;
};

//======================================================================================================================

#endif




