#ifndef __SERVER_AGREEMENT_H__
#define __SERVER_AGREEMENT_H__

#include "Socket.h"
#include "Common.h"
#include "AgreementDefine.h"
#include <stdio.h>

class ServerAgreement
{
public:
	ServerAgreement(void){};
	virtual ~ServerAgreement(void){};
	virtual bool init(void) = 0;
	virtual void unInit(void) = 0;
	virtual int recvData(char *strMsg) = 0;
	virtual int sendData(const char *strMsg, int nLen) = 0;
	virtual int sendHeartBeat(const int& nBusinessNum) = 0;
	virtual int sendData(const int& nBusinessNum, const PartRunStatus& stPartStatus) = 0;
	virtual int sendData(const int& nBusinessNum, const SysRunStatus& stSysStatus) = 0;
	virtual int sendData(const int& nBusinessNum, const UserRunStatus& stUserRunStatus) = 0;
	virtual int sendData(const int& nBusinessNum, const UserOperationInfo& stUserOperationInfo) = 0;
	virtual int sendData(const int& nBusinessNum, const UserSoftwareVersion& stUserSoftwareVersion) = 0;
	virtual int sendData(const int& nBusinessNum, const Time& stTime) = 0;
	virtual int sendData(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG) = 0;
	virtual void confirmMsg(int nBusinessNum) = 0;
	virtual void send_recovery_setting_result(int nBusinessNum, bool bRet) = 0;
	virtual void syncTime(int nBusinessNum, const Time& stTime) = 0;
	virtual void inspect_sentries(int nBusinessNum, int nFlag) = 0;
	virtual void setSocket(const Socket *pobjSocket){ m_pobjSocket = (Socket *)pobjSocket;};
	virtual void setUserID(unsigned long long lUserID){	m_lUserID = lUserID;};
	virtual void setCenterID(unsigned long long lCenterID){ m_lCenterID = lCenterID;};

protected:
	Socket *m_pobjSocket;
	unsigned long long m_lUserID;
	unsigned long long m_lCenterID;
};

#endif //__SERVER_AGREEMENT_H__
