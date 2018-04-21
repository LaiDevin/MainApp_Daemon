#include "FasFHSJ_JB_QGL_9000_Printer.h"
#include <stdio.h>
#include <string.h>
#include <Tool.h>
#include <math.h>

#define MSG_DEBUG	(1)

static unsigned char strStartSymbol[] = {0x1B, 0x40};	//起始符
static unsigned char strEndSymbol[] = {0x00, 0x0D, 0x0A};	//结束符
static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF};	//火警
static unsigned char strFaultSymbol[] = {0xB9, 0xCA, 0xD5, 0xCF};		//故障
static unsigned char strEliminateSymbol[] = {0xCF, 0xFB, 0xB3, 0xFD};	//消除
//static unsigned char strResetSymbol[] = {0xB8, 0xB4, 0xCE, 0xBB};		//复位

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

static bool findEliminate(const unsigned char* buffer, int nLen)
{
	if(find(buffer, nLen, strEliminateSymbol, ARRAY_SIZE(strEliminateSymbol)) >= 0){
		return true;
	}

	return false;
}

#define ADDRESS_START_POSITION		(26)
#define HOST_NO_LENGTH	(2)
#define LOOP_NO_LENGTH	(2)
#define PONIT_NO_LENGTH	(3)
static void parseAddress(const unsigned char *buffer, int nLen, int &nHostNo, int &nLoop, int &nPoint)
{

	if((ADDRESS_START_POSITION + HOST_NO_LENGTH + LOOP_NO_LENGTH + PONIT_NO_LENGTH) > nLen){
		return;
	}

	nHostNo = 0;
	nLoop = 0;
	nPoint = 0;
	int nStartPos = ADDRESS_START_POSITION;
	for(int i = 0; i < HOST_NO_LENGTH; ++ i) {
        nHostNo += (buffer[nStartPos + i] - 0x30)*pow(10.0, HOST_NO_LENGTH - i - 1);
    }

    nStartPos += HOST_NO_LENGTH;
    for(int i = 0; i < LOOP_NO_LENGTH; ++ i) {
        nLoop += (buffer[nStartPos + i] - 0x30)*pow(10.0, LOOP_NO_LENGTH - i - 1);
    }

    nStartPos += LOOP_NO_LENGTH;
    for(int i = 0; i < PONIT_NO_LENGTH; ++ i) {
        nPoint += (buffer[nStartPos + i] - 0x30)*pow(10.0, PONIT_NO_LENGTH - i - 1);
    }
}

FasFHSJ_JB_QGL_9000_Printer::FasFHSJ_JB_QGL_9000_Printer(void)
{
}

FasFHSJ_JB_QGL_9000_Printer::~FasFHSJ_JB_QGL_9000_Printer(void)
{
}

bool FasFHSJ_JB_QGL_9000_Printer::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasFHSJ_JB_QGL_9000_Printer::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return false;
}


int FasFHSJ_JB_QGL_9000_Printer::handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen)
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
			if(findEliminate(buffer, nLen)) {
				printf("故障消除\n");
				stPartStatus.PartStatus.StatusBit.nBit0 = 1;
			} else {
				printf("故障\n");
				stPartStatus.PartStatus.StatusBit.nBit2 = 1;
			}
		}

		int nHost = 0;
		int nPointNo = 0;
		int nLoopNo = 0;
		parseAddress(buffer, nLen, nHost, nLoopNo, nPointNo);
		unsigned char strAddrMsg[31] = {0};
	    //FasID
	    strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
	    strAddrMsg[1] = (m_nFasID & 0xFF);
	    //Host
	    strAddrMsg[2] = ((nHost >> 8) & 0xFF);
	    strAddrMsg[3] = (nHost & 0xFF);
	    //Loop
	    strAddrMsg[4] = ((nLoopNo >> 8) & 0xFF);
	    strAddrMsg[5] = (nLoopNo & 0xFF);
	    //Point
	    strAddrMsg[6] = ((nPointNo >> 8) & 0xFF);
	    strAddrMsg[7] = (nPointNo & 0xFF);
	    printf("%02d%02d%03d\n", nHost, nLoopNo, nPointNo);
	    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
	} else {
		stPartStatus.nRequestType = FAS_REQUEST_NONE;
	}
    return 0;
}


#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

int FasFHSJ_JB_QGL_9000_Printer::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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

	return nLen;
}

int FasFHSJ_JB_QGL_9000_Printer::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}
