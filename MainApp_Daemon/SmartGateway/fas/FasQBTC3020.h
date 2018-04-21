#ifndef __FAS_QB_TC3020_H__
#define __FAS_QB_TC3020_H__

#include "Fas.h"
#include "Common.h"

class FasQBTC3020: public Fas
{
public:
  	FasQBTC3020(void);
    ~FasQBTC3020(void);
    bool init(int nType,
              const char *sComDevName,
              int nBitrate = COM_BITRATE_9600,
              int nParity = COM_PARITY_NONE,
              int nDatabits = COM_DATABITS_8,
              int nStopbits = COM_STOPBITS_2,
              int nFlowCtrl = FLOW_CONTROL_NONE);
  	bool unInit(void);
    int recvData(int nComType, vector<PartRunStatus> &stPartStatus);
  	int sendData(int nComType/*zdst define data*/);

private:
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen);
};

#endif //__FAS_QB_TC3020_H__ 营口天成
