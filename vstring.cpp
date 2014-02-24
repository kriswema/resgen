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

// vstring.cpp: implementation of the VString class.
//
//////////////////////////////////////////////////////////////////////

// RESGen does it's own security checks, no need to add VS2005's layer
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#include <windows.h>
#else
#define stricmp strcasecmp
#endif

#include "vstring.h"
#include "leakcheck.h"

#ifdef VS_OPCOMP_NOCASE
#define VS_OPCOMP stricmp
#else
#define VS_OPCOMP strcmp
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

VString::VString()
{
	memuse = 1024;
	length = 0;

	data = new char[memuse]; // Make a new string array
	if (data == NULL) { MemError("constructor"); }

	data[0] = 0; // terminating NULL
}

VString::~VString()
{
	if (data) { delete [] data; }
}

void VString::MemError(const char *error)
{
	printf("\n\nMemory allocation error (VString - %s).\n", error);
	printf("\nTry to free more memory on your computer\n\n");
	printf("If this fails contact resgen@hltools.com\n");
	exit(1);
}

VString::VString(const VString& stringSrc)
{
	length = stringSrc.GetLength();

	memuse = 1024 * (1 + (length / 1024));

	data = new char[memuse]; // Make a new string array
	if (data == NULL) { MemError("constructor"); }

	memcpy(data, stringSrc.data, length + 1); // Copy the input string
}

VString::VString(const char *string, int len)
{
	memuse = 1024 * (1 + (len / 1024));

	data = new char[memuse]; // Make a new string array
	if (data == NULL) { MemError("constructor"); }

	strncpy(data, string, len); // Copy the input string
	data[len] = 0; // terminating NULL

	length = strlen(data); // Get the string length
}

VString::VString(const char *string)
{
	length = strlen(string); // Get the string length
	memuse = 1024 * (1 + (length / 1024));

	data = new char[memuse]; // Make a new string array
	if (data == NULL) { MemError("constructor"); }

	memcpy(data, string, length + 1); // Copy the input string
}

VString::operator LPCSTR() const
{
	return data;
}

int VString::GetLength() const
{
	return length;
}

void VString::SetLength()
{
	length = strlen(data); // re-evaluate lenght
	return;
}

bool VString::IsEmpty() const
{
	if (length)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void VString::Empty()
{
	delete [] data;

	memuse = 1024;
	length = 0;

	data = new char[memuse]; // Make a new string array
	if (data == NULL) { MemError("Empty"); }

	data[0] = 0; // terminating NULL
}

char VString::GetAt(int index) const
{
	return data[index];
}

char VString::operator [](int index) const
{
	return data[index];
}

void VString::SetAt(int index, char ch)
{
	data[index] = ch;
	if (ch == 0)
	{
		// tried to shorten list... compensate
		length = index;
	}
}

const VString& VString::operator=(const VString& stringSrc)
{
	// Set to other VString data.
	length = stringSrc.GetLength();

#ifdef VS_NO_SHRINK
	if (memuse <= length)
	{
		// not enough memory... Reallocate
#endif
		memuse = 1024 * (1 + (length / 1024));

		delete [] data;
		data = new char[memuse]; // Make a new string array
		if (data == NULL) { MemError("Operator ="); }
#ifdef VS_NO_SHRINK
	}
#endif

	memcpy(data, stringSrc.data, length + 1); // Copy the input string

	return *this;
}

const VString& VString::operator=(const char *string)
{
	// Set to other VString data.
	length = strlen(string);

#ifdef VS_NO_SHRINK
	if (memuse <= length)
	{
		// not enough memory... Reallocate
#endif
		memuse = 1024 * (1 + (length / 1024));

		delete [] data;
		data = new char[memuse]; // Make a new string array
		if (data == NULL) { MemError("Operator ="); }
#ifdef VS_NO_SHRINK
	}
#endif

	memcpy(data, string, length + 1); // Copy the input string

	return *this;
}

void VString::AddTwoStr(const char *str1, int str1len, const char *str2, int str2len)
{
	// both strings should be null terminated
	// both lengths should match their source strings (or weird stuff WILL happen)
	length = str1len + str2len;

#ifdef VS_NO_SHRINK
	if (memuse <= length)
	{
		// not enough memory... Reallocate
#endif
		memuse = 1024 * (1 + (length / 1024));

		delete [] data;
		data = new char[memuse]; // Make a new string array
		if (data == NULL) { MemError("AddTwoStr"); }
#ifdef VS_NO_SHRINK
	}
#endif

	memcpy(data, str1, str1len + 1); // Copy input string1
	memcpy(&data[str1len], str2, str2len + 1); // Add string 2
}

VString operator+(const VString& string1, const VString& string2)
{
	VString s;
	s.AddTwoStr(string1.data, string1.length, string2.data, string2.length);
	return s;
}

VString operator+(const VString& string1, const char *string2)
{
	VString s;
	s.AddTwoStr(string1.data, string1.length, string2, strlen(string2));
	return s;
}

VString operator+(const char *string1, const VString& string2)
{
	VString s;
	s.AddTwoStr(string1, strlen(string1), string2.data, string2.length);
	return s;
}

void VString::Cat(const char *string, int len)
{
	// len MUST match the source string or weird stuff WILL happen
	int newlength = len + length;

	if (memuse <= newlength)
	{
		// not enough memory... Reallocate
		memuse = 1024 * (1 + (newlength / 1024));

		char *temp = new char[memuse]; // Make a new string array
		if (temp == NULL) { MemError("Cat"); }

		memcpy(temp, data, length);
		memcpy(&temp[length], string, len);
		temp[newlength] = 0; // terminating null

		delete [] data;
		data = temp;
	}
	else
	{
		memcpy(&data[length], string, len + 1);
	}

	length = newlength;
}

const VString& VString::operator +=(const VString &string)
{
	Cat(string, string.GetLength());
	return *this;
}

const VString& VString::operator +=(const char *string)
{
	Cat(string, strlen(string));
	return *this;
}

int VString::Compare(const char *string) const
{
	return strcmp(data, string);
}

bool operator==(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2)) { return false; } else { return true; }
}

bool operator==(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2)) { return false; } else { return true; }
}

bool operator==(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2)) { return false; } else { return true; }
}

bool operator!=(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2)) { return true; } else { return false; }
}

bool operator!=(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2)) { return true; } else { return false; }
}

bool operator!=(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2)) { return true; } else { return false; }
}

bool operator<(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) < 0) { return true; } else { return false; }
}

bool operator<(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2) < 0) { return true; } else { return false; }
}

bool operator<(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) < 0) { return true; } else { return false; }
}

bool operator>(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) > 0) { return true; } else { return false; }
}

bool operator>(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2) > 0) { return true; } else { return false; }
}

bool operator>(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) > 0) { return true; } else { return false; }
}

bool operator<=(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) <= 0) { return true; } else { return false; }
}

bool operator<=(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2) <= 0) { return true; } else { return false; }
}

bool operator<=(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) <= 0) { return true; } else { return false; }
}

bool operator>=(const VString& s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) >= 0) { return true; } else { return false; }
}

bool operator>=(const VString& s1, LPCSTR s2)
{
	if (VS_OPCOMP(s1, s2) >= 0) { return true; } else { return false; }
}

bool operator>=(LPCSTR s1, const VString& s2)
{
	if (VS_OPCOMP(s1, s2) >= 0) { return true; } else { return false; }
}

int VString::CompareLimit(const char *string, int limit) const
{
	return strncmp(data, string, limit);
}

int VString::CompareNoCase(const char *string) const
{
	return stricmp(data, string);
}

VString VString::Mid(int index, int count) const
{
	// make sure we return something sensible
	if (index < 0) { index = 0; }
	if (index + count > length) { count = length - index; }

	if (index >= length || count <= 0)
	{
		// this will result in an empty string.
		VString retstr("");
		return retstr;
	}

	if (index == 0 && count == length)
	{
		// They want the entire string...
		return *this;
	}

	char *tmp = new char [count + 1];
	memcpy(tmp, &data[index], count);
	tmp[count] = 0; // terminating null
	VString retstr(tmp);
	delete [] tmp;
	return retstr;
}

VString VString::Mid(int index) const
{
	return Mid(index, length - index);
}

VString VString::Left(int count) const
{
	if (count <= 0)
	{
		VString retstr("");
		return retstr;
	}
	if (count >= length)
	{
		return *this;
	}

	char *tmp = new char [count + 1];
	memcpy(tmp, data, count);
	tmp[count] = 0; // terminating null
	VString retstr(tmp);
	delete [] tmp;
	return retstr;
}

VString VString::Right(int count) const
{
	if (count <= 0)
	{
		VString retstr("");
		return retstr;
	}
	if (count >= length)
	{
		return *this;
	}

	char *tmp = new char [count + 1];
	memcpy(tmp, &data[length - count], count + 1);
	VString retstr(tmp);
	delete [] tmp;
	return retstr;

}

void VString::MakeUpper()
{
	#ifdef WIN32
	_strupr(data);
	#else
	// Linux doesn't have strupr/strlwr
	int i;

	for (i=0; i < length; i++)
	{
		data[i] = toupper(data[i]);
	}
	#endif
}

void VString::MakeLower()
{
	#ifdef WIN32
	_strlwr(data);
	#else
	// Linux doesn't have strupr/strlwr
	int i;

	for (i=0; i < length; i++)
	{
		data[i] = tolower(data[i]);
	}
	#endif
}

void VString::TrimLeft()
{
	// search until no whitespace char is found
	char *cptr = data;

	while (*cptr != 0) // do till terminating NULL
	{
		if (isspace(*cptr))
		{
			// it's a whitespace
			cptr++; // increase pointer.
		}
		else
		{
			// current cptr points to first non-whitespace char
			break;
		}
	}

	if (cptr != data)
	{
		// move string forward
		memmove(data, cptr, length - (cptr - data) + 1);
		length -= (int)(cptr-data);
	}
}

void VString::TrimLeft(char chr)
{
	// search until no 'chr' is found
	char *cptr = data;

	while (*cptr != 0) // do till terminating NULL
	{
		if (*cptr == chr)
		{
			// it's a 'chr'
			cptr++; // increase pointer.
		}
		else
		{
			// current cptr points to first non-'chr' char
			break;
		}
	}

	if (cptr != data)
	{
		// move string forward
		memmove(data, cptr, length - (cptr - data) + 1);
		length -= (int)(cptr-data);
	}
}

void VString::TrimLeft(char *string)
{
	// search until none of the chars in 'string' is found
	char *cptr = data;

	while (*cptr != 0) // do till terminating NULL
	{
		if (strchr(string, *cptr))
		{
			// it's a char from 'string'
			cptr++; // increase pointer.
		}
		else
		{
			// current cptr points to first non-'string' char
			break;
		}
	}

	if (cptr != data)
	{
		// move string forward
		memmove(data, cptr, length - (cptr - data) + 1);
		length -= (int)(cptr-data);
	}

}

void VString::TrimRight()
{
	// search until no whitespace char is found
	int i;

	for (i = length-1; i >= 0; i--) // do till string start
	{
		if (!isspace(data[i]))
		{
			// i points to first non-whitespace char
			break;
		}
	}

	if (i != length-1)
	{
		// set terminating null on place of last whitespace
		data[i + 1] = 0;
		length = i + 1;
	}
}

void VString::TrimRight(char chr)
{
	// search until no 'chr' char is found
	int i;

	for (i = length-1; i >= 0; i--) // do till string start
	{
		if (data[i] != chr)
		{
			// i points to first non-'chr' char
			break;
		}
	}

	if (i != length-1)
	{
		// set terminating null on place of last whitespace
		data[i + 1] = 0;
		length = i + 1;
	}
}

void VString::TrimRight(char *string)
{
	// search until no 'chr' char is found
	int i;

	for (i = length-1; i >= 0; i--) // do till string start
	{
		if (!strchr(string, data[i]))
		{
			// i points to first non-'chr' char
			break;
		}
	}

	if (i != length-1)
	{
		// set terminating null on place of last whitespace
		data[i + 1] = 0;
		length = i + 1;
	}
}

void VString::Trim()
{
	TrimLeft();
	TrimRight();
}

void VString::Trim(char chr)
{
	TrimLeft(chr);
	TrimRight(chr);
}

void VString::Trim(char *string)
{
	TrimLeft(string);
	TrimRight(string);
}

int VString::CompareReverseLimitNoCase(const char *dst, int limit) const
{
	// first determine our maximum running length
	char *src = data;
	int i = length - 1;
	int j = strlen(dst) - 1;
	int ret = 0;

	limit--;

	if (i < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = i;
	}
	if (j < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = j;
	}

	src = src + i;
	dst = dst + j;

	while (limit >= 0)
	{
		i = tolower(*src);
		j = tolower(*dst);

		ret = i - j;
		if (ret)
		{
			if (ret < 0)
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
		src--;
		dst--;
		limit--;
	}

	return 0;
}

int VString::CompareReverseLimit(const char *dst, int limit) const
{
	// first determine our maximum running length
	char *src = data;
	int i = length - 1;
	int j = strlen(dst) - 1;
	int ret = 0;

	limit--;

	if (i < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = i;
	}
	if (j < limit)
	{
		if (i < j)
		{
			return -1;
		}
		else if (j < i)
		{
			return 1;
		}
		limit = j;
	}

	src = src + i;
	dst = dst + j;

	while (limit >= 0)
	{
		ret = *src - *dst;
		if (ret)
		{
			if (ret < 0)
			{
				return -1;
			}
			else
			{
				return 1;
			}
		}
		src--;
		dst--;
		limit--;
	}

	return 0;
}

int VString::StrRChr(char search, int start)
{
	char *ret;

	if (start == -1 || start >= length)
	{
		ret = strrchr(data, search);
	}
	else
	{
		int i;
		ret = NULL;
		for (i = start; i >= 0; i--)
		{
			if (data[i] == search)
			{
				// found the char
				ret = &data[i];
				break;
			}
		}
	}

	if (ret == NULL)
	{
		return -1;
	}
	else
	{
		return ret - data;
	}
}

int VString::StrChr(char search, int start) const
{
	char *ret;
	ret = strchr(&data[start], search);
	if (ret == NULL)
	{
		return -1;
	}
	else
	{
		return ret - data;
	}
}

void VString::StrRplChr(const char find, const char replace)
{
	// Replaces the found character with the replace character
	char *string = data;

	string = strchr(string, find);
	while(string) // find char
	{
		*string = replace; // replace char
		string++;

		string = strchr(string, find);
	}
}

bool VString::LoadFromFile(const char *filename)
{
	FILE *f;

	f = fopen(filename, "r"); // Open for reading

	if (f == NULL)
	{
		return false;
	}

	fseek(f, 0, SEEK_END); // Move to end of file
	length = ftell(f) + 1; // Get length
	fseek(f, 0, SEEK_SET); // Resturn to start of file.

	if (length == 0)
	{
		data[0] = 0;
		return true; // empty file, so we are successful
	}

#ifdef VS_NO_SHRINK
	if (memuse <= length)
	{
		// not enough memory... Reallocate
#endif
		memuse = 1024 * (1 + (length / 1024));

		delete [] data;
		data = new char[memuse]; // Make a new string array
		if (data == NULL) { MemError("LoadFromFile"); }
#ifdef VS_NO_SHRINK
	}
#endif

	// load file into datastring
	size_t readlen = fread(data, 1, length, f);
	if (readlen != (size_t)length)
	{
		// might be error
		if (ferror(f))
		{
			// Stream error
			data[0] = 0; // empty string to be sure
			length = 0;
			return false;
		}
		else
		{
			length = readlen; // adjust length
		}
	}

	data[length] = 0; // Add terminating null

	fclose(f);

	return true;
}
