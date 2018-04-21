/***
 * author: Devin
 * date:  2018/04/17
 * brief: @ file created by devin 2018/04/17
***/

#ifndef FASHAIWANGST200_MODBUSRTU_H
#define FASHAIWANGST200_MODBUSRTU_H
#include "Fas.h"


class FasHaiwanGST200_ModbusRTU : public Fas
{
public:
    FasHaiwanGST200_ModbusRTU() {}
    ~FasHaiwanGST200_ModbusRTU() {}

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
    unsigned short m_areaNum;//可以支持1000个区,(0-999),默认为0
    unsigned short m_point;//一次码范围是（1-242）,每个区能挂载242点，也就是设备
    unsigned short m_sendLen;// 查询寄存器的个数

    void setAreaNum(unsigned short area) {m_areaNum = area;}
    void setPoint(unsigned short point) {m_point = point;}
    void setSendLen(unsigned short len = 1) {m_sendLen = len;}

    unsigned short areaNum() const {return m_areaNum;}
    unsigned short point() const {return m_point;}
    unsigned short sendLen() const {return m_sendLen;}

    bool handleSendBuffer(unsigned char* dest,
                    unsigned short addr, unsigned short len);

};
#endif // FASHAIWANGST200_MODBUSRTU_H
