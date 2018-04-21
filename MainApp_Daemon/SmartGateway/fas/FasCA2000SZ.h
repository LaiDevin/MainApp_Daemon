#ifndef __FAS_CA_2000_SZ_H__
#define __FAS_CA_2000_SZ_H__

#include "Fas.h"
#include "Common.h"

class FasCA2000SZ: public Fas
{
public:
	FasCA2000SZ(void);
  ~FasCA2000SZ(void);
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

#endif //__FAS_CA_2000_SZ_H__ 成都安吉斯CA2000SZ

