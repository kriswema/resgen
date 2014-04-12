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

// vstring.h: interface for the VString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VSTRING_H__771580F0_7F31_4F1E_9D11_1270C84C03F9__INCLUDED_)
#define AFX_VSTRING_H__771580F0_7F31_4F1E_9D11_1270C84C03F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

typedef const char *LPCSTR;

// Define 'VS_NO_SHRINK' if you don't want to have memory reallocated
// as much as normally would happen. Please note that this means
// memory use will go up.
#define VS_NO_SHRINK

// Define 'VS_OPCOMP_NOCASE' if you want the comparison operators to
// work case insensitive. Otherwise they will work case sensitive.
#define VS_OPCOMP_NOCASE

class VString
{
public:
	bool LoadFromFile(const char *filename);
	void StrRplChr(const char find, const char replace);
	int StrChr(char search, int start = 0) const;
	int StrRChr(char search, int start = -1);
	int CompareReverseLimitNoCase(const char *dst, int limit) const;
	void Trim(char *string);
	void Trim(char chr);
	void Trim();
	void TrimRight(char *string);
	void TrimRight(char chr);
	void TrimRight();
	void TrimLeft(char *string);
	void TrimLeft(char chr);
	void TrimLeft();
	void MakeLower();
	VString Right(int count) const;
	VString Left(int count) const;
	VString Mid(int index) const;
	VString Mid(int index, int count) const;
	int CompareNoCase(const char* string) const;
	int CompareLimit(const char *string, int limit) const;
	const VString& operator+=(const char *string);
	const VString& operator+=(const VString& string);
	friend VString operator+(const char *string1, const VString& string2);
	friend VString operator+(const VString& string1, const char *string2);
	friend VString operator+(const VString& string1, const VString& string2);
	const VString& operator=(const char *string);
	const VString& operator=(const VString& stringSrc);
	void SetAt(int index, char ch);
	char operator [](int index) const;
	char GetAt(int index) const;
	void Empty();
	int GetLength() const;
	operator LPCSTR() const;
	VString (const char *string);
	VString (const char* string, int len);
	VString (const VString& stringSrc);
	VString();
	virtual ~VString();

private:
	int length;
	int memuse;
	char * data;
	void MemError(const char *error);

protected:
	void Cat(const char *string, int len);
	void AddTwoStr(const char *str1, int str1len, const char *str2, int str2len);
};

// compare helper functions
bool operator==(const VString& s1, const VString& s2);
bool operator==(const VString& s1, LPCSTR s2);
bool operator==(LPCSTR s1, const VString& s2);
bool operator!=(const VString& s1, const VString& s2);
bool operator!=(const VString& s1, LPCSTR s2);
bool operator!=(LPCSTR s1, const VString& s2);
bool operator<(const VString& s1, const VString& s2);
bool operator<(const VString& s1, LPCSTR s2);
bool operator<(LPCSTR s1, const VString& s2);
bool operator>(const VString& s1, const VString& s2);
bool operator>(const VString& s1, LPCSTR s2);
bool operator>(LPCSTR s1, const VString& s2);
bool operator<=(const VString& s1, const VString& s2);
bool operator<=(const VString& s1, LPCSTR s2);
bool operator<=(LPCSTR s1, const VString& s2);
bool operator>=(const VString& s1, const VString& s2);
bool operator>=(const VString& s1, LPCSTR s2);
bool operator>=(LPCSTR s1, const VString& s2);


#endif // !defined(AFX_VSTRING_H__771580F0_7F31_4F1E_9D11_1270C84C03F9__INCLUDED_)
