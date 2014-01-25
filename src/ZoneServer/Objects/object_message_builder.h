// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <memory>
#include <boost/optional.hpp>

#include "anh/event_dispatcher/event_dispatcher.h"
//#include "swganh/observer/observer_interface.h"

#include "MessageLib\messages/baselines_message.h"

namespace swganh
{
namespace messages
{
struct DeltasMessage;
struct BaselinesMessage;
}
} // swganh::messages


enum	CRC_Type;
class	Object;
class	ObjectController;

class	BaseMessageBuilder
{

public:
	BaseMessageBuilder(swganh::event_dispatcher::EventDispatcher* dispatcher)
		: event_dispatcher_(dispatcher)
	{
	}

	virtual ~BaseMessageBuilder() {}

	static void BuildCustomNameDelta(const std::shared_ptr<Object>& object);

	template<typename SubjectT, typename ReceiverT>
    void SendBaselines(const std::shared_ptr<SubjectT>& subject, const std::shared_ptr<ReceiverT>& receiver)
    {
        std::vector<boost::optional<swganh::messages::BaselinesMessage>> baselines;

        {
            auto lock = subject->AcquireLock();
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline1(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline3(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline4(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline6(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline7(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline8(subject, lock));
            baselines.emplace_back(SubjectT::MessageBuilderType::BuildBaseline9(subject, lock));
        }

		for (auto& baseline : baselines)
        {
            if (baseline)
            {
                receiver->Notify(&(*baseline));
            }
        }

        SendEndBaselines(subject, receiver);

	}
	
	static swganh::messages::BaselinesMessage CreateBaselinesMessage(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock, uint8_t view_type, uint16_t opcount = 0) ;

	//
    //static swganh::messages::DeltasMessage CreateDeltasMessage(const std::shared_ptr<Object>& object, uint8_t view_type, uint16_t update_type, uint16_t update_count = 1) ;
	static swganh::messages::DeltasMessage CreateDeltasMessage(const std::shared_ptr<Object>& object, uint8_t view_type, uint16_t update_type, uint32_t object_type, uint16_t update_count = 1) ;

    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline1(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock)
    {
        return boost::optional<swganh::messages::BaselinesMessage>();
    }
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline3(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline4(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock)
    {
        return boost::optional<swganh::messages::BaselinesMessage>();
    }
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline6(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock);
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline7(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock)
    {
        return boost::optional<swganh::messages::BaselinesMessage>();
    }
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline8(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock)
    {
        return boost::optional<swganh::messages::BaselinesMessage>();
    }
    static boost::optional<swganh::messages::BaselinesMessage> BuildBaseline9(const std::shared_ptr<Object>& object, boost::unique_lock<boost::mutex>& lock)
    {
        return boost::optional<swganh::messages::BaselinesMessage>();
    }


protected:
    
	swganh::event_dispatcher::EventDispatcher* event_dispatcher_;
};

class	ObjectMessageBuilder : public BaseMessageBuilder
{
public:
    ObjectMessageBuilder(swganh::event_dispatcher::EventDispatcher* dispatcher)	:
        BaseMessageBuilder(dispatcher)
    {
        RegisterEventHandlers();
    }

    virtual ~ObjectMessageBuilder() {}

    virtual void RegisterEventHandlers();
    //virtual void SendEndBaselines(const std::shared_ptr<Object>& object, const std::shared_ptr<swganh::observer::ObserverInterface>& observer);
    static void BuildComplexityDelta(const std::shared_ptr<Object>& object);
    static void BuildStfNameDelta(const std::shared_ptr<Object>& object);
    static void BuildCustomNameDelta(const std::shared_ptr<Object>& object);
    static void BuildVolumeDelta(const std::shared_ptr<Object>& object);
    // delta 6
    static void BuildServerIDDelta(const std::shared_ptr<Object>& object);

    
    typedef swganh::event_dispatcher::ValueEvent<std::shared_ptr<Object>> ObjectEvent;

};

