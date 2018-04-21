#include "ServerAgreementGBTool.h"
#include <windows.h>

static int s_nBusinessNum = 0;
static int s_nHeartBeatBusinessNum = 1;
static HANDLE s_BusinessNumMutex;

int ServerAgreementGBTool::getBusinessNum(void)
{
    WaitForSingleObject(s_BusinessNumMutex, INFINITE);
	do {
		++ s_nBusinessNum;
	}while(s_nBusinessNum == s_nHeartBeatBusinessNum);
	
	int nBusinessNum = s_nBusinessNum;
    ReleaseMutex(s_BusinessNumMutex);

	return nBusinessNum;
}

int ServerAgreementGBTool::getHeartBeatBusinessNum(void)
{
	return s_nHeartBeatBusinessNum;
}
