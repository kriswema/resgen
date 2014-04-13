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

// resgenclass.cpp: implementation of the RESGen class.
//
//////////////////////////////////////////////////////////////////////

// RESGen does it's own security checks, no need to add VS2005's layer
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "resgenclass.h"
#include "resgen.h"
#include "util.h"

// Half-Life BSP version
#define BSPVERSION 30

// Update status bar every x parsings.
// The higher the number, the less excess statbar writing, but also makes the statbar 'lag'
#define STAT_MAX 15

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RESGen::RESGen()
{
	checkforresources = false;
	checkforexcludes = false;
}

RESGen::~RESGen()
{
	// Clean up
	ClearResfile();
	ClearResources();
	ClearExcludes();
}

void RESGen::SetParams(bool beverbal, bool statline, bool overwrt, bool lcase, bool mcase, bool prsresource, bool preservewads_, bool cdisp)
{
	verbal = beverbal;
	statusline = statline;
	overwrite = overwrt;
	tolower = lcase;
	matchcase = mcase;
	parseresource = prsresource;
	preservewads = preservewads_;
	contentdisp = cdisp;
}

int RESGen::MakeRES(VString &map, int fileindex, int filecount)
{
	#ifdef WIN32
	WIN32_FIND_DATA filedata;
	HANDLE filehandle;
	#else
	glob_t globbuf;
	#endif

	// Create basefilename
	int i = map.StrRChr('/'); // Linux style path
	if (i == -1)
	{
		i = map.StrRChr('\\'); // windows style path
	}

	VString basefolder; // folder, including trailing /
	if (i == -1)
	{
		#ifdef WIN32
		basefolder = ".\\";
		#else
		basefolder = "./";
		#endif
	}
	else
	{
		basefolder = map.Left(i + 1);
	}
	VString basefilename = map.Mid(i + 1, map.GetLength() - i - 5);

	if (verbal)
	{
		printf("Creating .res file %s [%d/%d].\n", (LPCSTR)(basefolder + basefilename + ".res"), fileindex, filecount);
	}


	// Check if resfile doesn't already exist
	bool fileexists = false;
	#ifdef WIN32
	filehandle = FindFirstFile(basefolder + basefilename + ".res", &filedata);
	if (filehandle != INVALID_HANDLE_VALUE)
	{
		FindClose(filehandle);
		fileexists = true;
	}
	#else
	if (!glob((LPCSTR)(basefolder + basefilename + ".res"), GLOB_TILDE, NULL, &globbuf))
	{
		globfree(&globbuf);
		fileexists = true;
	}
	#endif

	if (!overwrite && fileexists)
	{
		// File found, but we don't want to overwrite.
		printf("%s already exists. Skipping file.\n", (LPCSTR)(basefolder + basefilename + ".res"));
		return 1;
	}

	// Clear the resfile list to be sure (SHOULD be empty)
	ClearResfile();

	// Clear the texture list to be sure (SHOULD be empty)
	ClearTextures();

	// first, get the enity data
	int entdatalen; // Length of entity data
	char *entdata = LoadBSPData(map, &entdatalen, &texturelist);
	if (entdata == NULL)
	{
		// error. return
		return 1;
	}

	// get mapinfo
	char *mistart;

	// Look for first entity block
	if ((mistart = strstr(entdata, "{")) == NULL)
	{
		// Something wrong with bsp entity data
		printf("Error parsing \"%s\". Entity data not in recognized text format.\n", (LPCSTR)map);
		delete [] entdata; // Clean up entiy data!
		return 1;
	}

	// Look for first block end (we do not need to parse the ather blocks vor skyname or wad info)
	size_t milen = ((size_t)strstr(mistart+1, "}") - (size_t)mistart) + 1;

	char *mapinfo = new char [milen + 1];
	memcpy(mapinfo, mistart, milen);
	mapinfo[milen] = 0; // terminating NULL

	// parse map info. We use StrTok for this...
	char *token = StrTok(mapinfo, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". No map information found.\n", (LPCSTR)map);
			delete [] mapinfo; // clean up mapinfo
			delete [] entdata; // Clean up entiy data!
			return 1;
	}

	token = StrTok(NULL, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". Entity data is corrupt.\n", (LPCSTR)map);
			delete [] mapinfo; // clean up mapinfo
			delete [] entdata; // Clean up entiy data!
			return 1;
	}

	while (token)
	{
		if (!strcmp(token, "wad"))
		{
			// wad file
			token = NextValue();
			if (!token)
			{
				printf("Error parsing \"%s\" for WADs. Entity data is corrupt.\n", (LPCSTR)map);
				delete [] mapinfo; // clean up mapinfo
				delete [] entdata; // Clean up entiy data!
				return 1;
			}
			// token has value

			VString value = token;
			if (value.GetLength() != 0) // Don't try to parse an empty listing
			{
				// seperate the WAD files and save
				i = 0;
				int seppos;

				while ((seppos = value.StrChr(';', i)) >= 0)
				{
					AddWad(value, i, seppos - i); // Add wad to reslist
					i = seppos + 1;
				}

				// There might be a wad file left in the list, check for it
				if (i < value.GetLength())
				{
					// it should be equal, there is a wadfile left!
					AddWad(value, i, value.GetLength() - i);
				}
			}

		}
		else if (!strcmp(token, "skyname"))
		{
			// wad file
			token = NextValue();
			if (!token)
			{
				printf("Error parsing \"%s\" skies. Entity data is corrupt.\n", (LPCSTR)map);
				delete [] mapinfo; // clean up mapinfo
				delete [] entdata; // Clean up entiy data!
				return 1;
			}
			// token has value

			VString value = token;

			// Add al 6 sky textures here
			AddRes(value, "gfx/env/", "up.tga");
			AddRes(value, "gfx/env/", "dn.tga");
			AddRes(value, "gfx/env/", "lf.tga");
			AddRes(value, "gfx/env/", "rt.tga");
			AddRes(value, "gfx/env/", "ft.tga");
			AddRes(value, "gfx/env/", "bk.tga");
		}
		else
		{
			// go to value
			token = NextValue();
			if (!token)
			{
				printf("Error parsing \"%s\" for map data. Entity data is corrupt.\n", (LPCSTR)map);
				delete [] mapinfo; // clean up mapinfo
				delete [] entdata; // Clean up entiy data!
				return 1;
			}
		}

		// Go to next key, if available
		token = NextToken(); // exit value
		if (!token)
		{
			printf("Error parsing \"%s\". Keys/values not alligned.\n", (LPCSTR)map);
			delete [] mapinfo; // clean up mapinfo
			delete [] entdata; // Clean up entiy data!
			return 1;
		}
		// token is 'empty'

		token = NextToken(); // try next key

		// No statbar - this is way too fast for it to even show up.
	}

	delete [] mapinfo; // clean up mapinfo


	statcount = STAT_MAX; // make statbar print at once


	// parse the entity data. We use StrTok for this...
	// Note that we reparse the mapinfo.
	token = StrTok(entdata, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". No initial key.\n", (LPCSTR)map);
			delete [] entdata; // Clean up entity data!
			return 1;
	}

	// Do again, to get to first key.
	token = StrTok(NULL, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". First key not found.\n", (LPCSTR)map);
			delete [] entdata; // Clean up entity data!
			return 1;
	}

	while (token)
	{
		// Move from key to value
		token = NextValue();
		if (!token)
		{
			printf("\rError parsing \"%s\". Key to value transition failed.\n", (LPCSTR)map);
			delete [] entdata; // Clean up entity data!
			return 1;
		}

		VString value = token; // asign token to a VString

		if (!value.CompareReverseLimitNoCase(".mdl", 4))
		{
			// mdl file
			AddRes(value);
		}
		else if (!value.CompareReverseLimitNoCase(".wav", 4))
		{
			// wave file
			AddRes(value, "sound/");
		}
		else if (!value.CompareReverseLimitNoCase(".spr", 4))
		{
			// sprite file
			AddRes(value);
		}
		else if (!value.CompareReverseLimitNoCase(".bmp", 4))
		{
			// bitmap file
			AddRes(value);
		}
		else if (!value.CompareReverseLimitNoCase(".tga", 4))
		{
			// targa file
			AddRes(value);
		}

		token = NextToken(); // exit value
		if (!token)
		{
			printf("\rError parsing \"%s\". Could not move on to next key.\n", (LPCSTR)map);
			delete [] entdata; // Clean up entiy data!
			return 1;
		}
		// token is 'empty'

		// update statbar
		if (statusline && statcount == STAT_MAX)
		{
			// Reset the statcount
			statcount = 0;

			// Calculate the percentage completed of the current file.
			int percentage = (((token-entdata)+1) * 101) / entdatalen; // Make the length one too long.
			if (percentage > 100)
			{
				 // Make sure we don;t go over 100%
				percentage = 100;
			}
			printf("\r(%d%%) [%d/%d]", percentage, fileindex, filecount);
		}
		else
		{
			statcount++;
		}


		token = StrTok(NULL, '\"'); // try next key
	}

	delete [] entdata; // clean up bsp entity data

	if (statusline)
	{
		// erase statusline
		printf("\r%-21s\r", ""); // easier to adjust length this way
	}

	// Try to find info txt and overview data
	#ifdef WIN32
	filehandle = FindFirstFile(basefolder + "..\\overviews\\" + basefilename + ".txt", &filedata); // try to find txt file for overview
	if (filehandle != INVALID_HANDLE_VALUE)
	{
		FindClose(filehandle);

		// file found, but we need the tga or bmp too
		filehandle = FindFirstFile(basefolder + "..\\overviews\\" + basefilename + ".tga", &filedata); // try to find tga file for overview
		if (filehandle != INVALID_HANDLE_VALUE)
		{
			FindClose(filehandle);

			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".tga");
			AddRes(basefilename, "overviews/", ".txt");
		}
		else
		{
			filehandle = FindFirstFile(basefolder + "..\\overviews\\" + basefilename + ".bmp", &filedata); // try to find bmp file for overview
			if (filehandle != INVALID_HANDLE_VALUE)
			{
				FindClose(filehandle);

				// txt found too, add both files to res list
				AddRes(basefilename, "overviews/", ".bmp");
				AddRes(basefilename, "overviews/", ".txt");
			}
		}
	}
	#else
	// We use glob to find the file in Linux.
	// Please note that params are NOT expanded (tilde will be).
	// So it might not work properly in cases where params are used
	// The glob man page tell us to use wordexp for expansion.. However, that function does not exist.
	if (!glob((LPCSTR)(basefolder + "../overviews/" + basefilename + ".txt"), GLOB_TILDE, NULL, &globbuf))
	{
		globfree(&globbuf);

		// file found, but we need the tga or bmp too
		if (!glob((LPCSTR)(basefolder + "../overviews/" + basefilename + ".tga"), GLOB_TILDE, NULL, &globbuf))
		{
			globfree(&globbuf);

			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".tga");
			AddRes(basefilename, "overviews/", ".txt");
		}
		else if (!glob((LPCSTR)(basefolder + "../overviews/" + basefilename + ".bmp"), GLOB_TILDE, NULL, &globbuf))
		{
			globfree(&globbuf);

			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".bmp");
			AddRes(basefilename, "overviews/", ".txt");
		}
	}
	#endif

	// Resource list has been made.
	int status = 0; // RES status, 0 means ok, 2 means missing resource

	// Check for resources on disk
	if (checkforresources)
	{
		//printf("\nStarting resource check:\n");
		for (i = 0; i < resfile.GetCount(); i++)
		{
			VString *tempres = resfile.GetAt(i);
			//printf("%s\n", (LPCSTR)*tempres);
			int resfileindex = resources.Find(tempres, RESGen_CompareVStringsFromList);
			if(resfileindex < 0)
			{
				// file not found - maybe it's excluded?
				resfileindex = excludelist.Find(tempres, RESGen_CompareVStringsFromList);
				if(resfileindex >= 0)
				{
					// file found - it's an exclude
					if (contentdisp)
					{
						printf("Resource is excluded: %s\n", (LPCSTR)*tempres);
					}

				}
				else if (tempres->CompareReverseLimitNoCase(".wad", 4))
				{
					// not a wad file
					if (verbal)
					{
						printf("Resource file not found: %s\n", (LPCSTR)*tempres);
					}
					status = 2; // res file might not be complete
				}
				else
				{
					// wad file is not critical, so no status change
					if (contentdisp)
					{
						printf("Resource file not found: %s\n", (LPCSTR)*tempres);
					}
				}

				// remove file from list
				resfile.RemoveAt(i);
				delete tempres;
				i = i - 1; // Next has become current. Don't skip it!
			}
			else
			{
				if (matchcase)
				{
					// match case
					*tempres = *resources.GetAt(resfileindex);
				}

				if (parseresource)
				{
					if (!tempres->CompareReverseLimitNoCase(".wad", 4))
					{
						// Check if wad file is used
						if (!CheckWadUse(*resources.GetAt(resfileindex))) // We MUST have the right file
						{
							// Wad is NOT being used
							if (contentdisp)
							{
								printf("WAD file not used: %s\n", (LPCSTR)*tempres);
							}

							if(!preservewads)
							{
								// remove file from list
								resfile.RemoveAt(i);
								delete tempres;
								i = i - 1; // Next has become current. Don't skip it!
							}
						}
					}
					else if (!tempres->CompareReverseLimitNoCase(".mdl", 4))
					{
						// Check model for external texture
						if (CheckModelExtTexture(*resources.GetAt(resfileindex)))
						{
							// Uses external texture, add
							VString *extmdltex = new VString(tempres->Left(tempres->GetLength() - 4)); // strip extention
							*extmdltex += "T.mdl"; // add T and extention

							// We can get away with this, since the model texture will be places AFTER the model
							if (!resfile.InsertSorted(extmdltex, RESGen_CompareVStringsFromList, false))
							{
								// Hmmm, weird, already in res file.. Oh well, can't complain!
								delete extmdltex;
							}
							else
							{
								if (contentdisp)
								{
									printf("MDL texture file added: %s\n", (LPCSTR)*extmdltex);
								}
							}

						}
					}

				}
			}
		}
	}

	// Check if resource has to be excluded
	if (checkforexcludes)
	{
		//printf("\nStarting exclude check:\n");
		for (i = 0; i < resfile.GetCount(); i++)
		{
			VString *tempres = resfile.GetAt(i);
			//printf("%s\n", (LPCSTR)*tempres);
			int resfileindex = excludelist.Find(tempres, RESGen_CompareVStringsFromList);
			if(resfileindex >= 0)
			{
				// file found
				if (contentdisp)
				{
					printf("Resource is excluded: %s\n", (LPCSTR)*tempres);
				}

				// remove file from list
				resfile.RemoveAt(i);
				delete tempres;
				i = i - 1; // Next has become current. Don't skip it!
			}
		}
	}

	// Give a list of missing textures
	if (parseresource && checkforresources && verbal)
	{
		if (texturelist.GetCount() > 0)
		{
			status = 2; // res file might not be complete
			for (i = 0; i < texturelist.GetCount(); i++)
			{
				printf("Texture not found in wad files: %s\n", (LPCSTR)*texturelist.GetAt(i));
			}
		}
	}

	if (resfile.GetCount() == 0 && rfastring.GetLength() == 0)
	{
		// no resources!
		if (verbal) { printf("No resources were found for \"%s.res\".", (LPCSTR)basefilename); }

		if (fileexists)
		{
			// File exists, delete it.
			// WHAT? No check for overwrite? No!
			// Think of it, if the file exists we MUST be in overwrite mode to even get to this point!
			remove(basefolder + basefilename + ".res");
			if (verbal)
			{
				printf(" Deleting existing res file.\n");
			}
		}
		else
		{
			// File doesn't exist, so we don't have to delete it.
			if (verbal)
			{
				printf(" Skipping file.\n");
			}
		}
		return status;
	}

	// Collecting resfile entries is done, now write the res file.
	if (!WriteRes(basefolder, basefilename))
	{
		return 1;
	}

	// File written successfully. We can safely erase the resfile and texture list
	ClearResfile();
	ClearTextures();

	return status;
}

void RESGen::BuildResourceList(VString &respath, bool checkpak, bool sdisp, bool rdisp)
{
	VString valvepath;

	searchdisp = sdisp;
	resourcedisp = rdisp;
	pakparse = checkpak;

	ClearResources(); // clear the current list first

	if (respath.GetLength() == 0)
	{
		// no respath, thus no reslist
		checkforresources = false;
		return;
	}

	checkforresources = true;

	// Check the respath and check ../valve if the respath doesn't point to valve

	// prepare folder name
	#ifdef WIN32
	if (respath[respath.GetLength() - 1] != '\\')
	{
		// No ending "\", add
		respath += "\\";
	}
	if (respath.CompareReverseLimitNoCase("\\valve\\", 7))
	{
		// NOT valve dir, so check it too
		int slashpos = respath.StrRChr('\\', respath.GetLength()-2);
		if (slashpos >= 0)
		{
			valvepath = respath.Left(slashpos);
			valvepath += "\\valve\\";
		}
		else
		{
			valvepath = respath + "..\\valve\\";
		}
	}
	#else
	if (respath[respath.GetLength() - 1] != '/')
	{
		// No ending "/", add
		respath += "/";
	}
	if (respath.CompareReverseLimitNoCase("/valve/", 7))
	{
		// NOT valve dir, so check it too
		int slashpos = respath.StrRChr('/', respath.GetLength()-2);
		if (slashpos >= 0)
		{
			valvepath = respath.Left(slashpos);
			valvepath += "/valve/";
		}
		else
		{
			valvepath = respath + "../valve/";
		}
	}
	#endif

	resourcepath = respath;
	valveresourcepath = valvepath;

	if (resourcedisp)
	{
		printf("Searching %s for resources:\n", (LPCSTR)respath);
	}
	else if (verbal)
	{
		printf("Searching %s for resources...\n", (LPCSTR)respath);
	}

	firstdir = true;
	VString filepath = "";
	ListDir(respath, filepath, true);

	// Check the valve dir too
	if (valvepath.GetLength() > 0)
	{
		firstdir = true;
		filepath = "";
		ListDir(valvepath, filepath, false);
	}

	printf("\n");
}

char * RESGen::LoadBSPData(const VString &file, int * const entdatalen, LinkedList<VString *> * const texlist)
{
	// first open the file.
	File bsp((LPCSTR)file, "rb"); // read in binary mode.

	if (bsp == NULL)
	{
		printf("Error opening \"%s\"\n", (LPCSTR)file);
		return NULL;
	}

	// file open.. read header
	bsp_header header;

	if (fread(&header, sizeof(bsp_header), 1, bsp) != 1)
	{
		// header NOT read properly!
		printf("Error opening \"%s\". Corrupt BSP file.\n", (LPCSTR)file);
		return NULL;
	}

	if (header.version != BSPVERSION)
	{
		printf("Error opening \"%s\". Incorrect BSP version.\n", (LPCSTR)file);
		return NULL;
	}

	if (header.ent_header.fileofs <= 0 || header.ent_header.filelen <= 0)
	{
		// File corrupted
		printf("Error opening \"%s\". Corrupt BSP header.\n", (LPCSTR)file);
		return NULL;
	}

	// read entity data
	char *entdata = new char [header.ent_header.filelen + 1];

	fseek(bsp, header.ent_header.fileofs, SEEK_SET);
	if ((int)fread(entdata, header.ent_header.filelen, 1, bsp) != 1)
	{
		// not the right ammount of data was read
		delete [] entdata;
		printf("Error opening \"%s\". BSP file corrupt.\n", (LPCSTR)file);
		return NULL;
	}

	if (parseresource && checkforresources)
	{
		// Load names of external textures
		fseek(bsp, header.tex_header.fileofs, SEEK_SET); // go to start of texture data
		int texcount;

		if (fread(&texcount, sizeof(int), 1, bsp) != 1) // first we want to know the number of files.
		{
			// header NOT read properly!
			delete [] entdata;
			printf("Error opening \"%s\". Corrupt texture header.\n", (LPCSTR)file);
			return NULL;
		}

		if (texcount < 0)
		{
			// File corrupted
			delete [] entdata;
			printf("Error opening \"%s\". Corrupt BSP textures.\n", (LPCSTR)file);
			return NULL;
		}

		if (texcount > 0)
		{
			// Textures available, read all offsets
			char *offsets = new char [sizeof(int) * texcount];

			int i = fread(offsets, sizeof(int), texcount, bsp);

			if (i != texcount) // load texture offsets
			{
				// header NOT read properly!
				printf("Error opening \"%s\". Corrupt texture data.\n  read: %d, expect: %d\n", (LPCSTR)file, i, texcount);
				delete [] entdata;
				delete [] offsets;
				return NULL;
			}

			for (i = 0; i < texcount; i++)
			{
				// go to texture location
				fseek(bsp, header.tex_header.fileofs + offsets[i], SEEK_SET);

				// read texture data
				texdata_s texdata;
				if (fread(&texdata, sizeof(texdata_s), 1, bsp) != 1)
				{
					// header NOT read properly!
					delete [] entdata;
					delete [] offsets;
					printf("Error opening \"%s\". Corrupt BSP file.\n", (LPCSTR)file);
					return NULL;
				}

				// is this a wad based texture?
				if (texdata.offsets[0] == 0 && texdata.offsets[1] == 0 && texdata.offsets[2] == 0 && texdata.offsets[3] == 0)
				{
					// No texture for any mip level, so must be in a wad
					VString *texfile = new VString(texdata.name);
					if (!texlist->InsertSorted(texfile, RESGen_CompareVStringsFromList, false))
					{
						// double detected
						delete texfile;
					}
				}
			}

			delete [] offsets; // done!
		}

		/*
		printf("\n\nTextures found:\n");

		for (int i = 0; i < texlist->GetCount(); i++)
		{
			printf("%s\n", (LPCSTR)(*texlist->GetAt(i)));
		}
		printf("\n\n");
		//*/
	}

	// add terminating NULL for entity data
	entdata[header.ent_header.filelen] = 0;

	if (entdatalen)
	{
		*entdatalen = header.ent_header.filelen;
	}

	#ifdef _DEBUG
	// Debug write entity data to file
	File tmp(file + "_ent.txt", "w");
	fprintf(tmp, "%s", entdata);
	#endif

	return entdata;
}

char * RESGen::NextToken()
{
	char *token = StrTok(NULL, '\"');

	return token;
}

char * RESGen::NextValue()
{
	// Goes to the next entity token (key->value or value->key)
	char *token = StrTok(NULL, '\"'); // exit key/value
	if (!token)
	{
		return NULL;
	}
	token = StrTok(NULL, '\"'); // enter key/value

	return token;
}

void RESGen::AddRes(VString res, const char * const prefix, const char * const suffix)
{
	// Sometimes res entries start with a non alphanumeric character. Strip it.
	while (!isalnum(res[0])) // keep stripping until a valid char is found
	{
		// Remove character
		res = res.Right(res.GetLength() - 1); // Kinda slow, but usually only happens once.
	}

	res.StrRplChr('\\', '/'); // replace backslashes

	// Add prefix and suffix
	if (prefix)
	{
		res = prefix + res; // kinda slow...
	}
	if (suffix)
	{
		res += suffix;
	}

	if (tolower)
	{
		// Convert name to lowercase
		res.MakeLower();
	}

	// Add file to list if it isn't in it yet.

	// File not found, must be new. Add to list
	VString *tempres = new VString(res);
	if (!resfile.InsertSorted(tempres, RESGen_CompareVStringsFromList, false))
	{
		// double file found, discard
		delete tempres;
		return;
	}

	// Report file found
	if (contentdisp)
	{
		printf("\r%-21s\n", (LPCSTR)res); // With 21 chars, there is support for up to 999999 bsp's to parse untill the statbar might remain in screen
	}

	statcount = STAT_MAX; // Make statbar print on next update

	return;
}

void RESGen::AddWad(const VString &wadlist, int start, int len)
{
	VString wadfile;

	wadfile = wadlist.Mid(start, len);

	wadfile.StrRplChr('\\', '/'); // replace backslashes

	// strip folders
	wadfile = wadfile.Right(len - wadfile.StrRChr('/') - 1); // We can get away with this because on not found, StrRChr returns -1. The right function is optimized for when the requested length equals the string length

	// Add file to reslist
	AddRes(wadfile);
}

bool RESGen::WriteRes(const VString &folder, const VString &mapname)
{
	// This function writes a standard res file.

	// Open the file
	File f(folder + mapname + ".res", "w");

	if (f == NULL)
	{
		printf("Failed to open %s for writing.\n", (LPCSTR)(folder + mapname + ".res"));
		return false;
	}

	// Header
	fprintf(f, "// %s - created with RESGen v%s.\n", (LPCSTR)(mapname + ".res"), VERSION);
	fprintf(f, "// RESGen is made by Jeroen \"ShadowLord\" Bogers,\n");
	fprintf(f, "// with serveral improvements and additions by Zero3Cool.\n");
	fprintf(f, "// For more info go to http://resgen.hltools.com\n");

	fprintf(f, "\n// .res entries (%d):\n", resfile.GetCount());

	// Resources
	for (int i = 0; i < resfile.GetCount(); i++)
	{
		VString *vtemp = resfile.GetAt(i);
		fprintf(f, "%s\n", (LPCSTR)*vtemp);
	}

	// RFA file, if needed
	if (rfastring.GetLength() > 0)
	{
		fprintf(f, "\n// Added .res content:\n%s\n", (LPCSTR)rfastring);
	}

	return true;
}

void RESGen::ClearResfile()
{
	// RESGen calls this function internally with MakeRes, so normally there is no need to run this.
	// You might want to use it or if you are running a lot of code after creating the resfile.

	while (resfile.GetCount() > 0)
	{
		// remove from list AND clean up memory from VString
		VString *temp = resfile.GetAt(0);
		resfile.RemoveAt(0);
		delete temp;
	}
}

void RESGen::ClearTextures()
{
	// RESGen calls this function internally with MakeRes, so normally there is no need to run this.
	// You might want to use it or if you are running a lot of code after creating the resfile.

	while (texturelist.GetCount() > 0)
	{
		// remove from list AND clean up memory from VString
		VString *temp = texturelist.GetAt(0);
		texturelist.RemoveAt(0);
		delete temp;
	}
}

void RESGen::ClearResources()
{
	// RESGen calls this function internally when it is destructed.
	// You might want to use it if you want to clear the resourcelist if you don't want to use resourcechecking anymore.
	checkforresources = false;

	while (resources.GetCount() > 0)
	{
		// remove from list AND clean up memory from VString
		VString *temp = resources.GetAt(0);
		resources.RemoveAt(0);
		delete temp;
	}
}

void RESGen::ClearExcludes()
{
	// RESGen calls this function internally when it is destructed.
	// You might want to use it if you want to clear the excludelist if you don't want to use exludes anymore.
	checkforexcludes = false;

	while (excludelist.GetCount() > 0)
	{
		// remove from list AND clean up memory from VString
		VString *temp = excludelist.GetAt(0);
		excludelist.RemoveAt(0);
		delete temp;
	}
}

bool RESGen::LoadRfaFile(VString &filename)
{
	if (filename.GetLength() == 0)
	{
		// no rfa file, ignore
		return true;
	}

	// Add .rfa extention if needed
	if (filename.CompareReverseLimitNoCase(".rfa", 4))
	{
		// not found, add
		filename += ".rfa";
	}

	if(!rfastring.LoadFromFile(filename))
	{
		printf("Error reading rfa file: \"%s\"\n", (LPCSTR)filename);
		return false;
	}

	return true;
}

char * RESGen::StrTok(char *string, char delimiter)
{
	// This replaces the normal strtok function because we don;t want to skip leading delimiters.
	// That 'feature' of strtok makes it kinda useless, unless you do a lot of checking for the skipping.

	// set temp to start of string to parse
	if (string == NULL)
	{
		string = strtok_nexttoken;
	}

	char *temp = string;

	// Search for token
	while (*temp)
	{
		if (*temp == delimiter)
		{
			// token found, remove it and set temp to next char
			*temp++ = 0; // terminate string at token
			break;
		}
		temp++;
	}

	if (temp == string)
	{
		// Token has ended
		return NULL;
	}

	// Save for next strtok run
	strtok_nexttoken = temp;

	// Return token
	return string;

}

#ifdef WIN32
// Win 32 DIR parser
void RESGen::ListDir(const VString &path, const VString &filepath, bool reporterror)
{
	WIN32_FIND_DATA filedata;

	// add *.* for searching all files.
	VString searchdir = path + filepath + "*.*";

	// find first file
	HANDLE filehandle = FindFirstFile(searchdir, &filedata);

	if (filehandle == INVALID_HANDLE_VALUE)
	{
		if (firstdir && reporterror)
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
		VString file = filepath + filedata.cFileName;

		// Check for directory
		if (filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Look for files in subdir, but ignore . and ..
			if (strcmp(filedata.cFileName, ".") && strcmp(filedata.cFileName, ".."))
			{
				// Call this function recursive
				ListDir(path, file + "\\", reporterror);
			}
		}
		else
		{
			// Check if the file is a possible resource
			if (
				!file.CompareReverseLimitNoCase(".mdl", 4) ||
				!file.CompareReverseLimitNoCase(".wav", 4) ||
				!file.CompareReverseLimitNoCase(".spr", 4) ||
				!file.CompareReverseLimitNoCase(".bmp", 4) ||
				!file.CompareReverseLimitNoCase(".tga", 4) ||
				!file.CompareReverseLimitNoCase(".txt", 4) ||
				!file.CompareReverseLimitNoCase(".wad", 4)
				)
			{
				// resource, add to list
				file.StrRplChr('\\', '/'); // replace backslashes

				VString *tmp = new VString(file);
				if (!resources.InsertSorted(tmp, RESGen_CompareVStringsFromList, false))
				{
					// double detected
					delete tmp;
				}

				if (resourcedisp)
				{
					printf("Added \"%s\" to resource list\n", (LPCSTR)file);
				}
			}

			if (!file.CompareReverseLimitNoCase(".pak", 4) && pakparse)
			{
				// get pakfilelist
				BuildPakResourceList(path + file);
			}
		}

	} while (FindNextFile(filehandle, &filedata));

	// Close search
	FindClose(filehandle);
}

#else
// Linux dir parser
void RESGen::ListDir(const VString &path, const VString &filepath, bool reporterror)
{
	struct stat filestatinfo; // Force as a struct for GCC

	VString searchpath = path + filepath;

	// Open the current dir
	DIR *directory = opendir(searchpath);

	// Is it open?
	if (directory == NULL)
	{
		// dir cannot be opened
		if (firstdir && reporterror)
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
		// We follow symlinks. people shouldn't mess with symlinks in the HL folder anyways.
		int i = stat(searchpath + direntry->d_name, &filestatinfo); // Get the info about the files the links point to

		VString file = filepath + direntry->d_name;

		if (i == 0)
		{
			// Check for directory
			if (S_ISDIR(filestatinfo.st_mode))
			{
				// Look for files in subdir, but ignore . and ..
				if (strcmp(direntry->d_name, ".") && strcmp(direntry->d_name, ".."))
				{
					// Call this function recursive
					ListDir(path, file + "/", reporterror);
				}
			}
			else
			{
				// Check if the file is a possible resource
				if (
					!file.CompareReverseLimitNoCase(".mdl", 4) ||
					!file.CompareReverseLimitNoCase(".wav", 4) ||
					!file.CompareReverseLimitNoCase(".spr", 4) ||
					!file.CompareReverseLimitNoCase(".bmp", 4) ||
					!file.CompareReverseLimitNoCase(".tga", 4) ||
					!file.CompareReverseLimitNoCase(".txt", 4) ||
					!file.CompareReverseLimitNoCase(".wad", 4)
					)
				{
					// resource, add to list
					file.StrRplChr('\\', '/'); // replace backslashes

					VString *tmp = new VString(file);
					if (!resources.InsertSorted(tmp, RESGen_CompareVStringsFromList, false))
					{
						// double detected
						delete tmp;
					}

					if (resourcedisp)
					{
						printf("Added \"%s\" to resource list\n", (LPCSTR)file);
					}
				}

				if (!file.CompareReverseLimitNoCase(".pak", 4) && pakparse)
				{
					// get pakfilelist
					BuildPakResourceList(path + file);
				}
			}
		}
	}

	// close the dir
	closedir(directory);

}

#endif

void RESGen::BuildPakResourceList(const VString &pakfilename)
{
	// open the pak file in binary read mode
	File pakfile(pakfilename, "rb");

	if (pakfile == NULL)
	{
		// error opening pakfile!
		printf("Could not find pakfile \"%s\".\n", (LPCSTR)pakfilename);
		return;
	}

	// Check a pakfile for resources
	// get the header
	int pakheadersize = sizeof(pakheader_s);
	pakheader_s pakheader;
	int retval = fread((void *)&pakheader, 1, pakheadersize, pakfile);

	if (retval != pakheadersize)
	{
		// unexpected size.
		if (verbal)
		{
			printf("Reading pakfile header failed. Wrong size (%d read, %d expected).\n", retval, pakheadersize);
			printf("Is \"%s\" a valid pakfile?\n", (LPCSTR)pakfilename);
		}
		return;
	}

	// verify pak identity
	if (pakheader.pakid != 1262698832)
	{
		if (verbal)
		{
			printf("Pakfile \"%s\" does not appear to be a Half-Life pakfile (ID mismatch).\n", (LPCSTR)pakfilename);
		}
		return;
	}

	// count the number of files in the pak
	int fileinfosize = sizeof(fileinfo_s);
	int filecount = pakheader.dirsize / fileinfosize;

	// re-verify integrity of header
	if (pakheader.dirsize % fileinfosize != 0 || filecount == 0)
	{
		if (verbal)
		{
			printf("Pakfile \"%s\" does not appear to be a Half-Life pakfile (invalid dirsize).\n", (LPCSTR)pakfilename);
		}
		return;
	}

	// load file list to memory
	if(fseek(pakfile, pakheader.diroffset, SEEK_SET))
	{
		if (verbal)
		{
			printf("Error seeking for file list.\nPakfile \"%s\" is not a pakfile, or is corrupted.\n", (LPCSTR)pakfilename);
		}
		return;
	}

	fileinfo_s *filelist = new fileinfo_s [filecount];
	retval = fread(filelist, 1, pakheader.dirsize, pakfile);
	if (retval != pakheader.dirsize)
	{
		if (verbal)
		{
			printf("Error seeking for file list.\nPakfile \"%s\" is not a pakfile, or is corrupted.\n", (LPCSTR)pakfilename);
		}
		delete [] filelist;
		return;
	}

	if (verbal)
	{
		printf("Scanning pak file \"%s\" for resources (%d files in pak)\n", (LPCSTR)pakfilename, filecount);
	}

	// Read filelist for possible resources
	for (int i = 0; i < filecount; i++)
	{
		if (
			!strrnicmp(filelist[i].name, ".mdl", 4) ||
			!strrnicmp(filelist[i].name, ".wav", 4) ||
			!strrnicmp(filelist[i].name, ".spr", 4) ||
			!strrnicmp(filelist[i].name, ".bmp", 4) ||
			!strrnicmp(filelist[i].name, ".tga", 4) ||
			!strrnicmp(filelist[i].name, ".txt", 4) ||
			!strrnicmp(filelist[i].name, ".wad", 4)
			)
		{
			// resource, add to list
			VString *tmp = new VString(filelist[i].name);
			tmp->StrRplChr('\\', '/'); // replace backslashes

			if (!resources.InsertSorted(tmp, RESGen_CompareVStringsFromList, false))
			{
				// double detected
				delete tmp;
			}

			if (resourcedisp)
			{
				printf("Added \"%s\" to resource list\n", (LPCSTR)*tmp);
			}
		}
	}

	// clean up
	delete [] filelist;
}

bool RESGen::LoadExludeFile(VString &listfile)
{
	if (&listfile == NULL)
	{
		return false;
	}

	if (listfile.GetLength() <= 0)
	{
		// Was called without a proper argument, fail
		return false;
	}

	if (listfile.CompareReverseLimitNoCase(".rfa", 4))
	{
		// .rfa extension missing, add it
		listfile += ".rfa";
	}

	File f(listfile, "rt"); // Text mode

	if (f == NULL)
	{
		// Error opening file, abort
		if (verbal)
		{
			printf("Error: Could not open the specified exclude list %s!\n",(LPCSTR)listfile);
		}
		return false;
	}

	checkforexcludes = true; // We want to check for excludes

	// loop to read file.. each line is an exclude
	VString *line = new VString;
	char linebuf[1024]; // optimal size for VString allocs
	while (fgets(linebuf, 1024, f))
	{
		*line += linebuf;
		if (line->GetAt(line->GetLength()-1) == '\n')
		{
			line->SetAt(line->GetLength()-1, 0); // delete \n
			line->Trim(); // Trim spaces
			#ifndef WIN32
			line->TrimRight('\r'); // forces linux compatibility
			line->TrimRight(); // remove remaining whitespace
			#endif
			if (line->CompareLimit("//", 2) && line->GetLength() != 0)
			{
				// Convert backslashes to slashes
				line->StrRplChr('\\', '/');
				// Not a comment or empty line
				if (!excludelist.InsertSorted(line, RESGen_CompareVStringsFromList, false))
				{
					// double detected
					line->Empty();
				}
				else
				{
					line = new VString;
				}
			}
			else
			{
				// Clear comment
				line->Empty();
			}
		}
	}

	if (line->GetLength() > 0)
	{
		line->SetAt(line->GetLength()-1, 0); // delete \n
		line->Trim(); // Trim spaces
		#ifndef WIN32
		line->TrimRight('\r'); // forces linux compatibility
		line->TrimRight(); // remove remaining whitespace
		#endif
		if (line->CompareLimit("//", 2) && line->GetLength() != 0)
		{
			// Convert backslashes to slashes
			line->StrRplChr('\\', '/');
			// Not a comment or empty line
			if (!excludelist.InsertSorted(line, RESGen_CompareVStringsFromList, false))
			{
				// double detected
				delete line;
			}
		}
		else
		{
			// Clean up comment
			delete line;
		}
	}
	else
	{
		delete line; // clean up
	}

	return true;
}

int RESGen_CompareVStringsFromList(VString *a, VString *b)
{
	return a->CompareNoCase(*b);
}

bool RESGen::CheckWadUse(const VString &wadfile)
{
	File wad((LPCSTR)(resourcepath+wadfile), "rb");

	if (!wad)
	{
		// try the valve folder
		if (valveresourcepath.GetLength() > 0)
		{
			wad.open((LPCSTR)(valveresourcepath+wadfile), "rb");
		}
		if (!wad)
		{
			printf("Failed to open WAD file \"%s\".\n", (LPCSTR)wadfile);
			return false;
		}
	}

	wadheader_s header;
	if (fread(&header, sizeof(wadheader_s), 1, wad) != 1)
	{
		printf("WAD file \"%s\" is corrupt.\n", (LPCSTR)wadfile);
		return false;
	}

	if (strncmp(header.identification, "WAD", 3))
	{
		printf("\"%s\" is not a WAD file.\n", (LPCSTR)wadfile);
		return false;
	}

	if (header.identification[3] != '2' && header.identification[3] != '3')
	{
		printf("Incorrect WAD file version for \"%s\"\n", (LPCSTR)wadfile);
		return false;
	}

	if (fseek(wad, header.infotableofs, SEEK_SET))
	{
		printf("Cannot find WAD info table in \"%s\"\n", (LPCSTR)wadfile);
		return false;
	}

	bool retval = false;
	for (int i = 0; i < header.numlumps; i++)
	{
		wadlumpinfo_s lumpinfo;
		if (fread(&lumpinfo, sizeof(wadlumpinfo_s), 1, wad) != 1)
		{
			printf("WAD file info table \"%s\" is corrupt.\n", (LPCSTR)wadfile);
			return false;
		}

		VString texture = lumpinfo.name;
		int texloc = texturelist.Find(&texture, RESGen_CompareVStringsFromList);

		if (texloc >= 0)
		{
			// found a texture, so wad is used
			retval = true;

			// update texture list
			VString *texstring = texturelist.GetAt(texloc);
			texturelist.RemoveAt(texloc);
			delete texstring;
		}
	}

	return retval;
}

bool RESGen::CheckModelExtTexture(const VString &model)
{
	File mdl((LPCSTR)(resourcepath+model), "rb");

	if (!mdl)
	{
		// try the valve folder
		if (valveresourcepath.GetLength() > 0)
		{
			mdl.open((LPCSTR)(valveresourcepath+model), "rb");
		}
		if (!mdl)
		{
			printf("Failed to open MDL file \"%s\".\n", (LPCSTR)model);
			return false;
		}
	}

	modelheader_s header;
	if (fread(&header, sizeof(modelheader_s), 1, mdl) != 1)
	{
		printf("MDL file \"%s\" is corrupt.\n", (LPCSTR)model);
		return false;
	}

	if (strncmp(header.id, "IDST", 4))
	{
		printf("\"%s\" is not a MDL file.\n", (LPCSTR)model);
		return false;
	}

	if (header.version != 10)
	{
		printf("Incorrect MDL file version for \"%s\"\n", (LPCSTR)model);
		return false;
	}

	if (header.textureindex == 0)
	{
		return true; // Uses seperate texture
	}

	return false;
}


