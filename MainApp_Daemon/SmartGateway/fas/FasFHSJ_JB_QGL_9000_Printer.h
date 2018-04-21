#ifndef __FAS_FHSJ_JB_QGL_9000_PRINTER__
#define __FAS_FHSJ_JB_QGL_9000_PRINTER__

#include "Fas.h"
#include "Common.h"

class FasFHSJ_JB_QGL_9000_Printer : public Fas
{
public:
  	FasFHSJ_JB_QGL_9000_Printer(void);
  	~FasFHSJ_JB_QGL_9000_Printer(void);
    bool init(int nType,
              const char *sComDevName,
              int nBitrate = COM_BITRATE_9600,
              int nParity = COM_PARITY_NONE,
              int nDatabits = COM_DATABITS_8,
              int nStopbits = COM_STOPBITS_2,
              int nFlowCtrl = FLOW_CONTROL_NONE);
  	bool unInit(void);
  	int recvData(int nComType, vector<PartRunStatus> &vtPartStatus);
  	int sendData(int nComType);

private:
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen);
};

#endif //__FAS_FHSJ_JB_QGL_9000_PRINTER__
