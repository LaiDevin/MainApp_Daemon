#ifndef __UDP_SOCKET_H__
#define __UDP_SOCKET_H__

#include "Socket.h"

class UdpSocket : public Socket
{
public:
	UdpSocket(void);
	~UdpSocket(void);
	int Connect(const char *sIP, int nPort, int nLocalPort = 0);
	void DisConnect(void);
	int Recv(char *strBuffer);
	int Send(const char* strBuffer, int nLen);

};

#endif //__UDP_SOCKET_H__
