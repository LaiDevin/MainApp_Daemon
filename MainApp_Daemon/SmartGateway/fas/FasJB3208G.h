#ifndef __FAS_JB_3208G_H__
#define __FAS_JB_3208G_H__

#include "Fas.h"
#include "Common.h"

class FasJB3208G : public Fas
{
public:
	  FasJB3208G(void);
    ~FasJB3208G(void);
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

#endif //__FAS_JB_3208G_H__
