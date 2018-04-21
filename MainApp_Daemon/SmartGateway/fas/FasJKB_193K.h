#ifndef FASJKB_193K_H
#define FASJKB_193K_H

#include "Fas.h"
#include "Common.h"

class FasJKB_193K : public Fas
{
public:
    FasJKB_193K();
    ~FasJKB_193K(void);
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

#endif // FASJKB_193K_H
