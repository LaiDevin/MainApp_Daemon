#include "FasJadeBird5012.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>

#define MSG_DEBUG	(1)

#define DOTTED_LINE_DATA_LENGTH (526) //虚线所占的数据长度

#define VALID_DATA_NUM (5) //每条数据流的有效信息区根据命令符划分为四个数组以提取信息
#define THREE_SIGNIFICANT_DIGITS (3) //最大三位有效数字
#define NUMBER_COUNT (10) //0~9共10个数字字符

enum JADEBIRD_FIRE_ALARM_TYPE
{
	FIRE_ALARM_TYPE_NONE = 0,
	FIRE_ALARM_TYPE_SMOKE_FIRE,  //感烟火警
	FIRE_ALARM_TYPE_MANUAL_FIRE, //手报火警
	FIRE_ALARM_TYPE_SMOKE_FAULT,  //感烟故障
	FIRE_ALARM_TYPE_MANUAL_FAULT, //手报故障
};


static unsigned char fireStrD[] = {0X00,0X01,0X82,0X04,0X08,0X30,0XC0,0X00,0XC0,0X20,0X18,0X0C,0X06,0X04,0X00,0X00, //"火警"下半部分的点阵数据
						   0X40,0X40,0X40,0X57,0X55,0X55,0XD5,0X55,0XD5,0X55,0X55,0X57,0X40,0X40,0XC0,0X00};

static unsigned char fireStrU[] = {0X00,0X00,0X00,0X0F,0X00,0X00,0X00,0XFF,0X40,0X02,0X04,0X18,0X08,0X00,0X00,0X00,//"火警"上半部分的点阵数据
						   0X04,0X58,0X5F,0XF5,0X55,0XF7,0X50,0X5F,0X08,0XF1,0X2A,0X24,0X3A,0X21,0X20,0X00};

static unsigned char  space[] = {0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00};//" "
static unsigned char number[10][8] = {
							{0X0F,0X1F,0X30,0X23,0X30,0X1F,0X0F,0X00},// 0 
							{0X00,0X08,0X18,0X3F,0X3F,0X00,0X00,0X00},// 1
							{0X10,0X30,0X21,0X23,0X26,0X3C,0X18,0X00},// 2
							{0X10,0X30,0X22,0X22,0X22,0X3F,0X1D,0X00},// 3
							{0X03,0X07,0X0D,0X19,0X3F,0X3F,0X01,0X00},// 4
							{0X3E,0X3E,0X22,0X22,0X22,0X23,0X21,0X00},// 5
							{0X0F,0X1F,0X32,0X22,0X22,0X03,0X01,0X00},// 6
							{0X30,0X30,0X20,0X21,0X23,0X3E,0X3C,0X00},// 7
							{0X1D,0X3F,0X22,0X22,0X22,0X3F,0X1D,0X00},// 8
							{0X1C,0X3E,0X22,0X22,0X22,0X3F,0X1F,0X00},// 9	
			 
	};


struct Info {
    int nIndex;
    unsigned char strMsg[64];
};

static bool find(const unsigned char *srcBuff, const int srcLen, const unsigned char *dirBuffD, const unsigned char *dirBuffU, const int dirLen){
	bool bFireD = false;
	bool bFireU = false;
	for(int i = 0; i < srcLen;i++){
		if((srcLen - i) > dirLen){
			if(!bFireD){
				if(srcBuff[i] == dirBuffD[0]){
					if(Tool::strncmp_d(&srcBuff[i],dirBuffD,dirLen))
					{
						bFireD = true;
					}
				}
			}else{
				if(srcBuff[i] == dirBuffU[0]){
					if(Tool::strncmp_d(&srcBuff[i],dirBuffU,dirLen))
					{
						bFireU = true;
					}	
				}
			}
		}
	}
	if(bFireD && bFireU){
		return true;
	}else{
		return false;
	}
}

static void findNumber(const unsigned char* srcBuffer, int srcLen, int& loop, int& point)
{
	unsigned char data[4][264] = {{0},{0}};
	int len[4] = {0};
	int ilen = 0;
	int i = 0;
	int j = 0;
	int pos = 0;
	for (i = 0; i < srcLen -1; i++){
		if(srcBuffer[i] == 0x1B && srcBuffer[i+1] == 0x4B){
			pos = i;
			j++;
			ilen = 0;
		}
		if(j >= VALID_DATA_NUM){
			break;
		}
		data[j-1][i-pos] = srcBuffer[i];
		len[j-1] = ilen++;
	}
	unsigned char* temp = NULL;
	if(len[0] == len[1] || len[2] == len[3]){
		if(len[0] > DATE_DATA_LENGTH && len[1] > DATE_DATA_LENGTH){
			temp = data[1];
		}
		else if(len[2] > DATE_DATA_LENGTH && len[3] > DATE_DATA_LENGTH){
			temp = data[3];
		}
	}
	if(temp != NULL){
		bool bSpace = false;
		for(i = 0; i < THREE_SIGNIFICANT_DIGITS; i++){
			for(int k = 0; k < 10; k++){

				if(Tool::strncmp_d(temp + ARRAY_SIZE(number[k])*i + DATE_DATA_LENGTH, space, ARRAY_SIZE(space))){
					bSpace = true;
				}
				else if(Tool::strncmp_d(temp + ARRAY_SIZE(number[k])*i + DATE_DATA_LENGTH, number[k], ARRAY_SIZE(number[k]))){
					loop = loop*10 + k;
				}
			}
			if(bSpace){
				break;	
			}
		}
		printf("loop = %d \n",loop);
	}
}



static bool findFireAlarm(const unsigned char *srcBuffer, const int srcLen){
	return find(srcBuffer,srcLen,fireStrD,fireStrU,ARRAY_SIZE(fireStrD));
}

int FasJadeBird5012::handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen)
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

	bool bFireAlarm = findFireAlarm(srcBuffer, srcLen - DOTTED_LINE_DATA_LENGTH);//源长度减去虚线的长度，减少循环次数。
	if(bFireAlarm){
		memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
		stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		int nLoopNo = 0;
		int nPointNo = 0;
		findNumber(srcBuffer,srcLen,nLoopNo,nPointNo);
		printf("%d  火警\n",nLoopNo);
		unsigned char strAddrMsg[31] = {0};
		//FasID
		strAddrMsg[0] = ((FAS_ID_JBQB_JBF5012 >> 8) & 0xFF);
		strAddrMsg[1] = (FAS_ID_JBQB_JBF5012 & 0xFF);
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

	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	return 0;
}


FasJadeBird5012::FasJadeBird5012(void)
{
	//m_nFasSysAddr = 0x01;
}

FasJadeBird5012::~FasJadeBird5012(void)
{
	
}

bool FasJadeBird5012::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJadeBird5012::unInit(void)
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
static bool s_bNew = false;

int FasJadeBird5012::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{ 
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);
	if(nLen == 6){

		return 6;
	}

	if(nLen > 0){//接收数据长度大于零
		if(s_nRemainLen + nLen >= RECV_SIZE - 1){
			printf("Error\n");
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
			if(s_nRemainLen >= 3){
				if(s_RemainBuffer[s_nRemainLen] == 0X00 && s_RemainBuffer[s_nRemainLen - 1] == 0X31 && s_RemainBuffer[s_nRemainLen - 2] == 0X1B){
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

int FasJadeBird5012::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'}; 
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}

