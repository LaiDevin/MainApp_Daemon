#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QProcess>

struct SerialConfig{//串口配置
    int nRS232;
    int nRS485;
};
struct NetworkConfig{//网络配置
    QString strType;
    int nIPDNS;
    int nDNSIndex;
    QString strServerIP;
    int nServerPort;
    int nUDPPort;
};
struct DelayConfig{//延时配置
    int nLcdBack;
    int nManualFire;
    int nLcdBright;
    int nFontSize;
};

struct RtcConfig{//RTC配置
    QString strTime;
};

struct UserConfig{//用户编码配置
    quint64 lUserID;
    quint64 lCenterID;
};

struct BanbenIDConfig{//协议 ID
    int nProtocolVer;
    int nOperatorID;
};

struct DisChargeConfig{//放电配置
    QString strType;
    QString strTime;
    int nInterval;
};
struct IPConfig {//本机IP配置
    int nInterfaceType;
    int nWirelessID;
    QString strType;
    QString strIP;
    QString strMask;
    QString strGate;
};
struct CodeConfig{//本机密码
    QString strAdminPwd;
    QString strUserPwd;
};
struct PhoneConfig{//电话号码
    QString strFireNum[3];    //火警电话
    QString strMonitorNum;      //监控中心电话
    QString strDutyNum;       //执勤电话
};

struct DebugConfig{//调试
    QString strDebugLog;
};

struct DNSConfig {
    QString strName;
    QString strDns;
};

struct ConfigGroup
{
    struct SerialConfig serial;
    struct NetworkConfig network;
    struct DelayConfig delay;
    struct RtcConfig rtc;
    struct UserConfig user;
    struct BanbenIDConfig verid;
    struct DisChargeConfig discharge;
    struct IPConfig myip;
    struct CodeConfig code;
    struct PhoneConfig phone;
    struct DebugConfig debug;
    QList<DNSConfig> dnsList;
};

extern struct ConfigGroup g_stConfigGroup;


struct OtherGroup
{
    QString stStartTime;
    bool bNetStatus;
    bool bDetectEnable;
};

extern struct OtherGroup g_stOtherGroup;

class config : public QObject
{
    Q_OBJECT
public:
    explicit config(QObject *parent = 0);
    static void setConfigfileName(const QString& strFileName);
    static QList <QString> read_file_someLine(QString FileAddr, int LineNum, int size);    //读多行
    static bool write_file_someLine(QString FileAddr, int LineNum, QList <QString> newList);//写多行

    static QString run(QString cmd, int time);
    static void check_file_exists();

    QStringList get_info();
    static QByteArray get_ver();

    static QList <QString> read_config(QString rTpye);
    static bool write_config(QString rTpye, QList <QString> content);
    static bool set_eth0_ip(QString type, QStringList arg);
    static bool get_dns_by_index(int nIndex, QString &strDNS);
    static bool get_wireless_decription_by_id(int nID, QString &strDescription);
private:
    static void mkfile_config();
    static bool config_item_isEmpty(QString item);
    static void ReadAndCheckAllConfig();
    static void readDnsConfig(void);
    static void write(QString strFileName, QString addr, QString val);
    static QString read(QString strFileName, QString addr);

public slots:
//    void on_read();
private:
    static QString m_strConfigFileName;
};

#endif // CONFIG_H
