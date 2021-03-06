﻿#include "FasQBTC3020.h"
#include <stdio.h>
#include <string.h>
#include <Tool.h>
#include <math.h>

#define MSG_DEBUG	(1)
enum FIRE_ALARM_TYPE
{
    FIRE_ALARM_TYPE_NONE = 0,
    FIRE_ALARM_TYPE_SMOKE,
    FIRE_ALARM_TYPE_MANUAL,
};

FasQBTC3020::FasQBTC3020(void)
{

}

FasQBTC3020::~FasQBTC3020(void)
{

}

bool FasQBTC3020::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasQBTC3020::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}

static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF}; //火警
static unsigned char strStartSymbol[] = {0x1B, 0x38, 0x00};	//开启符
static unsigned char strEndSymbol[] = {0x1B, 0x4A, 0x05};	//结束符

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = false;

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

#define POINT_DIGISTS   (3)
static int parse_point(const unsigned char * buffer, int nLen, int &nPoint)
{
	nPoint = 0;
	int nStartPos = find(buffer, nLen, strFireAlarmSymbol, ARRAY_SIZE(strFireAlarmSymbol)) + ARRAY_SIZE(strFireAlarmSymbol) + 1;
	if(nStartPos >= 0 && (nStartPos + POINT_DIGISTS) <= nLen) {
		for(int i = 0; i < POINT_DIGISTS; ++ i) {
			nPoint += ((buffer[nStartPos + i] - 0x30) * pow(10.0, POINT_DIGISTS - i - 1));
		}
	}

	return 0;
}

int FasQBTC3020::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{
	bool bFireAlarm = findFireAlarm(buffer, nLen);
	if(bFireAlarm) {
		stPartStatus.nRequestType = FAS_REQUEST_NONE;
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;//
        stPartStatus.PartStatus.StatusBit.nBit1 = 1;//正常状态
		printf("\nFire Alarm\n");
		int nPoint = 0;
		parse_point(buffer, nLen, nPoint);
		printf("号:%03d\n", nPoint);
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}

	return 0;
}

int FasQBTC3020::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
	Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }


    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
	if(nLen > 0){
 		//接收数据长度大于零
		if(s_nRemainLen + nLen >= RECV_SIZE - 1){
			printf("error\n");
			s_nRemainLen = 0;
			memset(s_RemainBuffer, 0, RECV_SIZE);
		}
		int nStartSymbolLen = ARRAY_SIZE(strStartSymbol);
    	int nEndSymbolLen = ARRAY_SIZE(strEndSymbol);
		for(int i = 0; i < nLen; i++){
			if(!s_bNew){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
			}else if(Tool::strncmp_d(strRecvBuffer + i, strStartSymbol, nStartSymbolLen)){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
				s_bNew = false;
			}else{
				continue;
			}
			s_nRemainLen++;
			if(s_nRemainLen >= nEndSymbolLen){//防止数组越界
				if(Tool::strncmp_d(s_RemainBuffer + (s_nRemainLen - nEndSymbolLen), strEndSymbol, nEndSymbolLen)){
					PartRunStatus stPartStatus;
					int ret = handleMsg(stPartStatus, s_RemainBuffer, s_nRemainLen + 1);
					if(ret == -1){
						printf("Incomplete data!\n");
					}
					vtPartStatus.push_back(stPartStatus);
					memset(s_RemainBuffer, 0, RECV_SIZE);
	            	s_nRemainLen = 0;
					s_bNew = true;
				}
			}
		}
	}

	return nLen;
}

int FasQBTC3020::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);
}

