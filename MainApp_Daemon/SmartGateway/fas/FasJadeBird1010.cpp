#include "FasJadeBird1010.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>

#define MSG_DEBUG	(1)
#define MIN_DATA_LENGTH   (365) //一条完整的码流数据的最小长度
static unsigned char fireStrD[] = {0X00,0X82,0X04,0X08,0X10,0X20,0XC0,0X00,0XC0,0X20,0X10,0X08,0X04,0X06,0X04,0X00};//"火"下半部分的点阵数据

static unsigned char fireStrU[] = {0X00,0X00,0X01,0X0E,0X00,0X00,0X00,0X7F,0X00,0X01,0X02,0X04,0X0C,0X00,0X00,0X00};//"火"上半部分的点阵数据

/*
static unsigned char manualFireD[] = {0X40,0X40,0X40,0X40,0X40,0X44,0X42,0XFC,0X40,0X40,0X40,0X40,0X40,0XC0,0X40,0X00,
							 0X40,0X44,0X82,0XFC,0X00,0X00,0X00,0XFE,0X44,0X28,0X10,0X68,0X84,0X02,0X02,0X00};//"手报"下半部分的点阵数据
							 
static unsigned char manualFireU[] = {0X00,0X10,0X12,0X12,0X12,0X12,0X12,0X1F,0X22,0X22,0X22,0X66,0X22,0X00,0X00,0X00,
							 0X08,0X08,0X08,0X7F,0X09,0X0A,0X00,0X7F,0X41,0X41,0X49,0X45,0X79,0X00,0X00,0X00};//"手报"上半部分的点阵数据
*/

#define CODE_NUM_POS  (19)
static int getNumberFromBuffer(const unsigned char* srcBuffer,int srcLen)
{
	// 0A 31 30 31 20 20 30 31 2D 31 34 20 31 31 3A 35 34 0A 0A 0A
	int ret = 0;
	if(srcLen >= MIN_DATA_LENGTH){

		ret = (int)(srcBuffer[srcLen-CODE_NUM_POS] - 0X30) * 100 + (int)(srcBuffer[srcLen-18] - 0X30) * 10 +(int)(srcBuffer[srcLen-17] - 0X30);  
	}
	return ret;
}

static bool find(const unsigned char *srcBuff, const int srcLen, const unsigned char *dirBuffD, const unsigned char *dirBuffU, const int dirLen){
	bool bFireL = false;
	bool bFireH = false;
	for(int i = 0; i < srcLen;i++){
		if((srcLen - i) > dirLen){
			if(!bFireL){
				if(srcBuff[i] == dirBuffD[0]){
					bool ret = Tool::strncmp_d(&srcBuff[i],dirBuffD,dirLen);
					if(ret){
						bFireL = true;
					}
				}
			}else{
				if(srcBuff[i] == dirBuffU[0]){
					bool ret = Tool::strncmp_d(&srcBuff[i],dirBuffU,dirLen);
					if(ret){
						bFireH = true;
					}	
				}
			}
		}
	}
	if(bFireL && bFireH){
		return true;
	}else{
		return false;
	}
}


static bool findFireAlarm(const unsigned char *srcBuffer, const int srcLen){
	return find(srcBuffer,srcLen,fireStrD,fireStrU,ARRAY_SIZE(fireStrD));
}


int FasJadeBird1010::handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen)
{
	stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
	stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;

    stPartStatus.nPartAddr[0] = 0x00;
    stPartStatus.nPartAddr[1] = 0x00;
    stPartStatus.nPartAddr[2] = 0x00;
    stPartStatus.nPartAddr[3] = 0x00;
    Time time;
	Tool::getTime(time);
    stPartStatus.time.nYear = time.nYear;
    stPartStatus.time.nMonth = time.nMonth;
    stPartStatus.time.nDay = time.nDay;
    stPartStatus.time.nHour = time.nHour;
    stPartStatus.time.nMin = time.nMin;
    stPartStatus.time.nSec = time.nSec;

	    /*判断是否含有“火警”这两个字的点阵字符*/
	bool bfire = findFireAlarm(srcBuffer,srcLen);
	if(bfire){
		memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
		stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		int ret = getNumberFromBuffer(srcBuffer, srcLen);
		printf("ret:%d ",ret);
		printf("火警\n");
		//todo...
		
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	return 0;
}

struct Info {
    int nIndex;
    unsigned char strMsg[64];
};

FasJadeBird1010::FasJadeBird1010(void)
{
	//m_nFasSysAddr = 0x01;
}

FasJadeBird1010::~FasJadeBird1010(void)
{
	
}

bool FasJadeBird1010::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJadeBird1010::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}


#define CONNECT_TIME_OUT    (40)
#define RECV_SIZE   (1024)
#define NON_DATA_LEN    (5)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = false;//判断是否为一条新的数据流
int FasJadeBird1010::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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
		for(int i = 0; i < nLen; i++){
			if(!s_bNew){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
			}else if(strRecvBuffer[i] == 0X1B && strRecvBuffer[i+1] == 0X4B){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
				s_bNew = false;
			}else{
				continue;
			}
			s_nRemainLen++;
			if(s_nRemainLen >= 3){//防止数组越界
				if(s_RemainBuffer[s_nRemainLen] == 0X0A && s_RemainBuffer[s_nRemainLen - 1] == 0X0A && s_RemainBuffer[s_nRemainLen - 2] == 0X0A){
					PartRunStatus stPartStatus;
					int ret = handleMsg(stPartStatus,s_RemainBuffer,s_nRemainLen + 1);
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


int FasJadeBird1010::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}

