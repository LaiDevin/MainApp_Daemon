#include "FasJB3208G.h"
#include "Tool.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

FasJB3208G::FasJB3208G(void)
{

}

FasJB3208G::~FasJB3208G(void)
{

}

static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF};	//火警
static unsigned char strFaultSymbol[] = {0xB9, 0xCA , 0xD5, 0xCF};		//故障
static unsigned char strManualSymbol[] = {0xCA, 0xD6, 0xB6, 0xAF};		//手动
static unsigned char strSmokeSymbol[] = {0xB8, 0xD0, 0xD1, 0xCC}; 		//感烟
static unsigned char strPointNoSymbol[] = {0xB5, 0xE3, 0xBA, 0xC5};		//点号
static unsigned char strPartitionSymbol[] = {0xB7, 0xD6, 0xC7, 0xF8};	//分区
static unsigned char strMachineNoSymbol[] = {0xBB, 0xFA , 0xBA, 0xC5};	//机号
static unsigned char strLoopNoSymbol[] = {0xB7, 0xD6, 0xC7, 0xF8};		//回路
static unsigned char strColonSymbol[] = {0xA3, 0xBA};					//冒号
static unsigned char strSpacerSymbol[] = {0x1C, 0x2E};					//间隔符
static unsigned char strStartSymbol[] = {0x0A, 0x1C, 0x26};	//起始符
static unsigned char strEndSymbol[] = {0x0A, 0x20, 0x0A};	//结束符

#define POINT_NO_DIGITS		(3) //点号位数
#define PARTITION_DIGITS	(4) //分区位数
#define MACHINE_NO_DIGITS	(2) //机号位数
#define LOOP_NO_DIGITS		(4) //回路位数

enum CODE_TYPE
{
	CODE_TYPE_POINT_NO = 0,		//点号
	CODE_TYPE_PARTITION,		//分区
	CODE_TYPE_MACHINE_NO,		//机号
	CODE_TYPE_LOOP_NO,			//回路
};

#define MSG_DEBUG	(1)

enum FIRE_ALARM_TYPE
{
	FIRE_ALARM_TYPE_NONE = 0,
	FIRE_ALARM_TYPE_SMOKE,
	FIRE_ALARM_TYPE_MANUAL,
};

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = false;

bool FasJB3208G::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJB3208G::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
    return true;
}

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

static bool findFault(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strFaultSymbol, ARRAY_SIZE(strFaultSymbol)) >= 0){
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


static int findCode(int nType, const unsigned char* buffer, int nLen)
{
	int nDigits = -1;
	unsigned char *strSymbol = NULL;
	int nSymbolLen = 0;
	switch(nType) {
		case CODE_TYPE_POINT_NO:
			nDigits = POINT_NO_DIGITS;
			strSymbol = strPointNoSymbol;
			nSymbolLen = ARRAY_SIZE(strPointNoSymbol);
			break;
		case CODE_TYPE_MACHINE_NO:
			nDigits = MACHINE_NO_DIGITS;
			strSymbol = strMachineNoSymbol;
			nSymbolLen = ARRAY_SIZE(strMachineNoSymbol);
			break;
		case CODE_TYPE_PARTITION:
			nDigits = PARTITION_DIGITS;
			strSymbol = strPartitionSymbol;
			nSymbolLen = ARRAY_SIZE(strPartitionSymbol);
			break;
		case CODE_TYPE_LOOP_NO:
			nDigits = LOOP_NO_DIGITS;
			strSymbol = strLoopNoSymbol;
			nSymbolLen = ARRAY_SIZE(strLoopNoSymbol);
			break;
		default:
			return -1;
	}

	int nPos = find(buffer, nLen, strSymbol, nSymbolLen);
	int nCodeStartPos = nPos + nSymbolLen + ARRAY_SIZE(strColonSymbol) + ARRAY_SIZE(strSpacerSymbol);
	if(nCodeStartPos >= 0 && (nCodeStartPos + nDigits) < nLen){
		int nCode = 0;
		for(int i = 0; i < nDigits; ++ i){
			nCode +=  ((buffer[nCodeStartPos + i] - 0x30) * pow(10.0, nDigits - i - 1));
		}
		return nCode;
	}

	return -1;
}

int FasJB3208G::handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen)
{
	bool bFireAlarm = findFireAlarm(buffer, nLen);
	bool bFault = findFault(buffer, nLen);

	if(bFireAlarm || bFault) {
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
	    if(bFireAlarm) {
	    	printf("火警\n");
	    	stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		} else if(bFault) {
			printf("故障\n");
			stPartStatus.PartStatus.StatusBit.nBit2 = 1;
		}
	    
	    //int nFireAlarm = findFireAlarmType(buffer, nLen);
		int nPointNo = findCode(CODE_TYPE_POINT_NO, buffer, nLen);
		int nMachineNo = findCode(CODE_TYPE_MACHINE_NO, buffer, nLen);
		int nPartitionNo = findCode(CODE_TYPE_PARTITION, buffer, nLen);
		int nLoopNo = findCode(CODE_TYPE_LOOP_NO, buffer, nLen);

		unsigned char strAddrMsg[31] = {0};
	    //FasID
	    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
	    strAddrMsg[1] = (m_nFasID & 0xFF);
	    //Host
	    strAddrMsg[2] = ((nMachineNo >> 8) & 0xFF);
	    strAddrMsg[3] = (nMachineNo & 0xFF);
	    //Loop
	    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
	    strAddrMsg[5] = (nLoopNo & 0xFF);
	    //Point
	    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
	    strAddrMsg[7] = (nPointNo & 0xFF);
	    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
#if MSG_DEBUG
		printf("\n----------------------\n");
		printf("点号: %03d\n", nPointNo);
		printf("分区: %04d\n", nPartitionNo);
		printf("机号: %02d\n", nMachineNo);
		printf("回路: %04d\n", nLoopNo);
		printf("----------------------\n");
#endif
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}

	return 0;
}

int FasJB3208G::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
	Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
	if(nLen > 0){
#if 0 //just for test
		if(nLen == 1 && strRecvBuffer[0] == 0x00) {
			PartRunStatus stPartStatus;
			stPartStatus.nRequestType = FAS_REQUEST_RESET;
			vtPartStatus.push_back(stPartStatus);
			return nLen;
		}
#endif
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

int FasJB3208G::sendData(int nComType/*zdst define data*/)
{
	return 0;
}
