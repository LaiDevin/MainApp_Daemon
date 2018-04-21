#include "FasJBLGQH8000.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>
#include <math.h>
#include <iostream>

#define MSG_DEBUG	(1)

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

FasJBLGQH8000::FasJBLGQH8000(void)
{

}

FasJBLGQH8000::~FasJBLGQH8000(void)
{

}

bool FasJBLGQH8000::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
{

    if(nType == COM_TYPE_RS232) {
       if(m_obj232Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
       }
    } else if(nType == COM_TYPE_RS485) {
       if(m_obj485Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
       }
    }

    return true;
}

bool FasJBLGQH8000::unInit(void)
{
    m_obj232Com.Close();
    m_obj485Com.Close();
    return true;
}

static unsigned char strFireAlarmSymbol[] = {0xBB,0xF0,0xA1,0xA1,0xA1,0xA1,0xBE,0xAF}; //火警
static unsigned char strFaultRecoverySymbol[] ={0xB9,0xCA,0xD5,0xCF,0xBB,0xD6,0xB8,0xB4};//故障恢复
static unsigned char strLoseSymbol[] = {0xB6,0xAA,0xA1,0xA1,0xA1,0xA1,0xCA,0xA7 };  //丢失
static unsigned char strAreaSymbol[] = {0xBB,0xFA}; //机
static unsigned char strNumSymbol[] = {0xBA,0xC5}; //号
static unsigned char strStartSymbol[] = {0x1B, 0x40, 0x1B, 0x63, 0x00, 0x1B, 0x38, 0x04};	//起始符
static unsigned char strEndSymbol[] = {0x0D, 0x0D, 0x20, 0x0D};	//结束符
static unsigned char strIntervalSymbol[] = {0x2D}; //"-"

static int find(const unsigned char *strSrc, int nSrcLen, const unsigned char *strTarget, int nTargetLen)
{
    if(nSrcLen < nTargetLen){
        return -1;
    }

    for(int i = 0; i < nSrcLen - nTargetLen; ++ i) {
        if(strSrc[i] == strTarget[0]){
            if(Tool::strncmp_d(strSrc + i, strTarget, nTargetLen)){
                return i;
            }
        }
    }

    return -1;
}

static bool findFireAlarm(const unsigned char* buffer, int nLen)
{
    if(find(buffer, nLen, strFireAlarmSymbol, ARRAY_SIZE(strFireAlarmSymbol)) >= 0){
        return true;
    }
    return false;
}


static bool findFaultRecovery(const unsigned char* buffer, int nLen)
{
    if(find(buffer, nLen, strFaultRecoverySymbol, ARRAY_SIZE(strFaultRecoverySymbol)) >= 0){
        return true;
    }
    return false;
}

static bool findLose(const unsigned char* buffer, int nLen)
{
    if(find(buffer, nLen, strLoseSymbol, ARRAY_SIZE(strLoseSymbol)) >= 0){
        return true;
    }
    return false;
}

static int parse_loop_and_point(const unsigned char * buffer, int nLen, int &nHost ,int &nLoop, int &nPoint)
{
    nHost =0;
    nLoop = 0;
    nPoint = 0;
    int nStartPos = find(buffer, nLen, strAreaSymbol, ARRAY_SIZE(strAreaSymbol)) + ARRAY_SIZE(strAreaSymbol);
    int nEndPos = find(buffer, nLen, strNumSymbol, ARRAY_SIZE(strNumSymbol));

    if(nStartPos >= 0 && nEndPos >= 0 && nEndPos > nStartPos) {
        #define SIZE (24)
        unsigned char strTemp[SIZE] = {0};
        strncpy((char *)strTemp, (char *)buffer + nStartPos, nEndPos - nStartPos);
        unsigned char *numstr = (unsigned char *)strtok ((char *)strTemp, (char *)strIntervalSymbol);

        int index = 0;
        while(numstr!=NULL) {

            switch (index) {
            case 0:{

                int len =strlen((char *)numstr);
                for(int i = 0; i < len; ++ i){
                    nHost += ((numstr[i] - 0x30) * pow(10.0, len - i - 1));

                }
                printf("nhost = %d\n", nHost);
                break;
            }
            case 1:{

                int len =strlen((char *)numstr);
                for(int i = 0; i < len; ++ i){
                    nLoop += ((numstr[i] - 0x30) * pow(10.0, len - i - 1));
                }
                printf("nhost = %d\n", nLoop);
                break;
            }
            case 2:{

                int len =strlen((char *)numstr);
                for(int i = 0; i < len; ++ i){
                    nPoint += ((numstr[i] - 0x30) * pow(10.0, len - i - 1));
                }
                printf("nPoint = %d\n", nPoint);
                break;
            }
            default:
                break;
            }
            numstr = (unsigned char *)strtok(NULL, (char *)strIntervalSymbol);
            index++;
        }
    }
    return 0;
}

int FasJBLGQH8000::handleMsg( PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{

    if(buffer == NULL) {
        return -1;
    }
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

    unsigned char strAddrMsg[31] = {0};
    int nHostNo = 0;
    int nLoopNo = 0;
    int nPointNo = 0;
    parse_loop_and_point(buffer, nLen, nHostNo, nLoopNo, nPointNo);
    //FasID
    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
    strAddrMsg[1] = (m_nFasID & 0xFF);
    //Host
    strAddrMsg[2] = ((nHostNo >> 8) & 0xFF);
    strAddrMsg[3] = (nHostNo & 0xFF);
    //Loop
    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
    strAddrMsg[5] = (nLoopNo & 0xFF);
    //Point
    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
    strAddrMsg[7] = (nPointNo & 0xFF);

    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);

    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
    if(findFireAlarm(buffer, nLen)) {
        printf("火警\n");
        stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
        stPartStatus.PartStatus.StatusBit.nBit1 = 1;

    }else if(findFaultRecovery(buffer, nLen)){
        printf("恢复\n");
        stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;

    }else if ( findLose(buffer, nLen)) {
        printf("丢失\n");
        stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
        stPartStatus.PartStatus.StatusBit.nBit2 = 1;
    }
    else{
        stPartStatus.nRequestType = FAS_REQUEST_NONE;
    }

    return 0;
}



int FasJBLGQH8000::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
        Com *pobjCom = getComObj(nComType);
        if(NULL == pobjCom) {
            return -1;
        }
        unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
        int nLen = pobjCom->Recv(strRecvBuffer);

        if(nLen > 0){
            // printfMsg(nLen, strRecvBuffer);
            //接收数据长度大于零
            if(s_nRemainLen + nLen >= RECV_SIZE -1){
                printf("error\n");
                s_nRemainLen = 0;
                memset(s_RemainBuffer, 0, RECV_SIZE);
            }

            int nStartSymbolLen = ARRAY_SIZE(strStartSymbol);
            int nEndSymbolLen = ARRAY_SIZE(strEndSymbol);
            for(int i = 0; i < nLen; i++){
                //如果不是新的一条数据流则直接拷贝，否则就要找到起始符才开始拷贝新的一条数据，这样保证每一条数据都是完整，没有多余数据的
                if(!s_bNew){
                    s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
                }else if(Tool::strncmp_d(strRecvBuffer + i, strStartSymbol, nStartSymbolLen)){
                    s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
                    s_bNew = false;
                }else{
                    continue;
                }
                s_nRemainLen++;
                if(s_nRemainLen > nEndSymbolLen){
                    if(s_RemainBuffer[s_nRemainLen -1] == strEndSymbol[nEndSymbolLen - 1]){
                        if(Tool::strncmp_d(&s_RemainBuffer[s_nRemainLen - nEndSymbolLen], strEndSymbol, nEndSymbolLen)){
                            //printf("ok");
                            PartRunStatus stPartStatus;
                            handleMsg(stPartStatus, s_RemainBuffer, s_nRemainLen + 1);
                            vtPartStatus.push_back(stPartStatus);
                            memset(s_RemainBuffer, 0, RECV_SIZE);
                            s_nRemainLen = 0;
                            s_bNew = true;
                        }
                    }
                }
            }
        }

        //printfMsg(nLen, strRecvBuffer);
        return nLen;

}

int FasJBLGQH8000::sendData(int nComType)
{
    unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    return pobjCom->Send(sBuffer, nLen);
}
