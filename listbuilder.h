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

// listbuilder.h: interface for the ListBuilder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LISTBUILDER_H__EBF81BE5_23F6_426C_82E6_F5EB2AEDE98F__INCLUDED_)
#define AFX_LISTBUILDER_H__EBF81BE5_23F6_426C_82E6_F5EB2AEDE98F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinkedList.h"
#include "vstring.h"

struct file_s
{
	bool folder;
	bool recursive;
	VString name;
};

class ListBuilder
{
public:
#ifndef WIN32
	void SetSymLink(bool slink);
#endif
	void BuildList(LinkedList *srclist);
	ListBuilder(LinkedList *flist, LinkedList *excludes, bool beverbal, bool sdisp);
	virtual ~ListBuilder();

private:
	LinkedList * exlist;
	bool firstdir;
	void ListDir(const VString &path);
	bool recursive;
#ifndef WIN32
	bool symlink;
#endif
	void PrepExList();
	void AddFile(const VString &filename, bool checkexlist);
	bool searchdisp;
	bool verbal;
	LinkedList * filelist;
};

#endif // !defined(AFX_LISTBUILDER_H__EBF81BE5_23F6_426C_82E6_F5EB2AEDE98F__INCLUDED_)
