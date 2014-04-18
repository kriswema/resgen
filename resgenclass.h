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
#include <memory>
#include <string>
#include <vector>

#include "LinkedList.h"

std::vector<std::string>::iterator findStringNoCase(std::vector<std::string> &vec, const std::string &element);

class RESGen
{
public:
	bool LoadExludeFile(std::string &listfile);
	bool LoadRfaFile(std::string &pakfilename);
	void BuildResourceList(std::string &respath, bool checkpak, bool fdisp, bool rdisp);
	int MakeRES(std::string &map, int fileindex, size_t filecount);
	void SetParams(bool beverbal, bool statline, bool overwrt, bool lcase, bool mcase, bool prsresource, bool preservewads, bool cdisp);
	RESGen();
	virtual ~RESGen();

private:
	bool CheckModelExtTexture(const std::string &model);
	bool CheckWadUse(const std::string &wadfile);
	void BuildPakResourceList(const std::string &pakfile);
	void ListDir(const std::string &path, const std::string &filepath, bool reporterror);
	char * StrTok(char *string, char delimiter);
	bool WriteRes(const std::string &folder, const std::string &mapname);
	void AddWad(const std::string &wadlist, size_t start, size_t len);
	void AddRes(std::string res, const char * const prefix = NULL, const char * const suffix = NULL);
	char * NextValue();
	char * NextToken();
	std::unique_ptr<char[]> LoadBSPData(const std::string &file, size_t & entdatalen, std::vector<std::string> & texlist);

	std::string valveresourcepath;
	std::string resourcepath;
	bool checkforresources;
	bool checkforexcludes;
	bool resourcedisp;
	bool pakparse;
	bool searchdisp;
	bool contentdisp;
	bool firstdir;
	char * strtok_nexttoken;
	int statcount; // statusbar counter
	std::vector<std::string> resources;
	std::vector<std::string> resfile;
	std::vector<std::string> texturelist;
	std::vector<std::string> excludelist;
	bool verbal;
	bool statusline;
	bool overwrite;
	bool tolower;
	bool matchcase;
	bool parseresource;
	bool preservewads;
	std::string rfastring;

	struct modelheader_s
	{
		char id[4]; // Orriginal header is int, but this is easier
		int version;

		char name[64];
		int length;

		float eyeposition[3];
		float min[3];
		float max[3];

		float bbmin[3];
		float bbmax[3];

		int flags;

		int numbones;
		int boneindex;

		int numbonecontrollers;
		int bonecontrollerindex;

		int numhitboxes;
		int hitboxindex;

		int numseq;
		int seqindex;

		int numseqgroups;
		int seqgroupindex;

		int numtextures; // Number of textures
		int textureindex; // Index of texture location - 0 means no textures present
		int texturedataindex;

		// incomplete - cut off for space saving
	};
	struct wadheader_s
	{
		char identification[4]; // Should be WAD2 or WAD3
		int numlumps; // Number of lumps
		int infotableofs; // Offset of lump data
	};
	struct wadlumpinfo_s
	{
		int filepos;
		int disksize;
		int size;
		char type;
		char compression;
		char pad1, pad2;
		char name[16]; // texture name, null terminated
	};
	struct pakheader_s
	{
		int pakid;
		int diroffset;
		size_t dirsize;
	};

	struct fileinfo_s
	{
		char name[56];
		int fileoffset;
		size_t filelen;
	};
	struct texdata_s
	{
		char name[16];
		unsigned width, height;
		unsigned offsets[4];
	};
	struct lumpinfo_s
	{
		int fileofs;
		size_t filelen;
	};
	struct bsp_header
	{
		int version;
		lumpinfo_s ent_header;
		lumpinfo_s dnt_care01; // don't care
		lumpinfo_s tex_header;
		// incomplete - cut off for space saving
	};
};

#endif // !defined(AFX_RESGENCLASS_H__5EDE8CED_D2D4_4D20_846F_5A1034433CDD__INCLUDED_)
