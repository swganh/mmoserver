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

#ifndef ANH_OBJCONTROLLER_COMMAND_MESSAGE_H
#define ANH_OBJCONTROLLER_COMMAND_MESSAGE_H

#include "Utils/typedefs.h"
#include "NetworkManager/MessageFactory.h"

class Message;
class ObjectControllerCmdProperties;

//======================================================================================================================

class ObjControllerCommandMessage
{
public:

    ObjControllerCommandMessage(uint32 opcode,const uint64 executionTime,uint64 targetId);

    virtual ~ObjControllerCommandMessage();
    virtual uint32 getOpcode() const;
    virtual void setOpcode(uint32 opcode);
    virtual uint64 getTargetId() const;
    virtual void setTargetId(uint32 targetId);

    virtual uint32	getSequence() const;
    virtual void		setSequence(uint32 sequence);

    virtual Message*	getData();
    virtual void		setData(Message* message);

    virtual uint64	getExecutionTime() const;
    virtual void		setExecutionTime(uint64 time);

    // virtual inline uint64	getQueuesTime() const { return mQueuedTime; }
    // virtual inline void		setQueuedTime(uint64 time){ mQueuedTime = time; }

    virtual ObjectControllerCmdProperties*	getCmdProperties();
    virtual void	setCmdProperties(ObjectControllerCmdProperties*	properties);

protected:
    Message*	mData;
    ObjectControllerCmdProperties*	mProperties;
    uint64		mExecutionTime;
    uint64		mQueuedTime;
    uint64		mTargetId;
    uint32		mOpcode;
    uint32		mSequence;
};

//===============================================================================================================
/*
// Not used
class CompareCommandMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getExecutionTime() > right->getExecutionTime());
		}
};
*/
//===============================================================================================================

/*
// Not used
class CompareCombatMsg
{
	public:

		bool operator () (const ObjControllerCommandMessage* left, const ObjControllerCommandMessage* right)
		{
			return(left->getSequence() > right->getSequence());
		}
};
*/

#endif

