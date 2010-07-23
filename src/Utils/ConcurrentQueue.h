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

#pragma warning(disable:4800)
#include <boost/atomic.hpp>
#pragma warning(default:4800)

using ::boost::atomic;

#define CACHE_LINE_SIZE 64

template <typename T>
class ConcurrentQueue {
private:
    struct Node {
        Node(T* val) : value(val), next(nullptr) {}

        T* value;
        atomic<Node*> next;
    };
    
    char pad0[CACHE_LINE_SIZE];

    // Accessed by one consumer at a time.
    Node* first_;

    char pad1[CACHE_LINE_SIZE - sizeof(Node*)];

    // Shared among consumers.
    atomic<bool> consumer_lock_;

    char pad2[CACHE_LINE_SIZE - sizeof(atomic<bool>)];
    
    // Accessed by one producer at a time.
    Node* last_;

    char pad3[CACHE_LINE_SIZE - sizeof(Node*)];
    
    // Shared among producers.
    atomic<bool> producer_lock_;
    
    char pad4[CACHE_LINE_SIZE - sizeof(atomic<bool>)];

public:
    ConcurrentQueue() {
        first_ = last_ = new Node(nullptr);
        consumer_lock_ = producer_lock_ = false;
    }

    ~ConcurrentQueue() {
        while (first_ != nullptr) {
            Node* tmp = first_;
            first_ = tmp->next;

            delete tmp->value;
            delete tmp;
        }
    }

    void push(const T& t) {
        Node* tmp = new Node(new T(t));

        // Spin-lock until exclusivity is acquired
        while (producer_lock_.exchange(true)) {}

        last_->next = tmp;
        last_ = tmp;

        producer_lock_ = false;
    }
    
    bool pop(T& t) {
        while (consumer_lock_.exchange(true)) {}

        Node* first = first_;
        Node* next = first_->next;

        if (next != nullptr) {
            T* value = next->value;
            next->value = nullptr;
            first_ = next;

            consumer_lock_ = false;

            t = *value;
            delete value;
            delete first;

            return true;
        }

        consumer_lock_ = false;
        return false;
    }

};

#endif  // SRC_UTILS_CONCURRENTQUEUE_H_
