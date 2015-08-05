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

#include "EVCmdProperty.h"
#include "ObjectController.h"
#include "ObjectControllerCommandMap.h"


#ifdef ERROR
#undef ERROR
#endif

#include "Utils/logger.h"

EVCmdProperty::EVCmdProperty(ObjectController* controller)
    : EnqueueValidator(controller)
{}

EVCmdProperty::~EVCmdProperty()
{}

bool EVCmdProperty::validate(uint32 &reply1,uint32 &reply2,uint64 targetId,uint32 opcode,ObjectControllerCmdProperties*& cmdProperties)
{
    // get the command properties
    CmdPropertyMap::iterator it = gObjControllerCmdPropertyMap.find(opcode);

    if(it == gObjControllerCmdPropertyMap.end())
    {
        // don't want to parse the annoying error, lets log it though
        // @todo find root cause of why command isn't in the map
        DLOG(INFO) <<  "Unknown command found " << opcode;
        reply1 = 0;
        reply2 = 1;

        return(false);
    }

    cmdProperties = ((*it).second);

    return(true);
}

