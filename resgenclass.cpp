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

std::vector<std::string>::iterator findStringNoCase(std::vector<std::string> &vec, const std::string &element)
{
	for(std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if(ICompareStrings(*it, element) == 0)
		{
			return it;
		}
	}

	return vec.end();
}

std::vector<StringKey>::iterator findStringNoCase(std::vector<StringKey> &vec, const StringKey &element)
{
	for(std::vector<StringKey>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		if(it->lower == element.lower)
		{
			return it;
		}
	}

	return vec.end();
}

std::vector<StringKey>::iterator findStringNoCaseSorted(std::vector<StringKey> &vec, const StringKey &element)
{
	std::vector<StringKey>::iterator it = std::lower_bound(vec.begin(), vec.end(), element, StringKeyLessThan);

	if(
		(it == vec.end())
	||	(it->lower > element.lower)
	)
	{
		return vec.end();
	}

	return it;
}

RESGen::RESGen()
{
	checkforresources = false;
	checkforexcludes = false;
}

RESGen::~RESGen()
{
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

int RESGen::MakeRES(std::string &map, int fileindex, size_t filecount)
{
	#ifdef WIN32
	WIN32_FIND_DATA filedata;
	HANDLE filehandle;
	#else
	glob_t globbuf;
	#endif

	// Create basefilename
	size_t lastSlashIndex = map.rfind('/'); // Linux style path
	if (lastSlashIndex == std::string::npos)
	{
		lastSlashIndex = map.rfind('\\'); // windows style path
	}

	std::string basefolder; // folder, including trailing /
	if (lastSlashIndex == std::string::npos)
	{
		#ifdef WIN32
		basefolder = ".\\";
		#else
		basefolder = "./";
		#endif
	}
	else
	{
		basefolder = map.substr(0, lastSlashIndex + 1);
	}
	std::string basefilename = map.substr(lastSlashIndex + 1, map.length() - lastSlashIndex - 5);

	std::string resName = basefolder + basefilename + ".res";

	if (verbal)
	{
		printf("Creating .res file %s [%d/" SIZE_T_SPECIFIER "].\n", resName.c_str(), fileindex, filecount);
	}


	// Check if resfile doesn't already exist
	bool fileexists = false;
	#ifdef WIN32
	filehandle = FindFirstFile(resName.c_str(), &filedata);
	if (filehandle != INVALID_HANDLE_VALUE)
	{
		FindClose(filehandle);
		fileexists = true;
	}
	#else
	if (!glob(resName.c_str(), GLOB_TILDE, NULL, &globbuf))
	{
		globfree(&globbuf);
		fileexists = true;
	}
	#endif

	if (!overwrite && fileexists)
	{
		// File found, but we don't want to overwrite.
		printf("%s already exists. Skipping file.\n", resName.c_str());
		return 1;
	}

	// Clear the resfile list to be sure (SHOULD be empty)
	resfile.clear();

	// Clear the texture list to be sure (SHOULD be empty)
	texturelist.clear();

	// first, get the enity data
	size_t entdatalen; // Length of entity data
	std::unique_ptr<char[]> entdata = LoadBSPData(map, entdatalen, texturelist);
	if (entdata == NULL)
	{
		// error. return
		return 1;
	}

	// get mapinfo
	char *mistart;

	// Look for first entity block
	if ((mistart = strstr(entdata.get(), "{")) == NULL)
	{
		// Something wrong with bsp entity data
		printf("Error parsing \"%s\". Entity data not in recognized text format.\n", map.c_str());
		return 1;
	}

	// Look for first block end (we do not need to parse the ather blocks vor skyname or wad info)
	size_t milen = static_cast<size_t>(strstr(mistart+1, "}") - mistart) + 1;

	std::unique_ptr<char[]> mapinfo(new char [milen + 1]);
	memcpy(mapinfo.get(), mistart, milen);
	mapinfo[milen] = 0; // terminating NULL

	// parse map info. We use StrTok for this...
	char *token = StrTok(mapinfo.get(), '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". No map information found.\n", map.c_str());
			return 1;
	}

	token = StrTok(NULL, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". Entity data is corrupt.\n", map.c_str());
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
				printf("Error parsing \"%s\" for WADs. Entity data is corrupt.\n", map.c_str());
				return 1;
			}
			// token has value

			std::string value = token;
			if (!value.empty()) // Don't try to parse an empty listing
			{
				// seperate the WAD files and save
				size_t i = 0;
				size_t seppos;

				while ((seppos = value.find(';', i)) != std::string::npos)
				{
					AddWad(value, i, seppos - i); // Add wad to reslist
					i = seppos + 1;
				}

				// There might be a wad file left in the list, check for it
				if (i < value.length())
				{
					// it should be equal, there is a wadfile left!
					AddWad(value, i, value.length() - i);
				}
			}

		}
		else if (!strcmp(token, "skyname"))
		{
			// wad file
			token = NextValue();
			if (!token)
			{
				printf("Error parsing \"%s\" skies. Entity data is corrupt.\n", map.c_str());
				return 1;
			}
			// token has value

			std::string value(token);

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
				printf("Error parsing \"%s\" for map data. Entity data is corrupt.\n", map.c_str());
				return 1;
			}
		}

		// Go to next key, if available
		token = NextToken(); // exit value
		if (!token)
		{
			printf("Error parsing \"%s\". Keys/values not alligned.\n", map.c_str());
			return 1;
		}
		// token is 'empty'

		token = NextToken(); // try next key

		// No statbar - this is way too fast for it to even show up.
	}

	mapinfo.reset();


	statcount = STAT_MAX; // make statbar print at once


	// parse the entity data. We use StrTok for this...
	// Note that we reparse the mapinfo.
	token = StrTok(entdata.get(), '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". No initial key.\n", map.c_str());
			return 1;
	}

	// Do again, to get to first key.
	token = StrTok(NULL, '\"');
	if (!token)
	{
			printf("Error parsing \"%s\". First key not found.\n", map.c_str());
			return 1;
	}

	while (token)
	{
		// Move from key to value
		token = NextValue();

		if (!token)
		{
			printf("\rError parsing \"%s\". Key to value transition failed.\n", map.c_str());
			return 1;
		}

		std::string value(token);
		std::string valueLower = strToLowerCopy(value);

		const size_t dotIndex = valueLower.find_last_of('.');

		if(dotIndex != std::string::npos)
		{
			const std::string extension = valueLower.substr(dotIndex + 1);

			if (extension == "mdl")
			{
				// mdl file
				AddRes(value);
			}
			else if (extension == "wav")
			{
				// wave file
				AddRes(value, "sound/");
			}
			else if (extension == "spr")
			{
				// sprite file
				AddRes(value);
			}
			else if (extension == "bmp")
			{
				// bitmap file
				AddRes(value);
			}
			else if (extension == "tga")
			{
				// targa file
				AddRes(value);
			}
		}

		token = NextToken(); // exit value
		if (!token)
		{
			printf("\rError parsing \"%s\". Could not move on to next key.\n", map.c_str());
			return 1;
		}
		// token is 'empty'

		// update statbar
		if (statusline && statcount == STAT_MAX)
		{
			// Reset the statcount
			statcount = 0;

			// Calculate the percentage completed of the current file.
			size_t progress = static_cast<size_t>(token - entdata.get());
			size_t percentage = ((progress + 1) * 101) / entdatalen; // Make the length one too long.
			if (percentage > 100)
			{
				 // Make sure we don;t go over 100%
				percentage = 100;
			}
			printf("\r(" SIZE_T_SPECIFIER "%%) [%d/" SIZE_T_SPECIFIER "]", percentage, fileindex, filecount);
		}
		else
		{
			statcount++;
		}


		token = StrTok(NULL, '\"'); // try next key
	}

	if (statusline)
	{
		// erase statusline
		printf("\r%-21s\r", ""); // easier to adjust length this way
	}

	entdata.reset();

	// Try to find info txt and overview data
	#ifdef WIN32
	filehandle = FindFirstFile((basefolder + "..\\overviews\\" + basefilename + ".txt").c_str(), &filedata); // try to find txt file for overview
	if (filehandle != INVALID_HANDLE_VALUE)
	{
		FindClose(filehandle);

		// file found, but we need the tga or bmp too
		filehandle = FindFirstFile((basefolder + "..\\overviews\\" + basefilename + ".tga").c_str(), &filedata); // try to find tga file for overview
		if (filehandle != INVALID_HANDLE_VALUE)
		{
			FindClose(filehandle);

			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".tga");
			AddRes(basefilename, "overviews/", ".txt");
		}
		else
		{
			filehandle = FindFirstFile((basefolder + "..\\overviews\\" + basefilename + ".bmp").c_str(), &filedata); // try to find bmp file for overview
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
	if (!glob((basefolder + "../overviews/" + basefilename + ".txt").c_str(), GLOB_TILDE, NULL, &globbuf))
	{
		globfree(&globbuf);

		// file found, but we need the tga or bmp too
		if (!glob((basefolder + "../overviews/" + basefilename + ".tga").c_str(), GLOB_TILDE, NULL, &globbuf))
		{
			globfree(&globbuf);

			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".tga");
			AddRes(basefilename, "overviews/", ".txt");
		}
		else if (!glob((basefolder + "../overviews/" + basefilename + ".bmp").c_str(), GLOB_TILDE, NULL, &globbuf))
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

	std::vector<std::string> extraResources;

	// Check for resources on disk
	if (checkforresources)
	{
		//printf("\nStarting resource check:\n");
		std::map<std::string, std::string>::iterator it = resfile.begin();

		while(it != resfile.end())
		{
			bool bErase = false;

			std::map<std::string, std::string>::iterator resourceIt = resources.find(it->first);

			if(resourceIt == resources.end())
			{
				// file not found - maybe it's excluded?
				if(excludelist.find(it->first) != excludelist.end())
				{
					// file found - it's an exclude
					if (contentdisp)
					{
						printf("Resource is excluded: %s\n", it->second.c_str());
					}

				}
				else if (CompareStrEndNoCase(it->first, ".wad"))
				{
					// not a wad file
					if (verbal)
					{
						printf("Resource file not found: %s\n", it->second.c_str());
					}
					status = 2; // res file might not be complete
				}
				else
				{
					// wad file is not critical, so no status change
					if (contentdisp)
					{
						printf("Resource file not found: %s\n", it->second.c_str());
					}
				}

				bErase = true;
			}
			else
			{
				if (matchcase)
				{
					// match case
					it->second = resourceIt->second;
				}

				if (parseresource)
				{
					if (!CompareStrEndNoCase(it->first, ".wad"))
					{
						// Check if wad file is used
						if (!CheckWadUse(resourceIt->second)) // We MUST have the right file
						{
							// Wad is NOT being used
							if (contentdisp)
							{
								printf("WAD file not used: %s\n", it->second.c_str());
							}

							if(!preservewads)
							{
								bErase = true;
							}
						}
					}
					else if (!CompareStrEndNoCase(it->first, ".mdl"))
					{
						// Check model for external texture
						if (CheckModelExtTexture(resourceIt->second))
						{
							// Uses external texture, add
							std::string extmdltex = it->second.substr(0, it->second.length() - 4); // strip extention
							extmdltex += "T.mdl"; // add T and extention

							if(
								(resfile.find(strToLowerCopy(extmdltex)) == resfile.end())
							&&	(findStringNoCase(extraResources, extmdltex) == extraResources.end())
							)
							{
								extraResources.push_back(extmdltex);

								if (contentdisp)
								{
									printf("MDL texture file added: %s\n", extmdltex.c_str());
								}
							}
						}
					}

				}
			}

			if(bErase)
			{
				it = resfile.erase(it);
			}
			else
			{
				++it;
			}
		}

		for(std::vector<std::string>::const_iterator extraIt = extraResources.begin(); extraIt != extraResources.end(); ++extraIt)
		{
			resfile[strToLowerCopy(*extraIt)] = *extraIt;
		}
	}

	// Check if resource has to be excluded
	if (checkforexcludes)
	{
		//printf("\nStarting exclude check:\n");
		std::map<std::string, std::string>::iterator it = resfile.begin();
		while(it != resfile.end())
		{
			if(excludelist.find(it->first) != excludelist.end())
			{
				// file found
				if (contentdisp)
				{
					printf("Resource is excluded: %s\n", it->second.c_str());
				}

				it = resfile.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	// Give a list of missing textures
	if (parseresource && checkforresources && verbal)
	{
		if (!texturelist.empty())
		{
			status = 2; // res file might not be complete
			for (size_t i = 0; i < texturelist.size(); i++)
			{
				printf("Texture not found in wad files: %s\n", texturelist[i].c_str());
			}
		}
	}

	if (resfile.empty() && rfastring.empty())
	{
		// no resources!
		if (verbal) { printf("No resources were found for \"%s.res\".", basefilename.c_str()); }

		if (fileexists)
		{
			// File exists, delete it.
			// WHAT? No check for overwrite? No!
			// Think of it, if the file exists we MUST be in overwrite mode to even get to this point!
			remove(resName.c_str());
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
	resfile.clear();
	texturelist.clear();

	return status;
}

void RESGen::BuildResourceList(std::string &respath, bool checkpak, bool sdisp, bool rdisp)
{
	searchdisp = sdisp;
	resourcedisp = rdisp;
	pakparse = checkpak;

	resources.clear();

	if (respath.empty())
	{
		// no respath, thus no reslist
		checkforresources = false;
		return;
	}

	checkforresources = true;

	// Check the respath and check ../valve if the respath doesn't point to valve

	#ifdef WIN32
	const char* pathSep = "\\";
	const char* valveStr = "\\valve\\";
	#else
	const char* pathSep = "/";
	const char* valveStr = "/valve/";
	#endif

	// prepare folder name
	
	if (respath[respath.length() - 1] != pathSep[0])
	{
		// No path separator, add
		respath += pathSep;
	}

	std::string valvepath;

	if(CompareStrEndNoCase(respath, valveStr))
	{
		// NOT valve dir, so check it too
		size_t slashpos = respath.rfind(pathSep[0], respath.length() - 2);
		if (slashpos != std::string::npos)
		{
			valvepath = respath.substr(0, slashpos);
			valvepath += valveStr;
		}
		else
		{
			valvepath = respath + ".." + valveStr;
		}
	}

	resourcepath = respath;
	valveresourcepath = valvepath;

	if (resourcedisp)
	{
		printf("Searching %s for resources:\n", respath.c_str());
	}
	else if (verbal)
	{
		printf("Searching %s for resources...\n", respath.c_str());
	}

	firstdir = true;
	ListDir(respath, "", true);

	// Check the valve dir too
	if (!valvepath.empty())
	{
		firstdir = true;
		ListDir(valvepath, "", false);
	}

	printf("\n");
}

std::unique_ptr<char[]> RESGen::LoadBSPData(const std::string &file, size_t & entdatalen, std::vector<std::string> & texlist)
{
	// first open the file.
	File bsp(file, "rb"); // read in binary mode.

	if (bsp == NULL)
	{
		printf("Error opening \"%s\"\n", file.c_str());
		return NULL;
	}

	// file open.. read header
	bsp_header header;

	if (fread(&header, sizeof(bsp_header), 1, bsp) != 1)
	{
		// header NOT read properly!
		printf("Error opening \"%s\". Corrupt BSP file.\n", file.c_str());
		return NULL;
	}

	if (header.version != BSPVERSION)
	{
		printf("Error opening \"%s\". Incorrect BSP version.\n", file.c_str());
		return NULL;
	}

	if (header.ent_header.fileofs <= 0 || header.ent_header.filelen <= 0)
	{
		// File corrupted
		printf("Error opening \"%s\". Corrupt BSP header.\n", file.c_str());
		return NULL;
	}

	// read entity data
	std::unique_ptr<char[]> entdata(new char[header.ent_header.filelen + 1]);

	fseek(bsp, header.ent_header.fileofs, SEEK_SET);
	if (fread(entdata.get(), header.ent_header.filelen, 1, bsp) != 1)
	{
		// not the right ammount of data was read
		printf("Error opening \"%s\". BSP file corrupt.\n", file.c_str());
		return NULL;
	}

	if (parseresource && checkforresources)
	{
		// Load names of external textures
		fseek(bsp, header.tex_header.fileofs, SEEK_SET); // go to start of texture data
		size_t texcount;

		if (fread(&texcount, sizeof(int), 1, bsp) != 1) // first we want to know the number of files.
		{
			// header NOT read properly!
			printf("Error opening \"%s\". Corrupt texture header.\n", file.c_str());
			return NULL;
		}

		if (texcount > 0)
		{
			// Textures available, read all offsets
			std::unique_ptr<char[]> offsets(new char[sizeof(int) * texcount]);

			size_t i = fread(offsets.get(), sizeof(int), texcount, bsp);

			if (i != texcount) // load texture offsets
			{
				// header NOT read properly!
				printf("Error opening \"%s\". Corrupt texture data.\n  read: %d, expect: %d\n", file.c_str(), i, texcount);
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
					printf("Error opening \"%s\". Corrupt BSP file.\n", file.c_str());
					return NULL;
				}

				// is this a wad based texture?
				if (texdata.offsets[0] == 0 && texdata.offsets[1] == 0 && texdata.offsets[2] == 0 && texdata.offsets[3] == 0)
				{
					// No texture for any mip level, so must be in a wad
					std::string texfile(texdata.name);
					texlist.push_back(texfile);
				}
			}
		}

		/*
		// TODO: Sort
		printf("\n\nTextures found:\n");

		for (int i = 0; i < texlist.GetCount(); i++)
		{
			printf("%s\n", texlist.GetAt(i).c_str());
		}
		printf("\n\n");
		//*/
	}

	// add terminating NULL for entity data
	entdata.get()[header.ent_header.filelen] = 0;

	entdatalen = header.ent_header.filelen;

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

void RESGen::AddRes(std::string res, const char * const prefix, const char * const suffix)
{
	// Sometimes res entries start with a non alphanumeric character. Strip it.
	while (!isalnum(res[0])) // keep stripping until a valid char is found
	{
		// Remove character
		res = res.substr(1); // Kinda slow, but usually only happens once.
	}

	res = replaceCharAll(res, '\\', '/');

	// Add prefix and suffix
	if (prefix)
	{
		res = std::string(prefix) + res;
	}
	if (suffix)
	{
		res += suffix;
	}

	if (tolower)
	{
		// Convert name to lowercase
		strToLower(res);
	}

	// Add file to list if it isn't in it yet.
	std::string resLower = strToLowerCopy(res);

	// File not found, must be new. Add to list
	if(resfile.find(resLower) != resfile.end())
	{
		// double file found, discard
		return;
	}

	resfile[resLower] = res;

	// Report file found
	if (contentdisp)
	{
		printf("\r%-21s\n", res.c_str()); // With 21 chars, there is support for up to 999999 bsp's to parse untill the statbar might remain in screen
	}

	statcount = STAT_MAX; // Make statbar print on next update

	return;
}

void RESGen::AddWad(const std::string &wadlist, size_t start, size_t len)
{
	std::string wadfile = wadlist.substr(start, len);

	wadfile = replaceCharAll(wadfile, '\\', '/');

	// strip folders
	wadfile = wadfile.substr(wadfile.rfind('/') + 1);

	// Add file to reslist
	AddRes(wadfile);
}

bool RESGen::WriteRes(const std::string &folder, const std::string &mapname)
{
	// This function writes a standard res file.

	// Open the file
	File f(folder + mapname + ".res", "w");

	if (f == NULL)
	{
		printf("Failed to open %s for writing.\n", (folder + mapname + ".res").c_str());
		return false;
	}

	// Header
	fprintf(f, "// %s - created with RESGen v%s.\n", (mapname + ".res").c_str(), VERSION);
	fprintf(f, "// RESGen is made by Jeroen \"ShadowLord\" Bogers,\n");
	fprintf(f, "// with serveral improvements and additions by Zero3Cool.\n");
	fprintf(f, "// For more info go to http://resgen.hltools.com\n");

	fprintf(f, "\n// .res entries (%d):\n", resfile.size());

	// Resources
	for (std::map<std::string, std::string>::const_iterator it = resfile.begin(); it != resfile.end(); ++it)
	{
		fprintf(f, "%s\n", it->second.c_str());
	}

	// RFA file, if needed
	if (!rfastring.empty())
	{
		fprintf(f, "\n// Added .res content:\n%s\n", rfastring.c_str());
	}

	return true;
}

bool RESGen::LoadRfaFile(std::string &filename)
{
	if (filename.empty())
	{
		// no rfa file, ignore
		return true;
	}

	// Add .rfa extention if needed
	if (CompareStrEndNoCase(filename,".rfa"))
	{
		// not found, add
		filename += ".rfa";
	}

	std::string str;
	const bool bSuccess = readFile(filename, str);

	if(bSuccess)
	{
		rfastring = str;
	}
	else
	{
		printf("Error reading rfa file: \"%s\"\n", filename.c_str());
	}

	return bSuccess;
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
void RESGen::ListDir(const std::string &path, const std::string &filepath, bool reporterror)
{
	WIN32_FIND_DATA filedata;

	// add *.* for searching all files.
	std::string searchdir = path + filepath + "*.*";

	// find first file
	HANDLE filehandle = FindFirstFile(searchdir.c_str(), &filedata);

	if (filehandle == INVALID_HANDLE_VALUE)
	{
		if (firstdir && reporterror)
		{
			if (GetLastError() & ERROR_PATH_NOT_FOUND || GetLastError() & ERROR_FILE_NOT_FOUND)
			{
				printf("The directory you specified (%s) can not be found or is empty.\n", path.c_str());
			}
			else
			{
				printf("There was an error with the directory you specified (%s) - ERROR NO: %lu.\n", path.c_str(), GetLastError());
			}
		}
		return;
	}

	firstdir = false;

	do
	{
		std::string file = filepath + filedata.cFileName;

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
			const std::string fileLower = strToLowerCopy(file);

			const size_t dotIndex = fileLower.find_last_of('.');

			if(dotIndex != std::string::npos)
			{
				const std::string extension = fileLower.substr(dotIndex + 1);

				if (
					extension == "mdl" ||
					extension == "wav" ||
					extension == "spr" ||
					extension == "bmp" ||
					extension == "tga" ||
					extension == "txt" ||
					extension == "wad"
				)
				{
					// resource, add to list
					file = replaceCharAll(file, '\\', '/'); // replace backslashes

					resources[strToLowerCopy(file)] = file;

					if (resourcedisp)
					{
						printf("Added \"%s\" to resource list\n", file.c_str());
					}
				}

				if ((extension == "pad") && pakparse)
				{
					// get pakfilelist
					BuildPakResourceList(path + file);
				}
			}
		}

	} while (FindNextFile(filehandle, &filedata));

	// Close search
	FindClose(filehandle);
}

#else
// Linux dir parser
void RESGen::ListDir(const std::string &path, const std::string &filepath, bool reporterror)
{
	struct stat filestatinfo; // Force as a struct for GCC

	std::string searchpath = path + filepath;

	// Open the current dir
	DIR *directory = opendir(searchpath.c_str());

	// Is it open?
	if (directory == NULL)
	{
		// dir cannot be opened
		if (firstdir && reporterror)
		{
			printf("There was an error with the directory you specified (%s)\nDid you enter the correct directory?\n", path.c_str());
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
		int i = stat((searchpath + direntry->d_name).c_str(), &filestatinfo); // Get the info about the files the links point to

		std::string file = filepath + direntry->d_name;

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
				std::string fileLower = strToLowerCopy(file);

				const size_t dotIndex = fileLower.find_last_of('.');

				if(dotIndex != std::string::npos)
				{
					const std::string extension = fileLower.substr(dotIndex + 1);

					// Check if the file is a possible resource
					if (
						extension == "mdl" ||
						extension == "wav" ||
						extension == "spr" ||
						extension == "bmp" ||
						extension == "tga" ||
						extension == "txt" ||
						extension == "wad"
					)
					{
						// resource, add to list
						file = replaceCharAll(file, '\\', '/'); // replace backslashes

						resources[strToLowerCopy(file)] = file;

						if (resourcedisp)
						{
							printf("Added \"%s\" to resource list\n", file.c_str());
						}
					}

					if ((extension == "pak") && pakparse)
					{
						// get pakfilelist
						BuildPakResourceList(path + file);
					}
				}
			}
		}
	}

	// close the dir
	closedir(directory);

}

#endif

void RESGen::BuildPakResourceList(const std::string &pakfilename)
{
	// open the pak file in binary read mode
	File pakfile(pakfilename, "rb");

	if (pakfile == NULL)
	{
		// error opening pakfile!
		printf("Could not find pakfile \"%s\".\n", pakfilename.c_str());
		return;
	}

	// Check a pakfile for resources
	// get the header
	size_t pakheadersize = sizeof(pakheader_s);
	pakheader_s pakheader;
	size_t retval = fread(&pakheader, 1, pakheadersize, pakfile);

	if (retval != pakheadersize)
	{
		// unexpected size.
		if (verbal)
		{
			printf("Reading pakfile header failed. Wrong size (%d read, %d expected).\n", retval, pakheadersize);
			printf("Is \"%s\" a valid pakfile?\n", pakfilename.c_str());
		}
		return;
	}

	// verify pak identity
	if (pakheader.pakid != 1262698832)
	{
		if (verbal)
		{
			printf("Pakfile \"%s\" does not appear to be a Half-Life pakfile (ID mismatch).\n", pakfilename.c_str());
		}
		return;
	}

	// count the number of files in the pak
	size_t fileinfosize = sizeof(fileinfo_s);
	size_t filecount = pakheader.dirsize / fileinfosize;

	// re-verify integrity of header
	if (pakheader.dirsize % fileinfosize != 0 || filecount == 0)
	{
		if (verbal)
		{
			printf("Pakfile \"%s\" does not appear to be a Half-Life pakfile (invalid dirsize).\n", pakfilename.c_str());
		}
		return;
	}

	// load file list to memory
	if(fseek(pakfile, pakheader.diroffset, SEEK_SET))
	{
		if (verbal)
		{
			printf("Error seeking for file list.\nPakfile \"%s\" is not a pakfile, or is corrupted.\n", pakfilename.c_str());
		}
		return;
	}

	std::unique_ptr<fileinfo_s[]> filelist(new fileinfo_s[filecount]);
	retval = fread(filelist.get(), 1, pakheader.dirsize, pakfile);
	if (retval != pakheader.dirsize)
	{
		if (verbal)
		{
			printf("Error seeking for file list.\nPakfile \"%s\" is not a pakfile, or is corrupted.\n", pakfilename.c_str());
		}
		return;
	}

	if (verbal)
	{
		printf("Scanning pak file \"%s\" for resources (%d files in pak)\n", pakfilename.c_str(), filecount);
	}

	// Read filelist for possible resources
	for (size_t i = 0; i < filecount; i++)
	{
		const std::string fileLower = strToLowerCopy(filelist[i].name);

		const size_t dotIndex = fileLower.find_last_of('.');

		if(dotIndex != std::string::npos)
		{
			const std::string extension = fileLower.substr(dotIndex + 1);

			if (
				extension == "mdl" ||
				extension == "wav" ||
				extension == "spr" ||
				extension == "bmp" ||
				extension == "tga" ||
				extension == "txt" ||
				extension == "wad"
			)
			{
				// resource, add to list
				std::string resStr = replaceCharAll(filelist[i].name, '\\', '/');

				resources[strToLowerCopy(resStr)] = resStr;

				if (resourcedisp)
				{
					printf("Added \"%s\" to resource list\n", resStr.c_str());
				}
			}
		}
	}
}

bool RESGen::LoadExludeFile(std::string &listfile)
{
	if (listfile.empty())
	{
		// Was called without a proper argument, fail
		return false;
	}

	if (CompareStrEndNoCase(listfile, ".rfa"))
	{
		// .rfa extension missing, add it
		listfile += ".rfa";
	}

	File f(listfile, "rt"); // Text mode

	if (f == NULL)
	{
		// Error opening file, abort
		printf("Error: Could not open the specified exclude list %s!\n", listfile.c_str());
		return false;
	}

	checkforexcludes = true; // We want to check for excludes

	// loop to read file.. each line is an exclude
	std::string line;
	char linebuf[1024]; // optimal size for VString allocs
	while (fgets(linebuf, 1024, f))
	{
		line += linebuf;
		if (line[line.length() - 1] == '\n')
		{
			leftTrim(line);
			rightTrim(line);
			
			if (line.compare(0, 2, "//") && line.length() != 0)
			{
				// Convert backslashes to slashes
				line = replaceCharAll(line, '\\', '/');
				// Not a comment or empty line
				excludelist[strToLowerCopy(line)] = line;
			}

			line.clear();
		}
	}

	if (line.length() > 0)
	{
		leftTrim(line);
		rightTrim(line);

		if (line.compare(0, 2, "//") && line.length() != 0)
		{
			// Convert backslashes to slashes
			line = replaceCharAll(line, '\\', '/');
			// Not a comment or empty line
			excludelist[strToLowerCopy(line)] = line;
		}
	}

	return true;
}

bool RESGen::CheckWadUse(const std::string &wadfile)
{
	File wad(resourcepath + wadfile, "rb");

	if (!wad)
	{
		// try the valve folder
		if (!valveresourcepath.empty())
		{
			wad.open(valveresourcepath + wadfile, "rb");
		}
		if (!wad)
		{
			printf("Failed to open WAD file \"%s\".\n", wadfile.c_str());
			return false;
		}
	}

	wadheader_s header;
	if (fread(&header, sizeof(wadheader_s), 1, wad) != 1)
	{
		printf("WAD file \"%s\" is corrupt.\n", wadfile.c_str());
		return false;
	}

	if (strncmp(header.identification, "WAD", 3))
	{
		printf("\"%s\" is not a WAD file.\n", wadfile.c_str());
		return false;
	}

	if (header.identification[3] != '2' && header.identification[3] != '3')
	{
		printf("Incorrect WAD file version for \"%s\"\n", wadfile.c_str());
		return false;
	}

	if (fseek(wad, header.infotableofs, SEEK_SET))
	{
		printf("Cannot find WAD info table in \"%s\"\n", wadfile.c_str());
		return false;
	}

	bool retval = false;
	for (int i = 0; i < header.numlumps; i++)
	{
		wadlumpinfo_s lumpinfo;
		if (fread(&lumpinfo, sizeof(wadlumpinfo_s), 1, wad) != 1)
		{
			printf("WAD file info table \"%s\" is corrupt.\n", wadfile.c_str());
			return false;
		}

		std::string texture = lumpinfo.name;

		std::vector<std::string>::iterator it = findStringNoCase(texturelist, texture);

		if(it != texturelist.end())
		{
			// found a texture, so wad is used
			retval = true;

			// update texture list
			texturelist.erase(it);
		}
	}

	return retval;
}

bool RESGen::CheckModelExtTexture(const std::string &model)
{
	File mdl(resourcepath + model, "rb");

	if (!mdl)
	{
		// try the valve folder
		if (!valveresourcepath.empty())
		{
			mdl.open(valveresourcepath + model, "rb");
		}
		if (!mdl)
		{
			printf("Failed to open MDL file \"%s\".\n", model.c_str());
			return false;
		}
	}

	modelheader_s header;
	if (fread(&header, sizeof(modelheader_s), 1, mdl) != 1)
	{
		printf("MDL file \"%s\" is corrupt.\n", model.c_str());
		return false;
	}

	if (strncmp(header.id, "IDST", 4))
	{
		printf("\"%s\" is not a MDL file.\n", model.c_str());
		return false;
	}

	if (header.version != 10)
	{
		printf("Incorrect MDL file version for \"%s\"\n", model.c_str());
		return false;
	}

	if (header.textureindex == 0)
	{
		return true; // Uses seperate texture
	}

	return false;
}


