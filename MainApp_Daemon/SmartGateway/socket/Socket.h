#ifndef __SOCKET_H__
#define __SOCKET_H__
#define MAX_SIZE	(2048)
#include <winsock2.h>
#include <windows.h>
#include <winsock.h>

class Socket {
public:
    Socket(void){};
    virtual ~Socket(void){};
	virtual int Connect(const char *strServerIP, int nServerPort, int nLocalPort = 0) = 0;
	virtual void DisConnect(void) = 0;
	virtual int Recv(char *strBuffer) = 0;
	virtual int Send(const char *strBuffer, int nLen) = 0;
protected:
    SOCKET m_nSockFd;
};

#endif
