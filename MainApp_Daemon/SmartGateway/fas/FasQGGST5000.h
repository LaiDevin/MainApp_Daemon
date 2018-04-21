#ifndef __FAS_QG_GST5000_H__
#define __FAS_QG_GST5000_H__

#include "Fas.h"
#include "Common.h"

class FasQGGST5000 : public Fas
{
public:
  	FasQGGST5000(void);
  	~FasQGGST5000(void);
    bool init(int nType,
              const char *sComDevName,
              int nBitrate = COM_BITRATE_9600,
              int nParity = COM_PARITY_NONE,
              int nDatabits = COM_DATABITS_8,
              int nStopbits = COM_STOPBITS_2,
              int nFlowCtrl = FLOW_CONTROL_NONE);
  	bool unInit(void);
  	int recvData(int nComType, vector<PartRunStatus> &vtPartStatus);
  	int sendData(int nComType/*zdst define data*/);

private:
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen);
};

#endif //__FAS_QG_GST5000_H__