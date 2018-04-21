#include "FasJKB_193K.h"

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

FasJKB_193K::FasJKB_193K(void)
{

}

FasJKB_193K::~FasJKB_193K(void)
{

}

bool FasJKB_193K::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJKB_193K::unInit(void)
{
    m_obj232Com.Close();
    m_obj485Com.Close();
    return true;
}


static unsigned char strStartSymbol[] = {0x82};	//起始符
static unsigned char strEndSymbol[] = {0x83};	//结束符

static bool judgeFrame(unsigned char * bufferFrame, int usDataLen)
{
    if(usDataLen !=22 ){
        return false;
    }
    unsigned char  addSum10 = (unsigned char)(((bufferFrame[usDataLen-3] - 0x30) << 4) + (bufferFrame[usDataLen-2] - 0x30));
    unsigned char indexCnt = 9;
    unsigned char  addSum1_9 = 0;
    for(unsigned char index = 0;index <indexCnt;index++){
        addSum1_9 += (unsigned char)(((bufferFrame[2*index+1] - 0x30) << 4) + (bufferFrame[2*index+1] - 0x30));
    }
    if(addSum10 != addSum1_9){
        return false;
    }
    return true;
}
static int parse_loop_and_point(const unsigned char * buffer, int nLen, unsigned char &nHost ,unsigned char &nLoop, unsigned char &nPoint)
{
    nHost =0;
    nLoop = 0;
    nPoint = 0;
    nHost = (unsigned char )(((buffer[3] - 0x30) << 4) + (buffer[4] - 0x30));
    nLoop = (unsigned char )(((buffer[5] - 0x30) << 4) + (buffer[6] - 0x30));
    nPoint = (unsigned char )(((buffer[7] - 0x30) << 4) + (buffer[8] - 0x30));
    return 0;
}

int FasJKB_193K::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{

    if(buffer == NULL) {
        return -1;
    }
    unsigned char strAddrMsg[31] = {0};
    unsigned char nHostNo = 0;
    unsigned char nLoopNo = 0;
    unsigned char nPointNo = 0;

    //Time
    Time stTime;
    Tool::getTime(stTime);

    //FasID
    strAddrMsg[0] = ((FAS_ID_JKB_193K >> 8) & 0xFF);
    strAddrMsg[1] = (FAS_ID_JKB_193K & 0xFF);

    stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
    //部件地址
    stPartStatus.nPartAddr[0] = 0x00;
    stPartStatus.nPartAddr[1] = 0x00;
    stPartStatus.nPartAddr[2] = 0x00;
    stPartStatus.nPartAddr[3] = 0x00;

    stPartStatus.time.nYear = stTime.nYear;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nHour = stTime.nHour;
    stPartStatus.time.nMin = stTime.nMin;
    stPartStatus.time.nSec = stTime.nSec;

    parse_loop_and_point(buffer, nLen, nHostNo, nLoopNo, nPointNo);

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

    if (buffer[1] == 0x38 && buffer[2] == 0x30)  {       //火警

      stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
      stPartStatus.PartStatus.StatusBit.nBit1 = 1;
    }
    else if (buffer[1] == 0x38 && buffer[2] == 0x31)  {      //故障

      stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
      stPartStatus.PartStatus.StatusBit.nBit2 = 1;
    }
    else if (buffer[1] == 0x38 && buffer[2] == 0x32)  {     //故障恢复

      stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
      stPartStatus.PartStatus.StatusBit.nBit0 = 1;
    }
    else if(buffer[1] == 0x30 && buffer[2] == 0x31){   //复位
      stPartStatus.nRequestType = FAS_REQUEST_RESET;
    }
    else{
        stPartStatus.nRequestType = FAS_REQUEST_NONE;
    }

    return 0;
}

int FasJKB_193K::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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
                            printfMsg(s_nRemainLen, s_RemainBuffer);
                            if(judgeFrame(s_RemainBuffer, s_nRemainLen)){

                                PartRunStatus stPartStatus;
                                handleMsg(stPartStatus, s_RemainBuffer, s_nRemainLen + 1);
                                vtPartStatus.push_back(stPartStatus);
                            }
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

int FasJKB_193K::sendData(int nComType)
{
    unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    return pobjCom->Send(sBuffer, nLen);
}
