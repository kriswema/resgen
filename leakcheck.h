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


// Leakcheck.h: interface for the Leakcheck class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

#if !defined(AFX_LEAKCHECK_H__283A9345_285D_472E_9DA0_7B0B2DF82BD7__INCLUDED_)
#define AFX_LEAKCHECK_H__283A9345_285D_472E_9DA0_7B0B2DF82BD7__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Leakcheck
{
public:
	Leakcheck();
	virtual ~Leakcheck();
	void AddNode(void *ptr, char *file, int line);
	void RemoveNode(void *ptr);

private:
	struct Leaknode_s
	{
		Leaknode_s * nextnode;
		char *file;
		int line;
		void *ptr;
	} *leaklist;

};

extern Leakcheck leakchecker;

// LEAK checker
#include <malloc.h>

inline void * operator new(size_t size, char *file, int line);
inline void operator delete(void *p);
inline void * operator new[](size_t size, char *file, int line);
inline void operator delete[](void *p);


inline void * operator new(size_t size, char *file, int line)
{
	void *ptr = (void *)malloc(size);

	leakchecker.AddNode(ptr, file, line);

	return(ptr);
}


inline void operator delete(void *p)
{

	leakchecker.RemoveNode(p);

	free(p);
}

inline void operator delete(void *p, char *file, int line)
{
	file; // Keep the compiler happy
	line;

	leakchecker.RemoveNode(p);

	free(p);
}

inline void * operator new[](size_t size, char *file, int line)
{
	void *ptr = (void *)malloc(size);

	leakchecker.AddNode(ptr, file, line);

	return(ptr);
}


inline void operator delete[](void *p)
{

	leakchecker.RemoveNode(p);

	free(p);
}

inline void operator delete[](void *p, char *file, int line)
{
	file; // Keep the compiler happy
	line;

	leakchecker.RemoveNode(p);

	free(p);
}


#define new new(__FILE__, __LINE__)

#endif // !defined(AFX_LEAKCHECK_H__283A9345_285D_472E_9DA0_7B0B2DF82BD7__INCLUDED_)

#endif
