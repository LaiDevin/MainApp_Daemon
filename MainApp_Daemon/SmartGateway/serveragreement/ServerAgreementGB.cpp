#include "ServerAgreementGB.h"
#include "Common.h"
#include "Tool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <time.h>  
#include <process.h>
#include <windows.h>
#include <iostream>

using namespace std;
#define RESEND_COUNT        (3)
#define RESEND_MAX_SIZE     (50)

enum MSG_TYPE {
	MSG_TYPE_SYS = 0,
	MSG_TYPE_PART,
	MSG_TYPE_USER,
	MSG_TYPE_USER_OPERATION_INFO,
	MSG_TYPE_SOFTWARE_VERSION,
	MSG_TYPE_TIME,
	MSG_TYPE_USER_CONFIG,
	MSG_TYPE_RESET,
	MSG_TYPE_INSPECT_SETNTRIES,//查岗
	MSG_TYPE_SYNC_TIME,
};

struct Msg
{
	int nBusinessNum;
	int nSendCount;
	int nMsgType;
	union {
		SysRunStatus stSysStatus;
		PartRunStatus stPartStatus;
		UserRunStatus stUserRunStatus;
		UserOperationInfo stUserOperationInfo;
	} Status;
	Time sTime;
};

static vector<Msg> s_stReSendMsgList;
static vector<Msg> s_stSendMsgList;
static int s_nSendRunnig = 1;
static int s_nReSendRunnig = 1;

static HANDLE s_nSendThreadID = 0;
static HANDLE s_nReSendThreadID = 0;


static HANDLE s_nDataMutex = 0;
static HANDLE s_nSendMutex = 0;
unsigned int __stdcall resend_func(LPVOID param)
{
    if(NULL == param) {
        return 0;
	}

    ServerAgreementGB *pobjServerAgreementGB = (ServerAgreementGB *)param;
	pobjServerAgreementGB->reSendFunc();
    return 0;
}

unsigned int __stdcall send_func(LPVOID param)
{
    if(NULL == param) {
        return 0;
	}

    ServerAgreementGB *pobjServerAgreementGB = (ServerAgreementGB *)param;
	pobjServerAgreementGB->sendFunc();
    return 0;
}

static int check_num(char *strMsg, int nLen)
{
	int nCheckNum = 0;
	if(strMsg == NULL) {
		return nCheckNum;
	}

	for(int i = 0; i < nLen; ++i) {
		nCheckNum += strMsg[i];
		//printf("%s\n",strMsg[i]);
	}

	return nCheckNum;
}

ServerAgreementGB::ServerAgreementGB(void)
{


}

ServerAgreementGB::~ServerAgreementGB(void)
{
    unInit();
}

void ServerAgreementGB::reSendFunc(void)
{
	while(s_nReSendRunnig){
        WaitForSingleObject(s_nDataMutex, INFINITE);
		vector<Msg>::iterator iter = s_stReSendMsgList.begin();
		for(; iter != s_stReSendMsgList.end();) {
			if(iter->nSendCount < RESEND_COUNT) {
				if(iter->nMsgType == MSG_TYPE_SYS) {
					send(iter->nBusinessNum, iter->Status.stSysStatus);
				} else if(iter->nMsgType == MSG_TYPE_PART) {
					send(iter->nBusinessNum, iter->Status.stPartStatus);
				} else if(iter->nMsgType == MSG_TYPE_USER){
					send(iter->nBusinessNum, iter->Status.stUserRunStatus);
				}
				++ (iter->nSendCount);
				++ iter;
			} else {
				iter = s_stReSendMsgList.erase(iter);
			}
			Tool::sleep_us(1000000);
		}

        int nSize = s_stReSendMsgList.size();
        if(nSize > RESEND_MAX_SIZE){
            iter = s_stReSendMsgList.begin();
            int nNeedReleaseSize = nSize - RESEND_MAX_SIZE;
            for(int i = 0; i < nNeedReleaseSize; ++ i){
                iter = s_stReSendMsgList.erase(iter);
            }
        }
        ReleaseMutex(s_nDataMutex);
        Tool::sleep_us(3000000);
	}
}

void ServerAgreementGB::sendFunc(void)
{
	vector<Msg> stMsgList;
	while(s_nSendRunnig){
		WaitForSingleObject(s_nSendMutex, INFINITE);
		if(s_stSendMsgList.size() == 0) {
			ReleaseMutex(s_nSendMutex);
			Tool::sleep_us(1000000);
			continue;
		}
		stMsgList.clear();
		stMsgList.insert(stMsgList.begin(), s_stSendMsgList.begin(), s_stSendMsgList.end());
		s_stSendMsgList.clear();
		ReleaseMutex(s_nSendMutex);

		vector<Msg>::iterator iter = stMsgList.begin();
		for(; iter != stMsgList.end(); ++ iter) {
            if(iter->nMsgType == MSG_TYPE_PART) {
				send(iter->nBusinessNum, iter->Status.stPartStatus);
                savePartStatus(iter->nBusinessNum, iter->Status.stPartStatus);
			}
		}
	}
}

bool ServerAgreementGB::init(void)
{
    s_nDataMutex = CreateMutex(NULL, FALSE, NULL);
    s_nSendMutex = CreateMutex(NULL, FALSE, NULL);
	s_stReSendMsgList.clear();
	s_stSendMsgList.clear();

    s_nReSendRunnig = 1;
    s_nReSendThreadID = (HANDLE)_beginthreadex(NULL, 0, resend_func, this, 0, NULL);

    s_nSendRunnig = 1;
    s_nSendThreadID = (HANDLE)_beginthreadex(NULL, 0, send_func, this, 0, NULL);
	return true;
}

void ServerAgreementGB::unInit(void)
{
    if(s_nReSendThreadID > 0) {
        s_nReSendRunnig = 0;
        CloseHandle(s_nReSendThreadID);
        s_nReSendThreadID = 0;
    }

    if(s_nReSendThreadID > 0) {
        s_nSendRunnig = 0;
        CloseHandle(s_nSendThreadID);
        s_nSendThreadID = 0;
    }

    if(s_nDataMutex > 0) {
        CloseHandle(s_nDataMutex);
        s_nDataMutex = 0;
    }

    if(s_nSendMutex > 0) {
        CloseHandle(s_nSendMutex);
		s_nSendMutex = 0;
    }
}

int ServerAgreementGB::recvData(char *strMsg)
{
    if(NULL == strMsg || NULL == m_pobjSocket) {
		return 0;
	}

	int nLen = m_pobjSocket->Recv(strMsg);
	if(nLen > 0) {
		if(strMsg[0] != _START_SIGN_1_ || strMsg[0] != _START_SIGN_2_ || strMsg[nLen - 2] != _END_SIGN_1_ || strMsg[nLen - 1] != _END_SIGN_2_) {
			return -1;
		}

		if(strMsg[nLen -3] != (check_num(strMsg + 2, nLen - 5) & 0xFF)){
			return -1;
		}
	}

	return nLen;
}

int ServerAgreementGB::sendData(const char *strMsg, int nLen)
{
	return m_pobjSocket->Send(strMsg, nLen);
}

void ServerAgreementGB::savePartStatus(int nBusinessNum, const PartRunStatus& stPartStatus)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum && iter->nMsgType == MSG_TYPE_PART) {
			ReleaseMutex(s_nDataMutex);
			return;
		}
	}

	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_PART;
	stMsg.nSendCount = 0;
	stMsg.Status.stPartStatus = stPartStatus;
	s_stReSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nDataMutex);
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const PartRunStatus& stPartStatus)
{
    WaitForSingleObject(s_nSendMutex, INFINITE);
    //int nLen = send(nBusinessNum, stPartStatus);
    //savePartStatus(nBusinessNum, stPartStatus);
	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_PART;
	stMsg.nSendCount = 0;
	stMsg.Status.stPartStatus = stPartStatus;
	s_stSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nSendMutex);

    return 0;
}

void ServerAgreementGB::saveSysStatus(int nBusinessNum, const SysRunStatus& stSysStatus)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum && iter->nMsgType == MSG_TYPE_SYS) {
			ReleaseMutex(s_nDataMutex);
			return;
		}
	}

	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_SYS;
	stMsg.nSendCount = 0;
	stMsg.Status.stSysStatus = stSysStatus;
	s_stReSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nDataMutex);
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const SysRunStatus& stSysStatus)
{
	int nLen = send(nBusinessNum, stSysStatus);
	saveSysStatus(nBusinessNum, stSysStatus);

	return nLen;
}

void ServerAgreementGB::saveUserRunStatus(int nBusinessNum, const UserRunStatus& stUserRunStatus)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum && iter->nMsgType == MSG_TYPE_USER) {
			ReleaseMutex(s_nDataMutex);
			return;
		}
	}

	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_USER;
	stMsg.nSendCount = 0;
	stMsg.Status.stUserRunStatus = stUserRunStatus;
	s_stReSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nDataMutex);
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const UserRunStatus& stUserRunStatus)
{
	int nLen = send(nBusinessNum, stUserRunStatus);
	saveUserRunStatus(nBusinessNum, stUserRunStatus);

	return nLen;
}

void ServerAgreementGB::saveUserOperationInfo(int nBusinessNum, const UserOperationInfo& stUserOperationInfo)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum && iter->nMsgType == MSG_TYPE_USER) {
			ReleaseMutex(s_nDataMutex);
			return;
		}
	}

	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_USER_OPERATION_INFO;
	stMsg.nSendCount = 0;
	stMsg.Status.stUserOperationInfo = stUserOperationInfo;
	s_stReSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nDataMutex);
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const UserOperationInfo& stUserOperationInfo)
{
	int nLen = send(nBusinessNum, stUserOperationInfo);
	saveUserOperationInfo(nBusinessNum, stUserOperationInfo);

	return nLen;
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const UserSoftwareVersion& stUserSoftwareVersion)
{
	int nLen = send(nBusinessNum, stUserSoftwareVersion);
	return nLen;
}

int  ServerAgreementGB::sendData(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG)
{
	int nLen = send(nBusinessNum, stUSER_CONFIG);
	return nLen;
}

void ServerAgreementGB::saveUserTimeStatus(int nBusinessNum, const Time& stTime)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum && iter->nMsgType == MSG_TYPE_USER) {
			ReleaseMutex(s_nDataMutex);
			return;
		}
	}

	Msg stMsg;
	stMsg.nBusinessNum = nBusinessNum;
	stMsg.nMsgType = MSG_TYPE_TIME;
	stMsg.nSendCount = 0;
	stMsg.sTime = stTime;
	s_stReSendMsgList.push_back(stMsg);
	ReleaseMutex(s_nDataMutex);
}

int ServerAgreementGB::sendData(const int& nBusinessNum, const Time& stTime)
{
	int nLen = send(nBusinessNum, stTime);
	saveUserTimeStatus(nBusinessNum, stTime);

	return nLen;
}


void ServerAgreementGB::confirmMsg(int nBusinessNum)
{
    WaitForSingleObject(s_nDataMutex, INFINITE);
	vector<Msg>::iterator iter = s_stReSendMsgList.begin();
	for(; iter != s_stReSendMsgList.end(); ++ iter) {
		if(iter->nBusinessNum == nBusinessNum) {
			s_stReSendMsgList.erase(iter);
			break;
		}
	}
	ReleaseMutex(s_nDataMutex);
}

void ServerAgreementGB::send_recovery_setting_result(int nBusinessNum, bool bRet)
{
	send(nBusinessNum, bRet);
}

bool SetTime(int nYear, int nMon, int nDay, int nHour, int nMin, int nSec)
{
    //Todo:for windows
    #if 0
	struct tm _tm;
	struct timeval tv;
	
	_tm.tm_sec = nSec;  
    _tm.tm_min = nMin;  
    _tm.tm_hour = nHour;  
    _tm.tm_mday = nDay;  
    _tm.tm_mon = nMon - 1;  
    _tm.tm_year = nYear - 1900;
	
	time_t utc_t = mktime(&_tm);
	tv.tv_sec = utc_t;
	tv.tv_usec = 0;

	if(settimeofday (&tv, (struct timezone *)0) < 0)  
    {  
    	//printf("Set system datatime error!/n");  
    	return false;  
    }
    #endif

    return true; 
}

void ServerAgreementGB::syncTime(int nBusinessNum, const Time& stTime)
{
	//printf("%d-%d-%d %d:%d:%d\n", stTime.nYear + 2000, stTime.nMonth, stTime.nDay, stTime.nHour, stTime.nMin, stTime.nSec);
	
	int res = SetTime((int)stTime.nYear + 2000, (int)stTime.nMonth, (int)stTime.nDay, (int)stTime.nHour, (int)stTime.nMin, (int)stTime.nSec);
	send(nBusinessNum, stTime, res);
	//return nLen;
}

void ServerAgreementGB::inspect_sentries(int nBusinessNum ,int nFlag)
{
	send(nBusinessNum, nFlag);
}

int ServerAgreementGB::send(const int& nBusinessNum,  bool bRet)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
    int nLen = packet_send_recovery_setting_result_msg(nBusinessNum, strMsg, bRet);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const Time& stTime, int res)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_sync_time_msg(nBusinessNum, stTime, strMsg, res);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const int& nFlag)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_inspect_sentries_response_msg(nBusinessNum, strMsg, nFlag);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const PartRunStatus& stPartStatus)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
    int nLen = packet_part_status_msg(stPartStatus, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const SysRunStatus& stSysStatus)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_sys_status_msg(stSysStatus, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const UserRunStatus& stUserStatus)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_user_status_msg(stUserStatus, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const UserOperationInfo& stUserOperationInfo)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_user_operation_msg(stUserOperationInfo, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const UserSoftwareVersion& stUserSoftwareVersion)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_user_software_version_msg(stUserSoftwareVersion, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_user_config_msg(stUSER_CONFIG, nBusinessNum, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}
	return nLen;
}

int ServerAgreementGB::send(const int& nBusinessNum, const Time& stTime)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[4096] = {'\0'};
	int nLen = packet_user_time_msg( nBusinessNum, stTime, strMsg);
	if(nLen > 0) {
		nLen = m_pobjSocket->Send(strMsg, nLen);
	}

	return nLen;
}

int ServerAgreementGB::sendHeartBeat(const int& nBusinessNum)
{
    if(NULL == m_pobjSocket){
        return -1;
    }

	char strMsg[34];
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本

	Time stTime;
	Tool::getTime(stTime);

	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (3 & 0xFF);
	strMsg[25] = ((3 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_SEND;
	strMsg[27] = DATA_TYPE_HEARTBEAT_190;
	strMsg[28] = 0x01;
	strMsg[29] = 0x00;
	strMsg[30] = check_num(strMsg + 2, 28) & 0xFF;
	strMsg[31] = _END_SIGN_1_;
	strMsg[32] = _END_SIGN_2_;

	return m_pobjSocket->Send(strMsg, 33);
}


int ServerAgreementGB::packet_part_status_msg(const PartRunStatus& stPartStatus, const int& nBusinessNum, char *strMsg)
{
    if(NULL == strMsg) {
		return -1;
	}
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stPartStatus.time.nSec; //时间:秒
	strMsg[7] = stPartStatus.time.nMin;
	strMsg[8] = stPartStatus.time.nHour;
	strMsg[9] = stPartStatus.time.nDay;
	strMsg[10] = stPartStatus.time.nMonth;
	strMsg[11] = stPartStatus.time.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (48 & 0xFF);
	strMsg[25] = ((48 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_SEND;
	strMsg[27] = DATA_TYPE_UPLOAD_2;
	strMsg[28] = 0x01;
	strMsg[29] = stPartStatus.nSysType;
	strMsg[30] = stPartStatus.nSysAddr;
	strMsg[31] = stPartStatus.nPartType;
	strMsg[32] = stPartStatus.nPartAddr[0];
	strMsg[33] = stPartStatus.nPartAddr[1];
	strMsg[34] = stPartStatus.nPartAddr[2];
	strMsg[35] = stPartStatus.nPartAddr[3];
	strMsg[36] = stPartStatus.PartStatus.StatusBit.nBit0 | stPartStatus.PartStatus.StatusBit.nBit1 << 1 | stPartStatus.PartStatus.StatusBit.nBit2 << 2 |
				 stPartStatus.PartStatus.StatusBit.nBit3 << 3 | stPartStatus.PartStatus.StatusBit.nBit4 << 4 | stPartStatus.PartStatus.StatusBit.nBit5 << 5 |
				 stPartStatus.PartStatus.StatusBit.nBit6 << 6 | stPartStatus.PartStatus.StatusBit.nBit7 << 7;
	strMsg[37] = stPartStatus.PartStatus.StatusBit.nBit8 | stPartStatus.PartStatus.StatusBit.nBit9 << 1 | stPartStatus.PartStatus.StatusBit.nBit10 << 2 |
				 stPartStatus.PartStatus.StatusBit.nBit11 << 3 | stPartStatus.PartStatus.StatusBit.nBit12 << 4 | stPartStatus.PartStatus.StatusBit.nBit13 << 5 |
				 stPartStatus.PartStatus.StatusBit.nBit14 << 6 | stPartStatus.PartStatus.StatusBit.nBit15 << 7;
	memcpy(strMsg + 38, stPartStatus.strPartDescription, 31);
	strMsg[70] = stPartStatus.time.nSec; //时间:秒
	strMsg[71] = stPartStatus.time.nMin;
	strMsg[72] = stPartStatus.time.nHour;
	strMsg[73] = stPartStatus.time.nDay;
	strMsg[74] = stPartStatus.time.nMonth;
	strMsg[75] = stPartStatus.time.nYear;
	strMsg[76] = check_num(strMsg + 2, 74) & 0xFF;
	strMsg[77] = _END_SIGN_1_;
	strMsg[78] = _END_SIGN_2_;
	return 79;
}

int ServerAgreementGB::packet_sys_status_msg(const SysRunStatus& stSysRunStatus, const int& nBusinessNum, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (12 & 0xFF);
	strMsg[25] = ((12 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_RESPONSE;
	strMsg[27] = DATA_TYPE_UPLOAD_1;
	strMsg[28] = 0x01;
	strMsg[29] = stSysRunStatus.nSysType;
	strMsg[30] = stSysRunStatus.nSysAddr;
	strMsg[31] = stSysRunStatus.SysStatus.StatusBit.nBit0 | stSysRunStatus.SysStatus.StatusBit.nBit1 << 1 | stSysRunStatus.SysStatus.StatusBit.nBit2 << 2 |
				 stSysRunStatus.SysStatus.StatusBit.nBit3 << 3 | stSysRunStatus.SysStatus.StatusBit.nBit4 << 4 | stSysRunStatus.SysStatus.StatusBit.nBit5 << 5 |
				 stSysRunStatus.SysStatus.StatusBit.nBit6 << 6 | stSysRunStatus.SysStatus.StatusBit.nBit7 << 7;
	strMsg[32] = stSysRunStatus.SysStatus.StatusBit.nBit8 | stSysRunStatus.SysStatus.StatusBit.nBit9 << 1 | stSysRunStatus.SysStatus.StatusBit.nBit10 << 2 |
				 stSysRunStatus.SysStatus.StatusBit.nBit11 << 3 | stSysRunStatus.SysStatus.StatusBit.nBit12 << 4 | stSysRunStatus.SysStatus.StatusBit.nBit13 << 5 |
				 stSysRunStatus.SysStatus.StatusBit.nBit14 << 6 | stSysRunStatus.SysStatus.StatusBit.nBit15 << 7;
	strMsg[33] = stTime.nSec; //时间:秒
	strMsg[34] = stTime.nMin;
	strMsg[35] = stTime.nHour;
	strMsg[36] = stTime.nDay;
	strMsg[37] = stTime.nMonth;
	strMsg[38] = stTime.nYear;
	strMsg[39] = check_num(strMsg + 2, 37) & 0xFF;
	strMsg[40] = _END_SIGN_1_;
	strMsg[41] = _END_SIGN_2_;
	return 42;
}

int ServerAgreementGB::packet_user_status_msg(const UserRunStatus& stUserRunStatus, const int& nBusinessNum, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (9 & 0xFF);
	strMsg[25] = ((9 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_RESPONSE;
	strMsg[27] = DATA_TYPE_UPLOAD_21;
	strMsg[28] = 0x01;
	strMsg[29] = stUserRunStatus.RunStatus.StatusBit.nBit0 | stUserRunStatus.RunStatus.StatusBit.nBit1 << 1 | stUserRunStatus.RunStatus.StatusBit.nBit2 << 2 |
				 stUserRunStatus.RunStatus.StatusBit.nBit3 << 3 | stUserRunStatus.RunStatus.StatusBit.nBit4 << 4 | stUserRunStatus.RunStatus.StatusBit.nBit5 << 5 |
				 stUserRunStatus.RunStatus.StatusBit.nBit6 << 6 | stUserRunStatus.RunStatus.StatusBit.nBit7 << 7;
	strMsg[30] = stTime.nSec; //时间:秒
	strMsg[31] = stTime.nMin;
	strMsg[32] = stTime.nHour;
	strMsg[33] = stTime.nDay;
	strMsg[34] = stTime.nMonth;
	strMsg[35] = stTime.nYear;
	strMsg[36] = check_num(strMsg + 2, 34) & 0xFF;
	strMsg[37] = _END_SIGN_1_;
	strMsg[38] = _END_SIGN_2_;

	return 39;
}

int ServerAgreementGB::packet_user_operation_msg(const UserOperationInfo& stUserOperationInfo, const int& nBusinessNum, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (10 & 0xFF);
	strMsg[25] = ((10 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_SEND;
	strMsg[27] = DATA_TYPE_UPLOAD_24;
	strMsg[28] = 0x01;
	strMsg[29] = stUserOperationInfo.OperationFlag.OperationBit.nBit0 | stUserOperationInfo.OperationFlag.OperationBit.nBit1 << 1 | stUserOperationInfo.OperationFlag.OperationBit.nBit2 << 2 |
				 stUserOperationInfo.OperationFlag.OperationBit.nBit3 << 3 | stUserOperationInfo.OperationFlag.OperationBit.nBit4 << 4 | stUserOperationInfo.OperationFlag.OperationBit.nBit5 << 5 |
				 stUserOperationInfo.OperationFlag.OperationBit.nBit6 << 6 | stUserOperationInfo.OperationFlag.OperationBit.nBit7 << 7;
	strMsg[30] = stUserOperationInfo.OperatorID;
	strMsg[31] = stTime.nSec; //时间:秒
	strMsg[32] = stTime.nMin;
	strMsg[33] = stTime.nHour;
	strMsg[34] = stTime.nDay;
	strMsg[35] = stTime.nMonth;
	strMsg[36] = stTime.nYear;
	strMsg[37] = check_num(strMsg + 2, 35) & 0xFF;
	strMsg[38] = _END_SIGN_1_;
	strMsg[39] = _END_SIGN_2_;

	return 40;
}

int ServerAgreementGB::packet_user_software_version_msg(const UserSoftwareVersion& stUserSoftwareVersion, const int& nBusinessNum, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = (4 & 0xFF);
	strMsg[25] = ((4 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_RESPONSE;
	strMsg[27] = DATA_TYPE_UPLOAD_25;
	strMsg[28] = 0x01;
	strMsg[29] = stUserSoftwareVersion.MajorVer;
	strMsg[30] = stUserSoftwareVersion.MinorVer;

	strMsg[31] = check_num(strMsg + 2, 29) & 0xFF;
	strMsg[32] = _END_SIGN_1_;
	strMsg[33] = _END_SIGN_2_;
	return 34;
}

int ServerAgreementGB::packet_user_config_msg(const USER_CONFIG& stUSER_CONFIG, const int& nBusinessNum, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);

	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;

	strMsg[24] = ((stUSER_CONFIG.ConfigSize + 3) & 0xFF);
	strMsg[25] = (((stUSER_CONFIG.ConfigSize + 3) >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_RESPONSE;
	strMsg[27] = DATA_TYPE_UPLOAD_26;
	strMsg[28] = 0x01;
	strMsg[29] = stUSER_CONFIG.ConfigSize;
	memcpy(strMsg + 30, stUSER_CONFIG.ConfigMsg, stUSER_CONFIG.ConfigSize); 

	strMsg[30 + stUSER_CONFIG.ConfigSize] = check_num(strMsg + 2, 28 + stUSER_CONFIG.ConfigSize) & 0xFF;
	
	strMsg[31 + stUSER_CONFIG.ConfigSize] = _END_SIGN_1_;
	strMsg[32 + stUSER_CONFIG.ConfigSize] = _END_SIGN_2_;
	return 33 + stUSER_CONFIG.ConfigSize;
}


int ServerAgreementGB::packet_user_time_msg(const int& nBusinessNum, const Time& sTime, char *strMsg)
{
	if(NULL == strMsg) {
		return -1;
	}
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = sTime.nSec; //时间:秒
	strMsg[7] = sTime.nMin;
	strMsg[8] = sTime.nHour;
	strMsg[9] = sTime.nDay;
	strMsg[10] = sTime.nMonth;
	strMsg[11] = sTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;
	
	strMsg[24] = (8 & 0xFF);
	strMsg[25] = ((8 >> 8 )& 0xFF);
	strMsg[26] = COMMAND_TYPE_RESPONSE;	
	strMsg[27] = DATA_TYPE_UPLOAD_28;
	strMsg[28] = 0x01;
	strMsg[29] = sTime.nSec; //时间:秒
	strMsg[30] = sTime.nMin;
	strMsg[31] = sTime.nHour;
	strMsg[32] = sTime.nDay;
	strMsg[33] = sTime.nMonth;
	strMsg[34] = sTime.nYear;
	strMsg[35] = check_num(strMsg + 2, 33) & 0xFF;
	strMsg[36] = _END_SIGN_1_;
	strMsg[37] = _END_SIGN_2_;

	return 38;
}

int ServerAgreementGB::packet_send_recovery_setting_result_msg(const int& nBusinessNum, char *strMsg, bool bRet)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);

	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;
	
	strMsg[24] = (0 & 0xFF);
	strMsg[25] = ((0 >> 8 )& 0xFF);
	strMsg[26] = (bRet == true) ? COMMAND_TYPE_CONFIRM : COMMAND_TYPE_DENY;
	strMsg[27] = check_num(strMsg + 2, 25) & 0xFF;
	strMsg[28] = _END_SIGN_1_;
	strMsg[29] = _END_SIGN_2_;

	return 30;
}


int ServerAgreementGB::packet_sync_time_msg(const int& nBusinessNum, const Time& sTime, char *strMsg ,int res)
{
	if(NULL == strMsg) {
		return -1;
	}

	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = sTime.nSec; //时间:秒
	strMsg[7] = sTime.nMin;
	strMsg[8] = sTime.nHour;
	strMsg[9] = sTime.nDay;
	strMsg[10] = sTime.nMonth;
	strMsg[11] = sTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;
	
	strMsg[24] = (0 & 0xFF);
	strMsg[25] = ((0 >> 8 )& 0xFF);
	strMsg[26] = (res == 1) ? COMMAND_TYPE_CONFIRM : COMMAND_TYPE_DENY;
	strMsg[27] = check_num(strMsg + 2, 25) & 0xFF;
	strMsg[28] = _END_SIGN_1_;
	strMsg[29] = _END_SIGN_2_;

	return 30;
}

int ServerAgreementGB::packet_inspect_sentries_response_msg(const int& nBusinessNum, char *strMsg, const int& nFlag)
{
	if(NULL == strMsg) {
		return -1;
	}

	Time stTime;
	Tool::getTime(stTime);
	
	strMsg[0] = _START_SIGN_1_;
	strMsg[1] = _START_SIGN_2_;
	strMsg[2] = nBusinessNum & 0xFF; //流水号
	strMsg[3] = (nBusinessNum >> 8) & 0xFF;
	strMsg[4] = MAIN_VERSION; //主版本
	strMsg[5] = USER_VERSION; //次版本
	strMsg[6] = stTime.nSec; //时间:秒
	strMsg[7] = stTime.nMin;
	strMsg[8] = stTime.nHour;
	strMsg[9] = stTime.nDay;
	strMsg[10] = stTime.nMonth;
	strMsg[11] = stTime.nYear;
	strMsg[12] = m_lUserID & 0xFF;//user_id
	strMsg[13] = (m_lUserID >> 8) & 0xFF;
	strMsg[14] = (m_lUserID >> 16) & 0xFF;
	strMsg[15] = (m_lUserID >> 24) & 0xFF;
	strMsg[16] = (m_lUserID >> 32) & 0xFF;
	strMsg[17] = (m_lUserID >> 40) & 0xFF;
	strMsg[18] = m_lCenterID & 0xFF;//server_id
	strMsg[19] = (m_lCenterID >> 8) & 0xFF;;
	strMsg[20] = (m_lCenterID >> 16) & 0xFF;
	strMsg[21] = (m_lCenterID >> 24) & 0xFF;
	strMsg[22] = (m_lCenterID >> 32) & 0xFF;
	strMsg[23] = (m_lCenterID >> 40) & 0xFF;
	
	strMsg[24] = (0 & 0xFF);
	strMsg[25] = ((0 >> 8 )& 0xFF);
	strMsg[26] = (nFlag == 1) ? COMMAND_TYPE_CONFIRM : COMMAND_TYPE_DENY;	
	strMsg[27] = check_num(strMsg + 2, 25) & 0xFF;
	strMsg[28] = _END_SIGN_1_;
	strMsg[29] = _END_SIGN_2_;

	return 30;
}

