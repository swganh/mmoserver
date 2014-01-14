
/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2010 The swgANH Team

---------------------------------------------------------------------------------------
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

