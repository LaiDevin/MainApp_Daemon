#include "FasJadeBird.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Tool.h>
#include <math.h>

#define DATE_DATA_LENGTH    (148) //日期跟时间的数据长度加上前面四位
#define THREE_SIGNIFICANT_DIGITS (3) //最多三位有效数字

#define MSG_DEBUG	(1)


static unsigned char fireStrD[] = {0X00,0X01,0X82,0X04,0X08,0X30,0XC0,0X00,0XC0,0X20,0X18,0X0C,0X06,0X04,0X00,0X00, //"火警"下半部分的点阵数据
						   0X40,0X40,0X40,0X57,0X55,0X55,0XD5,0X55,0XD5,0X55,0X55,0X57,0X40,0X40,0XC0,0X00};

static unsigned char fireStrU[] = {0X00,0X00,0X00,0X0F,0X00,0X00,0X00,0XFF,0X40,0X02,0X04,0X18,0X08,0X00,0X00,0X00,//"火警"上半部分的点阵数据
						   0X04,0X58,0X5F,0XF5,0X55,0XF7,0X50,0X5F,0X08,0XF1,0X2A,0X24,0X3A,0X21,0X20,0X00};
static unsigned char  separator[] = {0X01,0X01,0X01,0X01,0X01,0X01,0X01,0X00};//"-"
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
	bool bSpace = false;
	int sprPos = findPos(srcBuffer,srcLen,separator,ARRAY_SIZE(separator));
	if(sprPos < srcLen - 32){//32表示最多三位点位数据加“-”号的点阵数据长度space
		for(int i = 0; i < THREE_SIGNIFICANT_DIGITS; i++){
			for(int j = 0; j < 10; j++){

				if(Tool::strncmp_d(srcBuffer + ARRAY_SIZE(separator)*(i+1) + sprPos, space, ARRAY_SIZE(space))){
					bSpace = true;
				}
				else if(Tool::strncmp_d(srcBuffer + ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j]))){
					point = point*10 + j;
				}
				if(loop > 0){
					continue;
				}
				else if(Tool::strncmp_d(srcBuffer - ARRAY_SIZE(separator)*(i+1) + sprPos, number[j], ARRAY_SIZE(number[j]))){
					loop = j*pow(10.0,i) + loop;
				}
			}
			if(bSpace){
				break;	
			}
		}
	}
}




static bool findFireAlarm(const unsigned char *srcBuffer, const int srcLen){
	return find(srcBuffer,srcLen,fireStrD,fireStrU,ARRAY_SIZE(fireStrD));
}

int FasJadeBird::handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen)
{
	//printfMsg(srcLen, srcBuffer);
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

    /*判断是否含有“火警”这两个字的点阵字符*/
	bool bfire = findFireAlarm(srcBuffer,srcLen);
	if(bfire){
		memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
		stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
		stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		int nLoopNo = 0;
		int nPointNo = 0;
		findNumber(srcBuffer,srcLen,nLoopNo,nPointNo);
		printf("%d-%d火警\n",nLoopNo,nPointNo);
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
	}else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	return 0;
}





FasJadeBird::FasJadeBird(void)
{
	//m_nFasSysAddr = 0x01;
}

FasJadeBird::~FasJadeBird(void)
{
	
}

bool FasJadeBird::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJadeBird::unInit(void)
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
static bool s_bNew = false;

int FasJadeBird::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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
		for(int i = 0; i < nLen; i++){
			//如果不是新的一条数据流则直接拷贝，否则就要找到起始符才开始拷贝新的一条数据，这样保证每一条数据都是完整，没有多余数据的
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
				if(s_RemainBuffer[s_nRemainLen - 1] == 0X00 && s_RemainBuffer[s_nRemainLen - 2] == 0X31 && s_RemainBuffer[s_nRemainLen - 3] == 0X1B){
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


int FasJadeBird::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}

