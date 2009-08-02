/*
---------------------------------------------------------------------------------------
This source file is part of swgANH (Star Wars Galaxies - A New Hope - Server Emulator)
For more information, see http://www.swganh.org


Copyright (c) 2006 - 2008 The swgANH Team

---------------------------------------------------------------------------------------
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
				std::make_heap(begin(),end(),mCompare);
				mSorted = false;
			}

	//======================================================================================================================

			void assureHeap(bool force = false)
			{
				if(mSorted || force) 
				{
					std::make_heap(begin(),end(),mCompare);
					mSorted = false;
				}
			}

	//======================================================================================================================

			const T& top()
			{
				assureHeap();
				return front(); 
			}

	//======================================================================================================================

			void push(const T& x)
			{
				assureHeap();
				push_back(x);
				std::push_heap(begin(), end(),mCompare);
			}

	//======================================================================================================================

			void pop()
			{
				assureHeap();
				std::pop_heap(begin(), end(),mCompare);
				pop_back();
			}

	//======================================================================================================================

			void sort()
			{
				if(!mSorted)
				{
					std::sort_heap(begin(),end(),mCompare);
					std::reverse(begin(),end());
					mSorted = true;
				}
			}

		protected:

			Compare mCompare;
			bool	mSorted;
	};
}

#endif

