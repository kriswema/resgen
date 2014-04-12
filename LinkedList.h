/*
RESGen. A tool to create .res files for Half-Life.
Copyright (C) 2000-2005 Jeroen Bogers

This file is part of RESGen.

RESGen is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

RESGen is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RESGen; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// LinkedList.h: interface for the LinkedList class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_LINKEDLIST_H__F2761432_1A69_427E_801C_D910521E00C8__INCLUDED_
#define AFX_LINKEDLIST_H__F2761432_1A69_427E_801C_D910521E00C8__INCLUDED_

// single thread mode
#define LL_SINGLETHREAD

#ifndef LL_SINGLETHREAD
#ifndef WIN32
#include <pthread.h>
#endif
#endif

class LinkedList
{
public:
	int Find(void *compdata, int (*compare)(void *,void *));
	bool InsertSorted(void *info, int (*compare)(void *,void *), bool doublesallowed);
	void InsertAt(void *info, int index);
	void RemoveAt(int index);
	void *GetAt(int index);
	void AddHead(void *info);
	void AddTail(void *info);

	 // Returns the numer of nodes in the list
	int GetCount() const;

	LinkedList();
	virtual ~LinkedList();


private:
	int (*sortfunc)(void *,void *);
#ifndef LL_SINGLETHREAD
#ifdef WIN32
	HANDLE hCountLock;
	int ReaderCount;
	HANDLE hNoReaders;
	HANDLE hWriteLock;
#else
	pthread_mutex_t mReadWriteLock;
#endif
	void DoneWriting();
	void WaitToWrite();
	void DoneReading();
	void WaitToRead();
#endif

	struct node
	{
		node *next; //pointer to the next node
		node *prev; // pointer to previous node
		void *data;
	}; // A node in the list

	void MemError();
	int count; // number of nodes in the list
	node *head; // Pointer to the head (start) of the list
	node *tail; // Pointer to the tail (last node) of the list
	node *lastnode; // Pointer to the last node requested with 'get'
	LinkedList::node * ForwardSearch(LinkedList::node * startnode, int startindex, int searchindex);
	LinkedList::node * BackwardSearch(LinkedList::node * startnode, int startindex, int searchindex);
	LinkedList::node * GetNodeAt(int index);
	int lastget; // Number of the last node requested with 'get'

};

#endif // !defined(AFX_LINKEDLIST_H__F2761432_1A69_427E_801C_D910521E00C8__INCLUDED_)
