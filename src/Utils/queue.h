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

#ifndef ANH_UTILS_QUEUE_H
#define ANH_UTILS_QUEUE_H
/*
#include <memory.h>


//==============================================================================================================================
template <class T, unsigned int preAlloc = 64>
class Queue
{
public:
	// our internal node class
	template <class Tp>
	class Node
	{
	public:
		Tp										mData;
		Node*									mNext;
		Node*									mPrev;
	};

	// our iterator class
	class iterator
	{
	public:
		friend class Queue;

		iterator(void) : mNode(0), mNext(0), mPrev(0) {};
		iterator(Node<T>* node) : mNode(node), mNext(node->mNext), mPrev(node->mPrev) {};

		void			operator ++(void)					{ if (mNext) mNode = mNext; mNext = mNode->mNext; mPrev = mNode->mPrev; };
		void			operator ++(int i)				{ if (mNext) mNode = mNext; mNext = mNode->mNext; mPrev = mNode->mPrev; };
		void			operator --(void)					{ if (mPrev) mNode = mPrev; mNext = mNode->mNext; mPrev = mNode->mPrev; };
		void			operator --(int i)				{ if (mPrev) mNode = mPrev; mNext = mNode->mNext; mPrev = mNode->mPrev; };
		bool			operator ==(const iterator& in)	{ if (in.mNode == mNode) return true; else return false; };
		bool			operator !=(const iterator& in)	{ if (in.mNode == mNode) return false; else return true; };

		iterator&	operator =(const iterator& in)	{ if (!in.mNode->mPrev) mNode = in.mNode->mNext; else mNode = in.mNode; mNext = mNode->mNext; mPrev = mNode->mPrev; return *this;  };
		iterator&	operator =(Node<T>& in)		{ if (!in.mPrev) mNode = in.mNext; else mNode = &in; mNext = in.mNext; mPrev = in.mPrev; return *this;  };

		T&				operator *(void)					{ return mNode->mData;  };

		Node<T>*														mNode;
		Node<T>*														mNext;
		Node<T>*														mPrev;
	};

	// The reverse iterator class   This is a test
	class iterator_reverse : public iterator
	{
	public:
		friend class Queue;

		iterator&	operator =(const iterator& in)	{ if (!in.mNode->mNext) this->mNode = in.mNode->mPrev; else this->mNode = in.mNode; this->mNext = this->mNode->mNext; this->mPrev = this->mNode->mPrev; return *this;  };
	};

	friend class iterator;


	// This is our main list class
														Queue(void);
														~Queue(void);

	void											push(T object);
  void                      pop(void);

	T                         front(void);
	T 									      back(void);

	void											clear(void);

  bool                      empty(void)               { return (mSize == 0); }
	unsigned int							size(void)								{ return mSize; }

private:
	Node<T>*									GetFreeNode(void);
	void											ReleaseFreeNode(Node<T>* node);

	unsigned int							mSize;

	Node<T>*									mFirst;
	Node<T>*									mLast;
	Node<T>*									mTemp;

	Node<T>*									mFree;

	Node<T>										mPool[preAlloc];
};



//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline Queue<T, preAlloc>::Queue(void)
{
	unsigned int i = 0;

	mSize				= 0;
	mTemp				= 0;

	memset(&mPool[0], 0, sizeof(mPool));

	for (i = 0; i != preAlloc; i++)
	{
		mFree = &mPool[i];
		mFree->mNext = mTemp;
		if (i != preAlloc - 1)
		{
			mFree->mPrev = &mPool[i+1];
		}
		mTemp = mFree;
	}

	mFirst	= GetFreeNode();
	mLast	= GetFreeNode();
	mFirst->mNext = mLast;
	mLast->mPrev	= mFirst;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline Queue<T, preAlloc>::~Queue(void)
{

}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline void Queue<T, preAlloc>::push(T object)
{
	mTemp = GetFreeNode();

	mTemp->mData = object;
	mTemp->mNext = mLast;
	mTemp->mPrev = mLast->mPrev;

	mLast->mPrev->mNext = mTemp;
	mLast->mPrev = mTemp;

	mSize++;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline void Queue<T, preAlloc>::pop(void)
{
  if (mSize > 0)
  {
    mTemp = mFirst->mNext;
    mTemp->mNext->mPrev = mFirst;
    mFirst->mNext = mTemp->mNext;
    ReleaseFreeNode(mTemp);

	  mSize--;
  }
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
typename inline T Queue<T, preAlloc>::front(void)
{
	return mFirst->mNext->mData;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
typename inline T Queue<T, preAlloc>::back(void)
{
	return mLast->mPrev->mData;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline void Queue<T, preAlloc>::clear(void)
{
	Node<T>* node;

	while (mFirst->mNext != mLast)
	{
		node = mFirst->mNext;
		node->mNext->mPrev = mFirst;
		mFirst->mNext = node->mNext;
		ReleaseFreeNode(node);
	}
	mSize = 0;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
typename inline Queue<T, preAlloc>::template Node<T>* Queue<T, preAlloc>::GetFreeNode(void)
{
	Node<T>* node = mFree;
	if (!mFree->mNext)
	{
		mFree->mNext = new Node<T>;
		memset(mFree->mNext, 0, sizeof(Node<T>));
	}

	mFree->mNext->mPrev = 0;
	mFree = mFree->mNext;

	node->mNext = 0;
	node->mPrev = 0;

	return node;
}


//==============================================================================================================================
template <class T, unsigned int preAlloc>
inline void Queue<T, preAlloc>::ReleaseFreeNode(Node<T>* node)
{
	memset(&node->mData, 0, sizeof(node->mData));
	node->mPrev = 0;
	node->mNext = mFree;
	if (mFree)
	{
		mFree->mPrev = node;
	}

	mFree = node;
}
*/


#endif //ANH_UTILS_QUEUE_H

