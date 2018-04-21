#ifndef __COM_H__
#define __COM_H__

#include "ComDefine.h"
#include <Windows.h>
#include <vector>
#include <process.h>
#include <iostream>
#include <string>

class Com {
public:
    Com();
    ~Com();
    bool Open(const char* sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl);
	void Close(void);
    int Recv(unsigned char *buff);
    bool Send(const unsigned char* m_szWriteBuffer, int m_nToSend);

    /**
     * @brief GetAllSerialPorts
     * @return 返回可用的COM名字
     */
    std::vector<std::wstring> GetAllSerialPorts();

private:
    /** 串口句柄 */
    HANDLE  m_hComm;
    COMSTAT comstat;
    OVERLAPPED m_ov;
};
#endif
