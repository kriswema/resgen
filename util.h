#ifndef UTIL_H
#define UTIL_H

#ifdef WIN32
#define SIZE_T_SPECIFIER    "%Iu"
#else
#define SIZE_T_SPECIFIER    "%zu"
#endif

#include <algorithm>
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

#ifdef WIN32
	bool keypress; // t
#else
	bool symlink; // t
#endif
};

template <char Delimiter>
class Tokenizer
{
public:
	Tokenizer(std::string& str_)
		: str(str_)
		, strLength(str.length())
		, nextPos(0)
	{
	}

	bool FindNext()
	{
		if(nextPos == std::string::npos)
		{
			return false;
		}

		while(nextPos != strLength)
		{
			if(str[nextPos] == Delimiter)
			{
				str[nextPos] = 0;
				nextPos++;
				return true;
			}

			nextPos++;
		}

		nextPos = std::string::npos;
		return true;
	}

	bool SkipToken()
	{
		return FindNext();
	}

	const char * NextToken()
	{
		if(nextPos == std::string::npos)
		{
			return NULL;
		}

		const size_t startPos = nextPos;

		FindNext();
		return &str.c_str()[startPos];
	}

	const char * NextValue()
	{
		// Goes to the next entity token (key->value or value->key)
		if (!SkipToken()) // exit key/value
		{
			return NULL;
		}

		return NextToken(); // enter key/value
	}

	bool SkipValue()
	{
		return SkipToken() && SkipToken();
	}

private:
	Tokenizer(const Tokenizer &other);
	Tokenizer& operator=(const Tokenizer &other);

	std::string str;
	const size_t strLength;
	size_t nextPos;
};


std::string replaceCharAll(std::string str, const char find, const char replace);

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
