#include "PostureEvent.h"

using ::common::BaseEvent;
using ::common::EventType;
using ::common::EventCallback;
using ::common::ByteBuffer;

const EventType PostureUpdateEvent::type      = EventType("PostureUpdateEvent");

PostureUpdateEvent::PostureUpdateEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms) 
    : BaseEvent(subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}

PostureUpdateEvent::PostureUpdateEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback)
    : BaseEvent(subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}


PostureUpdateEvent::~PostureUpdateEvent(void){}

const EventType& PostureUpdateEvent::event_type() const { 
    return type; 
}

void PostureUpdateEvent::onSerialize(ByteBuffer& out) const {}
void PostureUpdateEvent::onDeserialize(ByteBuffer& in) {}

bool PostureUpdateEvent::onConsume(bool handled) const {
    return true;
}