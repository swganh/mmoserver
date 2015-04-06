/*
---------------------------------------------------------------------------------------
This source file is part of SWG:ANH (Star Wars Galaxies - A New Hope - Server Emulator)

For more information, visit http://www.swganh.com

Copyright (c) 2006 - 2010 The SWG:ANH Team
---------------------------------------------------------------------------------------
Use of this source code is governed by the GPL v3 license that can be found
in the COPYING file or at http://www.gnu.org/licenses/gpl-3.0.html

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
---------------------------------------------------------------------------------------
*/

#ifndef SRC_UTILS_CONCURRENTQUEUE_H_
#define SRC_UTILS_CONCURRENTQUEUE_H_

#include <tbb/atomic.h>

// @todo Need a way to programatically determine the cache line size for a given environment
// for now 64 will do for all environments we currently support.
#define CACHE_LINE_SIZE 64

namespace utils {

/**
 * ConcurrentQueue is a multi-producer, multi-consumer queue intended for use across multiple threads.
 *
 * This code is an implementation of a design discussed in a series of concept articles by Herb Sutter
 * on developing a concurrent queue that uses light weight spin-locks at the head and tail of the queue
 * to keep it concurrent without resulting to context switching.
 *
 * @see http://www.drdobbs.com/high-performance-computing/212201163
 */
template <typename T>
class ConcurrentQueue {
public:
    /// Default constructor initializes the queue to a default state.
    ConcurrentQueue() {
        first_ = last_ = new Node(nullptr);
        consumer_lock_ = producer_lock_ = false;

		queuesize = 0;
    }

    /// Default destructor cleans up any items remaining in the queue.
    ~ConcurrentQueue() {
        while (first_ != nullptr) {
            Node* tmp = first_;
            first_ = tmp->next;

            delete tmp->value;
            delete tmp;
        }
    }

	/**
     * Returns whether the queue is filled
     *
     * This function returns whether the queues is filled
     * 
     *
     */
	bool filled()	{
	
		// Spin-lock until exclusivity is acquired
		while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
			consumer_lock_ = false;
			return true;
        }

        consumer_lock_ = false;
        return false;
	}


	/**
     * Returns the Queues size
     *
     * This function returns the queues size 
     *
     */
	int size()	{
		return queuesize;
	}

    /**
     * Pushes an item onto the queue.
     *
     * This method pushes an item onto the queue. Keep in mind that a copy of the item
     * being put in is made so T must provide a copy constructor.
     *
     * \param t The item being pushed onto the queue.
     */
    void push(const T& t) {
        Node* tmp = new Node(new T(t));

        // Spin-lock until exclusivity is acquired
        while (producer_lock_.fetch_and_store(true)) {}

        last_->next = tmp;
        last_ = tmp;

		while (consumer_lock_.fetch_and_store(true)) {}
		queuesize++;
		consumer_lock_ = false;
        producer_lock_ = false;
    }

    /**
     * Pops an item off the front of the queue.
     *
     * Pops an item off the front of the queue and copies it into the container passed in.
     *
     * \param t The container to copy the queue item into.
     * \returns Returns true if an item was successfully popped, false if not or the queue was empty.
     */
    bool pop(T& t) {
        while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
            Node* old_first = first_;
            Node* first = first_->next;

            T* value = first->value;
            first->value = nullptr;
            first_ = first;

            consumer_lock_ = false;

            t = *value;
            delete value;
            delete old_first;
			while (producer_lock_.fetch_and_store(true)) {}
			queuesize--;
			producer_lock_ = false;
            return true;
        }

        consumer_lock_ = false;
        return false;
    }

	    /**
     * gives access to item at front of the queue.
     *
     * gives access to item at front of the queue. Please note that it might be snapped away if there are multiple consumers
     *
     * \returns Returns pointer to the first container on the queue
     */
    bool front(T& t) {
        while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
            
			Node* first = first_->next;
			T* value = first->value;

			consumer_lock_ = false;

			t = *value;
			return true;
         
        }

        consumer_lock_ = false;
        return false;
    }

private:
    // Node element used by the queue which holds the contained item and a pointer
    // to the next node in the queue.
    struct Node {
        Node(T* val) : value(val) {
            next = nullptr;
        }

        T* value;

        tbb::atomic<Node*> next;
        char pad[CACHE_LINE_SIZE - sizeof(T*) - sizeof(tbb::atomic<Node*>)];
    };

    // @note: All these pad* variables are here to prevent hidden contention caused by
    // the push and pop methods both accessing data in close proximity. By adding the padding
    // it ensures that these things are kept on separate cache lines which eliminates the contention.
    char pad0[CACHE_LINE_SIZE];

    // Accessed by one consumer at a time.
    Node* first_;
    char pad1[CACHE_LINE_SIZE - sizeof(Node*)];

    // Shared among consumers.
    tbb::atomic<bool> consumer_lock_;
    char pad2[CACHE_LINE_SIZE - sizeof(tbb::atomic<bool>)];

    // Accessed by one producer at a time.
    Node* last_;
    char pad3[CACHE_LINE_SIZE - sizeof(Node*)];

    // Shared among producers.
    tbb::atomic<bool> producer_lock_;
    char pad4[CACHE_LINE_SIZE - sizeof(tbb::atomic<bool>)];

	int queuesize;

};

template <typename T>
class ConcurrentQueueLight {
public:
    /// Default constructor initializes the queue to a default state.
    ConcurrentQueueLight() {
        first_ = last_ = new Node(nullptr);
        consumer_lock_ = producer_lock_ = false;
    }

    /// Default destructor cleans up any items remaining in the queue.
    ~ConcurrentQueueLight() {
        while (first_ != nullptr) {
            Node* tmp = first_;
            first_ = tmp->next;

            delete tmp->value;
            delete tmp;
        }
    }

	/**
     * Returns whether the queue is filled
     *
     * This function returns whether the queues is filled
     * 
     *
     */
	bool filled()	{
	
		// Spin-lock until exclusivity is acquired
		while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
			consumer_lock_ = false;
			return true;
        }

        consumer_lock_ = false;
        return false;
	}

    /**
     * Pushes an item onto the queue.
     *
     * This method pushes an item onto the queue. Keep in mind that a copy of the item
     * being put in is made so T must provide a copy constructor.
     *
     * \param t The item being pushed onto the queue.
     */
    void push(const T& t) {
        Node* tmp = new Node(new T(t));

        // Spin-lock until exclusivity is acquired
        while (producer_lock_.fetch_and_store(true)) {}

        last_->next = tmp;
        last_ = tmp;

        producer_lock_ = false;
    }

    /**
     * Pops an item off the front of the queue.
     *
     * Pops an item off the front of the queue and copies it into the container passed in.
     *
     * \param t The container to copy the queue item into.
     * \returns Returns true if an item was successfully popped, false if not or the queue was empty.
     */
    bool pop(T& t) {
        while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
            Node* old_first = first_;
            Node* first = first_->next;

            T* value = first->value;
            first->value = nullptr;
            first_ = first;

            consumer_lock_ = false;

            t = *value;
            delete value;
            delete old_first;
	
            return true;
        }

        consumer_lock_ = false;
        return false;
    }

	    /**
     * gives access to item at front of the queue.
     *
     * gives access to item at front of the queue. Please note that it might be snapped away if there are multiple consumers
     *
     * \returns Returns pointer to the first container on the queue
     */
    bool front(T& t) {
        while (consumer_lock_.fetch_and_store(true)) {}

        if (first_->next != nullptr) {
            
			Node* first = first_->next;
			T* value = first->value;

			consumer_lock_ = false;

			t = *value;
			return true;
         
        }

        consumer_lock_ = false;
        return false;
    }

private:
    // Node element used by the queue which holds the contained item and a pointer
    // to the next node in the queue.
    struct Node {
        Node(T* val) : value(val) {
            next = nullptr;
        }

        T* value;

        tbb::atomic<Node*> next;
        char pad[CACHE_LINE_SIZE - sizeof(T*) - sizeof(tbb::atomic<Node*>)];
    };

    // @note: All these pad* variables are here to prevent hidden contention caused by
    // the push and pop methods both accessing data in close proximity. By adding the padding
    // it ensures that these things are kept on separate cache lines which eliminates the contention.
    char pad0[CACHE_LINE_SIZE];

    // Accessed by one consumer at a time.
    Node* first_;
    char pad1[CACHE_LINE_SIZE - sizeof(Node*)];

    // Shared among consumers.
    tbb::atomic<bool> consumer_lock_;
    char pad2[CACHE_LINE_SIZE - sizeof(tbb::atomic<bool>)];

    // Accessed by one producer at a time.
    Node* last_;
    char pad3[CACHE_LINE_SIZE - sizeof(Node*)];

    // Shared among producers.
    tbb::atomic<bool> producer_lock_;
    char pad4[CACHE_LINE_SIZE - sizeof(tbb::atomic<bool>)];

};


}  // namespace utils

#endif  // SRC_UTILS_CONCURRENTQUEUE_H_
