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


// LinkedList.cpp: implementation of the LinkedList class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

#include "LinkedList.h"
#include "leakcheck.h"

#ifndef LL_SINGLETHREAD
#ifndef WIN32
/* The linux pthread headers do not have this function although it
exists in the pthread library and it works as documented */
extern "C" int pthread_mutexattr_setkind_np __P((pthread_mutexattr_t *attr, int kind));
#endif
#else
#define WaitToWrite()
#define WaitToRead()
#define DoneWriting()
#define DoneReading()
#endif



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LinkedList::LinkedList()
{
	count = 0;
	lastget = -1;
	lastnode = NULL;
	head = NULL;
	tail = NULL;
	sortfunc = NULL; // we are NOT sorted

	// set up thread safety
#ifndef LL_SINGLETHREAD
#ifdef WIN32
	ReaderCount = 0;

	hNoReaders = CreateEvent(NULL, true, true, NULL);
	hWriteLock = CreateMutex(NULL, false, NULL);
	hCountLock = CreateMutex(NULL, false, NULL);
#else
	pthread_mutexattr_t mutexattr;
	pthread_mutexattr_init(&mutexattr);
	pthread_mutexattr_setkind_np(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);

	pthread_mutex_init(&mReadWriteLock, &mutexattr);

	pthread_mutexattr_destroy(&mutexattr);
#endif
#endif
}

LinkedList::~LinkedList()
{
	// clean up list
	// note that this will NOT clean up any allocated memory for whatever the node is pointing to
	while (count > 0)
	{
		RemoveAt(0);
	}

	// clean up handles
#ifndef LL_SINGLETHREAD
#ifdef WIN32
	CloseHandle(hNoReaders);
	CloseHandle(hWriteLock);
	CloseHandle(hCountLock);
#else
	pthread_mutex_destroy(&mReadWriteLock);
#endif
#endif
}

int LinkedList::GetCount() // returns the numer of nodes in the list
{
	return count;
}

void LinkedList::AddTail(void *info)
{
	node *temp = new node;
	if (temp == NULL) { MemError(); }

	// set to null (defines beginning or end node)
	temp->next = NULL;
	temp->prev = NULL;

	WaitToWrite();
	sortfunc = NULL; // Not sorted anymore

	temp->data = info; // set data

	if (count == 0)
	{
		// first link in list
		head = temp;
	}
	else
	{
		tail->next = temp; // set 'next' for previous link
		temp->prev = tail; // set 'prev' for the new link
	}

	tail = temp;
	count++;

	DoneWriting();
}

void LinkedList::AddHead(void *info)
{
	node *temp = new node;
	if (temp == NULL) { MemError(); }

	// set to null (defines beginning or end node)
	temp->next = NULL;
	temp->prev = NULL;

	WaitToWrite();
	sortfunc = NULL; // Not sorted anymore

	temp->data = info; // set data

	if (count == 0)
	{
		// first link in list
		tail = temp;
	}
	else
	{
		head->prev = temp; // set 'prev' for previous link
		temp->next = head; // set 'next' for the new link
	}

	head = temp;
	count++;

	// increase lastget so it matches properly
	lastget++;

	DoneWriting();
}

void LinkedList::MemError()
{
	// Memmory error.. We cannot continue... quit!
#ifdef WIN32
	MessageBox(NULL, "Failed to allocate memmory for Linked List. Aborting.", "ERROR", MB_OK | MB_ICONHAND | MB_SYSTEMMODAL);
	ExitProcess(0);
#else
	printf("Failed to allocate memmory for Linked List. Aborting.\n");
	exit(0);
#endif
}

void * LinkedList::GetAt(int index)
{
	void *retval;

	WaitToRead();
	if (index >= count)
	{
		// request outside list
		DoneReading();
		return NULL;
	}

	retval = GetNodeAt(index)->data;

	DoneReading();

	return retval;
}

void LinkedList::RemoveAt(int index)
{
	node *temp; // temp node

	WaitToWrite();
	if (index >= count)
	{
		// request outside list
		DoneWriting();
		return;
	}

	// Get the node we want
	temp = GetNodeAt(index);

	// change count
	count--;

	// set lastget
	if (count == 0)
	{
		// invalidate lastget.
		lastget = -1;
		lastnode = NULL;
	}
	else
	{
		if (index != 0)
		{
			// set to previous node
			lastget = index - 1;
			lastnode = temp->prev;
		}
		else
		{
			// set to next node
			lastget = index;
			lastnode = temp->next;
		}
	}

	// modify previous node to point to next node. Also change the head and/or tail if needed.
	if (temp->next == NULL)
	{
		// we removed the tail node
		tail = temp->prev;
	}
	else
	{
		temp->next->prev = temp->prev;
	}

	if (temp->prev == NULL)
	{
		// we removed the head.
		head = temp->next;
	}
	else
	{
		temp->prev->next = temp->next;
	}

	// remove the node from memory
	delete temp;

	DoneWriting();
}

#ifndef LL_SINGLETHREAD
void LinkedList::WaitToRead()
{
#ifdef WIN32
	// if mutex is free, we can read (no writers)
	WaitForSingleObject(hWriteLock, INFINITE);

	// if the readercount is 0, reset the event to lock out writers
	WaitForSingleObject(hCountLock, INFINITE);
	if (!ReaderCount)
	{
		ResetEvent(hNoReaders);
	}

	ReaderCount++; // safe to do it this way becase we have the mutex
	ReleaseMutex(hCountLock);

	ReleaseMutex(hWriteLock);
#else
	WaitToWrite();
#endif
}

void LinkedList::DoneReading()
{
#ifdef WIN32
	WaitForSingleObject(hCountLock, INFINITE);
	ReaderCount--;

	if (!ReaderCount)
	{
		SetEvent(hNoReaders);
	}
	ReleaseMutex(hCountLock);
#else
	DoneWriting();
#endif
}

void LinkedList::WaitToWrite()
{
#ifdef WIN32
	HANDLE phHandles[2];

	// we can write if there are no readers and the mutex is free (no writers)
	phHandles[0] = hWriteLock;
	phHandles[1] = hNoReaders;
	WaitForMultipleObjects(2, phHandles, true, INFINITE);

	// we can write now
#else
	pthread_mutex_lock(&mReadWriteLock);
#endif
}

void LinkedList::DoneWriting()
{
#ifdef WIN32
	// done with writing. release the mutex
	ReleaseMutex(hWriteLock);
#else
	pthread_mutex_unlock(&mReadWriteLock);
#endif
}

void LinkedList::LockList()
{
	WaitToWrite();
}

void LinkedList::UnLockList()
{
	DoneWriting();
}
#endif

LinkedList::node * LinkedList::ForwardSearch(LinkedList::node * startnode, int startindex, int searchindex)
{
	int i;

	// search for node we want.
	for (i = startindex; i < searchindex; i++)
	{
		startnode = startnode->next;
	}

	lastget = searchindex;
	lastnode = startnode;

	return startnode;
}

LinkedList::node * LinkedList::BackwardSearch(LinkedList::node *startnode, int startindex, int searchindex)
{
	int i;

	// search for node we want.
	for (i = startindex; i > searchindex; i--)
	{
		startnode = startnode->prev;
	}

	lastget = searchindex;
	lastnode = startnode;

	return startnode;
}

LinkedList::node * LinkedList::GetNodeAt(int index)
{
	int middle;

	if (lastget > 0) //lastget is valid
	{
		if (index == lastget)
		{
			// We want the same again
			return lastnode;
		}
		if (index > lastget)
		{
			// after lastget
			// determine fastest way to goal
			middle = ((count - lastget) / 2) + lastget; // will be rounded down.
			if (index <= middle)
			{
				// search is closer to lastget
				return ForwardSearch(lastnode, lastget, index);
			}
			else
			{
				// search is closer to end of list
				return BackwardSearch(tail, count - 1, index);
			}
		}
		else
		{
			// before lastget
			// determine fastest way to goal
			middle = (lastget / 2); // will be rounded semi-equally.
			if (index <= middle)
			{
				// search is closer to head
				return ForwardSearch(head, 0, index);
			}
			else
			{
				// search is closer to lastget
				return BackwardSearch(lastnode, lastget, index);
			}
		}
	}
	else
	{
		// no lastget... Do a forward or backward search
		middle = (count / 2); // will be rouded semi-equally
		if (index <= middle)
		{
			// search is closer to head
			return ForwardSearch(head, 0, index);
		}
		else
		{
			// search is closer to tail
			return BackwardSearch(tail, count - 1, index);
		}
	}

	//return NULL; // can never happen
}

void LinkedList::BSort(int (*compare)(void *elem1, void *elem2))
{
	int max, bg, i;
	node *a;
	node *b;

	WaitToWrite();

	max = count - 1;
	while (max > 0)
	{
		bg = max - 1;
		max = 0;
		for (i = 0; i <= bg; i++)
		{
			a = GetNodeAt(i);
			b = GetNodeAt(i + 1);
			if (compare(a->data, b->data) > 0)
			{
				Swap(a, b);
				max = i;
			}
		}
	}

	sortfunc = compare; // we are now sorted

	DoneWriting();
}

void LinkedList::Swap(node *na, node *nb)
{
	void *tmp;

	// swap data
	tmp = na->data;
	na->data = nb->data;
	nb->data = tmp;
}

void LinkedList::QSort(int (*compare)(void *,void *))
{
	if (count < 8)
	{
		BSort(compare); // faster for small lists
	}
	else
	{
		WaitToWrite();

		QSortInternal(0, count-1, compare);

		sortfunc = compare; // We are now sorted

		DoneWriting();
	}
}

void LinkedList::QSortInternal(int begin, int end, int (*compare)(void *,void *))
{
	if (end > begin)
	{
		node *nl;
		Swap(GetNodeAt(begin), GetNodeAt((end-begin)/2+begin)); // this improves preformance with sorted lists
		void *pivot = GetNodeAt(begin)->data;
		int l = begin + 1;
		int r = end;
		while (l < r)
		{
			nl = GetNodeAt(l);
			if (compare(nl->data, pivot) <= 0)
			{
				l++;
			}
			else
			{
				r--;
				Swap(nl, GetNodeAt(r));
			}
		}
		l--;
		Swap(GetNodeAt(begin), GetNodeAt(l));
		QSortInternal(begin, l, compare);
		QSortInternal(r, end, compare);
	}
}

int LinkedList::Find(void *compdata, int (*compare)(void *,void *))
{
	// If the list is sorted, this function will preform a LOT faster.
	// Please note you have to use the SAME function for sorting as for finding
	// or the list will fall back to the slow lookup method.

	WaitToRead();

	if (sortfunc == compare)
	{
		// Returns -1 if not found, or the index of the found file
		int begin, end, pivot;
		int result;

		if (count == 0)
		{
			DoneReading();
			return -1; // No list, so not found!
		}

		begin = 0;
		end = count - 1;

		while (begin <= end)
		{
			pivot = (end-begin)/2+begin; // at least equal to begin and always smaller then end

			result = compare(compdata, GetNodeAt(pivot)->data);
			if (result < 0)
			{
				// if string is here, it should be before the current
				end = pivot-1;
			}
			else if (result > 0)
			{
				// if string is here, it should be after the current
				begin = pivot+1;
			}
			else
			{
				// string found!
				DoneReading();
				return pivot;
			}
		}
	}
	else
	{
		// do slow search, try every entry until found
		int i;

		for (i = 0; i < count; i++)
		{
			if (!compare(compdata, GetNodeAt(i)->data))
			{
				DoneReading();
				return i;
			}
		}
	}

	DoneReading();

	return -1; // not found
}

void LinkedList::FilterDoubles(int (*compare)(void *,void *), void (*erase)(void *))
{
	// filters double entries. VERY slow on an unsorted list!
	int i, j;
	void *start;
	void *next;

	WaitToWrite();

	if (count <= 1)
	{
		DoneWriting();
		return; // Doubles not possible!
	}

	if (sortfunc == compare)
	{
		// Filter doubles from list
		start = GetNodeAt(0)->data;
		for (i = 1; i < count; i++)
		{
			next = GetNodeAt(i)->data;
			if (!compare(start, next))
			{
				// double!
				RemoveAt(i);
				erase(next);
				i--; // shorten list
			}
			else
			{
				start = next;
			}
		}
	}
	else
	{
		// Filter doubles from list, but in a very slow way
		start = GetNodeAt(0)->data;
		for (i = 0; i < count-1; i++)
		{
			// check every item except the last one with EVERY item after it.
			start = GetNodeAt(i)->data;
			for (j = i + 1; j < count; j++)
			{
				next = GetNodeAt(j)->data;
				if (!compare(start, next))
				{
					// double!
					RemoveAt(i);
					erase(next);
					j--; // shorten list
				}
			}
		}
	}

	DoneWriting();
}

void LinkedList::InsertAt(void *info, int index)
{
	// Inserts a item BEFORE index (so the item currently at index gets bumped one place up)
	if (index <= 0)
	{
		// Add as head
		AddHead(info);
		return;
	}
	if (index >= count)
	{
		// index is bigger then existing, add as tail
		AddTail(info);
		return;
	}

	// insert
	node *temp = new node;
	if (temp == NULL) { MemError(); }

	WaitToWrite();
	sortfunc = NULL; // Not sorted anymore

	temp->data = info; // set data

	// Set our next and prev
	temp->next = GetNodeAt(index);
	temp->prev = temp->next->prev;

	// Set others next and prev
	temp->next->prev = temp;
	temp->prev->next = temp;

	count++;

	if (lastget >= index)
	{
		lastget++; // keep it matching with the actual index number
	}

	DoneWriting();
}

bool LinkedList::InsertSorted(void *info, int (*compare)(void *,void *), bool doublesallowed)
{
	// Inserts item in list, while preserving sorting
	if (sortfunc == compare && count >= 1) // if there is 1 item list is inherently sorted
	{
		int begin, end, pivot;
		int result;

		WaitToWrite();

		begin = 0;
		end = count - 1;

		// attempt to find the string
		while (begin <= end) // this should never evaluate to false
		{
			pivot = (end-begin)/2+begin; // at least equal to begin and always smaller then end

			result = compare(info, GetNodeAt(pivot)->data);
			if (result < 0)
			{
				// if string is here, it should be before the current
				end = pivot-1;
				if (end < begin)
				{
					// search ended and nothing found. insert element at pivot (will be placed in front)
					InsertAt(info, pivot);
					sortfunc = compare; // restore sortfunc
					return true;
				}
			}
			else if (result > 0)
			{
				// if string is here, it should be after the current
				begin = pivot+1;
				if (end < begin)
				{
					// search ended and nothing found. insert element after pivot
					InsertAt(info, pivot+1);
					sortfunc = compare; // restore sortfunc
					return true;
				}
			}
			else
			{
				if (doublesallowed)
				{
					InsertAt(info, pivot);
					sortfunc = compare; // restore sortfunc
					return true; // Done
				}

				return false; // doubles not allowed
			}
		}

		return false; // Should never occur, unless there is a bug
	}
	else if (count == 0)
	{
		AddTail(info); // Just add as tail

		sortfunc = compare; // only 1 item, so we are sorted

		return true; // list was not sorted
	}
	else
	{
		// List is not sorted, so we cannot insert sorted
		return false;
	}
}
