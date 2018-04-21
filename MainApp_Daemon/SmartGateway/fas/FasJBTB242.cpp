#include "FasJBTB242.h"
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
    FIRE_ALARM_TYPE_VOICE
};

FasJBTB242::FasJBTB242(void)
{

}

FasJBTB242::~FasJBTB242(void)
{

}

bool FasJBTB242::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasJBTB242::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return true;
}

static unsigned char strFireAlarmSymbol[] = {0xBB, 0xF0, 0xBE, 0xAF}; //感烟火警
static unsigned char strStartSymbol[] = {0x20, 0x0D};	//开始符
static unsigned char strEndSymbol[] = {0x0D, 0x0D};	//结束符

#define RECV_SIZE   (1024)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

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


static int findFireAlarmType(const unsigned char* buffer, int nLen)
{
	if((find(buffer, nLen, strFireAlarmSymbol, ARRAY_SIZE(strFireAlarmSymbol))) >= 0){
		return FIRE_ALARM_TYPE_SMOKE;
	}

	return FIRE_ALARM_TYPE_NONE;
}


static int parse_loop_and_point(const unsigned char * buffer, int nLen, int &nLoop, int &nPoint)
{
	nLoop = 0;
	nPoint = 0;
	int nFireAlarmStartPos =find(buffer, nLen, strStartSymbol, ARRAY_SIZE(strStartSymbol));
    for(int i=2; i<=4; i++){
            nPoint+=(buffer[nFireAlarmStartPos+i]-0x30)*pow(10.0, 4-i);
    }
    //printf("test nPoint： %d \n", nPoint);
	return 1;
}

int FasJBTB242::handleMsg(PartRunStatus &stPartStatus, const unsigned char * buffer, int nLen)
{
    //int nMachineNo=0;
    Time stTime;
    Tool::getTime(stTime);
    stPartStatus.time.nYear = stTime.nYear;
    stPartStatus.time.nDay = stTime.nDay;
    stPartStatus.time.nMonth =stTime.nMonth;
    stPartStatus.time.nHour = stTime.nHour;
    stPartStatus.time.nMin = stTime.nMin;
    stPartStatus.time.nSec = stTime.nSec;

    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
    //part addresss
    stPartStatus.nPartAddr[0] = 0x00;
    stPartStatus.nPartAddr[1] = 0x00;
    stPartStatus.nPartAddr[2] = 0x00;
    stPartStatus.nPartAddr[3] = 0x00;
    //Time
    stPartStatus.PartStatus.StatusBit.nBit1 = 1;
    unsigned char strAddrMsg[31] = {0};
    //FasID
    strAddrMsg[0] = ((FAS_ID_JBTB_242 >> 8) & 0xFF);
    strAddrMsg[1] = (FAS_ID_JBTB_242 & 0xFF);
    stPartStatus.nRequestType = FAS_REQUEST_NONE;
    memcpy(stPartStatus.strPartDescription, strAddrMsg, 31);
    int nFireAlarmType=findFireAlarmType(buffer, nLen);
    int nPoint=0;
	int nLoop=0;
    if(nFireAlarmType == FIRE_ALARM_TYPE_SMOKE)
    {
        //printf("FIRE_ALARM_TYPE_SMOKE in buffer\n");
        stPartStatus.nRequestType=FAS_REQUEST_SENDEVENT;
        stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
        parse_loop_and_point(buffer, nLen, nLoop, nPoint);
        //printf("test nPartNo： %d \n", nPoint);
    }else{
		stPartStatus.nRequestType = FAS_REQUEST_NONE;//
        stPartStatus.PartStatus.StatusBit.nBit0 = 1;//正常状态
	}
	return 0;
}

int FasJBTB242::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);

    if(nLen > 0){
#if 0
     printfMsg(nLen, strRecvBuffer);
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

int FasJBTB242::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);
}



