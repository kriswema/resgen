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

// resgenclass.h: interface for the RESGen class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESGENCLASS_H__5EDE8CED_D2D4_4D20_846F_5A1034433CDD__INCLUDED_)
#define AFX_RESGENCLASS_H__5EDE8CED_D2D4_4D20_846F_5A1034433CDD__INCLUDED_

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "util.h"

std::vector<std::string>::iterator findStringNoCase(std::vector<std::string> &vec, const std::string &element);

class RESGen
{
public:
	typedef std::map<std::string, std::string> StringMap;

	bool LoadExludeFile(std::string &listfile);
	bool LoadRfaFile(std::string &pakfilename);
	int MakeRES(std::string &map, int fileindex, size_t filecount, const StringMap &resources, std::vector<std::string> &resourcePaths_);
	void SetParams(bool beverbal, bool statline, bool overwrt, bool lcase, bool mcase, bool prsresource, bool preservewads, bool cdisp);
	RESGen();
	virtual ~RESGen();

private:
	typedef std::set<std::string> TextureSet;
	typedef std::map<std::string, TextureSet> WadCache;

	bool CheckModelExtTexture(const std::string &model);
	bool CacheWad(const std::string &wadfile);
	bool CheckWadUse(const StringMap::const_iterator &wadfileIt);
	bool WriteRes(const std::string &folder, const std::string &mapname);
	void AddWad(const std::string &wadlist, size_t start, size_t len);
	void AddRes(std::string res, const char * const prefix = NULL, const char * const suffix = NULL);
	bool LoadBSPData(const std::string &file, std::string &entdata, StringMap & texlist);
	bool OpenFirstValidPath(File &outFile, std::string fileName, const char* const mode);

private:
	bool checkforexcludes;
	bool resourcedisp;
	bool contentdisp;
	int statcount; // statusbar counter
	StringMap resfile;
	StringMap texturelist;
	StringMap excludelist;
	WadCache wadcache;
	bool verbal;
	bool statusline;
	bool overwrite;
	bool tolower;
	bool matchcase;
	bool parseresource;
	bool preservewads;
	std::string rfastring;
	std::vector<std::string> resourcePaths;
};

#endif // !defined(AFX_RESGENCLASS_H__5EDE8CED_D2D4_4D20_846F_5A1034433CDD__INCLUDED_)
