﻿/***
 * author: Devin
 * date:  2018/04/20
 * brief: @ file created by devin 2018/04/20
***/

#ifndef FASESSER_E98_RS232_H
#define FASESSER_E98_RS232_H
#include <deque>
#include "Fas.h"


class FasEsser_E98_RS232 : public Fas
{
public:
    FasEsser_E98_RS232() {}
    ~FasEsser_E98_RS232() {}

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
    int send(int nComType, const unsigned char *src, unsigned int nLen);
    int send(int nComType, std::deque<unsigned char> & cache);
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen);
};

#endif // FASESSER_E98_RS232_H
