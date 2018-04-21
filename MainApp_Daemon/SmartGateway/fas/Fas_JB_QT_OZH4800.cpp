#include "Fas_JB_QT_OZH4800.h"
#include <stdio.h>
#include <string.h>
#include <Tool.h>
#include <math.h>
#define MSG_DEBUG	(1)

Fas_JB_QT_OZH4800::Fas_JB_QT_OZH4800(void)
{

}
Fas_JB_QT_OZH4800::~Fas_JB_QT_OZH4800(void)
{

}
bool Fas_JB_QT_OZH4800::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool Fas_JB_QT_OZH4800::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}

//static unsigned char strManualSymbol[] = {0xCF, 0xFB, 0xBB, 0xF0, 0xCB, 0xA8};	//消火栓(手报)
//static unsigned char strSmokeSymbol[] = {0xB8, 0xD0, 0xD1, 0xCC};	//感烟31 38 C4 EA

static unsigned char strStartSymbol[] = {0x31, 0x38, 0xC4, 0xEA};	//开始符
static unsigned char strEndSymbol[] = {0x0A, 0x0A};	//结束符
static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF}; //火警
static unsigned char strFeedBackSymbol[] = {0xB7, 0xB4, 0xC0, 0xA1}; //反馈B7 B4 C0 A1
static unsigned char strBreakDownSymbol[] = {0xB9, 0xCA, 0xD5, 0xCF}; //故障

static unsigned char strMachineNoSymbol[] = {0xBB, 0xFA};	//机号
static unsigned char strLoopNoSymbol[] = {0xC2, 0xB7};		//回路
//static unsigned char strPointNoSymbol[] = {0xBA, 0xC5};		//点号

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;


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

static bool findFeedBack(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strFeedBackSymbol, ARRAY_SIZE(strFeedBackSymbol)) >= 0){
		return true;
	}

	return false;
}


static bool findBreakDown(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strBreakDownSymbol, ARRAY_SIZE(strBreakDownSymbol)) >= 0){
		return true;
	}

	return false;
}

static int parse_loop_and_point(const unsigned char * buffer, int nLen, int &nHost, int &nLoop, int &nPoint)
{
	//printfMsg(nLen, buffer);
	nHost = 0;
	nLoop = 0;
	nPoint = 0;
	for(int i = 0; i < nLen; i++){
		if(i+1 < nLen){
			if(buffer[i] == strMachineNoSymbol[0] && buffer[i+1] == strMachineNoSymbol[1]){
				nHost = buffer[i-1] - 0x30;
			}
			else if(buffer[i] == strLoopNoSymbol[0] && buffer[i+1] == strLoopNoSymbol[1]){
				nLoop = (buffer[i-2] - 0x30)*10 + buffer[i-1] - 0x30;
				nPoint = (buffer[i+2] - 0x30)*100 + (buffer[i+3] - 0x30)*10 + buffer[i+4] - 0x30;
			}
		}
	
	}
	
	return 0;
}


int Fas_JB_QT_OZH4800::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{
	
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


	int nHost = 0;
	int nLoop = 0;
	int nPoint = 0;
	parse_loop_and_point(buffer, nLen, nHost, nLoop, nPoint);
	printf("\n主机号：%d\n",nHost);
	printf("回路号：%d\n",nLoop);
	printf("点位号：%d\n",nPoint);
	memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
	if(findFireAlarm(buffer, nLen) || findFeedBack(buffer, nLen)) {
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
	    stPartStatus.PartStatus.StatusBit.nBit1 = 1;
	printf("火警\n");
	}else if(findBreakDown(buffer, nLen)){
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
	    stPartStatus.PartStatus.StatusBit.nBit2 = 1;
		printf("故障\n");
	}
	else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	unsigned char strAddrMsg[31] = {0};
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
	return 0;
}

int Fas_JB_QT_OZH4800::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
	Com *pobjCom = getComObj(nComType);
	if(NULL == pobjCom) {
		return -1;
	}

	unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
	//printfMsg(nLen, strRecvBuffer);
	if(nLen > 0){
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

int Fas_JB_QT_OZH4800::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);
}



