#ifndef __COMMON_H__
#define __COMMON_H__

#define BUFFER_SIZE     (9162)
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])
#define MAIN_VERSION   (0x01)
#define USER_VERSION   (0x11)
#define SYS_ADDR       (0x02)


enum NETWORK_PROTOCOL{
	NETWORK_PROTOCOL_TCP = 0,
	NETWORK_PROTOCOL_UDP,
};

enum COM_TYPE {
    COM_TYPE_RS485 = 1,
    COM_TYPE_RS232,
};

struct NetworkParam
{
    char sServerIP[16];
    int nPort;
    int nProtocol;
};

struct Time {
    unsigned char nSec;
    unsigned char nMin;
    unsigned char nHour;
    unsigned char nDay;
    unsigned char nMonth;
    unsigned char nYear;
};

struct SysStatusBits
{
    unsigned short nBit0:1;     //1.正常状态     0.测试状态
    unsigned short nBit1:1;     //1.火警         0.无火警
    unsigned short nBit2:1;     //1.故障         0.无故障
    unsigned short nBit3:1;     //1.屏蔽         0.无屏蔽
    unsigned short nBit4:1;     //1.监管         0.无监管
    unsigned short nBit5:1;     //1.启动(开启)   0.关闭(关闭)
    unsigned short nBit6:1;     //1.反馈         0.无反馈
    unsigned short nBit7:1;     //1.延时状态     0.无延时
    unsigned short nBit8:1;     //1.主电故障     0.主电正常
    unsigned short nBit9:1;     //1.备电故障     0.备电正常
    unsigned short nBit10:1;    //1.总线故障     0.主线正常
    unsigned short nBit11:1;    //1.手动状态     0.自动状态
    unsigned short nBit12:1;    //1.配置变化     0.无配置变化
    unsigned short nBit13:1;    //1.复位.正常
    unsigned short nBit14:1;    //0.预留
    unsigned short nBit15:1;    //0.预留
};

struct SysRunStatus {
    unsigned char nSysType;     //系统类型
    unsigned char nSysAddr;     //系统地址
    union {
        unsigned short nStatus;
        SysStatusBits StatusBit;
    }SysStatus;
    unsigned char strPartDescription[31];
    Time time;
};

struct PartStatusBits
{
    unsigned short nBit0:1;     //1.正常状态     0.测试状态
    unsigned short nBit1:1;     //1.火警         0.无火警
    unsigned short nBit2:1;     //1.故障         0.无故障
    unsigned short nBit3:1;     //1.屏蔽         0.无屏蔽
    unsigned short nBit4:1;     //1.监管         0.无监管
    unsigned short nBit5:1;     //1.启动         0.关闭
    unsigned short nBit6:1;     //1.反馈         0.无反馈
    unsigned short nBit7:1;     //1.延时状态     0.无延时
    unsigned short nBit8:1;     //1.电源故障     0.电源正常
    unsigned short nBit9:1;     //0.预留
    unsigned short nBit10:1;    //0.预留
    unsigned short nBit11:1;    //0.预留
    unsigned short nBit12:1;    //0.预留
    unsigned short nBit13:1;    //0.预留
    unsigned short nBit14:1;    //0.预留
    unsigned short nBit15:1;    //0.预留
};

struct PartRunStatus {
    int nRequestType;
    unsigned char nSysType;     //系统类型
    unsigned char nSysAddr;     //系统地址
    unsigned char nPartType;    //部件类型
    unsigned char nPartAddr[4]; //部件地址
    union {
        unsigned short nStatus;
        PartStatusBits StatusBit;
    }PartStatus;
    unsigned char strPartDescription[31];
    Time time;
};

struct PartSimula{
	unsigned char SysType;              //系统类型
    unsigned char SysAddr;              //系统地址
    unsigned char PartsType;			//部件类型
    unsigned char PartsAddr[4];         //部件地址
    unsigned char SimulaType;           //模拟量类型
    unsigned char SimulaValue;          //模拟量值
};

struct BuldingOperationInfoBits
{
	unsigned short nBit0:1;          //1.复位；		 0.无操作
    unsigned short nBit1:1;          //1.消音；		 0.无操作
    unsigned short nBit2:1;          //1.手动报警；       0.无操作
    unsigned short nBit3:1;          //1.警情消除；       0.无操作
    unsigned short nBit4:1;          //1.自检；		 0.无操作
    unsigned short nBit5:1;          //1.确认；		 0.无操作
    unsigned short nBit6:1;          //1.测试；		 0.无操作
    unsigned short nBit7:1;          //0.预留；
};

struct BuldingOperationInfo{
	unsigned char nSysType; 	//系统类型
	unsigned char nSysAddr; 	//系统地址
	unsigned char OperationID;  //操作员编号
	union {
		unsigned short nStatus;
        BuldingOperationInfoBits OperationBit;
	}OperationFlag;
	//unsigned char strPartDescription[31];
    Time time;
};

struct BuldingSoftwareVersion
{
    unsigned char SysType;              //系统类型
    unsigned char SysAddr;              //系统地址
    unsigned char MajorVer;				//主版本号
    unsigned char MinorVer;				//次版本号
};

struct BuldingConfig
{
    unsigned char SysType;              //系统类型
    unsigned char SysAddr;              //系统地址
    unsigned char ConfigSize;			//说明长度0-255
    unsigned char ConfigMsg[255];		//系统配置说明
};

struct BuldingPartConfig
{
    unsigned char SysType;              //系统类型
    unsigned char SysAddr;              //系统地址
    unsigned char PartsType;			//部件类型
    unsigned char PartsAddr[4];			//部件地址
    unsigned char PartsMsg[31];			//部件说明
};

struct SysTime{
	unsigned char SysType;              //系统类型
    unsigned char SysAddr;              //系统地址
    Time time;
};

struct UserRunStatusBits
{
	unsigned short nBit0:1;          //1.正常监视；						0.测试状态
    unsigned short nBit1:1;          //1.火警；						0.无火警
    unsigned short nBit2:1;          //1.故障；						0.无故障
    unsigned short nBit3:1;          //1.主电故障；						0.主电正常
    unsigned short nBit4:1;          //1.备电故障；						0.备电正常
    unsigned short nBit5:1;          //1.与监控中心通信信道故障；               0.通信信道正常
    unsigned short nBit6:1;          //1.检测连接线路故障；					0.检测连接线路正常
    unsigned short nBit7:1;          //0.预留
};

struct UserRunStatus{
	union {
		unsigned short nStatus;
        UserRunStatusBits StatusBit;
	}RunStatus;
	//unsigned char strUserRunDescription[31];
    Time time;
};

struct UserOperationInfoBits
{
	unsigned short nBit0:1;          //1.复位；			0.无操作
    unsigned short nBit1:1;          //1.消音；			0.无操作
    unsigned short nBit2:1;          //1.手动报警；          0.无操作
    unsigned short nBit3:1;          //1.警情消除；          0.无操作
    unsigned short nBit4:1;          //1.自检；			0.无操作
    unsigned short nBit5:1;          //1.查岗应答；          0.无操作
    unsigned short nBit6:1;          //1.测试；			0.无操作
    unsigned short nBit7:1;          //0.预留；
};

struct UserOperationInfo{
	int OperatorID;
	union {
		unsigned short nStatus;
        UserOperationInfoBits OperationBit;
	}OperationFlag;
	//unsigned char strUserOperationDescription[31];
    Time time;
};

struct UserSoftwareVersion
{
    unsigned char MajorVer;            //主版本号
    unsigned char MinorVer;            //次版本号
};

struct USER_CONFIG
{
    unsigned char ConfigSize;              //说明长度0-255
    char *ConfigMsg;          //系统配置说明
};

#endif
