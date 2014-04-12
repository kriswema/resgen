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

//#include <assert.h>
#define assert(expr) (expr || *(int*)0)

#include <stdexcept>

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
#endif


#ifndef LL_SINGLETHREAD
// RAII class for acquiring/releasing read lock
class ReadLock
{
public:
	ReadLock(LinkedList* const list_)
		: list(list_)
	{
		list->WaitToRead();
	}

	~ReadLock()
	{
		list->DoneReading();
	}

private:
	LinkedList* const list;
};

// RAII class for acquiring/releasing write lock
class WriteLock
{
public:
	WriteLock(LinkedList* const list_)
		: list(list_)
	{
		list->WaitToWrite();
	}

	~WriteLock()
	{
		list->DoneWriting();
	}

private:
	LinkedList* const list;
};
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

int LinkedList::GetCount() const
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

#ifndef LL_SINGLETHREAD
	WriteLock writeLock;
#endif

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
}

void LinkedList::AddHead(void *info)
{
	node *temp = new node;
	if (temp == NULL) { MemError(); }

	// set to null (defines beginning or end node)
	temp->next = NULL;
	temp->prev = NULL;

#ifndef LL_SINGLETHREAD
	WriteLock writeLock;
#endif

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

	if(lastget >= 0)
	{
		// increase lastget so it matches properly
		lastget++;
	}
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
#ifndef LL_SINGLETHREAD
	ReadLock readLock(this);
#endif

	if((index < 0) || (index >= count))
	{
		throw std::out_of_range("Index out of range");
	}

	node* const resultNode = GetNodeAt(index);
	assert(resultNode != NULL);

	return resultNode->data;
}

void LinkedList::RemoveAt(int index)
{
	node *temp; // temp node

#ifndef LL_SINGLETHREAD
	WriteLock writeLock;
#endif

	if((index < 0) || (index >= count))
	{
		throw std::out_of_range("Index out of range");
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

#endif

LinkedList::node * LinkedList::ForwardSearch(LinkedList::node * startnode, int startindex, int searchindex)
{
	// search for node we want.
	for (int i = startindex; i < searchindex; i++)
	{
		if(startnode == NULL)
		{
			throw std::out_of_range("Index out of range");
		}
		startnode = startnode->next;
	}

	lastget = searchindex;
	lastnode = startnode;

	return startnode;
}

LinkedList::node * LinkedList::BackwardSearch(LinkedList::node *startnode, int startindex, int searchindex)
{
	// search for node we want.
	for (int i = startindex; i > searchindex; i--)
	{
		if(startnode == NULL)
		{
			throw std::out_of_range("Index out of range");
		}
		startnode = startnode->prev;
	}

	lastget = searchindex;
	lastnode = startnode;

	return startnode;
}

LinkedList::node * LinkedList::GetNodeAt(int index)
{
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
			const int middle = ((count - lastget) / 2) + lastget; // will be rounded down.
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
			const int middle = (lastget / 2); // will be rounded semi-equally.
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
		const int middle = (count / 2); // will be rouded semi-equally
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

int LinkedList::Find(void *compdata, int (*compare)(void *,void *))
{
	// If the list is sorted, this function will preform a LOT faster.
	// Please note you have to use the SAME function for sorting as for finding
	// or the list will fall back to the slow lookup method.

#ifndef LL_SINGLETHREAD
	ReadLock readLock(this);
#endif

	if (sortfunc == compare)
	{
		// Returns -1 if not found, or the index of the found file
		int begin, end, pivot;
		int result;

		if (count == 0)
		{
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
				return i;
			}
		}
	}

	return -1; // not found
}

void LinkedList::InsertAt(void *info, int index)
{
	if ((index < 0) || (index > count))
	{
		throw std::out_of_range("Index out of range");
	}
	
	// Inserts a item BEFORE index (so the item currently at index gets bumped one place up)
	if (index == 0)
	{
		// Add as head
		AddHead(info);
		return;
	}
	if (index == count)
	{
		// index is bigger then existing, add as tail
		AddTail(info);
		return;
	}

	// insert
	node *temp = new node;
	if (temp == NULL) { MemError(); }

#ifndef LL_SINGLETHREAD
	WriteLock writeLock;
#endif
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
}

bool LinkedList::InsertSorted(void *info, int (*compare)(void *,void *), bool doublesallowed)
{
#ifndef LL_SINGLETHREAD
	WriteLock writeLock;
#endif

	// Inserts item in list, while preserving sorting
	if (sortfunc == compare && count >= 1) // if there is 1 item list is inherently sorted
	{
		int begin = 0;
		int end = count - 1;

		// attempt to find the string
		while (begin <= end) // this should never evaluate to false
		{
			const int pivot = ((end - begin) / 2) + begin; // at least equal to begin and always smaller then end

			const int result = compare(info, GetNodeAt(pivot)->data);

			if (result < 0)
			{
				// if string is here, it should be before the current
				end = pivot - 1;
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
				begin = pivot + 1;
				if (end < begin)
				{
					// search ended and nothing found. insert element after pivot
					InsertAt(info, pivot + 1);
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
