#ifndef __FAS_JB_TG_JBF_11SF_H__
#define __FAS_JB_TG_JBF_11SF_H__

#include "Fas.h"
#include "Common.h"


class Fas_JB_TG_JBF_11SF : public Fas
{
public:
  	Fas_JB_TG_JBF_11SF(void);
  	~Fas_JB_TG_JBF_11SF(void);
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

#endif //__FAS_JB_TG_JBF_11SF_H__