/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_ZONESERVER_PROCESS_VALIDATOR_H
#define ANH_ZONESERVER_PROCESS_VALIDATOR_H

#include "Utils/typedefs.h"

class ObjectController;
class ObjectControllerCmdProperties;

//=======================================================================

class ProcessValidator
{
public:
    explicit ProcessValidator(ObjectController* controller);
    virtual ~ProcessValidator();

    virtual bool validate(uint32 &reply1, uint32 &reply2, uint64 targetId, uint32 opcode, ObjectControllerCmdProperties*& cmdProperties) = 0;

    uint32	inline getLowestCommonBit(uint64 playerMask, uint64 cmdPropertiesMask)
    {
        // checks each bit and returns the value
        bool bFound = false;
        uint32 i = 0;
        for (; i < 64 && !bFound; ++i) {
            bFound = (playerMask & (cmdPropertiesMask << i)) != 0;
        }
        if (bFound) {
            return i;
        }
        return 0;
    }

protected:
    ObjectController* mController;
};

//=======================================================================

#endif


