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

#ifndef	ANH_UTILS_STACK_H
#define	ANH_UTILS_STACK_H

//==============================================================================================================================
template <class T, unsigned long int initialStackSize = 64>
class Stack
{
public:
    Stack() :
        allocSize(initialStackSize),
        blockCount(0),
        freeList(0),
        head(0),
        stackSize(0)
    {
        allocateNodes(initialStackSize);
    };

    ~Stack()
    {
        unsigned long int i;
        for(i = 0; i < blockCount; i++)
        {
            Node * n = blockAllocs[i];
            delete[] n;
        }
        head = 0;
        freeList = 0;
    };

    void	clear(void)
    {
        Node *	n = head;
        while(n)
        {
            head = head->next;
            n->next = freeList;
            freeList = n;
            n = head;
        }
        head = 0;
        stackSize = 0;
    };

    T		peek(void)
    {
        if(head) return head->element;
        return 0;
    };

    T		pop(void)
    {
        Node *	n = head;
        if(n)
        {
            head = head->next;
            stackSize--;
            n->next = freeList;
            freeList = n;
            return n->element;
        }
        return 0;
    }
    void	push(T element)
    {
        Node * n = freeList;

        if(n == 0)
        {
            grow();
            n = freeList;
        }
        freeList = freeList->next;

        n->next = head;
        memcpy(&n->element, &element, sizeof(T));
        head = n;
        stackSize++;
    }

    unsigned long int	size(void)
    {
        return stackSize;
    }

private:
    class Node
    {
    public:
        Node() :
            next(0)
        {
        };

        ~Node()
        {
            next = 0;
        };

        T 		element;
        Node *	next;
    };

    void allocateNodes(unsigned long int newStackSize)
    {
        Node * nblock = new Node[newStackSize];
        nblock[newStackSize-1].next = freeList;
        freeList = &nblock[0];
        blockAllocs[blockCount] = nblock;
        blockCount++;
        // now for the expensive part
        unsigned long int i;
        for(i = 0; i < newStackSize-2; i++)
        {
            nblock[i].next = &nblock[i+1];
        }
    };

    void grow(void)
    {
        allocateNodes(allocSize);
        allocSize += allocSize;
    };
    Stack(const Stack & other);
    Stack	operator=(const Stack & rhs);
private:
    unsigned long int	allocSize;
    Node *				blockAllocs[128];
    unsigned long int	blockCount;
    Node *				freeList;
    Node *				head;
    unsigned long int	stackSize;
};


#endif //STACK_H

