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

#ifndef RESOURCELISTBUILDER_H
#define RESOURCELISTBUILDER_H

#include <map>
#include <string>

#include "util.h"

class ResourceListBuilder
{
public:
	typedef std::map<std::string, std::string> StringMap;

	ResourceListBuilder(const config_s &config);
	void BuildResourceList(const std::vector<std::string> &paths, bool checkpak, bool rdisp);

private:
	#ifdef _WIN32
	// Win 32 DIR parser
	void ListDir(const std::string &path, const std::string &filepath, bool reporterror);
	#else
	// Linux dir parser
	void ListDir(const std::string &path, const std::string &filepath, bool reporterror);
	#endif

	void BuildPakResourceList(const std::string &pakfilename);

	bool resourcedisp;
	bool pakparse;
	bool firstdir;

	bool verbal;

// TODO: Make private
public:
	StringMap resources;
};

#endif
