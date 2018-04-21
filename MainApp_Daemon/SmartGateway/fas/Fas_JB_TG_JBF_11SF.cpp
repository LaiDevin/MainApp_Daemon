#include "Fas_JB_TG_JBF_11SF.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>
#include <math.h>

#define THREE_SIGNIFICANT_DIGITS (3) //最多三位有效数字

#define MSG_DEBUG	(1)


static unsigned char strStartSymbol[] = {0x1B,0x4B};	//起始符
static unsigned char strEndSymbol[] = {0x0D,0x1B,0x31,0x00};	//结束符


static unsigned char strfireStrD[] = {0x00, 0x01, 0x82, 0x04, 0x08, 0x30, 0xC0, 0x00, 0xC0, 0x20,
							 0x18, 0x0C, 0x06, 0x04, 0x00, 0x00, 0x40, 0x40, 0x40, 0x57,
							 0x55, 0x55, 0xD5, 0x55, 0xD5, 0x55, 0x55, 0x57, 0x40, 0x40,
							 0xC0, 0x00            // "火警"下半部分的点阵数据
						 	};

static unsigned char strfireStrU[] = {0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xFF, 0x40, 0x02,
							 0x04, 0x18, 0x08, 0x00, 0x00, 0x00, 0x04, 0x58, 0x5F, 0xF5,
							 0x55, 0xF7, 0x50, 0x5F, 0x08, 0xF1, 0x2A, 0x24, 0x3A, 0x21,
							 0x20, 0x00   //  "火警"上半部分的点阵数据
						 	};

static unsigned char strBreakDownRecoverD[] = {0x00, 0x00, 0xFF, 0x08, 0x12, 0x62, 0x84, 0x04, 0x98,	0xE0,
									  0x60, 0x90, 0x0C, 0x06, 0x04, 0x00       //"恢"下半部分的点阵数据
						             };

static unsigned char strBreakDownRecoverU[] = {0x0E, 0x00, 0xFF, 0x10, 0x0C, 0x10, 0x13, 0xFD, 0x10, 0x1F,
									  0x10, 0x10, 0x13, 0x10, 0x10, 0x00              //  "恢"上半部分的点阵数据
						             };


static unsigned char strBreakDownBackD[] = {0x00, 0x01, 0x09, 0x12, 0x22, 0xE4, 0x54, 0x48, 0x48, 0x54,
								   0x54, 0x62, 0x42, 0x03, 0x02, 0x00            //"复"下半部分的点阵数据
								  };

static unsigned char strBreakDownBackU[] = {0x00, 0x08, 0x10, 0x20, 0xDF, 0x55, 0x55, 0x55, 0x55, 0x55,
								   0x55, 0x55, 0x5F, 0x40, 0x00, 0x00            //  "复"上半部分的点阵数据
						          };

									 
static unsigned char strBreakDownD[] = {0x00, 0xFC, 0x08, 0x08, 0x08, 0xFA, 0x82, 0x04, 0x04, 0xC8,
							   0x30, 0xC8, 0x04, 0x06, 0x04, 0x00, 0x00, 0xFF, 0x40, 0x20,
							   0xC8, 0x08, 0xE8, 0xA8, 0xA8, 0xBF, 0xA8, 0xA8, 0xE8, 0x08,
							   0x08, 0x00          //"故障"下半部分的点阵数据
						      };

static unsigned char strBreakDownU[] = {0x08, 0x09, 0x09, 0xFF, 0x09, 0x09, 0x08, 0x01, 0x0F, 0xF8,
							   0x48, 0x08, 0x0F, 0x08, 0x08, 0x00, 0x00, 0x7F, 0x44, 0x5A,
							   0x61, 0x08, 0x4B, 0x6A, 0x5A, 0xCA, 0x4A, 0x5A, 0x6B, 0x48,
							   0x08, 0x00      //"故障"上半部分的点阵数据
						   };

static unsigned char strResetD[] = {0x00, 0x01, 0x09, 0x12, 0x22, 0xE4, 0x54, 0x48, 0x48, 0x54,
						   0x54, 0x62, 0x42, 0x03, 0x02, 0x00, 0x80, 0x00, 0x00, 0xFE,
						   0x04, 0x04, 0x04, 0xE4, 0x04, 0x0C, 0x74, 0x84, 0x04, 0x04,
						   0x04, 0x00                           // "复位"下半部分的点阵数据
						   };

static unsigned char strResetU[] = {0x00, 0x08, 0x10, 0x20, 0xDF, 0x55, 0x55, 0x55, 0x55, 0x55,
						   0x55, 0x55, 0x5F, 0x40, 0x00, 0x00, 0x00, 0x03, 0x0C, 0x37,
						   0xC0, 0x54, 0x13, 0x90, 0x50, 0x70, 0x10, 0x17, 0x12, 0x10,
						   0x00, 0x00                             // "复位"上半部分的点阵数据
						  };
						   
static unsigned char  separator[] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00};   //  "-"
static unsigned char  space[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};      //" "

static unsigned char number[10][8] = {
							{0x0F, 0x1F, 0x30, 0x23, 0x30, 0x1F, 0x0F, 0x00},      	// 0
							{0x00, 0x08, 0x18, 0x3F, 0x3F, 0x00, 0x00, 0x00},      	// 1
							{0x10, 0x30, 0x21, 0x23, 0x26, 0x3C, 0x18, 0x00},      	// 2
							{0x10, 0x30, 0x22, 0x22, 0x22, 0x3F, 0x1D, 0x00},		// 3
							{0x03, 0x07, 0x0D, 0x19, 0x3F, 0x3F, 0x01, 0x00},		// 4
							{0x3E, 0x3E, 0x22, 0x22, 0x22, 0x23, 0x21, 0x00},		// 5
							{0x0F, 0x1F, 0x32, 0x22, 0x22, 0x03, 0x01, 0x00},		// 6
							{0x30, 0x30, 0x20, 0x21, 0x23, 0x3E, 0x3C, 0x00},		// 7
							{0x1D, 0x3F, 0x22, 0x22, 0x22, 0x3F, 0x1D, 0x00},		// 8
							{0x1C, 0x3E, 0x22, 0x22, 0x22, 0x3F, 0x1F, 0x00},		// 9
			 
	};
								
struct Info {
    int nIndex;
    unsigned char strMsg[64];
};

static int findPos(const unsigned char *strSrc, int nSrcLen, const unsigned char *strTarget, int nTargetLen)
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

static void findNumber(const unsigned char*srcBuffer, int srcLen, int& loop, int& point){
	bool bSpace = false;
	int sprPos = findPos(srcBuffer,srcLen,separator,ARRAY_SIZE(separator));
	if(sprPos < srcLen - 32){//32表示最多三位点位数据加“-”号的点阵数据长度space
		for(int i = 0; i < THREE_SIGNIFICANT_DIGITS; i++){
			for(int j = 0; j < 10; j++){
				if(Tool::strncmp_d(srcBuffer + ARRAY_SIZE(separator)*(i+1) + sprPos, space, ARRAY_SIZE(space))){
					bSpace = true;
				} else if(Tool::strncmp_d(srcBuffer + ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j])-1)){
					point = point*10 + j;
				}
				if(loop > 0){
					continue;
				}else if(Tool::strncmp_d(srcBuffer - ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j])-1)){
					loop = j*pow(10.0,i) + loop;
				}
			}
			if(bSpace){
				break;	
			}
		}
	}
}




static bool findFireAlarm(const unsigned char *pSrcBuffer, const int nLen){
	if(((find(pSrcBuffer, nLen, strfireStrD, ARRAY_SIZE(strfireStrD))) >= 0) &&
		(find(pSrcBuffer, nLen, strfireStrU, ARRAY_SIZE(strfireStrU)) >= 0)) {
		return true;
	}

	return false;
}

static bool findBreakDown(const unsigned char *pSrcBuffer, const int nLen){
	if(((find(pSrcBuffer, nLen, strBreakDownD, ARRAY_SIZE(strBreakDownD))) >= 0) &&
		(find(pSrcBuffer, nLen, strBreakDownU, ARRAY_SIZE(strBreakDownU)) >= 0)) {
		return true;
	}
	
	return false;
}

static bool findBreakDownRecover(const unsigned char *pSrcBuffer, const int nLen){
	if(((find(pSrcBuffer, nLen, strBreakDownRecoverD, ARRAY_SIZE(strBreakDownRecoverD))) >= 0) &&
		(find(pSrcBuffer, nLen, strBreakDownRecoverU, ARRAY_SIZE(strBreakDownRecoverU)) >= 0)&&
		(find(pSrcBuffer, nLen, strBreakDownBackD, ARRAY_SIZE(strBreakDownBackD)) >= 0) &&
		(find(pSrcBuffer, nLen, strBreakDownBackU, ARRAY_SIZE(strBreakDownBackU)) >= 0)) {
		return true;
	}
	
	return false;
}

static bool findReset(const unsigned char *pSrcBuffer, const int nLen){
	if(((find(pSrcBuffer, nLen, strResetD, ARRAY_SIZE(strResetD))) >= 0) &&
		(find(pSrcBuffer, nLen, strResetU, ARRAY_SIZE(strResetU)) >= 0)) {
		return true;
	}
	
	return false;
}

int Fas_JB_TG_JBF_11SF::handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen)
{
	//打印机的打印方式是一行一行地打印的，每一行大约是256个点；
	//点阵数据分两部分，字符的上半部分跟下半部分，以1B 4B为起始符，起始符后两位应该是表示数据长度，0D应该表示换行；
	//一整条数据的结束标志是31 00；
	stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;

    stPartStatus.nPartAddr[0] = 0X00;
    stPartStatus.nPartAddr[1] = 0X00;
    stPartStatus.nPartAddr[2] = 0X00;
    stPartStatus.nPartAddr[3] = 0X00;
    
    Time time;
	Tool::getTime(time);
    stPartStatus.time.nYear = time.nYear;
    stPartStatus.time.nMonth = time.nMonth;
    stPartStatus.time.nDay = time.nDay;
    stPartStatus.time.nHour = time.nHour;
    stPartStatus.time.nMin = time.nMin;
    stPartStatus.time.nSec = time.nSec;

	memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
    /*判断是否含有“火警”这两个字的点阵字符*/
	bool bfire = findFireAlarm(srcBuffer,srcLen);
	if(bfire){
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
		stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		//printf("火警\n");
	}else if(findBreakDownRecover(srcBuffer,srcLen)){
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
		stPartStatus.PartStatus.StatusBit.nBit0 = 1;
		//printf("恢复\n");
	}else if(findBreakDown(srcBuffer,srcLen)){
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
		stPartStatus.PartStatus.StatusBit.nBit2 = 1;
		//printf("故障\n");
	}else if(findReset(srcBuffer,srcLen)){
		stPartStatus.nRequestType = FAS_REQUEST_RESET;
		//printf("复位\n");
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE; 
        stPartStatus.PartStatus.StatusBit.nBit0 = 1; 
		//printf("无信号 \n");
		return -1;
	}
	
	int nLoopNo = 0;
	int nPointNo = 0;
	findNumber(srcBuffer,srcLen,nLoopNo,nPointNo);
	//printf("%d-%d\n",nLoopNo,nPointNo);
	unsigned char strAddrMsg[31] = {0};
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
	return 0;
}

Fas_JB_TG_JBF_11SF::Fas_JB_TG_JBF_11SF(void)
{
	//m_nFasSysAddr = 0x01;
}

Fas_JB_TG_JBF_11SF::~Fas_JB_TG_JBF_11SF(void)
{
	
}

bool Fas_JB_TG_JBF_11SF::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool Fas_JB_TG_JBF_11SF::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}


#define CONNECT_TIME_OUT    (40)
#define RECV_SIZE   (2048)
#define NON_DATA_LEN    (5)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

int Fas_JB_TG_JBF_11SF::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{ 
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
	if(nLen > 0){
		//printfMsg(nLen, strRecvBuffer);
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
	return nLen;
}


int Fas_JB_TG_JBF_11SF::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}

