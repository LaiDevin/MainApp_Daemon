#ifndef __FAS_JADEBIRD_5012_H__
#define __FAS_JADEBIRD_5012_H__

#include "Fas.h"
#include "Common.h"

#define DATE_DATA_LENGTH    (148) //日期跟时间的数据长度加上前面四位


class FasJadeBird5012 : public Fas
{
public:
  	FasJadeBird5012(void);
    ~FasJadeBird5012(void);
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

#endif //__FAS_JADEBIRD_5012_H__

