#ifndef __FAS_MANAGER_H__
#define __FAS_MANAGER_H__

#ifdef LINUX
#include "GPIO.h"
#endif
#include "Fas.h"
#include <stdio.h>
#include <vector>
using namespace std;

struct FasComInfo
{
	int nID;
	int nComType;
	char strComName[64];
	int nBitrate;
	int nParity;
	int nDatabits;
	int nStopbits;
	int nFlowCtrl;
};

struct FasInfo
{
    int nID;
    char strName[64];
};

class FasManager {
public:
	static FasManager *getInstance(void);
	bool init(const vector<FasComInfo>& stFasList);
	bool unInit(void);
	int getFasName(int nFasID, char *strName);
	int recvData(int nFasID, int nComType, vector<PartRunStatus> &stPartStatus);
	int sendData(int nFasID, int nComType/*zdst define data*/);
    int getFasList(vector<FasInfo>& stFasInfoList);
#ifdef LINUX
    void setGPIOOperator(GPIO *pobjGPIO = NULL);
#endif
private:
	Fas *getFasObj(int nFasID);

private:
	FasManager(void);
	~FasManager(void);
	class Release {
		public:
			Release(void){};
			~Release(void){
				if(FasManager::m_pobjFasManager != NULL){
					delete FasManager::m_pobjFasManager;
					FasManager::m_pobjFasManager = NULL;
				}
			};
	};
	static Release m_objRelease;
	static FasManager *m_pobjFasManager;

#ifdef LINUX
private:
	GPIO *m_pobjGPIO;
#endif
};
#endif
