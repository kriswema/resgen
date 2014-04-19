#ifndef UTIL_H
#define UTIL_H

#ifdef WIN32
#define SIZE_T_SPECIFIER    "%Iu"
#else
#define SIZE_T_SPECIFIER    "%zu"
#endif

#include <algorithm>
#include <string>

// RAII wrapper for FILE
class File
{
public:
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

bool StringLessThanNoCase(const std::string &a, const std::string &b);

struct StringKey
{
	StringKey(const std::string &str_)
		: str(str_)
		, lower(strToLowerCopy(str_))
	{
	}

	std::string str;
	std::string lower;
};

struct SortByLower : public std::binary_function<StringKey, StringKey, bool>
{
	bool operator()(const StringKey &a, const StringKey &b)
	{
		return a.lower < b.lower;
	}
};

bool StringKeyLessThan(const StringKey &a, const StringKey &b);

#endif // UTIL_H
