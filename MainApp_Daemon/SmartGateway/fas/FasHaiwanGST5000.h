#ifndef FASHAIWANGST5000_H
#define FASHAIWANGST5000_H
#include "Fas.h"

class FasHaiwanGST5000:public Fas
{
public:
    FasHaiwanGST5000();
    virtual ~FasHaiwanGST5000();
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

#endif // FASHAIWANGST5000_H
