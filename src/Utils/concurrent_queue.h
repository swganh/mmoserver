/*
 This file is part of MMOServer. For more information, visit http://swganh.com
 
 Copyright (c) 2006 - 2011 The SWG:ANH Team

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef ANH_UTILS_CONCURRENT_QUEUE_H
#define ANH_UTILS_CONCURRENT_QUEUE_H

#include <boost/thread/recursive_mutex.hpp>
#include <queue>

//======================================================================================================================

namespace Anh_Utils
{
template<class T,class Container = std::deque<T>,class QueueMutex = boost::recursive_mutex >
class concurrent_queue
{
public:

    void push(const T& item)
    {
        boost::recursive_mutex::scoped_lock lk(mMutex);

        mContainer.push_back(item);
    }

    T pop()
    {
        boost::recursive_mutex::scoped_lock lk(mMutex);

        T item = mContainer.front();
        mContainer.pop_front();

        return(item);
    }

    bool empty() const
    {
        boost::recursive_mutex::scoped_lock lk(mMutex);

        bool result = mContainer.empty();

        return(result);
    }

    typename Container::size_type size()
    {
        boost::recursive_mutex::scoped_lock lk(mMutex);

        size_t result = mContainer.size();

        return(result);
    }

    T& front()
    {
        boost::recursive_mutex::scoped_lock lk(mMutex);

        T item = mContainer.front();

        return(item);
    }

private:

    // Win32 complains about stl during linkage, disable the warning.
#ifdef _WIN32
#pragma warning (disable : 4251)
#endif
    Container		mContainer;
    QueueMutex		mMutex;
    // Re-enable the warning.
#ifdef _WIN32
#pragma warning (default : 4251)
#endif
};
}

//======================================================================================================================

#endif

