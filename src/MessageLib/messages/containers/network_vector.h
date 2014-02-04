// This file is part of SWGANH which is released under the MIT license.
// See file LICENSE or go to http://swganh.com/LICENSE
#pragma once

#include <vector>
#include <queue>
#include <functional>

#include "MessageLib/messages/baselines_message.h"
#include "MessageLib/messages/deltas_message.h"

#include "default_serializer.h"

namespace swganh
{
namespace containers
{

template<typename T, typename Serializer=DefaultSerializer<T>>
		/*
		*@brief NetworkVector is the helper Class to deserialize a Vector to baselines and deltas
		*it is used for datastructures where the client expects an index of the elements position in the delta
		*like in many YALP data structures like commands (Abilities) or the DraftSchematicList or Ham in the creo
		*
		*/
        class NetworkVector 
        {
            public:
            typedef typename std::vector<T>::const_iterator const_iterator;
            typedef typename std::vector<T>::iterator iterator;

            NetworkVector()
                : update_counter_(0)
{
}

uint32_t get_counter ()	{
	return	update_counter_;
}

NetworkVector(uint32_t reserve)
    : update_counter_(0)
{
    for(uint32_t i=0; i < reserve; ++i)
    {
        data_.push_back(0);
    }
}

NetworkVector(const std::vector<T>& data)
    : update_counter_(0)
{
    data_ = data;
}

void serialize_it(swganh::ByteBuffer& data, const T& t)
{
	Serializer::SerializeDelta(data, (T)t);
}

void remove(const uint16_t index, bool update=true)
{
    data_.erase(data_.begin() + index);
    if(update)
    {
        deltas_.push([=] (swganh::messages::DeltasMessage& message)
        {
            message.data.write<uint8_t>(0);//_remove);
            message.data.write<uint16_t>(index);
        });
    }
}

void remove(iterator itr, bool update=true)
{
    remove(itr - data_.begin(), update);
}

void add_initialize(const T& data)
{
    data_.push_back(data);
}

void add(const T& data)
{

    data_.push_back(data);

    uint16_t index = data_.size()-1;
    T& new_data = data_[index];

    deltas_.push([=] (swganh::messages::DeltasMessage& message)
    {
		message.data.write<uint8_t>(1);//_add
        message.data.write<uint16_t>(index);
		serialize_it(message.data, new_data);
        //SerializeDelta(message.data, new_data);
    });
}

void update(const uint16_t index)
{
    deltas_.push([=] (swganh::messages::DeltasMessage& message)
    {
        message.data.write<uint8_t>(2);//_update);
        message.data.write<uint16_t>(index);
		serialize_it(message.data, data_[index]);
        //Serializer::SerializeDelta(message.data, data_[index]);
    });
}

void update(iterator itr)
{
    update(itr - data_.begin());
}

//update updates the given índex with a new value
//it returns true if the value was changed(different)
//otherwise NO UPDATE MUST BE SENT, else the client will desynchronize
bool  update(const uint16_t index, const T& new_value, bool serialize = true)
{
	if(data_[index] == new_value)	{
		return false;
	}
    data_[index] = new_value;
	
	//if the value gets initialized by the factory before the basleine
	//we MUST NOT send a delta otherwise the client will desynchronize
	if(serialize)	{
		update(index);
	}
	return true;
}

void reset(const std::vector<T>& other, bool update=true)
{
    if(update)
    {
        //A copy will be made for the update
        deltas_.push([=] (swganh::messages::DeltasMessage& message)
        {
            message.data.write<uint8_t>(3);//delta_flag::_reset);
            message.data.write<uint16_t>(other.size());
            for(auto& item : other)
            {
                Serializer::SerializeDelta(message.data, item);
            }
        });
    }

    //The original will be moved in.
    data_ = std::move(other);
}

void clear(bool update=true)
{
    data_.clear();
    if(update)
    {
        deltas_.push([] (swganh::messages::DeltasMessage& message)
        {
            message.data.write<uint8_t>(4);
        });
    }
}

void erase(const T& value)
{
    auto itr = std::find(data_.begin(), data_.end(), value);
    if(itr != data_.end())
    {
        data_.erase(itr);
    }
}

/*
*	@brief gives the size of the serialized updates sitting in the deltas_ Queue
*
*/
uint32_t update_size()
{
    return deltas_.size();
}

std::vector<T> data()
{
    return data_;
}

std::vector<T>& raw()
{
    return data_;
}

T& get(const uint16_t index)
{
    return data_.at(index);
}

T& operator[](const uint16_t index)
{
    return data_.at(index);
}

uint16_t size()
{
    return data_.size();
}

iterator begin()
{
    return data_.begin();
}

iterator end()
{
    return data_.end();
}

bool Serialize(swganh::messages::BaseSwgMessage* message)
{
    if(message->Opcode() == swganh::messages::BaselinesMessage::opcode)
    {
        Serialize(*((swganh::messages::BaselinesMessage*)message));
		return true;
    }
    else if(message->Opcode() == swganh::messages::DeltasMessage::opcode)
    {
        return Serialize(*((swganh::messages::DeltasMessage*)message));
    }
	return false;
}

void Serialize(swganh::messages::BaselinesMessage& message)
{
    message.data.write<uint32_t>(data_.size());
    //think of the players loggin in later and getting the baseline and then the deltas
	message.data.write<uint32_t>(update_counter_);
	auto& item = data_.begin();
	for(item = data_.begin(); item != data_.end(); item ++)
    {
        Serializer::SerializeBaseline(message.data, *item);
    }
}

bool Serialize(swganh::messages::DeltasMessage& message)
{
	if(deltas_.empty())	{
		return false;
	}

    message.data.write<uint32_t>(deltas_.size());
	//confirmed in captures for ham
	update_counter_ += deltas_.size();
    message.data.write<uint32_t>(update_counter_);

    while(!deltas_.empty())
    {
        deltas_.front()(message);
        deltas_.pop();
    }
	return true;
}

private:
std::vector<T> data_;
uint32_t update_counter_;
std::queue<std::function<void(swganh::messages::DeltasMessage&)>> deltas_;
/*
*most lists in the protocol use 0 for delete and 1 for add
*at least skillmods use 0 for add and 1 for remove!!!!! (use network_map for that)
*/
enum delta_flag {
	_remove = 0,
	_add,
	_update,
	_reset
};
  

        };//class

}//namespace
}
