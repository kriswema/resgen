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

#include <cstring>
#include <vector>

#ifdef WIN32
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "hltypes.h"
#include "resourcelistbuilder.h"

ResourceListBuilder::ResourceListBuilder(const config_s &config)
	: resourcedisp(false)
	, pakparse(false)
	, firstdir(false)
	, verbal(config.verbal)
{
}

void ResourceListBuilder::BuildResourceList(const std::vector<std::string> &paths, bool checkpak, bool rdisp)
{
	resourcedisp = rdisp;
	pakparse = checkpak;

	if (paths.empty())
	{
		// no paths, thus no reslist
		return;
	}

	for(std::vector<std::string>::const_iterator it = paths.begin(); it != paths.end(); ++it)
	{
		if (rdisp)
		{
			printf("Searching %s for resources:\n", it->c_str());
		}
		else if (verbal)
		{
			printf("Searching %s for resources...\n", it->c_str());
		}

		firstdir = true;
		ListDir(*it, "", true);
	}

	printf("\n");
}

#ifdef WIN32
// Win 32 DIR parser
void ResourceListBuilder::ListDir(const std::string &path, const std::string &filepath, bool reporterror)
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
void ResourceListBuilder::ListDir(const std::string &path, const std::string &filepath, bool reporterror)
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

void ResourceListBuilder::BuildPakResourceList(const std::string &pakfilename)
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
			printf("Reading pakfile header failed. Wrong size (" SIZE_T_SPECIFIER " read, " SIZE_T_SPECIFIER " expected).\n", retval, pakheadersize);
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

	std::vector<fileinfo_s> filelist(filecount);
	retval = fread(filelist.data(), 1, pakheader.dirsize, pakfile);
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
		printf("Scanning pak file \"%s\" for resources (" SIZE_T_SPECIFIER " files in pak)\n", pakfilename.c_str(), filecount);
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
