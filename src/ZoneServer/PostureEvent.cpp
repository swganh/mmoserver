#include "PostureEvent.h"

using ::common::BaseEvent;
using ::common::EventType;
using ::common::EventCallback;
using ::common::ByteBuffer;

const EventType BasePostureEvent::type       = EventType("PostureUpdateEvent");
const EventType PostureUpdateEvent::type     = EventType("PostureUpdateEvent");
const EventType PostureErrorEvent::type      = EventType("PostureErrorEvent");

BasePostureEvent::BasePostureEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms) 
    : BaseEvent(subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}

BasePostureEvent::BasePostureEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback)
    : BaseEvent(subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}

BasePostureEvent::~BasePostureEvent() {};

const EventType& BasePostureEvent::event_type() const { 
    return type; 
}

void BasePostureEvent::onSerialize(ByteBuffer& out) const {}
void BasePostureEvent::onDeserialize(ByteBuffer& in) {}

bool BasePostureEvent::onConsume(bool handled) const {
    return true;
}

// PostureUpdateEvent
PostureUpdateEvent::PostureUpdateEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms) 
    : BasePostureEvent(objID, oldPosture, newPosture, subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}

PostureUpdateEvent::PostureUpdateEvent(uint64 objID, CreaturePosture oldPosture,CreaturePosture newPosture,uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback)
    : BasePostureEvent(objID, oldPosture, newPosture, subject_id, delay_ms)
{
    mObjID    = objID;
    mOldPos   = oldPosture;
    mNewPos   = newPosture;
}


PostureUpdateEvent::~PostureUpdateEvent(void){}