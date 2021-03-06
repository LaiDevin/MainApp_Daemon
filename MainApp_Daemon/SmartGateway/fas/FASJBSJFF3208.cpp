#include "FASJBSJFF3208.h"
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
enum StatCode {
  FIRECODE =110,     //火警110故障111启动112反馈113监管114屏蔽115预留116
  FAULTCODE,
  STARTCODE,
  FEEDBACKCODE,
  SUPERVISONCODE,
  SHIELDCODE,
  RESERVECODE
};

FAS_JB_SJFF3208::FAS_JB_SJFF3208(void)
{

}

FAS_JB_SJFF3208::~FAS_JB_SJFF3208(void)
{

}

bool FAS_JB_SJFF3208::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FAS_JB_SJFF3208::unInit(void)
{
    m_obj232Com.Close();
    m_obj485Com.Close();
    return true;
}


static unsigned char strStartSymbol[] = {0xFE};	//起始符
static unsigned char strEndSymbol[] = {0xFF};	//结束符
static const unsigned char auchCRCHi[] = { 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01,0xC0, 0x80, 0x41,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81,0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01,0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01,0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x00, 0xC1, 0x81,0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01,0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
                              0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40 };

static const unsigned char auchCRCLo []= { 0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
                              0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,0x04,
                              0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
                              0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,0x08, 0xC8,
                              0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
                              0x1E, 0xDE, 0xDF, 0x1F, 0xDD,0x1D, 0x1C, 0xDC,
                              0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
                              0xD2, 0x12, 0x13, 0xD3,0x11, 0xD1, 0xD0, 0x10,
                              0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
                              0x36, 0xF6, 0xF7,0x37, 0xF5, 0x35, 0x34, 0xF4,
                              0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
                              0xFA, 0x3A,0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
                              0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
                              0xEE,0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
                              0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
                              0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
                              0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,0x62,
                              0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
                              0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,0x6E, 0xAE,
                              0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
                              0x78, 0xB8, 0xB9, 0x79, 0xBB,0x7B, 0x7A, 0xBA,
                              0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
                              0xB4, 0x74, 0x75, 0xB5,0x77, 0xB7, 0xB6, 0x76,
                              0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
                              0x50, 0x90, 0x91,0x51, 0x93, 0x53, 0x52, 0x92,
                              0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
                              0x9C, 0x5C,0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
                              0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
                              0x88,0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
                              0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
                              0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
                              0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,0x40 };

static unsigned short CRC16(unsigned char* puchMsg,int start, int usDataLen)
{
        unsigned char uchCRCHi = 0xFF; /* high byte of CRC initialized*/
        unsigned char uchCRCLo = 0xFF; /* low byte of CRC initialized*/
        int uIndex; /* will index into CRC lookup table*/
        for (int i = start; i < usDataLen + start; i++)
        {
            uIndex = uchCRCHi ^ puchMsg[i]; /* calculate the CRC*/
            uchCRCHi = (unsigned char)(uchCRCLo ^ auchCRCHi[uIndex]);
            uchCRCLo = auchCRCLo[uIndex];
        }

        return (unsigned short)(uchCRCHi << 8 | uchCRCLo);
}
static bool judgeFrame(unsigned char * bufferFrame, int usDataLen)
{
    if(usDataLen <= 4 ){
        return false;
    }
    unsigned char frameBody[RECV_SIZE] = {'\0'};
    int  bodyLength = usDataLen-4;
    strncpy((char *)frameBody, (char *)bufferFrame+1, bodyLength);
    unsigned short crcValue = CRC16(frameBody,0,bodyLength);
    if ((crcValue >> 8) == frameBody[bodyLength-2] && (crcValue & 0xFF) == frameBody[bodyLength-1]){
        return true;
    }
    return false;
}

int FAS_JB_SJFF3208::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
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
    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
    strAddrMsg[1] = (m_nFasID & 0xFF);

    unsigned short dataLeng = (unsigned short)((buffer[4] << 8) + buffer[3]);
    unsigned short groupCnt = (dataLeng-2)/32;

    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
    unsigned char infoCode = buffer[2];
    switch (infoCode) {
      case FIRECODE:
      {
          PartRunStatus stPartStatus;
          stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
          stPartStatus.nSysAddr = SYS_ADDR;
          stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
          //部件地址
          stPartStatus.nPartAddr[0] = 0x00;
          stPartStatus.nPartAddr[1] = 0x00;
          stPartStatus.nPartAddr[2] = 0x00;
          stPartStatus.nPartAddr[3] = 0x00;
          for (int i = 0; i < groupCnt; i++){
                unsigned char statusCode =  buffer[7 + i * 32 + 6];
                switch (statusCode) {
                  case 1:{                               //    火警
                    nHostNo = buffer[7 + i * 32 + 0];
                    nLoopNo = buffer[7 + i * 32 + 1];
                    nPointNo = buffer[7 + i * 32 + 2];
                    //Host
                    strAddrMsg[2] = ((nHostNo >> 8) & 0xFF);
                    strAddrMsg[3] = (nHostNo & 0xFF);
                    //Loop
                    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
                    strAddrMsg[5] = (nLoopNo & 0xFF);
                    //Point
                    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
                    strAddrMsg[7] = (nPointNo & 0xFF);

                    stPartStatus.time.nYear = stTime.nYear;
                    stPartStatus.time.nDay = stTime.nDay;
                    stPartStatus.time.nDay = stTime.nDay;
                    stPartStatus.time.nHour = stTime.nHour;
                    stPartStatus.time.nMin = stTime.nMin;
                    stPartStatus.time.nSec = stTime.nSec;

                    stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
                    stPartStatus.PartStatus.StatusBit.nBit1 = 1;

                    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
                    break;
                  }
                  case 8:{                               //    故障
                    nHostNo = buffer[7 + i * 32 + 0];
                    nLoopNo = buffer[7 + i * 32 + 1];
                    nPointNo = buffer[7 + i * 32 + 2];
                    //Host
                    strAddrMsg[2] = ((nHostNo >> 8) & 0xFF);
                    strAddrMsg[3] = (nHostNo & 0xFF);
                    //Loop
                    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
                    strAddrMsg[5] = (nLoopNo & 0xFF);
                    //Point
                    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
                    strAddrMsg[7] = (nPointNo & 0xFF);

                    stPartStatus.time.nYear = stTime.nYear;
                    stPartStatus.time.nDay = stTime.nDay;
                    stPartStatus.time.nDay = stTime.nDay;
                    stPartStatus.time.nHour = stTime.nHour;
                    stPartStatus.time.nMin = stTime.nMin;
                    stPartStatus.time.nSec = stTime.nSec;

                    stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
                    stPartStatus.PartStatus.StatusBit.nBit2 = 1;

                    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
                    break;
                  }
                  default:
                    break;
                }
          }
          break;
      }
      case FAULTCODE:
        break;
      case STARTCODE:
        break;
      case FEEDBACKCODE:
        break;
      case SUPERVISONCODE:
        break;
      case SHIELDCODE:
        break;
      case RESERVECODE:
        break;
      default:
        break;
    }

    return 0;
}

int FAS_JB_SJFF3208::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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

int FAS_JB_SJFF3208::sendData(int nComType)
{
    unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    return pobjCom->Send(sBuffer, nLen);
}
