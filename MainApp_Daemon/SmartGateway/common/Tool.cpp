#include "Tool.h"
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <chrono>
#include <winsock.h>

int Tool::getTime(Time& stTime)
{
    auto tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *t = localtime(&tp);

    stTime.nYear = (unsigned)(t->tm_year + 1900 - 2000);
    stTime.nMonth = t->tm_mon + 1;
    stTime.nDay = t->tm_mday;
    stTime.nHour = t->tm_hour;
    stTime.nMin = t->tm_min;
    stTime.nSec = t->tm_sec;
	return 0;
}

void Tool::sleep_us(unsigned long us)
{
    Sleep(us/1000);
}

bool Tool::strncmp_d(const unsigned char *strSrc, const unsigned char *strDest, int nLen)
{
	if(NULL == strSrc || NULL == strDest || nLen <= 0){
		return false;
	}

	int nIndex = 0;
	while(nIndex < nLen){
		if(strSrc[nIndex] != strDest[nIndex]) {
			return false;
		}
		++ nIndex;
	}

	return true;
}

//#pragma comment(lib, "ws2_32.lib")
char* Tool::getIpByDomain(const char *domain, char *ip)
{
    in_addr addr;
    hostent *ent = nullptr;

    ent = ::gethostbyname(domain);
    if (nullptr != ent) {
        memmove(&addr, ent->h_addr_list[0], 4);
        strncpy(ip, inet_ntoa(addr), strlen(inet_ntoa(addr)));
    }
    return ip;
}
