#include "ServerAgreementManager.h"
#include "ServerAgreementGB.h"
#include "TcpSocket.h"
#include "UdpSocket.h"
#include "Common.h"
#include <stdio.h>

static ServerAgreementGB s_objServerAgreementGB;
static UdpSocket s_objUdpSocket;
static TcpSocket s_objTcpSocket;
static bool s_bInitFlag = false;
int ServerAgreementManager::Communication_type = 0;

ServerAgreementManager::Release ServerAgreementManager::m_objRelease;

ServerAgreementManager *ServerAgreementManager::getInstance(void)
{
	return m_pobjServerAgreementManager;
}

ServerAgreementManager *ServerAgreementManager::m_pobjServerAgreementManager = new ServerAgreementManager();

ServerAgreementManager::ServerAgreementManager(void)
{
	m_pobjServerAgreement = NULL;
	m_nAgreementID = -1;
	m_pobjSocket = NULL;
	m_lUserID = 0;
	m_lCenterID = 0;
}

ServerAgreementManager::~ServerAgreementManager(void)
{
}

bool ServerAgreementManager::init(int nProtocol)
{
	if(s_bInitFlag) {
		return s_bInitFlag;
	}

	m_pobjServerAgreement = getServerAgreement(m_nAgreementID);
	if(NULL == m_pobjServerAgreement) {
		s_bInitFlag = false;
		return false;
	}

    ServerAgreementManager::Communication_type = nProtocol;
	if(nProtocol == NETWORK_PROTOCOL_TCP){
		m_pobjSocket = &s_objTcpSocket;
	} else if(nProtocol == NETWORK_PROTOCOL_UDP) {
		m_pobjSocket = &s_objUdpSocket;
	} else {
		s_bInitFlag = false;
		return false;
	}

	m_pobjServerAgreement->setCenterID(m_lCenterID);
	m_pobjServerAgreement->setUserID(m_lUserID);
	m_pobjServerAgreement->setSocket(m_pobjSocket);
	s_bInitFlag = m_pobjServerAgreement->init();
	
	return s_bInitFlag;
}

void ServerAgreementManager::unInit(void)
{
	if(s_bInitFlag) {
		disconnect();
		m_pobjSocket = NULL;
		m_pobjServerAgreement->setSocket(NULL);
		m_pobjServerAgreement->unInit();
		s_bInitFlag = false;
	}
}

bool ServerAgreementManager::connect(const char *strServerIP, int nPort, int nLocalPort)
{
    if(strServerIP == NULL || s_bInitFlag == false) {
		return false;
	}

	if(m_pobjSocket->Connect(strServerIP, nPort, nLocalPort) > 0){
		return true;
	}

	return false;
}

void ServerAgreementManager::disconnect(void)
{
	if(s_bInitFlag) {
		m_pobjSocket->DisConnect();
	}
}

void ServerAgreementManager::setAgreementID(int nAgreementID)
{
	m_nAgreementID = nAgreementID;
	if(m_pobjServerAgreement == NULL) {
		m_pobjServerAgreement = getServerAgreement(m_nAgreementID);
	}
}

int ServerAgreementManager::recvData(char *strMsg)
{
	if(NULL == m_pobjServerAgreement) {
		return -1;
	}

	return m_pobjServerAgreement->recvData(strMsg);
}

int ServerAgreementManager::sendData(const char *strMsg, int nLen)
{
	if(NULL == m_pobjServerAgreement) {
		return -1;
	}

	return m_pobjServerAgreement->sendData(strMsg, nLen);
}

int ServerAgreementManager::sendData(int nBusinessNum, const PartRunStatus& stPartStatus)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stPartStatus);
}

int ServerAgreementManager::sendData(int nBusinessNum, const SysRunStatus& stSysStatus)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stSysStatus);
}

int ServerAgreementManager::sendData(int nBusinessNum, const UserRunStatus& stUserRunStatus)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stUserRunStatus);
}

int ServerAgreementManager::sendData(int nBusinessNum, const UserOperationInfo& stUserOperationInfo)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stUserOperationInfo);
}

int ServerAgreementManager::sendData(int nBusinessNum, const UserSoftwareVersion & stUserSoftwareVersion)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stUserSoftwareVersion);
}

int ServerAgreementManager::sendData(int nBusinessNum, const Time& stTime)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stTime);
}

int ServerAgreementManager::sendData(const int& nBusinessNum, const USER_CONFIG& stUSER_CONFIG)
{
	return m_pobjServerAgreement->sendData(nBusinessNum, stUSER_CONFIG);
}

ServerAgreement *ServerAgreementManager::getServerAgreement(int nAgreementID)
{
	switch(nAgreementID){
		case SERVER_AGREEMENT_ID_GB:
			return &s_objServerAgreementGB;
		default:
			break;
	}

	return NULL;
}

int ServerAgreementManager::sendHeartBeat(int nBusinessNum)
{
	return m_pobjServerAgreement->sendHeartBeat(nBusinessNum);
}

void ServerAgreementManager::confirmMsg(int nBusinessNum)
{
	m_pobjServerAgreement->confirmMsg(nBusinessNum);
}

void ServerAgreementManager::send_recovery_setting_result(int nBusinessNum, bool nRet)
{
	m_pobjServerAgreement->send_recovery_setting_result(nBusinessNum, nRet);
}

void ServerAgreementManager::syncTime(int nBusinessNum, const Time& stTime)
{
	m_pobjServerAgreement->syncTime(nBusinessNum, stTime);
}

void ServerAgreementManager::inspect_sentries(int nBusinessNum, int nFlag)
{
	m_pobjServerAgreement->inspect_sentries(nBusinessNum, nFlag);
}

void ServerAgreementManager::setUserID(unsigned long long lUserID)
{
	m_lUserID = lUserID;
}

void ServerAgreementManager::setCenterID(unsigned long long lCenterID)
{
	m_lCenterID = lCenterID;
}
