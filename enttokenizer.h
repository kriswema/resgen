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

#ifndef ENTTOKENIZER_H
#define ENTTOKENIZER_H

#include <cstddef>
#include <string>

class EntTokenizer
{
public:
	typedef std::pair<const char*, const char*> KeyValuePair;

	EntTokenizer(std::string& str_);

	int GetCharNum() const;

	int GetNumBlocksRead() const;

	ptrdiff_t GetLatestKeyLength() const;
	ptrdiff_t GetLatestValueLength() const;

	const KeyValuePair* NextPair();

protected:
	EntTokenizer(const EntTokenizer &other);
	EntTokenizer& operator=(const EntTokenizer &other);

	bool Next();

	void ParseKVSeparator();

	bool NextKV();

protected:
	std::string str;

	// Current position in string
	char* currentPtr;

	// Pointer to char after last
	char* strEnd;

	bool bInBlock;
	int blocksRead;

	KeyValuePair pair;

	ptrdiff_t keyLength;
	ptrdiff_t valueLength;
};


#endif
