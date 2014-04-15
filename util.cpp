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

#include "util.h"

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
    fileHandle = fopen(fileName.c_str(), mode.c_str());
}

File::operator FILE*()
{
    return fileHandle;
}

FILE* File::operator->()
{
    return fileHandle;
}


int strrnicmp(const char *src, const char *dst, int limit)
{
	// derived from VString function
	// first determine our maximum running length
	int i = strlen(src) - 1;
	int j = strlen(dst) - 1;

	limit--;

	if (i < limit)
	{
        if (i != j)
        {
            return i - j;
        }

		limit = i;
	}
	if (j < limit)
	{
        if (i != j)
        {
            return i - j;
        }

		limit = j;
	}

	src = src + i;
	dst = dst + j;

	while (limit >= 0)
	{
		i = tolower(*src);
		j = tolower(*dst);

        if (i != j)
        {
            return i - j;
        }

		src--;
		dst--;
		limit--;
	}

	return 0;
}

std::string replaceCharAll(std::string str, const char find, const char replace)
{
	std::string result = str;
	std::replace(result.begin(), result.end(), find, replace);
	return result;
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

	return strLC.compare(str.length() - ending.length(), ending.length(), endingLC);
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
    str.erase(str.find_first_not_of(trimmedChars) + 1);
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
