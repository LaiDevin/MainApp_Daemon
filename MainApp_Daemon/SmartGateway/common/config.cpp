#include "config.h"
#include <QSettings>
#include <QFile>
#include <QProcess>
#include<QTextStream>
#include<QDebug>
//#include "myhelper.h"
//#include "ip_ioctl.h"
//#include "version.h"
#include "Common.h"

struct ConfigGroup g_stConfigGroup;
struct OtherGroup g_stOtherGroup;
typedef struct {
    //ini文件对应
    struct {
        QString NoteName;   //节点
        QString KeyName;    //键值
        QString DeaultVal;  //默认值
    }Ini;
    //全局变量地址
    struct {
        QVariant Val;       //地址:万能型:存放指针
        QString ValType;    //值类型
        QRegExp Reg;        //校验规则:正则表达式
    }Group;
}pIni2Group;

struct WirelessInfo
{
    int nID;
    char strDescription[128];
};

//static WirelessInfo stWirelessInfo[] = {
//    {WIRELESS_ID_NONE,  "无"},
//    {WIRELESS_ID_A8500, "中国移动A8500"},
//    {WIRELESS_ID_EC20,  "中国电信EC20"},
//};

#define DNS_SURPPORT_MAX_NUM    (20)


QString config::m_strConfigFileName = "";
static QString s_strDNSConfigFileName = "dns_config.ini";
/*
1.万能型 互转 自定义指针, 譬如QVariant <=> int*
QVariant v = QVariant::FromValue((void *) yourPointerHere);
yourPointer = (YourClass *) v.value<void *>();
2.正则表达式:
0~99:           QRegExp("^([1-9]\\d|\\d)$")
0~255:延时s      QRegExp("^(255|[1-9]?[0-9]|[1,2][0-5][0-4])$")
20~99:亮度%      QRegExp("^[2-9][0-9]$")
11~18:字体       QRegExp("^1[1-8]$")

wired/false:     QRegExp("^(wired|wireless)$")
static/dhcp:     QRegExp("^(static|dhcp)$")
tcp/udp:         QRegExp("^(tcp|udp)$")
true/false:      QRegExp("^(true|false)$")
ip地址:           QRegExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)")
ip地址:           QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")
端口:             QRegExp("^([0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$")
日期:年-月-日      QRegExp("^((?:19|20)\d\d)-(0[1-9]|1[012])-(0[1-9]|[12][0-9]|3[01])$")
Base64:6位        QRegExp("^([a-zA-Z0-9+/]{214}={2})$|^([a-zA-Z0-9+/]{215}={1})$|^([a-zA-Z0-9+/]{8})$")
整数:             QRegExp("^\d+$")
所有:             QRegExp("^[\d\D]*$")
*/
const pIni2Group ConfigArray[] = {
    //注释:232/485端口协议开关
    {{"serial", "rs232",        "1"},               {QVariant::fromValue((void *)&g_stConfigGroup.serial.nRS232),        "int",     QRegExp("^([1-9]\\d|\\d)$")}},
    {{"serial", "rs485",        "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.serial.nRS485),        "int",     QRegExp("^([1-9]\\d|\\d)$")}},

    //;注释:接口=wired/wireless, 类型=static/dhcp，本机IP=192.168.7.12,本机掩码=255.255.255.0,本机网管=192.168.7.1
    {{"myip", "my_interface",   "1"},               {QVariant::fromValue((void *)&g_stConfigGroup.myip.nInterfaceType),      "int",     QRegExp("^\\d$")}},
    {{"myip", "my_wireless",    "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.myip.nWirelessID),     "int",     QRegExp("^\\d$")}},
    {{"myip", "my_type",        "dhcp"},          {QVariant::fromValue((void *)&g_stConfigGroup.myip.strType),           "QString", QRegExp("^(static|dhcp)$")}},
    {{"myip", "my_ip",          "192.168.7.12"},    {QVariant::fromValue((void *)&g_stConfigGroup.myip.strIP),          "QString", QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")}},
    {{"myip", "my_mask",        "255.255.255.0"},   {QVariant::fromValue((void *)&g_stConfigGroup.myip.strMask),        "QString", QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")}},
    {{"myip", "my_gate",        "192.168.7.1"},     {QVariant::fromValue((void *)&g_stConfigGroup.myip.strGate),        "QString", QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")}},

    //;注释:类型=tcp/udp，服务器IP=192.168.7.10，服务器端口=8000，udp监听端口
    {{"network", "type",        "tcp"},             {QVariant::fromValue((void *)&g_stConfigGroup.network.strType),        "QString", QRegExp("^(tcp|udp)$")}},
    {{"network", "ip_dns",      "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.network.nIPDNS),      "int",     QRegExp("^\\d$")}},
    {{"network", "dns_index",   "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.network.nDNSIndex),      "int",     QRegExp("^([1-9]\\d|\\d|\\d)$")}},
    {{"network", "server_ip",   "192.168.7.10"},    {QVariant::fromValue((void *)&g_stConfigGroup.network.strServerIP),   "QString", QRegExp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b")}},
    {{"network", "server_port", "8000"},            {QVariant::fromValue((void *)&g_stConfigGroup.network.nServerPort), "int",     QRegExp("^([0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$")}},
    {{"network", "udp_port",    "8001"},            {QVariant::fromValue((void *)&g_stConfigGroup.network.nUDPPort),    "int",     QRegExp("^([0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])$")}},

    //;注释:用户编码和监控编码地址
    {{"user", "user_id",        "500"},             {QVariant::fromValue((void *)&g_stConfigGroup.user.lUserID),        "quint64", QRegExp("^\\d+$")}},
    {{"user", "centre_id",      "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.user.lCenterID),      "quint64", QRegExp("^\\d+$")}},

    //;注释:协议版本和操作ID
    {{"verid", "protocal_ver",  "0"},               {QVariant::fromValue((void *)&g_stConfigGroup.verid.nProtocolVer),  "int",     QRegExp("^(255|[1-9]?[0-9]|[1,2][0-5][0-4])$")}},
    {{"verid", "operator_id",   "1"},               {QVariant::fromValue((void *)&g_stConfigGroup.verid.nOperatorID),   "int",     QRegExp("^(255|[1-9]?[0-9]|[1,2][0-5][0-4])$")}},

    //;注释:不需要保存
    {{"rtc", "time",  "2016-09-06"},                {QVariant::fromValue((void *)&g_stConfigGroup.rtc.strTime),            "QString", QRegExp("^((?:19|20)\\d\\d)-(0[1-9]|1[012])-(0[1-9]|[12][0-9]|3[01])$")}},

    //;注释:手动火警报警延时=20秒, LCD背光灯=60秒，亮度80，字体14
    {{"delay", "lcd_back",      "60"},              {QVariant::fromValue((void *)&g_stConfigGroup.delay.nLcdBack),      "int",     QRegExp("^([1-9]\\d|\\d)$")}},
    {{"delay", "manual_fire",   "20"},              {QVariant::fromValue((void *)&g_stConfigGroup.delay.nManualFire),   "int",     QRegExp("^([1-9]\\d|\\d)$")}},
    {{"delay", "lcd_bright",    "80"},              {QVariant::fromValue((void *)&g_stConfigGroup.delay.nLcdBright),    "int",     QRegExp("^([1-9]\\d|\\d)$")}},
    {{"delay", "font_size",     "14"},              {QVariant::fromValue((void *)&g_stConfigGroup.delay.nFontSize),     "int",     QRegExp("^([1-9]\\d|\\d)$")}},

    //;注释:admin密码222222，user密码000000
    {{"code", "admin",    "MjIyMjIy"},              {QVariant::fromValue((void *)&g_stConfigGroup.code.strAdminPwd),          "QString", QRegExp("^([a-zA-Z0-9+/]{214}={2})$|^([a-zA-Z0-9+/]{215}={1})$|^([a-zA-Z0-9+/]{8})$")}},
    {{"code", "user",     "MDAwMDAw"},              {QVariant::fromValue((void *)&g_stConfigGroup.code.strUserPwd),           "QString", QRegExp("^([a-zA-Z0-9+/]{214}={2})$|^([a-zA-Z0-9+/]{215}={1})$|^([a-zA-Z0-9+/]{8})$")}},

    //;注释:5个电话\n"
    {{"phone", "fire_num1",      ""},               {QVariant::fromValue((void *)&g_stConfigGroup.phone.strFireNum[0]),    "QString",QRegExp("^[\\d\\D]*$")}},
    {{"phone", "fire_num2",      ""},               {QVariant::fromValue((void *)&g_stConfigGroup.phone.strFireNum[1]),    "QString",QRegExp("^[\\d\\D]*$")}},
    {{"phone", "fire_num3",      ""},               {QVariant::fromValue((void *)&g_stConfigGroup.phone.strFireNum[2]),    "QString",QRegExp("^[\\d\\D]*$")}},
    {{"phone", "monit_num",      ""},               {QVariant::fromValue((void *)&g_stConfigGroup.phone.strMonitorNum),      "QString",QRegExp("^[\\d\\D]*$")}},
    {{"phone", "duty_num",       ""},               {QVariant::fromValue((void *)&g_stConfigGroup.phone.strDutyNum),       "QString",QRegExp("^[\\d\\D]*$")}},

    //;注释:debug=true/false日志
    {{"debug", "debug_log",      "false"},          {QVariant::fromValue((void *)&g_stConfigGroup.debug.strDebugLog),      "QString",QRegExp("^(true|false)$")}},

    //;注释:自动放电开关type=true/false、时间=2016-09-06、间隔=15天
    {{"discharge", "type",       "true"},           {QVariant::fromValue((void *)&g_stConfigGroup.discharge.strType),       "QString",QRegExp("^(true|false)$")}},
    {{"discharge", "time",       "2016-09-06"},     {QVariant::fromValue((void *)&g_stConfigGroup.discharge.strTime),       "QString",QRegExp("^((?:19|20)\\d\\d)-(0[1-9]|1[012])-(0[1-9]|[12][0-9]|3[01])$")}},
    {{"discharge", "interval",   "15"},             {QVariant::fromValue((void *)&g_stConfigGroup.discharge.nInterval),   "int",    QRegExp("^(255|[1-9]?[0-9]|[1,2][0-5][0-4])$")}},

    //最后
    {{"", "",   ""},             {QVariant(), "",   QRegExp()}}
};

config::config(QObject *parent) :
    QObject(parent)
{
}

