#ifndef UTIL_H
#define UTIL_H

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

#endif // UTIL_H
