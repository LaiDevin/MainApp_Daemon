#include "FasFuan5101.h"
#include <stdio.h>
#include <string.h>

#define MSG_DEBUG	(1)

struct Info {
    int nIndex;
    unsigned char strMsg[64];
};

#if 0
static struct Info s_FuncAttrInfo[] = {
    { 0xc1, "启动" },
    { 0xc2, "故障" },
    { 0xc3, "火警" },
    { 0xc5, "屏蔽" },
    { 0xc6, "其他" },
};
#endif

static struct Info s_DevTypeInfo[] = {
    {0, "离子探头"},
    {1, "光电探头"},
    {2, "感温探头"},
    {3, "烟复合头"},
    {4, "光复合头"},
    {5, "三复合头"},
    {6, "激光探头"},
    {7, "一氧化碳"},
    {8, "可燃气体"},
    {9, "空气采样"},
    {10, "手动按钮"},
    {11, "消防栓钮"},
    {12, "压力开关"},
    {13, "水流指示"},
    {14, "信号阀"},
    {15, "接口模块"},
    {16, "消防栓泵启"},
    {17, "消防栓泵停"},
    {18, "喷淋泵启"},
    {19, "喷淋泵停"},
    {20, "泡沫泵启"},
    {21, "泡沫泵停"},
    {22, "备用泵启"},
    {23, "备用泵停" },
    {24, "稳压泵启" },
    {25, "稳压泵停" },
    {26, "雨淋泵启" },
    {27, "雨淋泵停" },
    {28, "正压风机启"},
    {29, "正压风机停"},
    {30, "排烟风机启"},
    {31, "排烟风机停"},
    {32, "干粉系统启"},
    {33, "干粉系统停"},
    {34, "新风机启"},
    {35, "新风机停"},
    {36, "发电机"},
    {37, "消防泵故障"},
    {38, "喷淋泵故障"},
    {39, "稳压泵故障"},
    {40, "水冷却"},
    {41, "泡沫喷淋"},
    {42, "送风机启"},
    {43, "送风机停"},
    {44, "讯响器"},
    {45, "试验阀"},
    {46, "消防栓灯"},
    {47, "泡沫阀"},
    {48, "空调"},
    {49, "消防广播"},
    {50, "消防警铃"},
    {51, "防火门"},
    {52, "防火阀"},
    {53, "排烟阀"},
    {54, "电梯"},
    {55, "疏散指示"},
    {56, "事故照明"},
    {57, "消防电源"},
    {58, "水幕"},
    {59, "送风口"},
    {60, "挡烟垂壁"},
    {61, "雨淋阀"},
    {62, "湿式阀"},
    {63, "水冷却阀"},
    {64, "卷帘半降"},
    {65, "卷帘全降"},
    {66, "排风机启"},
    {67, "排风机停"},
    {68, "气体灭火启"},
    {69, "气体灭火停"},
    {70, "备用设备启"},
    {71, "备用设备停"},
    {72, "码座离子"},
    {73, "码座光电"},
    {74, "码座感温"},
    {75, "低倍泡沫"},
    {76, "高倍泡沫"},
    {77, "水雾"},
    {78, "普通光电"},
    {79, "普通温感"},
    {80, "气体报警"},
    {81, "气体喷放"},
    {82, "气体故障"},
    {83, "气体失重"},
    {84, "高水位"},
    {85, "低水位"},
    {86, "电话模块"},
    {87, "总线电话"},
    {88, "总线显示盘"},
    {89, "声光警报器"},
    {90, "气体灭火"},
};

#if 0
static struct Info s_DevStatusInfo[] = {
    { 0x0, "正常" },           //探头、模块
    { 0x1, "地址丢失"},         //探头、模块
    { 0x2, "设备错误"},         //探头、模块
    { 0x3, "重码" },           //探头、模块
    { 0x4, "动作失败" },        //探头、模块
    { 0x5, "火警" },           //探头、输入模块
    { 0x6, "现场动作" },        //模块
    { 0x7, "动作成功" },        //模块
    { 0x8, "开路" },           //模块
    { 0x9, "故障" },           //主电、备电、充电
    { 0xa, "故障恢复" },        //主电、备电、充电
    { 0xb, "短路" },           //模块
    //0xc 0xd未知
    { 0xe, "多线短路" },        //多线
    { 0xf, "多线开路" },        //多线
    { 0x10, "多线故障" },        //多线
    { 0x11, "多线正常" },        //多线
    { 0x12, "自诊断故障" },      //探头
    { 0x13, "污染" },           //探头
    { 0x14, "通信故障" },        //回路、系统板、显示盘、多线、计算机
    { 0x15, "通信正常" },        //回路、系统板、显示盘、多线、计算机
    { 0x16, "flash故障" },      //系统板
    { 0x17, "flash故障恢复" },   //系统板
    { 0x18, "多线启动" },        //多线
    { 0x19, "多线停止" },        //多线
    { 0x1a, "多线启成功" },       //多线
    { 0x1b, "多线停成功" },       //多线
    { 0x1c, "多线现场启" },       //多线
    { 0x1d, "多线现场停" },       //多线
    { 0x1e, "多线启失败" },       //多线
    { 0x1f, "多线停失败" },       //多线
    //0x20未知
    { 0x21, "开机" },          //控制器
    { 0x22, "复位" },          //控制器
    { 0x23, "手动转自动" },     //控制器
    { 0x24, "火警确认" },       //控制器
    { 0x25, "预警确认" },       //控制器
    { 0x26, "预警复位" },       //控制器
    //0x27未知
    { 0x28, "屏蔽" },          //探头、模块
    { 0x29, "屏蔽解除" },       //探头、模块
    { 0x2a, "屏蔽" },          //回路
    { 0x2b, "屏蔽解除" },       //回路
    { 0x2c, "手动启动" },       //模块
    { 0x2d, "手动停止" },       //模块
    { 0x2e, "联动启动" },      //模块
    { 0x2f, "联动延时" },      //模块
    { 0x30, "总线短路" },       //回路
    { 0x31, "24V总线异常" },    //回路
    { 0x32, "24V总线漏电流异" }, //回路
    { 0x33, "总线参考电压异常" }, //回路
    { 0x34, "5v总线漏电流异常" }, //回路
    { 0x35, "5V总线电压异常" },  //回路
    { 0x36, "0V总线电压异常" },  //回路
    { 0x37, "0V总线漏电流异常" }, //回路
    { 0x38, "高压中断" },        //回路
    { 0x39, "总线电压过低" },     //回路
    { 0x3a, "总线电压过高" },     //回路
    { 0x3b, "正电流异常" },       //回路
    { 0x3c, "漏电流异常" },       //回路
    { 0x3d, "总线负线异常" },     //回路
    { 0x3e, "总线正线异常" },     //回路
    { 0x3f, "总线不能上电" },     //回路
    { 0x40, "总线正常" },        //回路
    { 0x41, "预警" },           //探头、输入模块
    { 0x42, "故障" },           //探头、模块
    { 0x43, "多线启动" },       //多线
    { 0x44, "多线停止" },       //多线
    { 0x45, "多线启成功" },     //多线
    { 0x46, "多线启失败" },     //多线
    { 0x47, "多线停成功" },     //多线
    { 0x48, "多线停失败" },     //多线
};

static struct Info s_InfoType[] = {
    { 0x1, "探头" },
    { 0x2, "模块" },
    { 0x3, "回路"},
    { 0x4, "主电" },
    { 0x5, "备电" },
    { 0x6, "计算机" },
    { 0x7, "显示盘" },
    { 0x8, "多线" },
    { 0x9, "控制器" },
    { 0xa, "网络模块" },
    { 0xb, "充电" },
    { 0xc, "系统板"},
};
#endif


const unsigned char *Type2Str(int nIndex, const Info *stInfo, int nLen) 
{
    if(NULL == stInfo) {
        return NULL;
    }

    for(int i = 0; i < nLen; ++ i) {
        if(stInfo[i].nIndex == nIndex) {
            return stInfo[i].strMsg;
        }
    }
    return NULL;
}

static unsigned short const crc_table[256] = {
    0x0000U, 0x1021U, 0x2042U, 0x3063U, 0x4084U, 0x50a5U, 0x60c6U, 0x70e7U,
    0x8108U, 0x9129U, 0xa14aU, 0xb16bU, 0xc18cU, 0xd1adU, 0xe1ceU, 0xf1efU,
    0x1231U, 0x0210U, 0x3273U, 0x2252U, 0x52b5U, 0x4294U, 0x72f7U, 0x62d6U,
    0x9339U, 0x8318U, 0xb37bU, 0xa35aU, 0xd3bdU, 0xc39cU, 0xf3ffU, 0xe3deU,
    0x2462U, 0x3443U, 0x0420U, 0x1401U, 0x64e6U, 0x74c7U, 0x44a4U, 0x5485U,
    0xa56aU, 0xb54bU, 0x8528U, 0x9509U, 0xe5eeU, 0xf5cfU, 0xc5acU, 0xd58dU,
    0x3653U, 0x2672U, 0x1611U, 0x0630U, 0x76d7U, 0x66f6U, 0x5695U, 0x46b4U,
    0xb75bU, 0xa77aU, 0x9719U, 0x8738U, 0xf7dfU, 0xe7feU, 0xd79dU, 0xc7bcU,
    0x48c4U, 0x58e5U, 0x6886U, 0x78a7U, 0x0840U, 0x1861U, 0x2802U, 0x3823U,
    0xc9ccU, 0xd9edU, 0xe98eU, 0xf9afU, 0x8948U, 0x9969U, 0xa90aU, 0xb92bU,
    0x5af5U, 0x4ad4U, 0x7ab7U, 0x6a96U, 0x1a71U, 0x0a50U, 0x3a33U, 0x2a12U,
    0xdbfdU, 0xcbdcU, 0xfbbfU, 0xeb9eU, 0x9b79U, 0x8b58U, 0xbb3bU, 0xab1aU,
    0x6ca6U, 0x7c87U, 0x4ce4U, 0x5cc5U, 0x2c22U, 0x3c03U, 0x0c60U, 0x1c41U,
    0xedaeU, 0xfd8fU, 0xcdecU, 0xddcdU, 0xad2aU, 0xbd0bU, 0x8d68U, 0x9d49U,
    0x7e97U, 0x6eb6U, 0x5ed5U, 0x4ef4U, 0x3e13U, 0x2e32U, 0x1e51U, 0x0e70U,
    0xff9fU, 0xefbeU, 0xdfddU, 0xcffcU, 0xbf1bU, 0xaf3aU, 0x9f59U, 0x8f78U,
    0x9188U, 0x81a9U, 0xb1caU, 0xa1ebU, 0xd10cU, 0xc12dU, 0xf14eU, 0xe16fU,
    0x1080U, 0x00a1U, 0x30c2U, 0x20e3U, 0x5004U, 0x4025U, 0x7046U, 0x6067U,
    0x83b9U, 0x9398U, 0xa3fbU, 0xb3daU, 0xc33dU, 0xd31cU, 0xe37fU, 0xf35eU,
    0x02b1U, 0x1290U, 0x22f3U, 0x32d2U, 0x4235U, 0x5214U, 0x6277U, 0x7256U,
    0xb5eaU, 0xa5cbU, 0x95a8U, 0x8589U, 0xf56eU, 0xe54fU, 0xd52cU, 0xc50dU,
    0x34e2U, 0x24c3U, 0x14a0U, 0x0481U, 0x7466U, 0x6447U, 0x5424U, 0x4405U,
    0xa7dbU, 0xb7faU, 0x8799U, 0x97b8U, 0xe75fU, 0xf77eU, 0xc71dU, 0xd73cU,
    0x26d3U, 0x36f2U, 0x0691U, 0x16b0U, 0x6657U, 0x7676U, 0x4615U, 0x5634U,
    0xd94cU, 0xc96dU, 0xf90eU, 0xe92fU, 0x99c8U, 0x89e9U, 0xb98aU, 0xa9abU,
    0x5844U, 0x4865U, 0x7806U, 0x6827U, 0x18c0U, 0x08e1U, 0x3882U, 0x28a3U,
    0xcb7dU, 0xdb5cU, 0xeb3fU, 0xfb1eU, 0x8bf9U, 0x9bd8U, 0xabbbU, 0xbb9aU,
    0x4a75U, 0x5a54U, 0x6a37U, 0x7a16U, 0x0af1U, 0x1ad0U, 0x2ab3U, 0x3a92U,
    0xfd2eU, 0xed0fU, 0xdd6cU, 0xcd4dU, 0xbdaaU, 0xad8bU, 0x9de8U, 0x8dc9U,
    0x7c26U, 0x6c07U, 0x5c64U, 0x4c45U, 0x3ca2U, 0x2c83U, 0x1ce0U, 0x0cc1U,
    0xef1fU, 0xff3eU, 0xcf5dU, 0xdf7cU, 0xaf9bU, 0xbfbaU, 0x8fd9U, 0x9ff8U,
    0x6e17U, 0x7e36U, 0x4e55U, 0x5e74U, 0x2e93U, 0x3eb2U, 0x0ed1U, 0x1ef0U
};

static int crc16(int crc,const unsigned char *data, int  len)
{
    for(int i = 1; i < len; ++i) {
        crc = crc_table[(crc >> 8 ^ data[i]) & 0xffU] ^ (crc << 8);
    }

    return crc;
}


static void printFASTime(const unsigned char *strBuffer)
{
	if(NULL != strBuffer) {
		printf("广播时间:%d年%d月%d日 %d时%d分%d秒\n", strBuffer[4]+2000, strBuffer[5],
			strBuffer[6], strBuffer[7], strBuffer[8], strBuffer[9]);
	}
}

FasFuan5101::FasFuan5101(void)
{
}

FasFuan5101::~FasFuan5101(void)
{
}

bool FasFuan5101::init(int nType, const char *sComDevName, int nBitrate, int nParity, int nDatabits, int nStopbits, int nFlowCtrl)
{
    if(nType == COM_TYPE_RS232) {
       if(!m_obj232Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
       }
    } else if(nType == COM_TYPE_RS485) {
       if(!m_obj485Com.Open(sComDevName, nBitrate, nParity, nDatabits, nStopbits, nFlowCtrl)) {
            return false;
       }
    }

    return true;
}

bool FasFuan5101::unInit(void)
{
	m_obj232Com.Close();
    m_obj485Com.Close();
	return false;
}

void FasFuan5101::reply(Com *pobjCom, unsigned short addr, bool bStatus)
{
	unsigned char strBuffer[8];
    strBuffer[0] = 0xF0;
    strBuffer[1] = addr;
    strBuffer[2] = 0x03;                              //数据长度：3
    strBuffer[3] = 0xA2;
    strBuffer[4] = (bStatus)?(0x00):(0xA1);           //数据标识：0x00 正常；0xA1 查询所有事件
    strBuffer[5] = 0x00;                              //事件个数：固定为 0。
    int checksum = crc16(0, strBuffer, 6);
    strBuffer[6] = (checksum >> 8);
    strBuffer[7] = (checksum & 0xFF);
	pobjCom->Send(strBuffer, 8);
}

int FasFuan5101::handleMsg(PartRunStatus &stPartStatus, const unsigned char* buffer, int nLen)
{
    if(buffer == NULL) {
        return 0;
    }

    if(25 != nLen && 45 != nLen) {
        return 0;
    }
	
    stPartStatus.nSysType = SYS_TYPE_FIRE_ALARM_SYSTEM;
    stPartStatus.nSysAddr = SYS_ADDR;
    stPartStatus.nPartType = PARTS_TYPE_ALARM_CONTROLLER;
    //部件地址
    stPartStatus.nPartAddr[0] = buffer[8];
    stPartStatus.nPartAddr[1] = buffer[15];
    stPartStatus.nPartAddr[2] = buffer[16];
    stPartStatus.nPartAddr[3] = buffer[20];
    //Time
    stPartStatus.time.nYear = buffer[9];
    stPartStatus.time.nMonth = buffer[10];
    stPartStatus.time.nDay = buffer[11];
    stPartStatus.time.nHour = buffer[12];
    stPartStatus.time.nMin = buffer[13];
    stPartStatus.time.nSec = buffer[14];
    memset(&stPartStatus.PartStatus.StatusBit, 0, sizeof(stPartStatus.PartStatus.StatusBit));
    stPartStatus.nRequestType = FAS_REQUEST_SENDEVENT;
    switch(buffer[6]) {
        case 0x00: //正常
            stPartStatus.PartStatus.StatusBit.nBit0 = 1;
            break;
        case 0x01://地址丢失
            stPartStatus.PartStatus.StatusBit.nBit2 = 1;
            break;
        case 0x05://火警
            stPartStatus.PartStatus.StatusBit.nBit1 = 1;
            break;
        default:
            stPartStatus.nRequestType = FAS_REQUEST_NONE;
            break;
    }

    unsigned char strMsg[64] = {'\0'};
    if(0x01 == buffer[5] || 0x02 == buffer[5] || 0x08 == buffer[5]) {//模块 探头 多线
        sprintf((char *)strMsg, "%s|%d栋%d区%d层 ",Type2Str(buffer[22], s_DevTypeInfo, ARRAY_SIZE(s_DevTypeInfo)), buffer[17], buffer[18], buffer[19]);
        if(45 == nLen) {
            memcpy((char *)strMsg + strlen((char *)strMsg), (unsigned char *)buffer[23], 20);
        }
    }
    memcpy(stPartStatus.strPartDescription, (char *)strMsg, 31);

    return nLen;
}

#define CONNECT_TIME_OUT    (40)
static int s_nRS485_TimeOut = CONNECT_TIME_OUT;
static int s_nRS232_TimeOut = CONNECT_TIME_OUT;
#define RECV_SIZE   (1024)
#define NON_DATA_LEN    (5)
static unsigned char s_RemainBuffer[RECV_SIZE] = {0};
static int  s_nRemainLen = 0;
static bool s_bDateRemain = false;
static bool s_bDateUsable = false;
int FasFuan5101::recvData(int nComType, vector<PartRunStatus> &vtPartStatus)
{
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }

    unsigned char strRecvBuffer[RECV_SIZE] = {'\0'};
	int nLen = pobjCom->Recv(strRecvBuffer);

#if 0
	if(nLen > 0){
		printfMsg(nLen, strRecvBuffer);
	}
#endif

	if(nLen > 0 || s_bDateRemain) {
        if(COM_TYPE_RS232 == nComType){
            s_nRS232_TimeOut = CONNECT_TIME_OUT;
        } else if(COM_TYPE_RS485 == nComType) {
            s_nRS485_TimeOut = CONNECT_TIME_OUT;
        }

        if(s_bDateRemain){
            if(nLen > 0){
                memcpy(s_RemainBuffer + s_nRemainLen, strRecvBuffer, nLen);
                s_nRemainLen += nLen;
            }
            memcpy(strRecvBuffer, s_RemainBuffer, s_nRemainLen);
            nLen = s_nRemainLen;
        }

        for(int i = 0; i < nLen; ++ i) {
            if(strRecvBuffer[i] == 0xF0){
                if((strRecvBuffer[i + 2] + NON_DATA_LEN) == (nLen - i)) {
                    if(s_bDateRemain) {
                        memset(s_RemainBuffer, 0, RECV_SIZE);
                        s_bDateRemain = false;
                        s_nRemainLen = 0;
                    }
                    s_bDateUsable = true;
                    unsigned char *strTmep = strRecvBuffer + i;
                    memcpy(strRecvBuffer, strTmep, (strRecvBuffer[i + 2] + NON_DATA_LEN));
                    nLen = strRecvBuffer[i + 2] + NON_DATA_LEN;
                } else if((strRecvBuffer[i + 2] + NON_DATA_LEN) < (nLen - i)){
                    unsigned char *strTmep = strRecvBuffer + i + (strRecvBuffer[i + 2] + NON_DATA_LEN);
                    memcpy(s_RemainBuffer, strTmep, nLen - (i + (strRecvBuffer[i + 2] + NON_DATA_LEN)));
                    s_nRemainLen = nLen - (i + (strRecvBuffer[i + 2] + NON_DATA_LEN));
                    s_bDateRemain = true;
                    s_bDateUsable = true;
                    strTmep = strRecvBuffer + i;
                    memcpy(strRecvBuffer, strTmep, (strRecvBuffer[i + 2] + NON_DATA_LEN));
                    nLen = strRecvBuffer[i + 2] + NON_DATA_LEN;
                } else {
                    memcpy(s_RemainBuffer, strRecvBuffer, nLen - i);
                    s_nRemainLen += (nLen - i);
                    s_bDateRemain = true;
                    s_bDateUsable = false;
                }
                break;
            }
        }

        if(!s_bDateUsable){
            return -1;
        }

		PartRunStatus stPartStatus;
		switch(strRecvBuffer[3]) {
			case 0xA1: //广播复位
				if(nLen == 6) {
                    stPartStatus.nRequestType = FAS_REQUEST_RESET;
				}
				break;
			case 0xA2: //巡检
				if(nLen == 6) {
					reply(pobjCom, strRecvBuffer[1], true);
				}
				break;
			case 0xA4: //广播时间
				if(nLen == 12) {
					printFASTime(strRecvBuffer);
				}
				break;
			case 0xA5: //发送事件
                if(25 == nLen || 45 == nLen) {
                    if(handleMsg(stPartStatus, strRecvBuffer, nLen)) {
						vtPartStatus.push_back(stPartStatus);
                        break;
                    }
                }
				break;
			default:
				break;
		}
	}

    if(nLen <= 0){
        if(COM_TYPE_RS232 == nComType){
            -- s_nRS232_TimeOut;
            if(0 == s_nRS232_TimeOut) {
                s_nRS232_TimeOut = CONNECT_TIME_OUT;
                return -2;
            }
        } else if(COM_TYPE_RS485 == nComType) {
            -- s_nRS485_TimeOut;
            if(0 == s_nRS485_TimeOut) {
                s_nRS485_TimeOut = CONNECT_TIME_OUT;
                return -2;
            }
        }
    }

	return nLen;
}

int FasFuan5101::sendData(int nComType/*zdst define data*/)
{
	unsigned char sBuffer[1024] = {'\0'};
    int nLen = 0;
    Com *pobjCom = getComObj(nComType);
    if(NULL == pobjCom) {
        return -1;
    }
	return pobjCom->Send(sBuffer, nLen);	
}
