#include "Com.h"

Com::Com()
{
    m_hComm = INVALID_HANDLE_VALUE;
}

Com::~Com()
{
    Close();
}

bool Com::Open(const char* sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
{
    /** 打开指定的串口 */
    m_hComm = CreateFileA(sComDevName,  /** 设备名,COM1,COM2等 */
                          GENERIC_READ | GENERIC_WRITE, /** 访问模式,可同时读写 */
                          0,                            /** 共享模式,0表示不共享 */
                          NULL,                         /** 安全性设置,一般使用NULL */
                          OPEN_EXISTING,                /** 该参数表示设备必须存在,否则创建失败 */
                          0,
                          0);
    if (m_hComm == INVALID_HANDLE_VALUE){
        printf("invalid handle value!");
        return FALSE;
    }

    DCB dcb;
    memset(&dcb,0,sizeof(dcb));
    if(!GetCommState(m_hComm,&dcb)){//获取当前DCB配置
        printf("get dcb fail");
        Close();
        return FALSE;
    }
    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = nBitrate;
    dcb.ByteSize = nDatabits;
    dcb.StopBits = nStopbits;
    dcb.Parity = nParity;
    dcb.fDtrControl = nFlowCtrl;
    if(!SetCommState(m_hComm,&dcb)){
        printf("set dcb fail");
        Close();
        return FALSE;
    }

    COMMTIMEOUTS CommTimeOuts;
    GetCommTimeouts(m_hComm,&CommTimeOuts);
    CommTimeOuts.ReadIntervalTimeout = MAXDWORD;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
    CommTimeOuts.ReadTotalTimeoutConstant = 0;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 10;
    CommTimeOuts.WriteTotalTimeoutConstant = 1000;
    if(!SetCommTimeouts(m_hComm,&CommTimeOuts)){
        printf("set timeout fail");
        Close();
        return FALSE;
    }

    static const int g_nZhenMax = 32768;
    if (!SetupComm(m_hComm, g_nZhenMax, g_nZhenMax)){
        Close();
        return FALSE;
    }
    return TRUE;
}

void Com::Close(void)
{
    /** 如果有串口被打开，关闭它 */
    if (m_hComm != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hComm);
        m_hComm = INVALID_HANDLE_VALUE;
    }
}

int Com::Recv(unsigned char *buff)
{
    //char buf[101];
    memset(buff, 0, 101);
    DWORD nLenOut = 0;
    COMSTAT ComStat;
    DWORD dwError=0;
    ClearCommError(m_hComm,&dwError,&ComStat);
    if(ReadFile(m_hComm, buff, ComStat.cbInQue, &nLenOut, NULL)){
        if(nLenOut){//成功
        } else {//超时
            printf("time out\n");
        }
    } else {//失败
        printf("read() error!\n");
    }
    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
    return ComStat.cbInQue;
}

bool Com::Send(const unsigned char *m_szWriteBuffer, int m_nToSend)
{
    BOOL bWrite = TRUE;
    BOOL bResult = TRUE;
    DWORD BytesSent = 0;
    HANDLE m_hWriteEvent = NULL;
    ResetEvent(m_hWriteEvent);

    printf("m_nToSend = %d\n",m_nToSend);
    if (bWrite){
        m_ov.Offset = 0;
        m_ov.OffsetHigh = 0;
        // Clear buffer
        bResult = WriteFile(m_hComm, // Handle to COMM Port
                            m_szWriteBuffer, // Pointer to message buffer in calling finction
                            m_nToSend,      // Length of message to send
                            &BytesSent,     // Where to store the number of bytes sent
                            &m_ov );        // Overlapped structure
        if (!bResult) {
            DWORD dwError = GetLastError();
            switch (dwError){
                case ERROR_IO_PENDING:
                {
                    // continue to GetOverlappedResults()
                    BytesSent = 0;
                    bWrite = FALSE;
                    break;
                }
                default:
                {
                    // all other error codes
                    break;
                }
            }
        }
    } // end if(bWrite)
    if (!bWrite){
        bWrite = TRUE;
        bResult = GetOverlappedResult(m_hComm,   // Handle to COMM port
                                      &m_ov,     // Overlapped structure
                                      &BytesSent,    // Stores number of bytes sent
                                      TRUE);         // Wait flag

        // deal with the error code
        if (!bResult) {
            printf("GetOverlappedResults() in WriteFile()");
        }
    }

    // Verify that the data size send equals what we tried to send
    if (BytesSent != m_nToSend){
        printf("WARNING: WriteFile() error.. Bytes Sent: %d; Message Length: %d\n", BytesSent, strlen((char*)m_szWriteBuffer));
    }
    return TRUE;
}

std::vector<std::wstring> Com::GetAllSerialPorts()
{
    const int MAX_KEY_LENGTH=255;
    const int MAX_VALUE_NAME = 16383;

    std::vector<std::wstring> vctTemp;

    HKEY hTestKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hTestKey) != ERROR_SUCCESS)
    {
        return vctTemp;
    }

    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name
    DWORD    cchClassName = MAX_PATH;  // size of class string
    DWORD    cSubKeys = 0;               // number of subkeys
    DWORD    cbMaxSubKey;              // longest subkey size
    DWORD    cchMaxClass;              // longest class string
    DWORD    cValues;              // number of values for key
    DWORD    cchMaxValue;          // longest value name
    DWORD    cbMaxValueData;       // longest value data
    DWORD    cbSecurityDescriptor; // size of security descriptor
    FILETIME ftLastWriteTime;      // last write time

    DWORD i, retCode;

    TCHAR  achValue[MAX_VALUE_NAME];
    DWORD cchValue = MAX_VALUE_NAME;
    BYTE byteData[MAX_VALUE_NAME];
    DWORD dwData = MAX_VALUE_NAME;

    retCode = RegQueryInfoKey(
        hTestKey,
        achClass,
        &cchClassName,
        NULL,
        &cSubKeys,
        &cbMaxSubKey,
        &cchMaxClass,
        &cValues,
        &cchMaxValue,
        &cbMaxValueData,
        &cbSecurityDescriptor,
        &ftLastWriteTime);
    if (cValues)
    {
        for (i = 0, retCode = ERROR_SUCCESS; i < cValues; i++)
        {
            cchValue = MAX_VALUE_NAME;
            achValue[0] = '\0';
            dwData = MAX_VALUE_NAME;
            byteData[0] = '\0';

            retCode = RegEnumValue(hTestKey, i,
                achValue,
                &cchValue,
                NULL,
                NULL,
                byteData,
                &dwData);

            if (retCode == ERROR_SUCCESS)
            {
                std::wstring strCOM = (wchar_t*)byteData;
                vctTemp.push_back(strCOM);
            }
        }
    }

    RegCloseKey(hTestKey);

    return vctTemp;
}
