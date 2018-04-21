#include "TcpSocket.h"
#include "Common.h"
#include <errno.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ws2tcpip.h>

#define CONNECT_TIME_OUT_SEC	(10)
#define CONNECT_TIME_OUT_USEC	(0)
TcpSocket::TcpSocket(void)
{
	m_nSockFd = -1;
}

TcpSocket::~TcpSocket(void)
{
    DisConnect();
}

int TcpSocket::Connect(const char *sIP, int nPort, int nLocalPort)
{ 
    SOCKET nSockFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nSockFd == INVALID_SOCKET) {
        printf("socket failed\n");
        return -1;
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(nPort);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(sIP);

    unsigned long nUl = 1;
    int nError= -1;
    int nLen = sizeof(int);

    ioctlsocket(nSockFd, FIONBIO, &nUl);

    if(connect(nSockFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)  {
        struct timeval stTimeoutVal;
        fd_set set;
        stTimeoutVal.tv_sec = CONNECT_TIME_OUT_SEC;
        stTimeoutVal.tv_usec = CONNECT_TIME_OUT_USEC;
        FD_ZERO(&set);
        FD_SET(nSockFd, &set);
        if(select(nSockFd+1, NULL, &set, NULL, &stTimeoutVal) > 0){
            getsockopt(nSockFd, SOL_SOCKET, SO_ERROR, (char *)&nError, (socklen_t *)&nLen);
        }
    }

    if(nError == 0) {
        m_nSockFd = nSockFd;
    } else {
        closesocket(nSockFd);
        return -1;
    }

    nUl = 0;
    ioctlsocket(nSockFd, FIONBIO, &nUl);

    return nSockFd;
}

void TcpSocket::DisConnect(void)
{
    if(m_nSockFd > 0) {
        closesocket(m_nSockFd);
        m_nSockFd = -1;
    }
}

int TcpSocket::Recv(char *sBuffer)
{
    if(NULL == sBuffer) {
        return -1;
    }

    int nRecvLen = 0;
    struct timeval stTimeoutVal;
    stTimeoutVal.tv_sec  = 0;
    stTimeoutVal.tv_usec = 50000;
    fd_set read_fds;
    if(m_nSockFd >= 0) {
        FD_ZERO(&read_fds);
        FD_SET(m_nSockFd, &read_fds);
        int nRet = select(FD_SETSIZE, &read_fds, NULL, NULL, &stTimeoutVal);
        if(nRet > 0) {
            nRecvLen = ::recv(m_nSockFd, sBuffer, BUFFER_SIZE, 0);
            if(nRecvLen <= 0) {
                return nRecvLen;
            }
            sBuffer[nRecvLen+1] = '\0';
        }
    }

    return nRecvLen;
}

int TcpSocket::Send(const char* strBuffer, int nLen)
{
    if(NULL == strBuffer) {
        return -1;
    }

    int nSendLen = 0;
    if(m_nSockFd >= 0) {
        nSendLen = ::send(m_nSockFd, strBuffer, nLen, 0);
    }

    return nSendLen;
}
