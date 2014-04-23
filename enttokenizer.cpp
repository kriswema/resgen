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

#include "enttokenizer.h"
#include "util.h"

EntTokenizer::EntTokenizer(std::string& str_)
	: str(str_)
	, currentPtr(&str[0])
	, strEnd(currentPtr + str.length())
	, bInBlock(false)
	, blocksRead(0)
	, keyLength(0)
	, valueLength(0)
{
}

int EntTokenizer::GetCharNum() const
{
	if(!currentPtr)
	{
		return -1;
	}

	return currentPtr - &str[0];
}

int EntTokenizer::GetNumBlocksRead() const
{
	return blocksRead;
}

ptrdiff_t EntTokenizer::GetLatestKeyLength() const
{
	return keyLength;
}

ptrdiff_t EntTokenizer::GetLatestValueLength() const
{
	return valueLength;
}

const EntTokenizer::KeyValuePair* EntTokenizer::NextPair()
{
	// Have we finished parsing?
	if(!currentPtr)
	{
		return NULL;
	}

	// Skip end of line
	if(!NextKV())
	{
		// If we've run out key-values, make sure we are in an accept state
		if(bInBlock)
		{
			throw ParseException("Reached end of entity data while inside a block.", GetCharNum());
		}

		return NULL;
	}

	// Read key
	pair.first = currentPtr;

	if(!Next())
	{
		throw ParseException("Failed to parse key of key-value pair.", GetCharNum());
	}
	// currentPtr points to char after NUL
	keyLength = (currentPtr ? currentPtr - 1 : strEnd) - pair.first;

	// Ignore space between key/value
	ParseKVSeparator();

	pair.second = currentPtr;

	if(!Next())
	{
		throw ParseException("Failed to parse value of key-value pair.", GetCharNum());
	}
	// currentPtr points to char after NUL
	valueLength = (currentPtr ? currentPtr - 1 : strEnd) - pair.second;

	return &pair;
}

bool EntTokenizer::Next()
{
	if(!currentPtr)
	{
		return false;
	}

	while(currentPtr != strEnd)
	{
		if(*currentPtr == '\"')
		{
			*currentPtr = 0;
			currentPtr++;
			return true;
		}

		currentPtr++;
	}

	throw ParseException("Found end of data while parsing string");
}

void EntTokenizer::ParseKVSeparator()
{
	while(currentPtr != strEnd)
	{
		if(*currentPtr == '\"')
		{
			*currentPtr = 0;
			currentPtr++;
			return;
		}
		else if(*currentPtr != ' ')
		{
			throw ParseException("Found non-whitespace between key and value.", GetCharNum());
		}

		currentPtr++;
	}

	throw ParseException("Failed to parse key-value pair", GetCharNum());
}

bool EntTokenizer::NextKV()
{
    while(currentPtr != strEnd)
    {
		switch(*currentPtr)
		{
			case '\"':
				*currentPtr = 0;
				currentPtr++;
				return true;
			case '{':
				if(bInBlock)
				{
					throw ParseException("Unexpected start of block.", GetCharNum());
				}
				bInBlock = true;
				break;
			case '}':
				if(!bInBlock)
				{
					throw ParseException("Unexpected end of block.", GetCharNum());
				}
				bInBlock = false;
				blocksRead++;
				break;
			default:
				break;
		}

		currentPtr++;
    }

    currentPtr = NULL;
    return false;
}


