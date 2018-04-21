#include "FasHZQH8000B.h"
#include <stdio.h>
#include <stdlib.h>
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

FasHZQH8000B::FasHZQH8000B(void)
{

}

FasHZQH8000B::~FasHZQH8000B(void)
{

}

bool FasHZQH8000B::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasHZQH8000B::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}

static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF}; //火警
static unsigned char strManualSymbol[] = {0xCA, 0xD6, 0xB1, 0xA8};	//手报
static unsigned char strSmokeSymbol[] = {0xB8, 0xD0, 0xD1, 0xCC};	//感烟
static unsigned char strAreaSymbol[] = {0xC7, 0xF8}; //区
static unsigned char strNumSymbol[] = {0xBA, 0xC5}; //号
static unsigned char strStartSymbol[] = {0x1B, 0x40, 0x1B, 0x63, 0x00, 0x1B, 0x38, 0x04};	//起始符
static unsigned char strEndSymbol[] = {0x0D, 0x0D, 0x20, 0x0D};	//结束符
static unsigned char strIntervalSymbol[] = {0x2D}; //"-"

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

static int findFireAlarmType(const unsigned char* buffer, int nLen)
{
	if((find(buffer, nLen, strManualSymbol, ARRAY_SIZE(strManualSymbol))) >= 0){
		return FIRE_ALARM_TYPE_MANUAL;
	}

	if((find(buffer, nLen, strSmokeSymbol, ARRAY_SIZE(strSmokeSymbol))) >= 0){
		return FIRE_ALARM_TYPE_SMOKE;
	}

	return FIRE_ALARM_TYPE_NONE;
}

#define AREA_NUM_DIGISTS	(4)
static int parse_area_num(const unsigned char * buffer, int nLen)
{
	int nAreaNum = 0;
	int nPos = find(buffer, nLen, strAreaSymbol, ARRAY_SIZE(strAreaSymbol));
	if(nPos >= 0 && ((nPos - AREA_NUM_DIGISTS) >= 0)){
		for(int i = AREA_NUM_DIGISTS; i > 0; -- i){
			nAreaNum += ((buffer[nPos - i] - 0x30) * pow(10.0, i - 1));
		}
	}
	return nAreaNum;
}

static int parse_loop_and_point(const unsigned char * buffer, int nLen, int &nLoop, int &nPoint)
{
	nLoop = 0;
	nPoint = 0;
	int nStartPos = find(buffer, nLen, strAreaSymbol, ARRAY_SIZE(strAreaSymbol)) + ARRAY_SIZE(strAreaSymbol);
	int nEndPos = find(buffer, nLen, strNumSymbol, ARRAY_SIZE(strNumSymbol));

	if(nStartPos >= 0 && nEndPos >= 0 && nEndPos > nStartPos) {
		#define SIZE (24)
		unsigned char strTemp[SIZE] = {0};
		strncpy((char *)strTemp, (char *)buffer + nStartPos, nEndPos - nStartPos);
		int nTempLen = nEndPos - nStartPos;
		int nIntervalPos = find(strTemp, nTempLen, strIntervalSymbol, ARRAY_SIZE(strIntervalSymbol));
		if(nIntervalPos > 0) {
			for(int i = 0; i < nIntervalPos; ++ i){
				nLoop += ((strTemp[i] - 0x30) * pow(10.0, nIntervalPos - i - 1));
			}

			for(int i = nTempLen - 1; i > nIntervalPos; -- i) {
				nPoint += ((strTemp[i] - 0x30) * pow(10.0, nTempLen - i - 1));
			}
		}
	}

	return 0;
}

int FasHZQH8000B::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{
	bool bFireAlarm = findFireAlarm(buffer, nLen);
	if(bFireAlarm) {
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

	    int nLoopNo = 0;
		int nPointNo = 0;
		parse_loop_and_point(buffer, nLen, nLoopNo, nPointNo);
	    //FasID
	    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
	    strAddrMsg[1] = (m_nFasID & 0xFF);
	    //Host
	    strAddrMsg[2] = 0x00;
	    strAddrMsg[3] = 0x00; 
	    //Loop
	    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
	    strAddrMsg[5] = (nLoopNo & 0xFF); 
	    //Point
	    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
	    strAddrMsg[7] = (nPointNo & 0xFF); 

	    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
#if MSG_DEBUG
		printf("\nFire Alarm:");
		int nType = findFireAlarmType(buffer, nLen);
		if(nType != FIRE_ALARM_TYPE_NONE) {
			printf("%s\n", nType == FIRE_ALARM_TYPE_SMOKE?"烟感":"手报");
		}
		int nAreaNum = parse_area_num(buffer, nLen);
		if(nAreaNum) {
			printf("区:%04d\n", nAreaNum);
		}
		printf("回路:%d\n", nLoopNo);
		printf("点:%04d\n", nPointNo);
#endif
	} else {
		stPartStatus.nRequestType = FAS_REQUEST_NONE;
	}
	return 0;
}

int FasHZQH8000B::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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

int FasHZQH8000B::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);
}

