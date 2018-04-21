#ifndef __SERVER_AGREEMENT_GB_H__
#define __SERVER_AGREEMENT_GB_H__

#define _START_SIGN_1_    (64)
#define _START_SIGN_2_    (64)

#define _END_SIGN_1_    (35)
#define _END_SIGN_2_    (35)

#include "ServerAgreement.h"
class ServerAgreementGB : public ServerAgreement
{
public:
	ServerAgreementGB(void);
	~ServerAgreementGB(void);
	bool init(void);
	void unInit(void);
	int recvData(char *strMsg);
	int sendHeartBeat(const int& nBusinessNum);
	int sendData(const char *strMsg, int nLen);
	int sendData(const int& nBusinessNum, const PartRunStatus& stPartStatus);
	int sendData(const int& nBusinessNum, const SysRunStatus& stSysStatus);
	int sendData(const int& nBusinessNum, const UserRunStatus& stUserRunStatus);
	int sendData(const int& nBusinessNum, const UserOperationInfo& stUserOperationInfo);
	int sendData(const int & nBusinessNum, const UserSoftwareVersion & stUserSoftwareVersion);
	int sendData(const int& nBusinessNum, const Time& stTime);
	int sendData(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG);
	void confirmMsg(int nBusinessNum);
	void send_recovery_setting_result(int nBusinessNum, bool bRet);
	void syncTime(int nBusinessNum, const Time& stTime);
	void inspect_sentries(int nBusinessNum, int nFlag);
	void reSendFunc(void);
	void sendFunc(void);

private:
	void savePartStatus(int nBusinessNum, const PartRunStatus& stPartStatus);
	void saveSysStatus(int nBusinessNum, const SysRunStatus& stSysStatus);
	void saveUserRunStatus(int nBusinessNum, const UserRunStatus& stUserRunStatus);
	void saveUserOperationInfo(int nBusinessNum, const UserOperationInfo& stUserOperationInfo);
	void saveUserTimeStatus(int nBusinessNum, const Time& stTime);

private:
	int send(const int& nBusinessNum, const PartRunStatus& stPartStatus);
	int send(const int& nBusinessNum, const SysRunStatus& stSysStatus);
	int send(const int& nBusinessNum, const UserRunStatus& stUserStatus);
	int send(const int& nBusinessNum, const UserOperationInfo& stUserOperationInfo);
	int send(const int& nBusinessNum, const UserSoftwareVersion& stUserSoftwareVersion);
	int send(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG);
	int send(const int& nBusinessNum, const Time& stTime);
	int send(const int&   nBusinessNum, bool bRet);
	int send(const int& nBusinessNum, const Time& stTime, int res);
	int send(const int& nBusinessNum, const int& nFlag);
	int packet_part_status_msg(const PartRunStatus& stPartStatus, const int& nBusinessNum, char *strMsg);
	int packet_sys_status_msg(const SysRunStatus& stSysRunStatus, const int& nBusinessNum, char *strMsg);
	int packet_user_status_msg(const UserRunStatus& stUserRunStatus, const int& nBusinessNum, char *strMsg);
	int packet_user_operation_msg(const UserOperationInfo& stUserOperationInfo, const int& nBusinessNum, char *strMsg);
	int packet_user_config_msg(const USER_CONFIG& stUSER_CONFIG, const int& nBusinessNum, char *strMsg);
	int packet_user_software_version_msg(const UserSoftwareVersion& stUserSoftwareVersion, const int& nBusinessNum, char *strMsg);
	int packet_user_time_msg(const int& nBusinessNum, const Time& sTime, char *strMsg);
	int packet_send_recovery_setting_result_msg(const int& nBusinessNum, char *strMsg, bool bRet);
	int packet_sync_time_msg(const int& nBusinessNum, const Time& sTime, char *strMsg ,int res);
	int packet_inspect_sentries_response_msg(const int& nBusinessNum, char *strMsg, const int& nFlag);
};

#endif //__SERVER_AGREEMENT_GB_H__
