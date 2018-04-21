#include "FasFHSJ_JB_QGL_9000.h"
#include <stdio.h>
#include <string.h>
#include <Tool.h>

#define MSG_DEBUG	(1)

static unsigned char strStartSymbol[] = {0xAA, 0xAA, 0xAA, 0xAA};	//起始符
static unsigned char strEndSymbol[] = {0xAF};	//结束符


static void printFASTime(const unsigned char *strBuffer)
{
	if(NULL != strBuffer) {
		printf("广播时间:%x年%x月%x日 %x时%x分%x秒\n", strBuffer[8]+2000, strBuffer[9],
			strBuffer[10], strBuffer[11], strBuffer[12], strBuffer[13]);
	}
}

FasFHSJ_JB_QGL_9000::FasFHSJ_JB_QGL_9000(void)
{
}

FasFHSJ_JB_QGL_9000::~FasFHSJ_JB_QGL_9000(void)
{
}

bool FasFHSJ_JB_QGL_9000::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
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

bool FasFHSJ_JB_QGL_9000::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return false;
}

static unsigned char checkSum(unsigned char *buff, int size)
{
	unsigned char cksum = 0;
	for(int i = 0;i < size; ++ i){
		cksum ^= buff[i];
	}
	return cksum;
}

void FasFHSJ_JB_QGL_9000::reply(Com *pobjCom, unsigned char cmd, unsigned char type)
{
	unsigned char replybuff[10] = {0XAA,0XAA,0XAA,0XAA,0XDF,0X63,0X00,0X00,0XAF,0XBC};
	unsigned char checkBuff[4] = {cmd,replybuff[5],0x00,type};
    replybuff[4] = cmd;   
    replybuff[7] = type;
    replybuff[9] = checkSum(checkBuff, 4);
	pobjCom->Send(replybuff, 10);
}

int FasFHSJ_JB_QGL_9000::handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen)
{
    if(buffer == NULL) {
        return -1;
    }
	stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
    //部件地址
    stPartStatus.nPartAddr[0] = buffer[12];
    stPartStatus.nPartAddr[1] = buffer[13];
    stPartStatus.nPartAddr[2] = buffer[14];
    stPartStatus.nPartAddr[3] = FAS_ID_JB_QGL_9000;
    //Time
    stPartStatus.time.nYear = buffer[9];
    stPartStatus.time.nMonth = buffer[10];
    stPartStatus.time.nDay = buffer[11];
    stPartStatus.time.nHour = buffer[12];
    stPartStatus.time.nMin = buffer[13];
    stPartStatus.time.nSec = buffer[14];
    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
	if(buffer[7] == 0x01){//第一包火警数据
		if(buffer[10] == 0x02 && buffer[11] == 0x01){
			//printf("火警\n");
			stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
            stPartStatus.PartStatus.StatusBit.nBit1 = 1;
		}else{
			stPartStatus.nRequestType = FAS_REQUEST_NONE;
            stPartStatus.PartStatus.StatusBit.nBit0 = 1;
		}
	}
	/*
    unsigned char strMsg[64] = {'\0'};
    if(0x01 == buffer[5] || 0x02 == buffer[5] || 0x08 == buffer[5]) {//模块 探头 多线
        sprintf(strMsg, "%s|%d栋%d区%d层 ",Type2Str(buffer[22], s_DevTypeInfo, ARRAY_SIZE(s_DevTypeInfo)), buffer[17], buffer[18], buffer[19]);
        if(45 == nLen) {
            memcpy(strMsg + strlen(strMsg), (unsigned char *)buffer[23], 20);
        }
    }
    memcpy(stPartStatus.strPartDescription, strMsg, 31);
	
	*/

    return 0;
}




#define CONNECT_TIME_OUT    (40)
static int s_nRS485_TimeOut = CONNECT_TIME_OUT;
static int s_nRS232_TimeOut = CONNECT_TIME_OUT;
#define RECV_SIZE   (1024)
#define NON_DATA_LEN    (5)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bNew = true;

int FasFHSJ_JB_QGL_9000::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
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
			}else if(Tool::strncmp_d(strRecvBuffer + i, strStartSymbol, ARRAY_SIZE(strStartSymbol))){
				s_RemainBuffer[s_nRemainLen] = strRecvBuffer[i];
				s_bNew = false;
			}else{
				continue;
			}
			s_nRemainLen++;
			if(s_nRemainLen){
				if(s_RemainBuffer[s_nRemainLen-1] == strEndSymbol[0]){
					printfMsg(s_nRemainLen, s_RemainBuffer);
					if(s_RemainBuffer[6] == 0x63) {//只处理发给本机地址的数据
						PartRunStatus stPartStatus;
						switch(s_RemainBuffer[4]) {
						case 0xD0: //握手
							reply(pobjCom,0XDF,0X00);
							break;
						case 0xBB: //广播
							if(nLen >= 9){
								switch (s_RemainBuffer[7]){
									case 0X00://复位
										break;
									case 0X01://消音
										break;
									case 0X0E://时间同步
										printFASTime(s_RemainBuffer);
										break;
									default:
										break;
								}
							}
							break;
						case 0xF1: //连接请求
							reply(pobjCom,0xF4,0X00);
							break;
						case 0xE7: //查询命令
							reply(pobjCom,0xE2,0X00);
							break;
						case 0xE0: //火警数据
							handleMsg(stPartStatus, s_RemainBuffer, s_nRemainLen + 1);
							vtPartStatus.push_back(stPartStatus);
							reply(pobjCom,0xE2,s_RemainBuffer[7]);
							memset(s_RemainBuffer, 0, RECV_SIZE);
							s_nRemainLen = 0;
							s_bNew = true;
							break;
						case 0xE8: //传输结束
							reply(pobjCom,0xE9,0x00);
							break;
						case 0xE2: //接收确认
							reply(pobjCom,0xE1,0x00);
							break;
						case 0xF2: //终止连接
							reply(pobjCom,0xF8,0x00);
							break;
						default:
							break;
						}
					}
				}
			}
		}
	}
		//printfMsg(nLen, strRecvBuffer);
		
	if(nLen <= 0){
		if(COM_TYPE_RS232 == nComType){
			-- s_nRS232_TimeOut;
			if(0 == s_nRS232_TimeOut) {
				s_nRS232_TimeOut = CONNECT_TIME_OUT;
				return -2;
			}
		} else if(COM_TYPE_RS485 == nComType) {
			-- s_nRS485_TimeOut;
			if(0 == s_nRS485_TimeOut) {
				s_nRS485_TimeOut = CONNECT_TIME_OUT;
				return -2;
			}
		}
	}

	return nLen;
}

int FasFHSJ_JB_QGL_9000::sendData(int nComType)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}
