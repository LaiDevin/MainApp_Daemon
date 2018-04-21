#include "FasFHSJ_JB_QBL_MN300.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>

#define MSG_DEBUG	(1)

FasFHSJ_JB_QBL_MN300::FasFHSJ_JB_QBL_MN300(void)
{

}

FasFHSJ_JB_QBL_MN300::~FasFHSJ_JB_QBL_MN300(void)
{

}

bool FasFHSJ_JB_QBL_MN300::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasFHSJ_JB_QBL_MN300::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}


static unsigned char strStartSymbol[] = {0x1B,0x40,0x1C,0x2E,0x0D,0x0A};	//起始符
static unsigned char strEndSymbol[] = {0x0A,0x0A,0x1B,0x69,0x01};	//结束符
static unsigned char strFireAlarmSymbol[] = {0xBB,0xF0,0xBE,0xAF};  //火警
static unsigned char strBreakDownSymbol[] = {0xB9,0xCA,0xD5,0xCF};  //故障
static unsigned char strEliminateSymbol[] = {0xCF,0xFB,0xB3,0xFD};  //消除

static unsigned char strAddressSymbol[] = {0x61,0x64,0x64,0x72,0x65,0x73,0x73,0x3A};  //address: 




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


static bool findBreakDown(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strBreakDownSymbol, ARRAY_SIZE(strBreakDownSymbol)) >= 0){
		return true;
	}
	return false;
}

static bool findEliminate(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strEliminateSymbol, ARRAY_SIZE(strEliminateSymbol)) >= 0){
		return true;
	}
	return false;
}


static int calcAddress(const unsigned char *buffer, int  index)
{
	return (buffer[index + 1] - 0x30)*100 + (buffer[index + 2] - 0x30)*10 + buffer[index + 3] - 0x30;
}

int FasFHSJ_JB_QBL_MN300::handleMsg( PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
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
	
	memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
	int nId = 0;
	int nAddrPos = find(buffer, nLen, strAddressSymbol,ARRAY_SIZE(strAddressSymbol));
	if(findFireAlarm(buffer, nLen)) {
		printf("火警\n");
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
	    stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		nId = calcAddress(buffer,nAddrPos + ARRAY_SIZE(strAddressSymbol));
	}else if(findBreakDown(buffer, nLen)){
		if(findEliminate(buffer, nLen)){
			printf("故障消除\n");
			stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
			stPartStatus.PartStatus.StatusBit.nBit2 = 0;
			nId = calcAddress(buffer,nAddrPos + ARRAY_SIZE(strAddressSymbol));
		}else{
			printf("故障\n");
			stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
			stPartStatus.PartStatus.StatusBit.nBit2 = 1;
			nId = calcAddress(buffer,nAddrPos + ARRAY_SIZE(strAddressSymbol));
		}
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;
	}
	printf("id = %d\n",nId);

	return 0;
}

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

int FasFHSJ_JB_QBL_MN300::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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

int FasFHSJ_JB_QBL_MN300::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}



