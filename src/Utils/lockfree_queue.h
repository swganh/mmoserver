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


#ifndef ANH_UTILS_LOCKFREE_QUEUE_H
#define ANH_UTILS_LOCKFREE_QUEUE_H

#include "typedefs.h"


//==============================================================================================================================

namespace Anh_Utils
{

//==============================================================================================================================
//
// queue node
//

template<typename T> struct node
{
    node() : mValue(),mNext(NULL) {}
    node(T v) : mNext(NULL),mValue(v) {}

    T					mValue;
    node<T> * volatile	mNext;
};

//==============================================================================================================================
//
// compare and swap
//

template<typename T>
bool CAS(node<T>* volatile* _ptr,node<T>* oldVal,node<T>* newVal)
{
    register bool f;

    _asm
    {
        mov ecx,_ptr
        mov eax,oldVal
        mov ebx,newVal
        lock cmpxchg [ecx],ebx
        setz f
    }

    return f;
}

//==============================================================================================================================
//
// compare and swap II
//

template<typename T>
bool CAS2(node<T>* volatile* _ptr,node<T>* old1,uint32 old2,node<T>* new1,uint32 new2)
{
    register bool f;

    _asm
    {
        mov esi,_ptr
        mov eax,old1
        mov edx,old2
        mov ebx,new1
        mov ecx,new2
        lock cmpxchg8b [esi]
        setz f
    }

    return f;
}

//==============================================================================================================================
//
// queue
//

template<typename T> class lockfree_queue
{
    public:

    lockfree_queue(node<T>* dummy) : mPopCount(0),mPushCount(0)
    {
        mHead = mTail = dummy;
    }

    void		push(node<T>* element);
    node<T>*	pop();

    node<T>*	volatile	mHead;
    volatile	uint32		mPopCount;
    node<T>*	volatile	mTail;
    volatile	uint32		mPushCount;
};

//==============================================================================================================================

template<typename T> void lockfree_queue<T>::push(node<T>* element)
{
    element->mNext = NULL;

    uint32		pushCount;
    node<T> *	tail;

    while(true)
    {
        pushCount	= mPushCount;
        tail		= mTail;

        if(CAS(&(mTail->mNext),reinterpret_cast<node<T>*>(NULL),element))
        {
            break;
        }
        else
        {
            CAS2(&mTail,tail,pushCount,mTail->mNext,pushCount + 1);
        }
    }

    CAS2(&mTail,tail,pushCount,element,pushCount + 1);
}

//==============================================================================================================================

template<typename T> node<T>* lockfree_queue<T>::pop()
{
    T			value = T();
    node<T>*	head;

    while(true)
    {
        uint32 popCount		= mPopCount;
        uint32 pushCount	= mPushCount;

        head = mHead;

        node<T>* next = head->mNext;

        if(popCount != mPopCount)
        {
            continue;
        }

        if(head == _pTail)
        {
            if(!next)
            {
                head = NULL;
                break;
            }

            CAS2(&mTail,head,pushCount,next,pushCount + 1);
        }
        else if(next)
        {
            value = next->mValue;

            if(CAS2(&mHead,head,popCount,next,popCount + 1))
            {
                break;
            }
        }
    }

    if(head)
    {
        head->mValue = value;
    }

    return head;
}

//==============================================================================================================================
}

//==============================================================================================================================

#endif




