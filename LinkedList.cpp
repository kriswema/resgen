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


#include "LinkedList.h"

void MemError()
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

