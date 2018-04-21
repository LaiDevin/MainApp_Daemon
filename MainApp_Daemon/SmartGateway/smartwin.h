#ifndef SMARTWIN_H
#define SMARTWIN_H

/***
 * author: Devin
 * date:  2018/03/28
 * brief: @
***/


#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QPushButton>
#include <QListView>
#include <QCheckBox>
#include <QTimer>
#include <QStringListModel>
#include <QDebug>
#include "Com.h"
#include "ComDefine.h"
#include "FasManager.h"
#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>
#include <deque>
#include <QMutex>

#include <QSystemTrayIcon>
#include "log.h"
#include "IniFile.h"

#include "ServerAgreementManager.h"


#define RECV_DATA_TIME_OUT 500
#define CONFIG_INI_FILE "config.ini"
#define HEARTBEAT_COUNT 10

#define CENTER_ID 0x3836

class SmartWin : public QMainWindow
{
    Q_OBJECT

public:
    explicit SmartWin(QWidget *parent = 0);
    ~SmartWin();

    bool openSerial(std::vector<FasComInfo> &fasList);
    void closeSerial();

    bool openServer();
    void closeServer();

    void setServerConnected(bool connected) {m_connect = connected;}
    bool serverConnected() const { return m_connect; }

    void setSerialOpened(bool isOpen) {m_isOpenSerial = isOpen;}
    bool serialOpened() const {return m_isOpenSerial;}

    void setFasId(int id) { m_fasId = id;}
    const int fasId() const {return m_fasId;}

    void setComType(int type) { m_comType = type;}
    const int comType() const {return m_comType;}

    void setFlowCtl(int flow) { m_flowCtl = flow;}
    const int flowCtl() const {return m_flowCtl;}

    std::string fasName() const {
        return m_FasListCbx != nullptr ?
                    m_FasListCbx->currentText().toStdString() : "";
    }

    void readGateWayIni(std::vector<IniFile::T_LineConf> &vecSection);
    void readGateWayIni();

    bool autoConnect() const {return m_autoConnect;}

signals:
    void serialInfoMsg(const QString& msg);
    void gatewayInfoMsg(const QString& msg);

public slots:

    void gatewayServerConfigTrigger();
    void powerTrigger();
    void gatewayLinker();

    void activatedSysTrayIcon(QSystemTrayIcon::ActivationReason activationReason);
    void saveTxtBtn();

private:
    void delResources();
    QMutex  *m_saveMutex;

protected slots:

    void closeEvent(QCloseEvent *event);

private:
    void initWin();
    void createActions();
    void createSerial();
    void initConnectSlots();

    void initData();
    void loadFasList();
    void initSerialData();

    void receiveSerialData(bool isOpenCon);

    void setSerialFasComInfo(std::vector<FasComInfo>& fasInfoList);

    void initSystemTray();
    int  openTipsMsgBox(const QString& title, const QString& content);

    void runSerialThread();
    void runServerThread();
    void runSendGateWayDataTread();
    void sendHeartThread();

    void saveSerialCfg();
    void readSerialCfg();
    void saveSerialTxt();

    QByteArray read_config(QString gbk);

    void appendMsgBuffer(const QString& msg) {
        if (m_msgBufferList.count() > 300) {
            m_msgBufferList.clear();
            msgLisModel.removeRows(0, msgLisModel.rowCount());
        }
        m_msgBufferList.append(msg);
        msgLisModel.setStringList(m_msgBufferList);
        QModelIndex nIndex = msgLisModel.index(msgLisModel.rowCount() - 1, 0);
        m_msgBrowser->setCurrentIndex(nIndex);
    }

    void CtrlHandle(const char *buffer);
    void ConfirmHandle(const char *buffer);
    void RequestHandle(const char * buffer);

    void SendRecoverySettingResult(const char *buffer);
    void syncTime(const char *buffer);
    void inspectSentries(const char *buffer);
    void SendUserRunStatus(const char *buffer);
    void SendUserOprationInfo(const char * buffer);
    void SendUserSoftwareVersion(const char * buffer);
    void SendUserConfig(const char * buffer);
    void SendUserTime(const char * buffer);

private slots:
    void clickOpenSerialBtnSlot();

private:
    //[0] the Actions of MenuBar on the Window
    QAction *m_gateCfgAction;
    QAction *m_powerCfgAction;
    QAction *m_linkCfgAction;
    QAction *m_quitAction;

    QAction *m_useHelpAction;
    QAction *m_aboutAction;
    //[0] the Actions of MenuBar on the Window

    //[1]
    QComboBox *m_FasListCbx; //fas facility list

    QComboBox *m_serialCbx;
    QComboBox *m_baudRateCbx;
    QComboBox *m_dataBitCbx;
    QComboBox *m_parityCbx;
    QComboBox *m_stopCbx;

    QPushButton *m_serialBtn;

    //[1]

    //[2]
    QListView *m_msgBrowser;
    //[2]

    QCheckBox *m_displaySerialMsgBtn;
    QCheckBox *m_displayGateMsgBtn;

    QPushButton *m_saveMsgBtn;
    QPushButton *m_clearMsgBtn;

    //[] other configuration
    QPushButton *m_gateCfgBtn;
    QPushButton *m_powerCfgBtn;
    QPushButton *m_linkageCfgBtn;
    //[]

private:
    bool m_isOpenSerial;
    std::vector<FasInfo> stFasInfoList;
    QSystemTrayIcon *m_systemTrayIcon;
    //Translation  *trans;
    static int  s_nHeartbeatBusinessNum ;
    static bool s_bHeartbeatReply;

private:
    bool m_connect;
    bool m_autoConnect;

    bool m_isDisplaySerialMsg = false;
    bool m_isDisplayServerMsg = false;

    bool m_autoStartSerial = false;

    std::string strServerIP;
    int nPort;
    int nLocalPort;

    quint64 m_gatewayId;

    int m_fasId;
    int m_comType;
    int m_flowCtl;
    int m_Network_Protocol;

    QStringList m_msgBufferList;

    QStringListModel msgLisModel;

    mutable std::mutex m_mtx;
    std::deque<PartRunStatus> m_dequeData;
    std::condition_variable m_deque_cv;

};
#endif // SMARTWIN_H
