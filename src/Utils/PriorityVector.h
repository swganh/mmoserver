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

#ifndef ANH_UTILS_PRIORITY_VECTOR_H
#define ANH_UTILS_PRIORITY_VECTOR_H

#include <vector>
#include <algorithm>
#include <queue>


namespace Anh_Utils
{
//======================================================================================================================

template<class T, class Compare = std::less<typename std::vector<T>::value_type> >
class priority_vector : public std::vector<T>
{
public:

    priority_vector(const Compare& cmp = Compare()) : mCompare(cmp)
    {
        std::make_heap(this->begin(),this->end(),mCompare);
        mSorted = false;
    }

    //======================================================================================================================

    void assureHeap(bool force = false)
    {
        if(mSorted || force)
        {
            std::make_heap(this->begin(),this->end(),mCompare);
            mSorted = false;
        }
    }

    //======================================================================================================================

    const T& top()
    {
        assureHeap();
        return this->front();
    }

    //======================================================================================================================

    void push(const T& x)
    {
        assureHeap();
        this->push_back(x);
        std::push_heap(this->begin(), this->end(),mCompare);
    }

    //======================================================================================================================

    void pop()
    {
        assureHeap();
        std::pop_heap(this->begin(), this->end(),mCompare);
        this->pop_back();
    }

    //======================================================================================================================

    void sort()
    {
        if(!mSorted)
        {
            std::sort_heap(this->begin(),this->end(),mCompare);
            std::reverse(this->begin(),this->end());
            mSorted = true;
        }
    }

protected:

    Compare mCompare;
    bool	mSorted;
};
}

#endif

