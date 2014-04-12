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

// listbuilder.cpp: implementation of the ListBuilder class.
//
//////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "listbuilder.h"
#include "leakcheck.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ListBuilder::ListBuilder(LinkedList<VString *> *flist, LinkedList<file_s *> *excludes, bool beverbal, bool sdisp)
{
#ifdef _DEBUG
	if (flist == NULL)
	{
		printf("P_ERROR (ListBuilder::ListBuilder): No file list.\n");
		return;
	}
#endif

	filelist = flist;
	verbal = beverbal;
	searchdisp = sdisp;
	exlist = excludes;
}

ListBuilder::~ListBuilder()
{

}

void ListBuilder::BuildList(LinkedList<file_s *> *srclist)
{
#ifdef _DEBUG
	if (srclist == NULL)
	{
		printf("P_ERROR (ListBuilder::BuildList): No source list.\n");
		return;
	}

	if (srclist->GetCount() == 0)
	{
		printf("P_ERROR (ListBuilder::BuildList): Source list empty.\n");
		return;
	}

	if (exlist == NULL)
	{
		printf("P_ERROR (ListBuilder::BuildList): No exlucdes list.\n");
		return;
	}
#endif

	PrepExList();

	// walk entries and take appropritate actions.
	for (int i = 0; i < srclist->GetCount(); i++)
	{
		file_s *file = srclist->GetAt(i);

		if (file->folder == false)
		{
			// single file processing
			AddFile(file->name, false);
		}
		else
		{
			// folder processing

			// prepare folder name
			#ifdef WIN32
			if (file->name[file->name.GetLength() - 1] != '\\')
			{
				// No ending "\", add
				file->name += "\\";
			}
			#else
			if (file->name[file->name.GetLength() - 1] != '/')
			{
				// No ending "/", add
				file->name += "/";
			}
			#endif

			recursive = file->recursive;

			if (verbal)
			{
				if (recursive)
				{
					printf("Searching %s and subdirectories for bsp files...\n", (LPCSTR)file->name);
				}
				else
				{
					printf("Searching %s for bsp files...\n", (LPCSTR)file->name);
				}
			}

			firstdir = true;

			ListDir(file->name);
		}
	}

}

void ListBuilder::AddFile(const VString &filename, bool checkexlist)
{
#ifdef _DEBUG
	if (filename.GetLength() == 0)
	{
		printf("P_ERROR (ListBuilder::AddFile): No file name.\n");
		return;
	}
#endif

	VString *tmp = new VString(filename);

	if (tmp->CompareReverseLimitNoCase(".bsp", 4))
	{
		// add file extension
		*tmp += ".bsp";
	}

	if (checkexlist) // Process exceptions
	{
		for (int i = 0; i < exlist->GetCount(); i++)
		{
			file_s *tmpex = exlist->GetAt(i);
			if(!tmp->CompareReverseLimitNoCase((LPCSTR)tmpex->name, tmpex->name.GetLength()))
			{
				// make sure mapname is not longer.
				if (tmp->GetLength() <= tmpex->name.GetLength())
				{
					// they must be equal
					if (verbal)
					{
						printf ("Excluded \"%s\" from res file generation\n", (LPCSTR)*tmp);
					}
					delete tmp;
					return;
				}

				// check for folder char
				char prechar = (*tmp)[(tmp->GetLength() - tmpex->name.GetLength()) - 1];
				if (prechar == '\\' || prechar == '/')
				{
					// folder. They are equal
					if (verbal)
					{
						printf ("Excluded \"%s\" from res file generation\n", (LPCSTR)*tmp);
					}
					delete tmp;
					return;
				}
			}
		}
	}

	// file can be added to filelist
	filelist->AddTail(tmp);

	if (verbal && searchdisp)
	{
		printf("Added \"%s\" to the map list\n", (LPCSTR)*tmp);
	}
}

void ListBuilder::PrepExList()
{
	// Prepares Exceptionlist by adding .bsp to filenames that need it
	for (int i = 0; i < exlist->GetCount(); i++)
	{
		file_s *tmp = exlist->GetAt(i);

		if (tmp->name.CompareReverseLimitNoCase(".bsp", 4))
		{
			// add file extension
			tmp->name += ".bsp";
		}
	}
}

#ifndef WIN32
void ListBuilder::SetSymLink(bool slink)
{
	symlink = slink;
}
#endif


#ifdef WIN32
// Win 32 DIR parser
void ListBuilder::ListDir(const VString &path)
{
	WIN32_FIND_DATA filedata;

	// add *.* for searching all files.
	VString searchdir = path + "*.*";

	// find first file
	HANDLE filehandle = FindFirstFile(searchdir, &filedata);

	if (filehandle == INVALID_HANDLE_VALUE)
	{
		if (firstdir)
		{
			if (GetLastError() & ERROR_PATH_NOT_FOUND || GetLastError() & ERROR_FILE_NOT_FOUND)
			{
				printf("The directory you specified (%s) can not be found or is empty.\n", (LPCSTR)path);
			}
			else
			{
				printf("There was an error with the directory you specified (%s) - ERROR NO: %lu.\n", (LPCSTR)path, GetLastError());
			}
		}
		return;
	}

	firstdir = false;

	do
	{
		VString file = path + filedata.cFileName;

		// Check for directory
		if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Should we process the directory?
			if (recursive)
			{
				// Look for files in subdir, but ignore . and ..
				if (strcmp(filedata.cFileName, ".") && strcmp(filedata.cFileName, ".."))
				{
					ListDir(file + "\\");
				}
			}
		}
		else
		{
			// Check if the file is a .bsp
			if (!file.CompareReverseLimitNoCase(".bsp", 4))
			{
				AddFile(file, true);
			}
		}

	} while (FindNextFile(filehandle, &filedata));

	// Close search
	FindClose(filehandle);
}

#else
// Linux dir parser
void ListBuilder::ListDir(const VString &path)
{
	struct stat filestatinfo; // Force as a struct for GCC

	// Open the current dir
	DIR *directory = opendir(path);
	// Is it open?
	if (directory == NULL)
	{
		// dir cannot be opened
		if (firstdir)
		{
			printf("There was an error with the directory you specified (%s)\nDid you enter the correct directory?\n", (LPCSTR)path);
		}
		return;
	}

	firstdir = false;

	// Start going through dirs finding files.
	while (true)
	{
		const dirent * const direntry = readdir(directory);
		if(direntry == NULL)
		{
			break;
		}

		// Do we have a dir?
		VString file = path + direntry->d_name;

		int i;

		if (symlink)
		{
			i = stat(file, &filestatinfo); // Get the info about the files the links point to
		}
		else
		{
			i = lstat(file, &filestatinfo); // Get the info about the links
		}

		if (i == 0)
		{
			if (!S_ISLNK(filestatinfo.st_mode) || symlink)
			{
				// Check for directory
				if (S_ISDIR(filestatinfo.st_mode))
				{
					// Should we process the directory?
					if (recursive)
					{
						// Look for files in subdir, but ignore . and ..
						if (strcmp(direntry->d_name, ".") && strcmp(direntry->d_name, ".."))
						{
							ListDir(file + "/");
						}
					}
				}
				else
				{
					// Check if the file is a .bsp
					if (!file.CompareReverseLimitNoCase(".bsp", 4))
					{
						AddFile(file, true);
					}
				}
			}
		}
	}

	// close the dir
	closedir(directory);

}

#endif
