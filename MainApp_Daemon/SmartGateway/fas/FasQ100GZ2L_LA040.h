#ifndef __FAS_Q100GZ2L_LA040_H__
#define __FAS_Q100GZ2L_LA040_H__

#include "Fas.h"
#include "Common.h"

class FasQ100GZ2L_LA040: public Fas
{
public:
    FasQ100GZ2L_LA040(void);
    ~FasQ100GZ2L_LA040(void);
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

#endif //__FAS_Q100FZ2L_LA040_H__ 泰和安
