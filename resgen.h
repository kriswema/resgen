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

#ifdef _DEBUG
#define VERSION "2.0.2 DBG"
#else
#define VERSION "2.0.2"
#endif

struct config_s
{
	bool help; // f
	bool credits ; // f
	bool warranty; // f

	bool verbal; // t -
	bool statusline; // f
	bool searchdisp; // f
	bool resourcedisp; // f
	bool contentdisp; // f

	LinkedList files;
	LinkedList excludes; // Map exclude list - not resource!
	LinkedList excludelists; // Exclude resource list files - not maps!

	bool overwrite; // f
	VString rfafile;

	bool tolower; // f
	bool matchcase; // f
	bool checkpak; // t
	bool parseresource; // f
	VString resource_path;

#ifdef WIN32
	bool keypress; // t
#else
	bool symlink; // t
#endif
};

int strrnicmp(const char *src, const char *dst, int limit);
void getexitkey(bool verbal, bool keypress);

void showcopyright();
void showhelp();
void showwarranty();
void showcredits();

int main(int argc, char* argv[]);
