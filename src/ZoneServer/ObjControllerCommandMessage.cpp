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

#include "objcontrollercommandmessage.h"
#include "ObjectControllerCommandMap.h"

ObjControllerCommandMessage::ObjControllerCommandMessage(uint32 opcode,const uint64 executionTime,uint64 targetId)
    : mData(NULL)
    , mProperties(NULL)
    , mExecutionTime(executionTime)
    , mTargetId(targetId)
    , mOpcode(opcode)
    , mSequence(0)
{}


ObjControllerCommandMessage::~ObjControllerCommandMessage()
{
    if (mData)
    {
        gMessageFactory->DestroyMessage(mData);
    }
}

uint32 ObjControllerCommandMessage::getOpcode() const
{
    return mOpcode;
}

void ObjControllerCommandMessage::setOpcode(uint32 opcode)
{
    mOpcode = opcode;
}

uint64 ObjControllerCommandMessage::getTargetId() const
{
    return mTargetId;
}

void ObjControllerCommandMessage::setTargetId(uint32 targetId)
{
    mTargetId = targetId;
}

uint32 ObjControllerCommandMessage::getSequence() const
{
    return mSequence;
}

void ObjControllerCommandMessage::setSequence(uint32 sequence)
{
    mSequence = sequence;
}

Message* ObjControllerCommandMessage::getData()
{
    return mData;
}

void ObjControllerCommandMessage::setData(Message* message)
{
    mData = message;
}

uint64 ObjControllerCommandMessage::getExecutionTime() const
{
    return mExecutionTime;
}

void ObjControllerCommandMessage::setExecutionTime(uint64 time)
{
    mExecutionTime = time;
}

ObjectControllerCmdProperties* ObjControllerCommandMessage::getCmdProperties()
{
    return mProperties;
}

void ObjControllerCommandMessage::setCmdProperties(ObjectControllerCmdProperties* properties)
{
    mProperties = properties;
}

