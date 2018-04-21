#include "Fas_JB_JBF_11S.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>
#include <math.h>

#define THREE_SIGNIFICANT_DIGITS (3) //最多三位有效数字

#define MSG_DEBUG	(1)

static unsigned char strStartSymbol[] = {0x0A,0x1B,0x31,0x00};	//起始符
static unsigned char strEndSymbol[] = {0x00,0x0D,0x0D};	//结束符


static unsigned char fireStrD[] = {0x00,0x82,0x04,0x08,0x10,0x20,0xC0,0x00,0xC0,0x20,
					      0x10,0x08,0x04,0x06,0x04,0x00,0x00,0x40,0x40,0x4E,
					      0x4A,0x6A,0x5A,0xCA,0x6A,0x5A,0x4A,0x4E,0x40,0x40,0x00,0x00, //"火警"下半部分的点阵数据
						 };

static unsigned char fireStrU[] = {0x00,0x00,0x01,0x0E,0x00,0x00,0x00,0x7F,0x00,0x01,
					      0x02,0x04,0x0C,0x00,0x00,0x00,0x24,0x68,0x3E,0x2A,
					      0x2F,0x69,0x2E,0x00,0x12,0x6A,0x24,0x2A,0x31,0x21,0x21,0x00,//"火警"上半部分的点阵数据
						 };

static unsigned char BreakDownD[] = {0x00,0xFC,0x08,0x08,0x08,0xFC,0x02,0x02,
					        0x84,0x48,0x30,0x48,0x84,0x06,0x04,0x00,
					        0xFE,0x20,0x10,0x20,0xC8,0x08,0xE8,0xA8,
					        0xA8,0xBE,0xA8,0xA8,0xE8,0x08,0x08,0x00, //"故障"下半部分的点阵数据
						   };

static unsigned char BreakDownU[] = {0x08,0x09,0x09,0x7F,0x09,0x09,0x0A,0x04,
					        0x0B,0x78,0x08,0x08,0x0F,0x18,0x08,0x00,
					        0x3F,0x24,0x2A,0x31,0x08,0x28,0x2B,0x3A,
					        0x2A,0x6A,0x2A,0x3A,0x2B,0x28,0x08,0x00,//"故障"上半部分的点阵数据
						   };

static unsigned char strRecoveryU[]= {0x01,0x0E,0x00,0x7F,0x08,0x14,0x10,0x17,0x78,0x10, //恢复
									  0x17,0x10,0x10,0x31,0x10,0x00,0x04,0x08,0x10,0x60,
									  0x3F,0x35,0x35,0x35,0x35,0x35,0x35,0x3F,0x60,0x20,
									  0x00,0x00};
static unsigned char strRecoveryD[]= {0x00,0x00,0x00,0xFE,0x08,0x30,0xC2,0x02,0x84,0x58,
								      0xE0,0x58,0x84,0x86,0x04,0x00,0x00,0x08,0x12,0x22,
									  0x44,0xC4,0x68,0x58,0x50,0x68,0x44,0x44,0x02,0x02,
									  0x02,0x00};

static unsigned char strResetU[]= {0x04,0x08,0x10,0x60,0x3F,0x35,0x35,0x35,0x35,0x35,
								   0x35,0x3F,0x60,0x20,0x00,0x00,0x01,0x02,0x04,0x1F,
								   0x60,0x08,0x0B,0x08,0x48,0x28,0x08,0x08,0x1B,0x08,
								   0x00,0x00};

static unsigned char strResetD[]= {0x00,0x08,0x12,0x22,0x44,0xC4,0x68,0x58,0x50,0x68,
								   0x44,0x44,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0xFE,
								   0x08,0x08,0x08,0xC8,0x28,0x08,0x38,0xC8,0x08,0x18,
								   0x08,0x00};
						  
static unsigned char  separator[] = {0X01,0X01,0X01,0X01,0X01,0X01,0X01,0X00};//"-"
//static unsigned char  space[] = {0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};//" "

static unsigned char number[10][8] = {
							{0x07,0x0F,0x18,0x11,0x18,0x0F,0x07,0x00},// 0 
							{0x00,0x04,0x0C,0x1F,0x1F,0x00,0x00,0x00},// 1
							{0x08,0x18,0x10,0x11,0x13,0x1E,0x0C,0x00},// 2
							{0x08,0x18,0x11,0x11,0x11,0x1F,0x0E,0x00},// 3
							{0x01,0x03,0x06,0x0C,0x1F,0x1F,0x00,0x00},// 4
							{0x1F,0x1F,0x11,0x11,0x11,0x11,0x10,0x00},// 5
							{0x07,0x0F,0x19,0x11,0x11,0x01,0x00,0x00},// 6
							{0x18,0x18,0x10,0x10,0x11,0x1F,0x1E,0x00},// 7
							{0x0E,0x1F,0x11,0x11,0x11,0x1F,0x0E,0x00},// 8
							{0x0E,0x1F,0x11,0x11,0x11,0x1F,0x0F,0x00,},// 9
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

static bool find(const unsigned char *srcBuff, const int srcLen, const unsigned char *dirBuffD, const unsigned char *dirBuffU, const int dirLen){
	bool fireD = false;
	bool fireU = false;
	for(int i = 0; i < srcLen;i++){
		if((srcLen - i) > dirLen){
			if(!fireD){
				if(srcBuff[i] == dirBuffD[0]){
					bool ret = Tool::strncmp_d(&srcBuff[i],dirBuffD,dirLen);
					if(ret)
					{
						fireD = true;
					}
				}
			}else{
				if(srcBuff[i] == dirBuffU[0]){
					bool ret = Tool::strncmp_d(&srcBuff[i],dirBuffU,dirLen);
					if(ret)
					{
						fireU = true;
					}	
				}
			}
		}
	}
	if(fireD && fireU){
		return true;
	}else{
		return false;
	}
}

static void findNumber(const unsigned char*srcBuffer, int srcLen, int& loop, int& point){
	int sprPos = findPos(srcBuffer,srcLen,separator,ARRAY_SIZE(separator));
	if(sprPos < srcLen - 32){//32表示最多三位点位数据加“-”号的点阵数据长度space
		for(int i = 0; i < THREE_SIGNIFICANT_DIGITS; i++){
			for(int j = 0; j < 10; j++){
				if (Tool::strncmp_d(srcBuffer + ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j]))){
					point = point*10 + j;
				}

				if (Tool::strncmp_d(srcBuffer - ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j]))){
					loop = j*pow(10.0,i) + loop;
				}
			}
		}
	}
}

static bool findFireAlarm(const unsigned char *srcBuffer, const int srcLen){
	return find(srcBuffer,srcLen,fireStrD,fireStrU,ARRAY_SIZE(fireStrD));
}

static bool findBreakDown(const unsigned char *srcBuffer, const int srcLen){
	return find(srcBuffer,srcLen,BreakDownD,BreakDownU,ARRAY_SIZE(BreakDownD));
}

static bool findRecovery(const unsigned char *srcBuffer, const int srcLen)
{
	return find(srcBuffer,srcLen,strRecoveryD,strRecoveryU,ARRAY_SIZE(BreakDownD));
}

static bool findReset(const unsigned char *srcBuffer, const int srcLen)
{
	return find(srcBuffer,srcLen,strResetD,strResetU,ARRAY_SIZE(BreakDownD));
}

int Fas_JB_JBF_11S::handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen)
{
	if(findReset(srcBuffer,srcLen)){
		stPartStatus.nRequestType = FAS_REQUEST_RESET;
		return srcLen;
	}

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
    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
	bool bFire = findFireAlarm(srcBuffer,srcLen);
	bool bFault = findBreakDown(srcBuffer,srcLen);
	if(bFire || bFault){
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
		if(bFire) {
			stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		} else {
			if(findRecovery(srcBuffer,srcLen)) {
				stPartStatus.PartStatus.StatusBit.nBit0 = 1;
			} else {
				stPartStatus.PartStatus.StatusBit.nBit2 = 1;
			}
		}
		int nLoopNo = 0;
		int nPointNo = 0;
		findNumber(srcBuffer,srcLen,nLoopNo,nPointNo);
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
	} else {
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	return 0;
}





Fas_JB_JBF_11S::Fas_JB_JBF_11S(void)
{
	//m_nFasSysAddr = 0x01;
}

Fas_JB_JBF_11S::~Fas_JB_JBF_11S(void)
{
	
}

bool Fas_JB_JBF_11S::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool Fas_JB_JBF_11S::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}


#define CONNECT_TIME_OUT	(40)
#define RECV_SIZE		(1024)
#define NON_DATA_LEN    (5)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = false;

int Fas_JB_JBF_11S::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{ 
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
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
	return nLen;
}


int Fas_JB_JBF_11S::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}

