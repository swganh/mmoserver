#include "ActionStateEvent.h"

using ::common::BaseEvent;
using ::common::EventType;
using ::common::EventCallback;
using ::common::ByteBuffer;

const EventType ActionStateUpdateEvent::event_type_       = EventType("ActionStateUpdateEvent");

ActionStateUpdateEvent::ActionStateUpdateEvent(PlayerObject* player, uint64_t subject_id = 0, uint64_t delay_ms = 0) 
    : BaseEvent(subject_id, delay_ms){}

ActionStateUpdateEvent::ActionStateUpdateEvent(PlayerObject* player, uint64_t subject_id, uint64_t delay_ms, ::common::EventCallback callback)
    : BaseEvent(subject_id, delay_ms){}


ActionStateUpdateEvent::~ActionStateUpdateEvent(void){}

const EventType& ActionStateUpdateEvent::event_type() const { 
    return event_type_; 
}

void ActionStateUpdateEvent::onSerialize(ByteBuffer& out) const {}
void ActionStateUpdateEvent::onDeserialize(ByteBuffer& in) {}

bool ActionStateUpdateEvent::onConsume(bool handled) const {
    return true;
}
