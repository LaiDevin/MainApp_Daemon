#include "FasQGGST5000.h"
#include "Tool.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

enum MSG_TYPE {
	MSG_TYPE_NONE = 0,
	MSG_TYPE_ALARM,
	MSG_TYPE_FAULT,
	MSG_TYPE_RECOVERY,
	MSG_TYPE_RESET,
};

static unsigned char strFireAlarmSymbolPart1[] = {0x00,0x00,0x80,0x70,0x00,0x00,0x00,0xFF,0x00,0x80,
								    0x40,0x20,0x30,0x00,0x00,0x00,0x00,0x12,0x0A,0x3F,
								    0x2A,0xBB,0x8A,0x7A,0x84,0x47,0x2A,0x12,0x2E,0x42,
								    0x40,0x00};

static unsigned char strFireAlarmSymbolPart2[] = {0x00,0x81,0x40,0x20,0x10,0x0C,0x03,0x00,0x03,0x0C,
									0x10,0x20,0x40,0xC0,0x40,0x00,0x01,0x01,0x01,0xF1,
									0x93,0x95,0x99,0x93,0x95,0x99,0x91,0xF1,0x01,0x01,
        							0x01,0x00};

static unsigned char strFaultSymbolPart1[] = {0x10,0x10,0x10,0xFF,0x10,0x10,0x50,0x20,0xD0,0x1F,
						        0x10,0x10,0xF0,0x18,0x10,0x00,0x00,0xFE,0x22,0x5A,
						        0x86,0x10,0xD2,0x56,0x5A,0x53,0x5A,0x56,0xD2,0x10,
						        0x10,0x00};

static unsigned char strFaultSymbolPart2[] = {0x00,0x7F,0x21,0x21,0x21,0x7F,0x80,0x80,0x43,0x24,
						        0x18,0x24,0x43,0xC0,0x40,0x00,0x00,0xFF,0x04,0x08,
						        0x17,0x10,0x17,0x15,0x15,0xFD,0x15,0x15,0x17,0x10,
						        0x10,0x00};

static unsigned char strResetSymbolPart1[] = {0x20,0x10,0x0C,0x03,0xFA,0xAA,0xAA,0xAA,0xAA,0xAA,
						        0xAA,0xFA,0x03,0x02,0x00,0x00,0x80,0x40,0x20,0xF8,
						        0x07,0x10,0xD0,0x10,0x11,0x16,0x10,0x10,0xD8,0x10,
						        0x00,0x00};

static unsigned char strResetSymbolPart2[] = {0x00,0x20,0x90,0x88,0x46,0x47,0x2A,0x32,0x12,0x2A,
						        0x46,0x42,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0xFF,
						        0x20,0x20,0x20,0x23,0x2C,0x20,0x38,0x27,0x20,0x30,
						        0x20,0x00};

static unsigned char strRecoverySymbolPart1[] = {0x80,0x70,0x00,0xFF,0x10,0x28,0x08,0xE8,0x1F,0x08,
							       0xE8,0x08,0x08,0x8C,0x08,0x00,0x20,0x10,0x0C,0x03,
							       0xFA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xFA,0x03,0x02,
							       0x00,0x00};

static unsigned char strRecoverySymbolPart2[] = {0x00,0x00,0x00,0xFF,0x20,0x18,0x87,0x80,0x41,0x32,
							       0x0F,0x32,0x41,0xC1,0x40,0x00,0x00,0x20,0x90,0x88,
							       0x46,0x47,0x2A,0x32,0x12,0x2A,0x46,0x42,0x80,0x80,
							       0x80,0x00};

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


static bool findAlarm(const unsigned char * buffer, int nLen)
{
	if(((find(buffer, nLen, strFireAlarmSymbolPart1, ARRAY_SIZE(strFireAlarmSymbolPart1))) >= 0) &&
		(find(buffer, nLen, strFireAlarmSymbolPart2, ARRAY_SIZE(strFireAlarmSymbolPart1)) >= 0)) {
		return true;
	}

	return false;
}

static bool findFault(const unsigned char * buffer, int nLen)
{
	if(((find(buffer, nLen, strFaultSymbolPart1, ARRAY_SIZE(strFaultSymbolPart1))) >= 0) &&
		(find(buffer, nLen, strFaultSymbolPart2, ARRAY_SIZE(strFaultSymbolPart2)) >= 0)) {
		return true;
	}

	return false;
}

static bool findReset(const unsigned char * buffer, int nLen)
{
	if(((find(buffer, nLen, strResetSymbolPart1, ARRAY_SIZE(strResetSymbolPart1))) >= 0) &&
		(find(buffer, nLen, strResetSymbolPart2, ARRAY_SIZE(strResetSymbolPart2)) >= 0)) {
		return true;
	}

	return false;
}

static bool findRecovery(const unsigned char * buffer, int nLen)
{
	if(((find(buffer, nLen, strRecoverySymbolPart1, ARRAY_SIZE(strRecoverySymbolPart1))) >= 0) &&
		(find(buffer, nLen, strRecoverySymbolPart2, ARRAY_SIZE(strRecoverySymbolPart2)) >= 0)) {
		return true;
	}

	return false;
}

#define ADDRESS_START_POSITION		(13)
#define HOST_NO_LENGTH	(2)
#define LOOP_NO_LENGTH	(2)
#define PONIT_NO_LENGTH	(2)
static void parseAddress(const unsigned char *buffer, int nLen, int &nHostNo, int &nLoop, int &nPoint)
{
	if((HOST_NO_LENGTH + HOST_NO_LENGTH + LOOP_NO_LENGTH + PONIT_NO_LENGTH) < nLen){
		return;
	}

	nHostNo=0;
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

int FasQGGST5000::handleMsg(PartRunStatus &stPartStatus, const unsigned char *buffer, int nLen)
{
	int nMsgType = MSG_TYPE_NONE;
	stPartStatus.nRequestType = FAS_REQUEST_NONE;
	if(findAlarm(buffer, nLen)) {
		nMsgType = MSG_TYPE_ALARM;
	} else if(findFault(buffer, nLen)) {
		nMsgType = MSG_TYPE_FAULT;
	} else if(findRecovery(buffer, nLen)) {
		nMsgType = MSG_TYPE_RECOVERY;
	}else if(findReset(buffer, nLen)) {
		nMsgType = MSG_TYPE_RESET;
	} else {
		return -1;
	}

	memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
	stPartStatus.nRequestType = FAS_REQUEST_NONE;
	if(nMsgType == MSG_TYPE_RESET) {
		printf("复位\n");
		stPartStatus.nRequestType = FAS_REQUEST_RESET;
		return nLen;
	} else if(nMsgType == MSG_TYPE_ALARM) {
		printf("火警\n");
		stPartStatus.PartStatus.StatusBit.nBit1 = 1;
	} else if(nMsgType == MSG_TYPE_FAULT) {
		printf("故障\n");
		stPartStatus.PartStatus.StatusBit.nBit2 = 1;
	} else if(nMsgType == MSG_TYPE_RECOVERY) {
		printf("恢复\n");
		stPartStatus.PartStatus.StatusBit.nBit0 = 1;
	}
	stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;

	Time stTime;
    Tool::getTime(stTime);
    stPartStatus.time.nYear = stTime.nYear;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nMonth =stTime.nMonth;
    stPartStatus.time.nHour = stTime.nHour;
    stPartStatus.time.nMin = stTime.nMin;
    stPartStatus.time.nSec = stTime.nSec;
     
	stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
	stPartStatus.nSysAddr = SYS_ADDR;
	stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
	
	stPartStatus.nPartAddr[0] = 0x00;
	stPartStatus.nPartAddr[1] = 0x00;
	stPartStatus.nPartAddr[2] = 0x00;
	stPartStatus.nPartAddr[3] = 0x00;
	
	int nHostNo = 0;
	int nLoop = 0;
	int nPoint = 0;
	parseAddress(buffer, nLen, nHostNo, nLoop, nPoint);
	printf("%02d%02d%02d\n", nHostNo, nLoop, nPoint);
	unsigned char strAddrMsg[31] = {0};
	//FasID
	strAddrMsg[0] = ((m_nFasID >> 8) & 0xFF);
	strAddrMsg[1] = (m_nFasID & 0xFF);
	//Host
	strAddrMsg[2] = ((nHostNo >> 8) & 0xFF);
	strAddrMsg[3] = (nHostNo & 0xFF);
	strAddrMsg[4] = ((nLoop >> 8) & 0xFF);
	strAddrMsg[5] = (nLoop & 0xFF);
	strAddrMsg[6] = ((nPoint >> 8) & 0xFF);
	strAddrMsg[7] = (nPoint & 0xFF);
	memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);

	return nLen;
}


FasQGGST5000::FasQGGST5000()
{

}

FasQGGST5000::~FasQGGST5000()
{

}

bool FasQGGST5000::unInit(void)
{
    m_obj232Com.Close();
    m_obj485Com.Close();
	return false;
}

bool FasQGGST5000::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

int FasQGGST5000::sendData(int nComType)
{
    unsigned char sBuffer[1024] = {'\0'};
    int nLength = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom){
        return -1;
    }
    return pobjCom->Send(sBuffer, nLength);
}


#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true; 

int FasQGGST5000::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
    if(nLen > 0){
		if(s_nRemainLen + nLen >= RECV_SIZE - 1){
			printf("error\n");
			s_nRemainLen = 0;
			memset(s_RemainBuffer, 0, RECV_SIZE);
		}

		for(int i = 0; i < nLen; i++){
			if(!s_bNew){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
			} else if(strRecvBuffer[i + 2] == 0x8D && strRecvBuffer[i + 5] == 0x8E){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
				s_bNew = false;
			}else{
				continue;
			}

			s_nRemainLen++;
			if(s_nRemainLen >= 3){
				if(s_RemainBuffer[s_nRemainLen - 1] == 0X0D && s_RemainBuffer[s_nRemainLen - 2] == 0X03 && s_RemainBuffer[s_nRemainLen - 3] == 0X04){
				    PartRunStatus stPartStatus;
				    handleMsg(stPartStatus, s_RemainBuffer, s_nRemainLen);
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

