#ifndef UTIL_H
#define UTIL_H

#ifdef _WIN32
#define SIZE_T_SPECIFIER    "%Iu"
#else
#define SIZE_T_SPECIFIER    "%zu"
#endif

#ifdef _WIN32
#define INLINE __forceinline
#else
#define INLINE inline
#endif

#ifdef _WIN32
const char PATH_SEPARATOR = '\\';
#else
const char PATH_SEPARATOR = '/';
#endif

#include <algorithm>
#include <cstddef>
#include <exception>
#include <string>
#include <vector>

// RAII wrapper for FILE
class File
{
public:
	File();
	File(const char* fileName, const std::string& mode);
	File(const std::string& fileName, const std::string& mode);
	~File();

	void close();
	void open(const std::string& fileName, const std::string& mode);

	operator FILE*();
	FILE* operator->();

private:
	FILE* fileHandle;
};

struct file_s;

struct config_s
{
	bool verbal; // t -
	bool statusline; // f
	bool overwrite; // f
	bool tolower; // f
	bool matchcase; // f
	bool parseresource; // f
	bool preservewads; // f
	bool contentdisp; // f

	bool searchdisp; // f

	bool help; // f
	bool credits ; // f
	bool warranty; // f

	bool resourcedisp; // f

	std::vector<file_s> files;
	std::vector<file_s> excludes; // Map exclude list - not resource!
	std::vector<std::string> excludelists; // Exclude resource list files - not maps!

	std::string rfafile;

	bool checkpak; // t
	std::string resource_path;

#ifdef _WIN32
	bool keypress; // t
#else
	bool symlink; // t
#endif
};

class ParseException : public std::runtime_error
{
public:
	ParseException(const char* message)
		: std::runtime_error(message)
		, charNum(-1)
	{
	}

	ParseException(const char* message, int charNum_)
		: std::runtime_error(message)
		, charNum(charNum_)
	{
	}

	int GetCharNum() const
	{
		return charNum;
	}

private:
	int charNum;
};

template <char Delimiter>
class Tokenizer
{
public:
	Tokenizer(std::string& str_)
		: str(str_)
		, currentPtr(&str[0])
		, strEnd(currentPtr + str.length())
	{
	}

	INLINE bool Next()
	{
		if(!currentPtr)
		{
			return false;
		}

		while(currentPtr != strEnd)
		{
			if(*currentPtr == Delimiter)
			{
				*currentPtr = 0;
				currentPtr++;
				return true;
			}

			currentPtr++;
		}

		throw ParseException("Found end of data while parsing string");
	}

protected:
	Tokenizer(const Tokenizer &other);
	Tokenizer& operator=(const Tokenizer &other);

	std::string str;

	// Current position in string
	char* currentPtr;

	// Pointer to char after last
	char* strEnd;
};

class EntTokenizer : public Tokenizer<'\"'>
{
public:
	typedef std::pair<const char*, const char*> KeyValuePair;

	EntTokenizer(std::string& str_)
		: Tokenizer(str_)
		, bInBlock(false)
		, blocksRead(0)
	{
	}

	int GetCharNum() const
	{
		if(!currentPtr)
		{
			return -1;
		}

		return currentPtr - &str[0];
	}

	int GetNumBlocksRead() const
	{
		return blocksRead;
	}

	ptrdiff_t GetLatestValueLength() const
	{
		// currentPtr points to char after NUL
		const char* const end = (currentPtr ? currentPtr - 1 : strEnd);
		return end - pair.second;
	}

	const KeyValuePair* NextPair()
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

		// Ignore space between key/value
		ParseKVSeparator();

		pair.second = currentPtr;

		if(!Next())
		{
			throw ParseException("Failed to parse value of key-value pair.", GetCharNum());
		}

		return &pair;
	}

private:
	void ParseKVSeparator()
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

	bool NextKV()
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

	KeyValuePair pair;
	bool bInBlock;
	int blocksRead;
};


void splitPath(const std::string &fullPath, std::string &baseFolder, std::string &baseFileName);

bool fileExists(const std::string &fileName);

std::string replaceCharAllCopy(const std::string &str, const char find, const char replace);
void replaceCharAll(std::string &str, const char find, const char replace);

std::string strToLowerCopy(const std::string &str);

void strToLower(std::string &str);

int CompareStrEndNoCase(const std::string &str, const std::string &ending);
int CompareStrEnd(const std::string &str, const std::string &ending);

void leftTrim(std::string &str);
void leftTrim(std::string &str, const std::string &trimmedChars);

void rightTrim(std::string &str);
void rightTrim(std::string &str, const std::string &trimmedChars);

bool readFile(const std::string &filename, std::string &outStr);

int ICompareStrings(const std::string &a, const std::string &b);

std::string BuildValvePath(const std::string &respath);
void EndWithPathSep(std::string &str);

#endif // UTIL_H
