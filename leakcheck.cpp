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

// Leakcheck.cpp: implementation of the Leakcheck class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

#ifdef WIN32
#include <windows.h>
#define sleep(t) Sleep(t*1000)
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>

#include "leakcheck.h"

//////////////////////////////////////////////////////////////////////

// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Leakcheck leakchecker;

Leakcheck::Leakcheck()
{
	leaklist = NULL;
}

Leakcheck::~Leakcheck()
{
	// Get a listing of still-open new's
	Leaknode_s *currtemp;
	int i;
	char *file;

	printf("\n\nI got the following news for you:\n");
	if (!leaklist)
	{
		printf("You have cleaned up very well!\n");
		return;
	}

	// search first node with ptr
	currtemp = leaklist;
	i = 0;
	do
	{
		i++;

		#ifdef WIN32
		file = strrchr(currtemp->file, '\\');
		#else
		file = strrchr(currtemp->file, '/');
		#endif

		if (!file)
		{
			file = currtemp->file;
		}

		printf("Possible leak in '%s' on line %d\n", file, currtemp->line);

		currtemp = currtemp->nextnode; // advance
	} while (currtemp);

	printf("%d possible leaks found!\n", i);

}

void Leakcheck::AddNode(void *ptr, char *file, int line)
{
	Leaknode_s *tempnode;
	char *dispfile;

	#ifdef WIN32
	dispfile = strrchr(file, '\\');
	#else
	dispfile = strrchr(file, '/');
	#endif

	if (!dispfile)
	{
		dispfile = file;
	}

	tempnode = (Leaknode_s *)malloc(sizeof(Leaknode_s));
	tempnode->ptr = ptr;
	tempnode->file = file;
	tempnode->line = line;
	tempnode->nextnode = leaklist;
	leaklist = tempnode;

	//printf("NEW: %s, %d\n", dispfile, line);
	// Done adding
}

void Leakcheck::RemoveNode(void *ptr)
{
	Leaknode_s *currtemp, *prevtemp;

	if (!leaklist)
	{
		printf("Possible error with 'delete': Too many deletes.\n");
		sleep(5);
		return;
	}

	// search first node with ptr
	prevtemp = NULL;
	currtemp = leaklist;
	do
	{
		if (currtemp->ptr == ptr)
		{
			// Found it. Remove from list
			if (prevtemp)
			{
				prevtemp->nextnode = currtemp->nextnode; // relink list
			}
			else
			{
				leaklist = currtemp->nextnode; // we were the list start
			}

			free(currtemp); // cleanup

			//printf("DELETE\n");
			return; // done
		}

		prevtemp = currtemp;
		currtemp = currtemp->nextnode; // advance
	} while (currtemp);

	printf("Possible error with 'delete': Deleted non-existing node.\n");
	sleep(5);
}


#endif
