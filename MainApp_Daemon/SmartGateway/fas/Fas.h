#ifndef __FAS_H__
#define __FAS_H__

#include "Com.h"
#include "Common.h"
#include "AgreementDefine.h"
#include <stdio.h>
#include <vector>

#ifdef LINUX
#include "GPIO.h"
#endif

using namespace std;

class Fas {
public:
    Fas(){}
    virtual ~Fas(){}

    virtual bool init(int nType,
                      const char *sComDevName,
                      int nBitrate = COM_BITRATE_9600,
                      int nParity = COM_PARITY_NONE,
                      int nDatabits = COM_DATABITS_8,
                      int nStopbits = COM_STOPBITS_2,
                      int nFlowCtrl = FLOW_CONTROL_NONE) = 0;
	virtual bool unInit(void) = 0;
	//返回值(-2:失去连接 -1:连接正常无数据 大于0:连接正常有数据)
    virtual int recvData(int nComType, vector<PartRunStatus> &stPartStatus) = 0;
	virtual int sendData(int nComType/*zdst define data*/) = 0;
	#ifdef LINUX
	void setGPIOOperator(GPIO *pobjGPIO = NULL)
	{
		m_obj485Com.setGPIOOperator(pobjGPIO);
	}
	#endif

	Com *getComObj(int nComType) {
		if(nComType == COM_TYPE_RS232) {
        	return &m_obj232Com;
	    } else if(nComType == COM_TYPE_RS485) {
	        return &m_obj485Com;
	    }

	    return NULL;
	};

	void printfMsg(int nLen, const unsigned char *strMsg)
	{
		if(0 > nLen || NULL == strMsg) {
			return;
		}
		printf("Len:%d\n", nLen);
		for(int i = 0; i < nLen; ++i) {
			printf("%02X ", strMsg[i]);
		}
		printf("\n");
	};

	virtual void setFasID(int nFasID)
	{
		m_nFasID = nFasID;
	};

protected:
	Com m_obj232Com;
	Com m_obj485Com;
	int m_nFasID;
};

#endif //__FAS_H__
