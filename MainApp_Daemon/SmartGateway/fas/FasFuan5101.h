#ifndef __FAS_FUAN_5101_H__
#define __FAS_FUAN_5101_H__

#include "Fas.h"
#include "Common.h"

class FasFuan5101 : public Fas
{
public:
  	FasFuan5101(void);
  	~FasFuan5101(void);
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
    void reply(Com *pobjCom, unsigned short addr, bool bStatus);
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen);
};

#endif //__FAS_FUAN_5101_H__
