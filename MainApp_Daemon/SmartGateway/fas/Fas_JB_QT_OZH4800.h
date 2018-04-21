/******************************************************************************

  中电数通科技有限公司

 ******************************************************************************
  File Name     : Fas_JB_QT_OZH4800.h
  Version       : Initial Draft
  Author        : Dengguanquan
  Created       : 2018/3/23
  Last Modified :
  Description   : Fas_JB_QT_OZH4800.cpp header file
  Function List :
  History       :
  1.Date        : 2018/3/23
    Author      : Dengguanquan
    Modification: Created file

******************************************************************************/

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * external routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

/*----------------------------------------------*
 * module-wide global variables                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * constants                                    *
 *----------------------------------------------*/

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifndef __FAS_JB_QT_OZH4800_H__
#define __FAS_JB_QT_OZH4800_H__


#include "Fas.h"
#include "Common.h"

class Fas_JB_QT_OZH4800: public Fas
{
public:
  	Fas_JB_QT_OZH4800(void);
    ~Fas_JB_QT_OZH4800(void);
    bool init(int nType,
              const char *sComDevName,
              int nBitrate = COM_BITRATE_9600,
              int nParity = COM_PARITY_NONE,
              int nDatabits = COM_DATABITS_8,
              int nStopbits = COM_STOPBITS_2,
              int nFlowCtrl = FLOW_CONTROL_NONE);
  	bool unInit(void);
    int recvData(int nComType, vector<PartRunStatus> &stPartStatus);
  	int sendData(int nComType/*zdst define data*/);

private:
    int handleMsg(PartRunStatus &stPartStatus, const unsigned char* srcBuffer, int srcLen);
};


#endif //__FAS_OZH_4800_H__ 奥瑞娜

