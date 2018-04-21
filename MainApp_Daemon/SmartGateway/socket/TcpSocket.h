#ifndef __TCP_SOCKET_H__
#define __TCP_SOCKET_H__

#include "Socket.h"

class TcpSocket : public Socket 
{
public:
	TcpSocket(void);
	~TcpSocket(void);
	int Connect(const char *sIP, int nPort, int nLocalPort = 0);
	void DisConnect(void);
	int Recv(char *strBuffer);
	int Send(const char* strBuffer, int nLen);
};

#endif //__TCP_SOCKET_H__
