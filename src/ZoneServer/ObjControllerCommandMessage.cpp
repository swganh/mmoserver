
#include "ObjControllerCommandMessage.h"
#include "ObjectControllerCommandMap.h"

ObjControllerCommandMessage::ObjControllerCommandMessage(uint32 opcode,const uint64 executionTime,uint64 targetId)
: mOpcode(opcode)
, mData(NULL)
, mExecutionTime(executionTime)
, mTargetId(targetId)
, mSequence(0)
, mProperties(NULL)
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

