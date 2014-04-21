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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "hltypes.h"
#include "resgenclass.h"
#include "resgen.h"
#include "resourcelistbuilder.h"
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

RESGen::RESGen()
{
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

int RESGen::MakeRES(std::string &map, int fileindex, size_t filecount, const StringMap &resources, std::vector<std::string> &resourcePaths_)
{
	resourcePaths = resourcePaths_;

	std::string basefolder;
	std::string basefilename;
	splitPath(map, basefolder, basefilename);

	const std::string resName = basefolder + basefilename + ".res";

	if (verbal)
	{
		printf("Creating .res file %s [%d/" SIZE_T_SPECIFIER "].\n", resName.c_str(), fileindex, filecount);
	}


	// Check if resfile doesn't already exist
	const bool fileexists = fileExists(resName);

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
	std::string entdata;

	if(!LoadBSPData(map, entdata, texturelist))
	{
		// error. return
		return 1;
	}

	// get mapinfo
	const char *mistart;

	// Look for first entity block
	if ((mistart = strstr(entdata.c_str(), "{")) == NULL)
	{
		// Something wrong with bsp entity data
		printf("Error parsing \"%s\". Entity data not in recognized text format.\n", map.c_str());
		return 1;
	}

	// Look for first block end (we do not need to parse the ather blocks vor skyname or wad info)
	// TODO: This can find braces inside strings
	size_t milen = static_cast<size_t>(strstr(mistart+1, "}") - mistart) + 1;

	mapinfo.clear();
	mapinfo.assign(mistart, milen);

	// parse map info. We use StrTok for this...
	EntTokenizer mapTokenizer(mapinfo);

	const EntTokenizer::KeyValuePair* kv = mapTokenizer.NextPair();

	if (!kv)
	{
		printf("Error parsing \"%s\". No map information found.\n", map.c_str());
		return 1;
	}

	while (kv)
	{
		if (!strcmp(kv->first, "wad"))
		{
			std::string value(kv->second);
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
		else if (!strcmp(kv->first, "skyname"))
		{
			std::string value(kv->second);

			// Add al 6 sky textures here
			AddRes(value, "gfx/env/", "up.tga");
			AddRes(value, "gfx/env/", "dn.tga");
			AddRes(value, "gfx/env/", "lf.tga");
			AddRes(value, "gfx/env/", "rt.tga");
			AddRes(value, "gfx/env/", "ft.tga");
			AddRes(value, "gfx/env/", "bk.tga");
		}

		kv = mapTokenizer.NextPair();
	}

	mapinfo.clear();


	statcount = STAT_MAX; // make statbar print at once

	EntTokenizer entDataTokenizer(entdata);

	kv = entDataTokenizer.NextPair();

	// Note that we reparse the mapinfo.
	if(!kv)
	{
		printf("Error parsing \"%s\".\n", map.c_str());
		return 1;
	}

	while (kv)
	{
		const ptrdiff_t tokenLength = entDataTokenizer.GetLatestValueLength();

		const char *token = kv->second;

		// TODO: This is fast, but should be made more robust if possible
		// Need at least 5 chars, assuming filename is:
		// [alpha][.][alpha]{3}
		if(tokenLength >= 5)
		{
			if(token[tokenLength - 4] == '.')
			{
				const int c1 = ::tolower(token[tokenLength - 3]);
				const int c2 = ::tolower(token[tokenLength - 2]);
				const int c3 = ::tolower(token[tokenLength - 1]);

				if(c1 == 'm' && c2 == 'd' && c3 == 'l')
				{
					// mdl file
					AddRes(token);
				}
				if(c1 == 'w' && c2 == 'a' && c3 == 'v')
				{
					// wave file
					AddRes(token, "sound/");
				}
				if(c1 == 's' && c2 == 'p' && c3 == 'r')
				{
					// sprite file
					AddRes(token);
				}
				if(c1 == 'b' && c2 == 'm' && c3 == 'p')
				{
					// bitmap file
					AddRes(token);
				}
				if(c1 == 't' && c2 == 'g' && c3 == 'a')
				{
					// targa file
					AddRes(token);
				}
			}
		}

		// update statbar
		if (statusline && statcount == STAT_MAX)
		{
			// Reset the statcount
			statcount = 0;

			// Calculate the percentage completed of the current file.
			size_t progress = static_cast<size_t>(token - &entdata[0]);
			size_t percentage = ((progress + 1) * 101) / entdata.length(); // Make the length one too long.
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

		kv = entDataTokenizer.NextPair();
	}

	if (statusline)
	{
		// erase statusline
		printf("\r%-21s\r", ""); // easier to adjust length this way
	}

	entdata.clear();

	// Try to find info txt and overview data
	std::string overviewPath = basefolder + ".." + PATH_SEPARATOR + "overviews" + PATH_SEPARATOR + basefilename;
	if(fileExists(overviewPath + ".txt"))
	{
		// file found, but we need the tga or bmp too
		if(fileExists(overviewPath + ".tga"))
		{
			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".tga");
			AddRes(basefilename, "overviews/", ".txt");
		}
		else if(fileExists(overviewPath + ".bmp"))
		{
			// txt found too, add both files to res list
			AddRes(basefilename, "overviews/", ".bmp");
			AddRes(basefilename, "overviews/", ".txt");
		}
	}

	// Resource list has been made.
	int status = 0; // RES status, 0 means ok, 2 means missing resource

	std::vector<std::string> extraResources;

	// Check for resources on disk
	if (!resourcePaths.empty())
	{
		//printf("\nStarting resource check:\n");
		StringMap::iterator it = resfile.begin();

		while(it != resfile.end())
		{
			bool bErase = false;

			StringMap::const_iterator resourceIt = resources.find(it->first);

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
				else if (CompareStrEnd(it->first, ".wad"))
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
					if (!CompareStrEnd(it->first, ".wad"))
					{
						// Check if wad file is used
						if (!CheckWadUse(resourceIt)) // We MUST have the right file
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
					else if (!CompareStrEnd(it->first, ".mdl"))
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
		StringMap::iterator it = resfile.begin();
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
	if (parseresource && !resourcePaths.empty() && verbal)
	{
		if (!texturelist.empty())
		{
			status = 2; // res file might not be complete
			for(StringMap::const_iterator it = texturelist.begin(); it != texturelist.end(); ++it)
			{
				printf("Texture not found in wad files: %s\n", it->second.c_str());
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

bool RESGen::LoadBSPData(const std::string &file, std::string &entdata, StringMap & texlist)
{
	// first open the file.
	File bsp(file, "rb"); // read in binary mode.

	if (bsp == NULL)
	{
		printf("Error opening \"%s\"\n", file.c_str());
		return false;
	}

	// file open.. read header
	bsp_header header;

	if (fread(&header, sizeof(bsp_header), 1, bsp) != 1)
	{
		// header NOT read properly!
		printf("Error opening \"%s\". Corrupt BSP file.\n", file.c_str());
		return false;
	}

	if (header.version != BSPVERSION)
	{
		printf("Error opening \"%s\". Incorrect BSP version.\n", file.c_str());
		return false;
	}

	if (header.ent_header.fileofs <= 0)
	{
		// File corrupted
		printf("Error opening \"%s\". Corrupt BSP header.\n", file.c_str());
		return false;
	}

	// read entity data
	entdata.resize(header.ent_header.filelen);

	fseek(bsp, header.ent_header.fileofs, SEEK_SET);
	if (fread(&entdata[0], header.ent_header.filelen, 1, bsp) != 1)
	{
		// not the right ammount of data was read
		printf("Error opening \"%s\". BSP file corrupt.\n", file.c_str());
		return false;
	}

	if (parseresource && !resourcePaths.empty())
	{
		// Load names of external textures
		fseek(bsp, header.tex_header.fileofs, SEEK_SET); // go to start of texture data
		size_t texcount;

		if (fread(&texcount, sizeof(int), 1, bsp) != 1) // first we want to know the number of files.
		{
			// header NOT read properly!
			printf("Error opening \"%s\". Corrupt texture header.\n", file.c_str());
			return false;
		}

		if (texcount > 0)
		{
			// Textures available, read all offsets
			std::vector<char> offsets(sizeof(int) * texcount);

			size_t i = fread(offsets.data(), sizeof(int), texcount, bsp);

			if (i != texcount) // load texture offsets
			{
				// header NOT read properly!
				printf("Error opening \"%s\". Corrupt texture data.\n  read: " SIZE_T_SPECIFIER ", expect: " SIZE_T_SPECIFIER "\n", file.c_str(), i, texcount);
				return false;
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
					return false;
				}

				// is this a wad based texture?
				if (texdata.offsets[0] == 0 && texdata.offsets[1] == 0 && texdata.offsets[2] == 0 && texdata.offsets[3] == 0)
				{
					// No texture for any mip level, so must be in a wad
					std::string texfileLower = strToLowerCopy(texdata.name);
					texlist[texfileLower] = texdata.name;
				}
			}
		}
	}

	#ifdef _DEBUG
	// Debug write entity data to file
	File tmp(file + "_ent.txt", "w");
	fprintf(tmp, "%s", entdata.c_str());
	#endif

	return true;
}

void RESGen::AddRes(std::string res, const char * const prefix, const char * const suffix)
{
	// Sometimes res entries start with a non alphanumeric character. Strip it.
	while (!isalnum(res[0])) // keep stripping until a valid char is found
	{
		// Remove character
		res.erase(res.begin());
	}

	replaceCharAll(res, '\\', '/');

	// Add prefix and suffix
	if (prefix)
	{
		res.insert(0, prefix);
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
	// We shouldn't care if this overwrites a previous entry (it shouldn't) -
	// it'll only differ by case
	resfile[strToLowerCopy(res)] = res;

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

	replaceCharAll(wadfile, '\\', '/');

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

	fprintf(f, "\n// .res entries (" SIZE_T_SPECIFIER "):\n", resfile.size());

	// Resources
	for (StringMap::const_iterator it = resfile.begin(); it != resfile.end(); ++it)
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
				replaceCharAll(line, '\\', '/');
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
			replaceCharAll(line, '\\', '/');
			// Not a comment or empty line
			excludelist[strToLowerCopy(line)] = line;
		}
	}

	return true;
}

bool RESGen::CacheWad(const std::string &wadfile)
{
	File wad;
	if(!OpenFirstValidPath(wad, wadfile, "rb"))
	{
		printf("Failed to open WAD file \"%s\".\n", wadfile.c_str());
		return false;
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

	const std::string wadFileLower(strToLowerCopy(wadfile));

	for (int i = 0; i < header.numlumps; i++)
	{
		wadlumpinfo_s lumpinfo;
		if (fread(&lumpinfo, sizeof(wadlumpinfo_s), 1, wad) != 1)
		{
			printf("WAD file info table \"%s\" is corrupt.\n", wadfile.c_str());
			return false;
		}

		std::string lumpNameLower(lumpinfo.name);
		strToLower(lumpNameLower);
		wadcache[wadFileLower].insert(lumpNameLower);
	}

	return true;
}

bool RESGen::CheckWadUse(const StringMap::const_iterator &wadfileIt)
{
	WadCache::const_iterator wadIt = wadcache.find(wadfileIt->first);

	if(wadIt == wadcache.end())
	{
		// Haven't read this wad yet
		if(!CacheWad(wadfileIt->second))
		{
			// Failed to read wad
			// Cache this failure with an empty set to prevent wad being marked
			// as used
			wadcache[wadfileIt->first] = TextureSet();
			return false;
		}

		wadIt = wadcache.find(wadfileIt->first);
	}

	assert(wadIt != wadcache.end());

	bool bWadUsed = false;

	const TextureSet& textureSet = wadIt->second;

	StringMap::iterator it = texturelist.begin();

	// Look through all unfound textures and remove any that appear in this wad
	while(it != texturelist.end())
	{
		TextureSet::const_iterator textureIt = textureSet.find(it->first);

		if(textureIt != textureSet.end())
		{
			// found a texture, so wad is used
			bWadUsed = true;

			// update texture list
			it = texturelist.erase(it);
		}
		else
		{
			++it;
		}
	}

	return bWadUsed;
}

bool RESGen::CheckModelExtTexture(const std::string &model)
{
	File mdl;
	if(!OpenFirstValidPath(mdl, model, "rb"))
	{
		printf("Failed to open MDL file \"%s\".\n", model.c_str());
		return false;
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

bool RESGen::OpenFirstValidPath(File &outFile, std::string fileName, const char* const mode)
{
	for(std::vector<std::string>::const_iterator it = resourcePaths.begin(); it != resourcePaths.end(); ++it)
	{
		outFile.open(*it + fileName, mode);

		if(outFile)
		{
			return true;
		}
	}

	return false;
}

