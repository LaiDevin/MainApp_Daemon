#ifndef __TOOL_H__
#define __TOOL_H__
#include "Common.h"

class Tool
{
public:
	Tool() {};
	~Tool() {};

	static int getTime(Time& stTime);
	static void sleep_us(unsigned long us);
    static bool strncmp_d(const unsigned char* strSrc, const unsigned char* strDest, int nLen);
    static char* getIpByDomain(const char *domain, char* ip);
};

#endif
