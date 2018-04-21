#ifndef FASTAIHEAN_TX3607_H
#define FASTAIHEAN_TX3607_H
#include"Fas.h"
#include"Common.h"

class FasTaihean_TX3607 : public Fas
{
public:
    FasTaihean_TX3607();
    ~FasTaihean_TX3607();

    bool init(int nType,
              const char *sComDevName,
              int nBitrate = COM_BITRATE_28800,
              int nParity = COM_PARITY_NONE,
              int nDatabits = COM_DATABITS_8,
              int nStopbits = COM_STOPBITS_1P5,
              int nFlowCtrl = FLOW_CONTROL_NONE);
    bool unInit(void);
    int recvData(int nComType, vector<PartRunStatus> &stPartStatus);
    int sendData(int nComType/*zdst define data*/);

private:
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char *buffer, int nLen);
};

#endif // FASTAIHEAN_TX3607_H 泰和安TX3607
