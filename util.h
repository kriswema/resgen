#ifndef UTIL_H
#define UTIL_H

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

int strrnicmp(const char *src, const char *dst, int limit);

std::string replaceCharAll(std::string str, const char find, const char replace);

std::string strToLowerCopy(const std::string &str);

void strToLower(std::string &str);

int CompareStrEndNoCase(const std::string &str, const std::string &ending);

#endif // UTIL_H
