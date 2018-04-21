#include "FasEsser_E98.h"
#include "crc.h"
#include "Tool.h"
#include <string.h>
#include <math.h>

#define MSG_DEBUG                   (1)

#define _CMD_POLL                   0XA1
#define _CMD_REPLY                  0XA2
#define _CMD_RETRY                  0XA3
#define _CMD_RESET                  0XA4
#define _CMD_CONFIRM                0XA5

//火警码
#define _ALARM_FIRE                 0X02

//复位码
#define _FAS_RESET                  0X3A

const static unsigned char      _const_pc_addr = 0x00;

//起始位0位,1位
const static unsigned char      _const_start_1 = 0xAA;
const static unsigned char      _const_start_2 = 0x55;


bool FasEsser_E98::init(int nType, const char *sComDevName,
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

bool FasEsser_E98::unInit()
{
    m_obj232Com.Close();
    m_obj485Com.Close();
    return false;
}

int FasEsser_E98::handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen)
{
    if(buffer == NULL || nLen < 9) {
        return -1;
    }

    unsigned short _type = buffer[0];

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
    stPartStatus.time.nYear = buffer[1] + 2000;
    stPartStatus.time.nMonth = buffer[2];
    stPartStatus.time.nDay = buffer[3];
    stPartStatus.time.nHour = buffer[4];
    stPartStatus.time.nMin = buffer[5];
    stPartStatus.time.nSec = 0x00;

        unsigned char strAddrMsg[31] = {0};

    int nHost = buffer[6];
    int nLoop = buffer[7];
    int nPoint = buffer[8];

    //FasID
    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
    strAddrMsg[1] = (m_nFasID & 0xFF);
    //Host
    strAddrMsg[2] = ((nHost >> 8) & 0xFF);
    strAddrMsg[3] = (nHost & 0xFF);
    //Loop
    strAddrMsg[4] = ((nLoop >> 8) & 0xFF);
    strAddrMsg[5] = (nLoop & 0xFF);
    //Point
    strAddrMsg[6] = ((nPoint >> 8) & 0xFF);
    strAddrMsg[7] = (nPoint & 0xFF);

    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);

    if (_type == _ALARM_FIRE) {
       stPartStatus.PartStatus.StatusBit.nBit1 = 1;

    } else if (_type == _FAS_RESET) {
        return -1;
    } else return -1;

    return 0;
}


#define RECV_SIZE       (256)
static unsigned char    s_RemainBuffer[RECV_SIZE] = {0};
static int              s_nRemainLen = 0;


int FasEsser_E98::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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

        //[0]
        for (int i = 0; i < nLen; i++) {

            s_RemainBuffer[s_nRemainLen++] = strRecvBuffer[i];

            //0xAA,0X55,PC地址,控制器地址,长度,命令字
            if (s_nRemainLen >= 6) {

                if (s_RemainBuffer[0] == _const_start_1 && s_RemainBuffer[1] == _const_start_2
                        && s_RemainBuffer[2] == _const_pc_addr) {

                    //s_RemainBuffer[3] 是控制器地址
                    unsigned short _replyLen = s_RemainBuffer[4];

                    if (s_nRemainLen >= (_replyLen + 5 )) {

                        unsigned char _remoteCrcLow = s_RemainBuffer[_replyLen + 3];
                        unsigned char _remoteCrcHigh = s_RemainBuffer[_replyLen + 4];

                        unsigned short resCrc = CRC16_CCITT(s_RemainBuffer, _replyLen + 3);
                        unsigned char _localCrcLow = resCrc & 0x00FF;
                        unsigned char _localCrcHigh = resCrc >> 8;

                        //crc校验通过
                        if (_localCrcHigh == _remoteCrcHigh && _localCrcLow == _remoteCrcLow) {

                            unsigned char _cmd = s_RemainBuffer[5];

                            switch (_cmd) {

                            //火警
                            case _ALARM_FIRE: {

                                if (_replyLen == 0x22) {//完整的数据长度是0x22字节长

                                    //判断是否烟感 ，温感，手报。过滤存在这几个的
                                    if (s_RemainBuffer[9] == 0x00 || s_RemainBuffer[9] == 0x01 ||
                                            s_RemainBuffer[9] == 0x03 || s_RemainBuffer[9] == 0x08) {

                                        unsigned char _buffer[9] = {0};
                                        _buffer[0] = _ALARM_FIRE;
                                        _buffer[1] = s_RemainBuffer[_replyLen + 5 - 7];//年
                                        _buffer[2] = s_RemainBuffer[_replyLen + 5 - 6];//月
                                        _buffer[3] = s_RemainBuffer[_replyLen + 5 - 5];//日
                                        _buffer[4] = s_RemainBuffer[_replyLen + 5 - 4];//时
                                        _buffer[5] = s_RemainBuffer[_replyLen + 5 - 3];//分
                                        _buffer[6] = s_RemainBuffer[6];//机号
                                        _buffer[7] = s_RemainBuffer[7];//板卡号
                                        _buffer[8] = s_RemainBuffer[11];//拨码号

                                        PartRunStatus stPartStatus;
                                        int ret = handleMsg(stPartStatus, _buffer, ARRAY_SIZE(_buffer));
                                        if(ret == -1){
                                            printf("Incomplete data!\n");
                                        } else {
                                            vtPartStatus.push_back(stPartStatus);
                                        }

                                    }


                                }

                            } break;

                                //复位
                            case _FAS_RESET: {

                            } break;

                            default:
                                break;
                            }

                        }

                        //不管校验通没通过，接收到正常的数据字节，就清空接受缓存，以方便后面的数据接受
                        memset(s_RemainBuffer, 0, RECV_SIZE);
                        s_nRemainLen = 0;
                    }
                }
            }

        }
        //[0]
    }
    return nLen;
}

int FasEsser_E98::sendData(int nComType)
{
    unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    return pobjCom->Send(sBuffer, nLen);
}
