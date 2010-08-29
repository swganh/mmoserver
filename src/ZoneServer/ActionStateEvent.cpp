#include "ActionStateEvent.h"

using ::common::BaseEvent;
using ::common::EventType;
using ::common::EventCallback;
using ::common::ByteBuffer;

const EventType ActionStateUpdateEvent::type       = EventType("ActionStateUpdateEvent");

ActionStateUpdateEvent::ActionStateUpdateEvent(CreatureObject* obj, CreatureState oldState,CreatureState newState,uint64_t subject_id, uint64_t delay_ms) 
    : BaseEvent(subject_id, delay_ms)
{
    mObj        = obj;
    mOldState   = oldState;
    mNewState   = newState;
}

ActionStateUpdateEvent::ActionStateUpdateEvent(CreatureObject* obj, CreatureState oldState,CreatureState newState,uint64_t subject_id, uint64_t delay_ms, EventCallback callback)
    : BaseEvent(subject_id, delay_ms)
{
    mObj        = obj;
    mOldState   = oldState;
    mNewState   = newState;
}


ActionStateUpdateEvent::~ActionStateUpdateEvent(void){}

const EventType& ActionStateUpdateEvent::event_type() const { 
    return type; 
}

void ActionStateUpdateEvent::onSerialize(ByteBuffer& out) const {}
void ActionStateUpdateEvent::onDeserialize(ByteBuffer& in) {}

bool ActionStateUpdateEvent::onConsume(bool handled) const {
    return true;
}
