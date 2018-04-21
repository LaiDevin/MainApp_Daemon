#include "UdpSocket.h"
#include "Common.h"
#include <fcntl.h>  
#include <errno.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <winsock2.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <iostream>

static struct sockaddr_in s_serveraddr;

UdpSocket::UdpSocket(void)
{
    m_nSockFd = -1;
}

UdpSocket::~UdpSocket(void)
{
    DisConnect();
}

int UdpSocket::Connect(const char *sIP, int nPort, int nLocalPort)
{

    SOCKET nSockFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (nSockFd == INVALID_SOCKET) {
        printf("create socket failed\n");
        return -1;
    }

    sockaddr_in cli_sockaddr;
    cli_sockaddr.sin_family = AF_INET;
    cli_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_sockaddr.sin_port = htons(nLocalPort);
    if((bind(nSockFd, (struct sockaddr *) &cli_sockaddr, sizeof(cli_sockaddr))) < 0){
        perror("bind");
        return -1;
    }


    s_serveraddr.sin_family = AF_INET;
    s_serveraddr.sin_port = htons(nPort);
    s_serveraddr.sin_addr.S_un.S_addr = inet_addr(sIP);
    if (s_serveraddr.sin_addr.s_addr == INADDR_NONE) {
        closesocket(nSockFd);
        return -1;
    }

    m_nSockFd = nSockFd;
    return m_nSockFd;
}

void UdpSocket::DisConnect(void)
{
	if(m_nSockFd >= 0) {
        closesocket(m_nSockFd);
		m_nSockFd = -1;
	}
}

int UdpSocket::Recv(char *strBuffer)
{
    if(strBuffer == NULL) {
        return -1;
    }
    int nRecvLen = 0;
	struct timeval stTimeoutVal;
    stTimeoutVal.tv_sec  = 0;
    stTimeoutVal.tv_usec = 50000;
    fd_set read_fds;
    socklen_t nSockLen = sizeof(s_serveraddr);
    if(m_nSockFd >= 0) {
        FD_ZERO(&read_fds);
        FD_SET(m_nSockFd, &read_fds);
        int nRet = select(FD_SETSIZE, &read_fds, NULL, NULL, &stTimeoutVal);
        if(nRet > 0) {
            nRecvLen = recvfrom(m_nSockFd, strBuffer, MAX_SIZE, 0, (struct sockaddr *)&s_serveraddr, &nSockLen);
            if(nRecvLen <= 0) {
                return nRecvLen;
            }
            strBuffer[nRecvLen + 1] = '\0';
        }
    }

    return nRecvLen;
}

int UdpSocket::Send(const char* strBuffer, int nLen)
{
    if(strBuffer == NULL) {
        return -1;
    }

    int nSendLen = sendto(m_nSockFd, strBuffer, nLen, 0, (struct sockaddr *)&s_serveraddr, sizeof(s_serveraddr));
    if(nSendLen < 0){
        perror("sendto");
    }

    return nSendLen;
}
