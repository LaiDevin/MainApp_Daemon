#ifndef FAS_JB_SJFF3208_H
#define FAS_JB_SJFF3208_H
#include "Fas.h"
#include "Common.h"

class FAS_JB_SJFF3208 : public Fas
{
public:
    FAS_JB_SJFF3208();
    ~FAS_JB_SJFF3208(void);
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
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen);
};

#endif // FAS_JB_SJFF3208_H
