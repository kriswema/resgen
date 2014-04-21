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

#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
#else
#include <glob.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include "util.h"

File::File()
    : fileHandle(NULL)
{
}

File::File(const char* const fileName, const std::string& mode)
    : fileHandle(NULL)
{
    open(fileName, mode);
}

File::File(const std::string& fileName, const std::string& mode)
    : fileHandle(NULL)
{
    open(fileName, mode);
}

File::~File()
{
    close();
}

void File::close()
{
    if(fileHandle != NULL)
    {
        fclose(fileHandle);
        fileHandle = NULL;
    }
}

void File::open(const std::string& fileName, const std::string& mode)
{
    close();
#ifdef WIN32
    if(fopen_s(&fileHandle, fileName.c_str(), mode.c_str()))
    {
        fileHandle = NULL;
    }
#else
    fileHandle = fopen(fileName.c_str(), mode.c_str());
#endif
}

File::operator FILE*()
{
    return fileHandle;
}

FILE* File::operator->()
{
    return fileHandle;
}

void splitPath(const std::string &fullPath, std::string &baseFolder, std::string &baseFileName)
{
    size_t lastSlashIndex = fullPath.rfind('/'); // Linux style path
    if (lastSlashIndex == std::string::npos)
    {
	lastSlashIndex = fullPath.rfind('\\'); // windows style path
    }

    // folder, including trailing /
    if (lastSlashIndex == std::string::npos)
    {
	#ifdef WIN32
	baseFolder = ".\\";
	#else
	baseFolder = "./";
	#endif
    }
    else
    {
	baseFolder = fullPath.substr(0, lastSlashIndex + 1);
    }
    const size_t basenameStart =
	(lastSlashIndex != std::string::npos) ? lastSlashIndex + 1 : 0;

    // Subtract 4 for the '.bsp' extension
    baseFileName = fullPath.substr(basenameStart, fullPath.length() - basenameStart - 4);
}

bool fileExists(const std::string &fileName)
{
#ifdef WIN32
    WIN32_FIND_DATA filedata;
    HANDLE filehandle = FindFirstFile(fileName.c_str(), &filedata);
    if (filehandle != INVALID_HANDLE_VALUE)
    {
	FindClose(filehandle);
	return true;
    }
#else
    // We use glob to find the file in Linux.
    // Please note that params are NOT expanded (tilde will be).
    // So it might not work properly in cases where params are used
    // The glob man page tell us to use wordexp for expansion.. However, that function does not exist.
    glob_t globbuf;
    if (!glob(fileName.c_str(), GLOB_TILDE, NULL, &globbuf))
    {
	globfree(&globbuf);
	return true;
    }
#endif

    return false;
}

std::string replaceCharAllCopy(const std::string &str, const char find, const char replace)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), find, replace);
    return result;
}

void replaceCharAll(std::string &str, const char find, const char replace)
{
    std::replace(str.begin(), str.end(), find, replace);
}

std::string strToLowerCopy(const std::string &str)
{
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void strToLower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

int CompareStrEndNoCase(const std::string &str, const std::string &ending)
{
    if(str.length() < ending.length())
    {
        return -1;
    }

    std::string strLC = strToLowerCopy(str);
    std::string endingLC = strToLowerCopy(ending);

    return CompareStrEnd(strLC, endingLC);
}

int CompareStrEnd(const std::string &str, const std::string &ending)
{
    const size_t strLength = str.length();
    const size_t endingLength = ending.length();

    if(strLength < endingLength)
    {
        return -1;
    }

    return str.compare(strLength - endingLength, endingLength, ending);
}

void leftTrim(std::string &str)
{
    leftTrim(str, " \n\r\t");
}

void leftTrim(std::string &str, const std::string &trimmedChars)
{
    str.erase(0, str.find_first_not_of(trimmedChars));
}

void rightTrim(std::string &str)
{
    rightTrim(str, " \n\r\t");
}

void rightTrim(std::string &str, const std::string &trimmedChars)
{
    str.erase(str.find_last_not_of(trimmedChars) + 1);
}

bool readFile(const std::string &filename, std::string &outStr)
{
    std::ifstream f(filename.c_str());

    if(!f.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << f.rdbuf();
    outStr = buffer.str();
    return true;
}

int ICompareStrings(const std::string &a, const std::string &b)
{
    return strToLowerCopy(a).compare(strToLowerCopy(b));
}

std::string BuildValvePath(const std::string &respath)
{
	// Check the respath and check ../valve if the respath doesn't point to valve

	#ifdef WIN32
	const char* pathSep = "\\";
	const char* valveStr = "\\valve\\";
	#else
	const char* pathSep = "/";
	const char* valveStr = "/valve/";
	#endif

	if(CompareStrEndNoCase(respath, valveStr))
	{
		// NOT valve dir, so check it too
		size_t slashpos = respath.rfind(pathSep[0], respath.length() - 2);
		if (slashpos != std::string::npos)
		{
			return respath.substr(0, slashpos) + valveStr;
		}
		else
		{
			return respath + ".." + valveStr;
		}
	}

	return "";
}

void EndWithPathSep(std::string &str)
{
	#ifdef WIN32
	const char pathSep = '\\';
	#else
	const char pathSep = '/';
	#endif
	
	if (str[str.length() - 1] != pathSep)
	{
		// No path separator, add
		str += pathSep;
	}
}
