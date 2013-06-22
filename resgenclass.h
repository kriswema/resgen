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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

int RESGen_CompareVStringsFromList(void *a, void *b); // For sorting of lists
void RESGen_DeleteVString(void *a); // For sorting and searching of lists

class RESGen  
{
public:
	bool LoadExludeFile(VString &listfile);
	bool LoadRfaFile(VString &pakfilename);
	void ClearTextures();
	void ClearExcludes();
	void ClearResources();
	void ClearResfile();
	void BuildResourceList(VString &respath, bool checkpak, bool fdisp, bool rdisp);
	int MakeRES(VString &map, int fileindex, int filecount);
	void SetParams(bool beverbal, bool statline, bool overwrt, bool lcase, bool mcase, bool prsresource, bool cdisp);
	RESGen();
	virtual ~RESGen();

private:
	bool CheckModelExtTexture(VString &model);
	VString valveresourcepath;
	VString resourcepath;
	bool CheckWadUse(VString &wadfile);
	LinkedList texturelist;
	bool checkforexcludes;
	LinkedList excludelist;
	bool resourcedisp;
	void BuildPakResourceList(const VString &pakfile);
	bool pakparse;
	bool searchdisp;
	bool contentdisp;
	bool firstdir;
	void ListDir(const VString &path, const VString &filepath, bool reporterror);
	char * strtok_nexttoken;
	char * StrTok(char *string, char delimiter);
	int statcount; // statusbar counter
	bool WriteRes(VString &folder, VString &mapname);
	void AddWad(VString &wadlist, int start, int len);
	LinkedList resfile;
	void AddRes(VString res, char *prefix = NULL, char *suffix = NULL);
	void BStoS(char *string);
	char * NextValue();
	char * NextToken();
	char * LoadBSPData(VString &file, int *entdatalen = NULL, LinkedList *texlist = NULL);
	bool checkforresources;
	LinkedList resources;
	bool verbal;
	bool statusline;
	bool overwrite;
	bool tolower;
	bool matchcase;
	bool parseresource;
	VString rfastring;

	typedef struct modelheader_s
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
	typedef struct wadheader_s
	{
		char identification[4]; // Should be WAD2 or WAD3
		int numlumps; // Number of lumps
		int infotableofs; // Offset of lump data
	};
	typedef struct wadlumpinfo_s
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
		int dirsize;
	};

	struct fileinfo_s
	{
		char name[56];
		int fileoffset;
		int filelen;
	};
	struct texdata_s
	{
		char name[16];
		unsigned width, height;
		unsigned offsets[4];
	};
	struct texheader_s
	{
		int texcount;
		int offsets[4];
	};
	struct lumpinfo_s
	{
		int fileofs;
		int filelen;
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
