#include <deque>
#include "FasHaiwanGST200_ModbusRTU.h"
#include "crc.h"
#include "Tool.h"
#include <string.h>
#include <math.h>

#define MSG_DEBUG	(1)



//从机地址
#define SLAVE_ADDR 0X01

#define MAX_POINT_SIZE 242

//查询功能码为0x03，modbus rtu的工作方式必须主机发起查询，从机返回结果
const unsigned char _query_code = 0x03;


bool FasHaiwanGST200_ModbusRTU::init(int nType, const char *sComDevName,
                                     int nBitrate, int nParity,
                                     int nDatabits, int nStopbits,
                                     int nFlowCtrl)
{
    if(nType == COM_TYPE_RS232) {
        if(!m_obj232Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
        }
    } else if(nType == COM_TYPE_RS485) {
        if(!m_obj485Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
        }
    }

    return true;
}

bool FasHaiwanGST200_ModbusRTU::unInit()
{
    m_obj232Com.Close();
    m_obj485Com.Close();
    return false;
}

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;


static void handleAlarmTempMsg(PartRunStatus& stPartStatus, unsigned short loop, unsigned short point)
{
    stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
    stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
    //部件地址
    stPartStatus.nPartAddr[0] = 0x00;
    stPartStatus.nPartAddr[1] = 0x00;
    stPartStatus.nPartAddr[2] = 0x00;
    stPartStatus.nPartAddr[3] = 0x00;
    //Time
    Time stTime;
    Tool::getTime(stTime);
    stPartStatus.time.nYear = stTime.nYear;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nHour = stTime.nHour;
    stPartStatus.time.nMin = stTime.nMin;
    stPartStatus.time.nSec = stTime.nSec;
    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
    stPartStatus.PartStatus.StatusBit.nBit1 = 1;

    unsigned char strAddrMsg[31] = {0};

    //FasID
    //strAddrMsg[0] = ((FAS_ID_GST200_MODBUS_RTU >> 8) & 0xFF);
    //strAddrMsg[1] = (FAS_ID_GST200_MODBUS_RTU & 0xFF);
    //Host
    strAddrMsg[2] = 0x00;
    strAddrMsg[3] = 0x00;
    //Loop
    strAddrMsg[4] = ((loop >> 8) & 0xFF);
    strAddrMsg[5] = (loop & 0xFF);
    //Point
    strAddrMsg[6] = ((point >> 8) & 0xFF);
    strAddrMsg[7] = (point & 0xFF);

    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
}

static int handleMsg(std::deque<PartRunStatus> &vPartStatus,
                     const unsigned char * buffer, int nLen, unsigned short areaNum, unsigned short point)
{
    for (int i = 0; i < nLen; ++i) {

            unsigned short _alarmArea = (areaNum * MAX_POINT_SIZE + point - 1) % 16;

            unsigned short _indexDevice = ((unsigned short)buffer[i + 2] << 8) & buffer[ i+ 3];

            PartRunStatus stPartStatus;

            if ( (_indexDevice >> (16 - _alarmArea)) & 0x0001) {
                handleAlarmTempMsg(stPartStatus, areaNum, point);
                vPartStatus.push_back(stPartStatus);
            }

    }

    return 0;

}

int FasHaiwanGST200_ModbusRTU::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
    int nLen = pobjCom->Recv(strRecvBuffer);

    if (nLen > 0) {
        //接收数据长度大于零
        if (s_nRemainLen + nLen >= RECV_SIZE - 1) {
            printf("error\n");
            s_nRemainLen = 0;
            memset(s_RemainBuffer, 0, RECV_SIZE);
        }

        for (int i = 0; i < nLen; i++) {

            s_RemainBuffer[s_nRemainLen++] = strRecvBuffer[i];

            //byte1	从机地址 byte2 功能码 byte3 应答字节数
            if (s_nRemainLen >= 3) {
                if (s_RemainBuffer[0] == SLAVE_ADDR && s_RemainBuffer[1] == _query_code) {
                    unsigned char _replyLen = s_RemainBuffer[2];

                    if (s_nRemainLen >= (_replyLen + 4 + 1)) {//0x01 0x03 和两位crc16校验位，和自身 1, 所以加5
                        unsigned char _remoteCrcLow = s_RemainBuffer[_replyLen + 2 + 1];
                        unsigned char _remoteCrcHigh = s_RemainBuffer[_replyLen + 2 + 2];

                        unsigned short resCrc = CRC16_MODBUS(s_RemainBuffer, _replyLen + 3);
                        unsigned char _localCrcLow = resCrc & 0x00FF;
                        unsigned char _localCrcHigh = resCrc >> 8;

                        if (_localCrcHigh == _remoteCrcHigh && _localCrcLow == _remoteCrcLow) {

                            std::deque<PartRunStatus> stPartStatus;

                            int ret = handleMsg(stPartStatus, s_RemainBuffer, _replyLen / 2, areaNum(), point());

                            if(ret == -1){
                                printf("Incomplete data!\n");
                            } else {
                                vtPartStatus.insert(vtPartStatus.end(), stPartStatus.begin(), stPartStatus.end());
                                //while (stPartStatus.size() > 0) {
                                    //vtPartStatus.push_back(std::move(stPartStatus.front()));
                                    //stPartStatus.pop_front();
                                //}
                            }
                        }

                        memset(s_RemainBuffer, 0, RECV_SIZE);
                        s_nRemainLen = 0;
                    }
                }
            }
        }
    }
    return nLen;
}

int FasHaiwanGST200_ModbusRTU::sendData(int nComType)
{
    unsigned char sBuffer[8] = {'\0'};

    unsigned short _startAddr = 0;
    _startAddr = (areaNum() * MAX_POINT_SIZE + point() - 1) / 16;
    setSendLen(1);

    if (handleSendBuffer(sBuffer, _startAddr, sendLen()) == false) {
        return 0;
    }

    int nLen = ARRAY_SIZE(sBuffer);
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    return pobjCom->Send(sBuffer, nLen);
}

bool FasHaiwanGST200_ModbusRTU::handleSendBuffer(unsigned char *dest,
                                                 unsigned short addr, unsigned short len)
{
    if (!dest || ARRAY_SIZE(dest) != 8) return false;

    dest[0] = SLAVE_ADDR;
    dest[1] = _query_code;
    dest[2] = addr >> 8;
    dest[3] = addr;
    dest[4] = len >> 8;
    dest[5] = len;

    unsigned short crcCode = CRC16_MODBUS(dest, 6);
    dest[6] = crcCode; //crc low bit
    dest[6] = crcCode >> 8; //crc high bit

    return true;
}
