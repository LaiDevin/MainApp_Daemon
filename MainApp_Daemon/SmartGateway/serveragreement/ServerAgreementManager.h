#ifndef __SERVER_AGREEMENT_MANAGER_H__
#define __SERVER_AGREEMENT_MANAGER_H__

#include "ServerAgreement.h"
#include <stdio.h>

class ServerAgreementManager {
public:
	static ServerAgreementManager *getInstance(void);
	bool init(int nProtocol);
	void unInit(void);
	bool connect(const char *strServerIP, int nPort, int nLocalPort = 0);
	void disconnect(void);
	void setAgreementID(int nAgreementID);
	void setUserID(unsigned long long lUserID);
	void setCenterID(unsigned long long lCenterID);
	int recvData(char *strMsg);
	int sendHeartBeat(int nBusinessNum);
	int sendData(const char *strMsg, int nLen);
	int sendData(int nBusinessNum, const PartRunStatus& stPartStatus);
	int sendData(int nBusinessNum, const SysRunStatus& stSysStatus);
	int sendData(int nBusinessNum, const UserRunStatus& stUserRunStatus);
	int sendData(int nBusinessNum, const UserOperationInfo& stUserOperationInfo);
	int sendData(int nBusinessNum, const UserSoftwareVersion & stUserSoftwareVersion);
	int sendData(int nBusinessNum,     const Time& stTime);
	int sendData(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG);
	void confirmMsg(int nBusinessNum);
	void send_recovery_setting_result(int nBusinessNum, bool nRet);
	void syncTime(int nBusinessNum, const Time& stTime);
	void inspect_sentries(int nBusinessNum, int nFlag);

private:
	ServerAgreementManager(void);
	~ServerAgreementManager(void);
	class Release {
		public:
			Release(void){};
			~Release(void){
				if(ServerAgreementManager::m_pobjServerAgreementManager != NULL){
					delete ServerAgreementManager::m_pobjServerAgreementManager;
					ServerAgreementManager::m_pobjServerAgreementManager = NULL;
				}
			};
	};
	static Release m_objRelease;

	ServerAgreement *getServerAgreement(int nAgreementID);
	static ServerAgreementManager *m_pobjServerAgreementManager;
    static int Communication_type;

private:
	ServerAgreement *m_pobjServerAgreement;
	Socket *m_pobjSocket;
	int m_nAgreementID;
	unsigned long long m_lUserID;
	unsigned long long m_lCenterID;
};
#endif //__SERVER_AGREEMENT_MANAGER_H__
