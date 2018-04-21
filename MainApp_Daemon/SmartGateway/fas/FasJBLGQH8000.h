#ifndef FASJBLGQH8000_H
#define FASJBLGQH8000_H
#include "Fas.h"
#include "Common.h"

class FasJBLGQH8000 : public Fas
{
public:
    FasJBLGQH8000();
    ~FasJBLGQH8000(void);
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

#endif // FASJBLGQH8000_H
