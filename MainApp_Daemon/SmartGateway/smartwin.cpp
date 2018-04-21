#include "smartwin.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QThread>
#include <QGroupBox>
#include <QFileDialog>
#include <QDebug>
#include <QLabel>
#include <QTextCodec>
#include <QTextStream>
#include <QFile>
#include "powerconfigdlg.h"
#include "gatewaylinkerdlg.h"
#include "GatewayServerDlg.h"
#include "ServerAgreementGBTool.h"
#include "Tool.h"

#ifdef DAEMON
#include "daemon.h"
#endif

static bool _runingSerialThread = true;
static bool _runingServerThread = true;
static bool _runingHeartThread = true;
static int s_nHeartbeatCount = 0;

int  SmartWin::s_nHeartbeatBusinessNum= 1;
bool SmartWin::s_bHeartbeatReply = false;
//static UserRunStat s_objUserRunStat;

static bool getDescription(const unsigned char *strSrc, char *strDescription)
{
    if(NULL == strSrc || NULL == strDescription){
        return false;
    }

    int nFasID = (strSrc[0] << 8 | strSrc[1]);
    if(nFasID == FAS_ID_JBQB_FS5101) {
        return false;
    } else {
        int nHost = (strSrc[2] << 8 | strSrc[3]);
        int nLoop = (strSrc[4] << 8 | strSrc[5]);
        int nPoint = (strSrc[6] << 8 | strSrc[7]);
        switch(nFasID) {
        case FAS_ID_JBQT_GST5000:
        case FAS_ID_JBQG_GST5000:
        case FAS_ID_JBQB_GST500:
            sprintf(strDescription, "%02d%02d%02d", nHost, nLoop, nPoint);
            break;
        case FAS_ID_JBTT_JBF11S:
            sprintf(strDescription, "H%03dL%03dD%03d", nHost, nLoop, nPoint);
            break;
        case FAS_ID_JBQB_JBF5012:
            break;
        case FAS_ID_JBQB_LN1010:
            break;
        case FAS_ID_Q100GZ2L_LA040:
            sprintf(strDescription, "%03d-%03d", nLoop, nPoint);
            break;
        case FAS_ID_TB_TC3000:
            break;
        case FAS_ID_JBQB_OZH4800:
            sprintf(strDescription, "H%03dL%03dD%03d", nHost, nLoop, nPoint);
            break;
        case FAS_ID_JBLB_QH8000B:
            break;
        case FAS_ID_JB_3208B:
        case FAS_ID_JB_3208G:
            sprintf(strDescription, "H%03dL%03dD%03d", nHost, nLoop, nPoint);
            break;
        case FAS_ID_JBQB_TC3020:
            break;
        case FAS_ID_JBTB_242:
            break;
        case FAS_ID_JBLB_CA2000SZ:
            break;
        case FAS_ID_JB_QGL_9000:
            break;
        case FAS_ID_JB_QBL_MN300:
            break;
        case FAS_ID_JB_TB_JBF_11S:
            break;
        case FAS_ID_JB_QT_OZH4800:
            break;
        case FAS_ID_JB_LG_QH8000:
            break;
        case FAS_ID_JB_TG_JBF_11SF:
            break;
        default:
            return false;
        }
    }
    return true;
}

SmartWin::SmartWin(QWidget *parent)
    :QMainWindow(parent), m_isOpenSerial(false),
      m_connect(false), nPort(8000), nLocalPort(8001), strServerIP(""),
      m_comType(COM_TYPE_RS232), m_flowCtl(FLOW_CONTROL_NONE), m_gatewayId(0),m_Network_Protocol(0)
{
    initWin();
    runSerialThread();
    //runSendGateWayDataTread();
    runServerThread();
    sendHeartThread();
}

SmartWin::~SmartWin()
{
    WSACleanup();
    closeSerial();
    closeServer();
    WSACleanup();
    delResources();
    m_dequeData.clear();
}

void SmartWin::initWin()
{
    resize(800, 500);
    this->setWindowTitle(tr("SmartGateway"));
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    createActions();
    createSerial();
    initConnectSlots();
    initData();

    initSystemTray();
    readGateWayIni();
    readSerialCfg();

    m_displaySerialMsgBtn->setChecked(m_isDisplaySerialMsg);
    m_displayGateMsgBtn->setChecked(m_isDisplayServerMsg);

    WSAData wsa;
    ::WSAStartup(MAKEWORD(2,2),&wsa);

    if (m_autoConnect) {
        openServer();
    }

    if (m_autoStartSerial) {
        clickOpenSerialBtnSlot();
    }
    m_saveMutex = new QMutex();
}

void SmartWin::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    m_gateCfgAction = new QAction(tr("GateWay Configuration"), this);
    m_powerCfgAction = new QAction(tr("Power Configuration"), this);
    m_linkCfgAction = new QAction(tr("Linkage Configuration"), this);

    m_quitAction = new QAction(tr("Quit"), this);

    m_useHelpAction = new QAction(tr("Usage Guide"), this);
    m_aboutAction = new QAction(tr("About"), this);

    fileMenu->addAction(m_gateCfgAction);
    fileMenu->addAction(m_powerCfgAction);
    fileMenu->addAction(m_linkCfgAction);

    fileMenu->addSeparator();

    fileMenu->addAction(m_quitAction);

    helpMenu->addAction(m_useHelpAction);
    helpMenu->addAction(m_aboutAction);
}

void SmartWin::createSerial()
{

    QHBoxLayout *grid = new QHBoxLayout();

    QGroupBox *leftw = new QGroupBox(tr("Smart monitor"));

    leftw->setMinimumWidth(250);
    leftw->setMaximumWidth(400);

    grid->addWidget(leftw);

    m_msgBrowser = new QListView();
    m_msgBrowser->setMinimumWidth(300);
    grid->addWidget(m_msgBrowser);

    QWidget *w = new QWidget();
    w->setMinimumHeight(350);
    w->setLayout(grid);

    setCentralWidget(w);


    int offset_y = 20, offset_x = 2;
    int init_lab_width = 80, lab_height = 20, com_width = 150;

    QLabel *lab_1 = new QLabel(tr("Facility:"), leftw);
    lab_1->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_FasListCbx = new QComboBox(leftw);

    lab_1->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_FasListCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);


    QLabel *lab_2 = new QLabel(tr("Serial:"), leftw);
    lab_2->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_serialCbx = new QComboBox(leftw);

    offset_y += 40;

    lab_2->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_serialCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);

    //    QLabel *lab_3 = new QLabel(Translation::getLabelname("Baud Rate")+":", leftw);
    QLabel *lab_3 = new QLabel(tr("Baud Rate:"), leftw);

    lab_3->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_baudRateCbx = new QComboBox(leftw);

    offset_y += 28;

    lab_3->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_baudRateCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);

    QLabel *lab_4 = new QLabel(tr("Data Bit:"), leftw);
    lab_4->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_dataBitCbx = new QComboBox(leftw);

    offset_y += 28;

    lab_4->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_dataBitCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);

    QLabel *lab_5 = new QLabel(tr("Parity:"), leftw);
    lab_5->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_parityCbx = new QComboBox(leftw);

    offset_y += 28;

    lab_5->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_parityCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);

    QLabel *lab_6 = new QLabel(tr("Stop Bit:"), leftw);
    lab_6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_stopCbx = new QComboBox(leftw);

    offset_y += 28;

    lab_6->setGeometry(QRect(offset_x, offset_y, init_lab_width, lab_height));
    m_stopCbx->setGeometry(init_lab_width + 5, offset_y, com_width, lab_height);

    offset_y += 28;
    m_serialBtn = new QPushButton(tr("Open Serial"), leftw);
    m_serialBtn->setGeometry(com_width + m_stopCbx->x() - 80, offset_y, 80, 25);

    offset_y += 40;

    //[0] Receive data
    QGroupBox *receive_box = new QGroupBox(tr("Receive Data"), leftw);
    receive_box->setGeometry(offset_x + 10, offset_y, m_serialBtn->x() + m_serialBtn->width() - 12, 80);

    m_displaySerialMsgBtn = new QCheckBox(tr("Serial Msg"), receive_box);
    m_displayGateMsgBtn = new QCheckBox(tr("Gateway Msg"), receive_box);

    m_saveMsgBtn = new QPushButton(tr("Save"), receive_box);
    m_clearMsgBtn = new QPushButton(tr("Clear Msg"), receive_box);

    m_displaySerialMsgBtn->setGeometry(6, 20, 100, 20);
    m_displayGateMsgBtn->setGeometry(116, 20, 100, 20);

    m_clearMsgBtn->setGeometry(6, 46, 80, 20);
    m_saveMsgBtn->setGeometry(116, 46, 80, 20);

    //[0]

    //[1]other configuration
    QGroupBox *otherCfg_box = new QGroupBox(tr("Configuration"), leftw);
    otherCfg_box->setGeometry(offset_x + 10, receive_box->height() + offset_y, receive_box->width(), 56);

    m_gateCfgBtn = new QPushButton(tr("Gateway"));
    m_powerCfgBtn = new QPushButton(tr("Power"));
    m_linkageCfgBtn = new QPushButton(tr("linkage"));

    QHBoxLayout *otherLay = new QHBoxLayout;

    otherLay->addWidget(m_gateCfgBtn);
    otherLay->addWidget(m_powerCfgBtn);
    otherLay->addWidget(m_linkageCfgBtn);

    otherCfg_box->setLayout(otherLay);
    //[1]

}

void SmartWin::initConnectSlots()
{
    connect(m_quitAction, &QAction::triggered, [&]() {
#ifdef DAEMON
        killProcess(_WINDOWS_FAULT_EXE, _DAEMON_TARGET);
#endif

        _runingServerThread = false;
        _runingHeartThread = false;
        _runingSerialThread = false;
        qApp->quit();
    });

    connect(m_useHelpAction, &QAction::triggered, [&](){});
    connect(m_aboutAction, &QAction::triggered, [&](){});

    connect(m_serialBtn, &QPushButton::clicked, this, &SmartWin::clickOpenSerialBtnSlot);

    connect(m_powerCfgAction,&QAction::triggered, this, &SmartWin::powerTrigger);
    connect(m_linkCfgAction,&QAction::triggered, this, &SmartWin::gatewayLinker);

    connect(m_gateCfgAction,&QAction::triggered,this,&SmartWin::gatewayServerConfigTrigger);

    connect(m_gateCfgBtn, &QPushButton::clicked, this, &SmartWin::gatewayServerConfigTrigger);
    connect(m_powerCfgBtn, &QPushButton::clicked, this, &SmartWin::powerTrigger);
    connect(m_linkageCfgBtn, &QPushButton::clicked, this, &SmartWin::gatewayLinker);
    connect(m_saveMsgBtn, &QPushButton::clicked, this, &SmartWin::saveTxtBtn);


    connect(m_clearMsgBtn, &QPushButton::clicked, [&](){
        m_msgBufferList.clear();
        msgLisModel.removeRows(0, msgLisModel.rowCount());
    });

    connect(m_displaySerialMsgBtn, &QCheckBox::toggled, [&](bool checked){
        m_isDisplaySerialMsg = checked;

        IniFile configFile;

        QString path = qApp->applicationDirPath() + "/";
        configFile.LoadFile(path.toStdString() + CONFIG_INI_FILE);

        configFile.WriteInteger("GatewayServerConfiguration", "showSerialMsg", checked ? 1:0);
    });

    connect(m_displayGateMsgBtn, &QCheckBox::toggled, [&](bool checked){
        m_isDisplayServerMsg = checked;

        IniFile configFile;

        QString path = qApp->applicationDirPath() + "/";
        configFile.LoadFile(path.toStdString() + CONFIG_INI_FILE);

        configFile.WriteInteger("GatewayServerConfiguration", "showServerMsg", checked ? 1:0);
    });
}

void SmartWin::initData()
{
    m_dequeData.clear();
    loadFasList();
    initSerialData();

    m_msgBrowser->setModel(&msgLisModel);
}

void SmartWin::loadFasList()
{
    stFasInfoList.clear();
    FasManager::getInstance()->getFasList(stFasInfoList);

    m_FasListCbx->clear();
    for (auto &item : stFasInfoList) {
        m_FasListCbx->addItem(QString::fromLocal8Bit(item.strName), QVariant(item.nID));
    }
    setFasId(0);
}

void SmartWin::initSerialData()
{
    Com com;
    std::vector<std::wstring> vctTemp = com.GetAllSerialPorts();
    m_serialCbx->clear();
    for (auto &item : vctTemp) {
        m_serialCbx->addItem( QString::fromStdWString(item));
    }

    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_1200), QVariant(COM_BITRATE_1200));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_2400), QVariant(COM_BITRATE_2400));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_4800), QVariant(COM_BITRATE_4800));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_9600), QVariant(COM_BITRATE_9600));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_19200), QVariant(COM_BITRATE_19200));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_38400), QVariant(COM_BITRATE_38400));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_57600), QVariant(COM_BITRATE_57600));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_115200), QVariant(COM_BITRATE_115200));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_230400), QVariant(COM_BITRATE_230400));

    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_460800), QVariant(COM_BITRATE_460800));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_500000), QVariant(COM_BITRATE_500000));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_576000), QVariant(COM_BITRATE_576000));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_921600), QVariant(COM_BITRATE_921600));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_1000000), QVariant(COM_BITRATE_1000000));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_1152000), QVariant(COM_BITRATE_1152000));

    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_1500000), QVariant(COM_BITRATE_1500000));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_2000000), QVariant(COM_BITRATE_2000000));
    m_baudRateCbx->addItem(tr("%1").arg(COM_BITRATE_2500000), QVariant(COM_BITRATE_2500000));

    int index_baud =  m_baudRateCbx->findData(QVariant(COM_BITRATE_9600));
    if (index_baud >= 0) m_baudRateCbx->setCurrentIndex(index_baud);

    m_dataBitCbx->addItem(tr("%1").arg(COM_DATABITS_5), QVariant(COM_DATABITS_5));
    m_dataBitCbx->addItem(tr("%1").arg(COM_DATABITS_6), QVariant(COM_DATABITS_6));
    m_dataBitCbx->addItem(tr("%1").arg(COM_DATABITS_7), QVariant(COM_DATABITS_7));
    m_dataBitCbx->addItem(tr("%1").arg(COM_DATABITS_8), QVariant(COM_DATABITS_8));

    int index_data =  m_dataBitCbx->findData(QVariant(COM_DATABITS_8));
    if (index_data >= 0) {
        m_dataBitCbx->setCurrentIndex(index_data);
    }

    m_parityCbx->addItem(tr("None"), QVariant(COM_PARITY_NONE));
    m_parityCbx->addItem(tr("Even"), QVariant(COM_PARITY_EVEN));
    m_parityCbx->addItem(tr("Odd"), QVariant(COM_PARITY_ODD));

    m_stopCbx->addItem(tr("%1").arg(COM_STOPBITS_1 + 1), QVariant(COM_STOPBITS_1));
    m_stopCbx->addItem(tr("%1").arg(COM_STOPBITS_1P5 + 0.5), QVariant(COM_STOPBITS_1P5));
    m_stopCbx->addItem(tr("%1").arg(COM_STOPBITS_2), QVariant(COM_STOPBITS_2));

}

void SmartWin::clickOpenSerialBtnSlot()
{
    receiveSerialData(m_isOpenSerial);

    m_serialBtn->setText(m_isOpenSerial ? tr("Close Serial") : tr("Open Serial"));

    m_FasListCbx->setEnabled(!m_isOpenSerial);
    m_serialCbx->setEnabled(!m_isOpenSerial);
    m_baudRateCbx->setEnabled(!m_isOpenSerial);
    m_dataBitCbx->setEnabled(!m_isOpenSerial);
    m_parityCbx->setEnabled(!m_isOpenSerial);
    m_stopCbx->setEnabled(!m_isOpenSerial);
}

void SmartWin::receiveSerialData(bool isOpenCon)
{
    closeSerial();

    if (!isOpenCon) {
        std::vector<FasComInfo> fasInfoList;
        setSerialFasComInfo(fasInfoList);

        if (openSerial(fasInfoList)) {
            LOG_INFO(tr("open serial success!").toLocal8Bit().data());
            setSerialOpened(true);
            saveSerialCfg();
            appendMsgBuffer(tr("开启Fas通信!"));

        } else {
            LOG_ERROR(tr("open serial failed or occupied!").toLocal8Bit().data());
            openTipsMsgBox(tr("information"), tr("serial open failed or occupied!"));
        }
    } else {
        setSerialOpened(false);
        appendMsgBuffer(tr("关闭Fas通信!"));
    }

}

void SmartWin::setSerialFasComInfo(std::vector<FasComInfo> &fasInfoList)
{
    struct FasComInfo info;

    strcpy(info.strComName, m_serialCbx->currentText().toLocal8Bit().data());

    info.nID =  m_FasListCbx->currentData().toInt();
    info.nComType = comType();

    info.nBitrate = m_baudRateCbx->currentData().toInt();
    info.nDatabits = m_dataBitCbx->currentData().toInt();
    info.nFlowCtrl = flowCtl();

    info.nParity = m_parityCbx->currentData().toInt();
    info.nStopbits = m_stopCbx->currentData().toInt();

    fasInfoList.push_back(info);
}
void SmartWin::powerTrigger()
{
    PowerConfigDlg powerConfigDlg;
    powerConfigDlg.exec();
}

void SmartWin::gatewayServerConfigTrigger(){
    GatewayServerDlg  gatewaySerDlg;

    std::vector<IniFile::T_LineConf> vecSection;
    readGateWayIni(vecSection);
    gatewaySerDlg.readGateWayIni(vecSection);

    connect(&gatewaySerDlg, &GatewayServerDlg::clickOkSig, [&](){
        this->readGateWayIni();
        closeServer();

        if (autoConnect())
            openServer();
    });
    gatewaySerDlg.exec();
}

void SmartWin::gatewayLinker()
{
    GatewayLinkerDlg gatewayLinkerDlg;
    gatewayLinkerDlg.exec();
}
void SmartWin::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->hide();
    m_systemTrayIcon->show();
}
void SmartWin::initSystemTray()
{
    m_systemTrayIcon = new QSystemTrayIcon(this);
    m_systemTrayIcon->setToolTip(QObject::trUtf8("TrayIcon"));
    QIcon icon = QIcon(":/images/trash.png");

    m_systemTrayIcon->setIcon(icon);
    connect(m_systemTrayIcon,&QSystemTrayIcon::activated,this, &SmartWin::activatedSysTrayIcon);
}
void SmartWin::activatedSysTrayIcon(QSystemTrayIcon::ActivationReason activationReason)
{
    switch(activationReason){
    case QSystemTrayIcon::Trigger:
        this->show();
        break;

    case QSystemTrayIcon::DoubleClick:
        this->show();
        break;
    default:
        break;
    }
}

bool SmartWin::openSerial(std::vector<FasComInfo> &fasList)
{
    return FasManager::getInstance()->init(fasList);
}

void SmartWin::closeSerial()
{
    FasManager::getInstance()->unInit();
    setSerialOpened(false);
}

bool SmartWin::openServer()
{
    //closeServer();
    ServerAgreementManager::getInstance()->setAgreementID(SERVER_AGREEMENT_ID_GB);
    setServerConnected(false);

    qDebug()<<"gatewayid: "<<m_gatewayId;
    ServerAgreementManager::getInstance()->setUserID(m_gatewayId);
    ServerAgreementManager::getInstance()->setCenterID(CENTER_ID);

    bool initFlag = ServerAgreementManager::getInstance()->init(m_Network_Protocol);

    if (initFlag) {

        if (strServerIP.empty()) {
            openTipsMsgBox(tr("information"), tr("open server failed, ip is empty!"));
        }

        m_connect =  ServerAgreementManager::getInstance()->connect(strServerIP.c_str(),
                                                                    nPort, nLocalPort);

        std::string info = "connect to " + strServerIP + ", port: " + std::to_string(nPort)
                + (m_connect ? " success!" : " failed");
        LOG_INFO(info.c_str());
        appendMsgBuffer(QString::fromStdString(PARSE_MSG(info.c_str())));
    }

    return m_connect;
}

void SmartWin::closeServer()
{
    ServerAgreementManager::getInstance()->disconnect();
    setServerConnected(false);
    appendMsgBuffer(tr("close Gateway server!"));
}

int SmartWin::openTipsMsgBox(const QString &title, const QString &content)
{
    QMessageBox box;
    box.setWindowTitle(title);
    box.setText(content);
    box.addButton(tr("Ok"), QMessageBox::YesRole);
    return box.exec();
}

void SmartWin::runSerialThread()
{
    std::thread t([&]() {

        while (_runingSerialThread) {

            std::chrono::duration<int,std::milli> mm(RECV_DATA_TIME_OUT);
            std::this_thread::sleep_for(mm);

            if (serialOpened()) {
                std::vector<PartRunStatus> vec;
                int length = -1;

                length = FasManager::getInstance()->recvData(fasId(), comType(), vec);
                if (length > 0) {

                    for (auto &v : vec) {
#if 0
                        qDebug()<<"nBit0: "<<v.PartStatus.StatusBit.nBit0;
                        qDebug()<<"nBit1: "<<v.PartStatus.StatusBit.nBit1;
                        qDebug()<<"nBit2: "<<v.PartStatus.StatusBit.nBit2;
                        qDebug()<<"nBit3: "<<v.PartStatus.StatusBit.nBit3;
                        qDebug()<<"nBit4: "<<v.PartStatus.StatusBit.nBit4;
                        qDebug()<<"nBit5: "<<v.PartStatus.StatusBit.nBit5;
                        qDebug()<<"nBit6: "<<v.PartStatus.StatusBit.nBit6;
#endif

                        PartStatusBits st = v.PartStatus.StatusBit;
                        std::string writeL = fasName();

                        char fasAddr[128] = {0};
                        getDescription(v.strPartDescription, fasAddr);

                        std::string tmp(fasAddr);
                        if (!tmp.empty()) {
                            writeL += ", 位置:" + tmp;
                        }

                        if (st.nBit1 == 1) {
                            writeL += ", 状态: 火警";
                        } else if (st.nBit2 == 1) {
                            writeL += ", 状态: 故障";
                        } else if (st.nBit3 == 1) {
                            writeL += ", 状态: 屏蔽";
                        } else if (st.nBit4 == 1) {
                            writeL += ", 状态: 监管";
                        } else if (st.nBit5 == 1) {
                            writeL += ", 状态: 启动";
                        } else if (st.nBit6 == 1) {
                            writeL += ", 状态: 反馈";
                        } else if (st.nBit7 == 1) {
                            writeL += ", 状态: 延时状态";
                        } else if (st.nBit8 == 1) {
                            writeL += ", 状态: 电源故障";
                        }

                        if (st.nBit0 != 1) {
                            LOG_INFO(writeL.c_str());

                            if (m_isDisplaySerialMsg) {
                                appendMsgBuffer(QString::fromStdString(PARSE_MSG(writeL.c_str())));
                            }
                        }

                        if (serverConnected()) {
                            /*
                            std::unique_lock<std::mutex> lck(m_mtx);
                            m_dequeData.push_back(v);
                            lck.unlock();
                            m_deque_cv.notify_all();
                            */
                            ServerAgreementManager::getInstance()->sendData(
                                        ServerAgreementGBTool::getBusinessNum(), v);
                        }

                    }

                }
            }

        }
    });

    t.detach();
}

void SmartWin::runSendGateWayDataTread()
{
    std::thread t([&]() {
        while (_runingServerThread) {
            std::chrono::duration<int,std::milli> mm(200);
            std::this_thread::sleep_for(mm);

            if (serverConnected()) {

                std::unique_lock<std::mutex> lck(m_mtx);
                while(m_dequeData.empty()) {
                    m_deque_cv.wait(lck);
                }

                PartRunStatus msgPart = m_dequeData.front();
                m_dequeData.pop_front();

                ServerAgreementManager::getInstance()->sendData(
                            ServerAgreementGBTool::getBusinessNum(), msgPart);

                lck.unlock();
            }
        }
    });

    t.detach();
}

void SmartWin::runServerThread()
{
    std::thread t([&]() {
        while(_runingServerThread) {
            std::chrono::duration<int,std::milli> mm(500);
            std::this_thread::sleep_for(mm);

            if (serverConnected()) {

                char strMsg[4096] = {'\0'};
                memset(strMsg, 0, 4096);
                int length = ServerAgreementManager::getInstance()->recvData(strMsg);
                qDebug()<<"get data from server length: " << length;
                std::string str= strMsg;
                if(length >0){
                    qDebug()<<QString::fromStdString(strMsg);
                    switch(strMsg[26]){
                    case 1://control command
                        CtrlHandle(strMsg);
                        break;
                    case 2://send data
                        qDebug() << "send data\n";
                        break;
                    case 3://confirm data
                        ConfirmHandle(strMsg);
                        break;
                    case 4:// require data
                        RequestHandle(strMsg);
                        break;
                    case 5://response
                        qDebug() << "Response\n";
                        break;
                    case 6: // deny  ... ignore
                        qDebug() << "deny\n";
                        break;
                    default :break;
                    }
                }
            } else {
                // disconnect
                qDebug()<<"server disconnect";
            }
        }
    });

    t.detach();
}

void SmartWin::CtrlHandle(const char *buffer){
    qDebug()<<"CtrlHandle";
    if(buffer == NULL)
        return;
    switch(buffer[27]){
    case DATA_TYPE_READ_89:
        SendRecoverySettingResult(buffer);
        break;
    case DATA_TYPE_READ_90:
        syncTime(buffer);
        break;
    case DATA_TYPE_READ_91:
        inspectSentries(buffer);
        break;
    default:
        break;
    }
}

void SmartWin::SendRecoverySettingResult(const char *buffer){
    if(NULL == buffer){
        return;
    }
    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
}

void SmartWin::sendHeartThread()
{
    static bool bWaitHeartBeatRepley = false;
    std::thread t([&]() {
        while(_runingHeartThread) {
            std::chrono::duration<int,std::milli> mm(1000);
            std::this_thread::sleep_for(mm);

            if((++s_nHeartbeatCount) == HEARTBEAT_COUNT){
                if (serverConnected()) {
                    char strMsg[4096] = {'\0'};
                    memset(strMsg, 0, 4096);
                    int length = ServerAgreementManager::getInstance()
                            ->sendHeartBeat(ServerAgreementGBTool::getHeartBeatBusinessNum());
                    qDebug()<<"send heartNum length: " << length;
                    s_nHeartbeatCount = 0;
                    s_bHeartbeatReply = false;
                }
            }


        }
    });

    t.detach();
}

void SmartWin::syncTime(const char *buffer){
    if(NULL == buffer){
        return;
    }

    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    Time stTime;
    stTime.nSec = buffer[29];
    stTime.nMin = buffer[30];
    stTime.nHour = buffer[31];
    stTime.nDay = buffer[32];
    stTime.nMonth = buffer[33];
    stTime.nYear = buffer[34];
    ServerAgreementManager::getInstance()->syncTime(nBusinessNum,stTime);
}

void SmartWin::inspectSentries(const char *buffer){  // suspend deal with
    if(NULL == buffer){
        return;
    }
}

void SmartWin::ConfirmHandle(const char *buffer){
    qDebug()<<"ConfirmHandle";
    if(NULL == buffer){
        return;
    }
    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    if(s_nHeartbeatBusinessNum == nBusinessNum) {
        qDebug()<<"ConfirmHandle ==  ";
        s_bHeartbeatReply = true;
    } else {
        qDebug()<<"ConfirmHandle !=  ";
        ServerAgreementManager::getInstance()->confirmMsg(nBusinessNum);
    }
}

void SmartWin::RequestHandle(const char * buffer){
    qDebug()<<"RequestHandle";
    if(NULL == buffer){
        return;
    }
    switch (buffer[27]){
    case DATA_TYPE_READ_61:
        break;
    case DATA_TYPE_READ_62:
        break;
    case DATA_TYPE_READ_63:
        break;
    case DATA_TYPE_READ_64:
        break;
    case DATA_TYPE_READ_65:
        break;
    case DATA_TYPE_READ_66:
        break;
    case DATA_TYPE_READ_67:
        break;
    case DATA_TYPE_READ_68:
        break;
    case DATA_TYPE_READ_81:
        SendUserRunStatus(buffer);
        break;
    case DATA_TYPE_READ_84:
        SendUserOprationInfo(buffer);
        break;
    case DATA_TYPE_READ_85:
        SendUserSoftwareVersion(buffer);
        break;
    case DATA_TYPE_READ_86:
        SendUserConfig(buffer);
        break;
    case DATA_TYPE_READ_88:
        SendUserTime(buffer);
        break;
    default: break;
    }
}

void SmartWin::SendUserRunStatus(const char *buffer){
    if(NULL == buffer){
        return;
    }
    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    UserRunStatus stUserRunStatus;
    //    stUserRunStatus.RunStatus.StatusBit =  s_objUserRunSta
    //            ServerAgreementManager::getInstance()->sendData(nBusinessNum,);
}

void SmartWin::SendUserOprationInfo(const char *buffer){   //suspend
    if(NULL == buffer){
        return;
    }

}

void SmartWin::SendUserSoftwareVersion(const char *buffer){
    if(NULL == buffer){
        return;
    }
    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    // version  ignore
}

void SmartWin::SendUserConfig(const char *buffer){    // character encoding information , ignore
    if(NULL == buffer){
        return;
    }

    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    //    QByteArray config;
    //    config = read_config("GB18030");
    //    USER_CONFIG stUSER_CONFIG;
    //    stUSER_CONFIG.ConfigMsg = config.data();
    //    stUSER_CONFIG.ConfigSize = strlen(config.data());
    //    s_pobjServerAgreementManager->sendData(nBusinessNum, stUSER_CONFIG);
}

QByteArray SmartWin::read_config(QString gbk){
    //    QString dir = "./" + QString(s_strConfigFileName);
    //    QFile file(dir);
    //    QTextStream in(&file);
    //    QString buff;
    //    QByteArray buff_ba;
    //    QByteArray out_ba;
    //    QByteArray GBK_ba;

    //    int allSize;
    //    if(gbk == "GB18030"){       //变长,一个汉字:1-3byte
    //        allSize = 240*3/3;
    //    }else if(gbk == "GB2312"){  //一个汉字:2byte
    //        allSize = 240*3/2;
    //    }else{
    //        return out_ba;
    //    }


    //    if(!file.open(QIODevice::ReadOnly)){
    //        qDebug()<< "打开失败:"<< dir;
    //        return out_ba;
    //    }

    //    //out_ba = in.device()->readAll();
    //    in.seek(0);
    //    while (!in.atEnd()){        //到结尾
    //        buff = in.readLine() + "\n";
    //        buff_ba = buff.toAscii();
    //        if(out_ba.size() + buff_ba.size() < allSize){ //转utf-8 实际体积会小
    //            out_ba += buff_ba;
    //        }else{
    //            out_ba += "...";
    //            qDebug()<<"未读完";
    //            break;
    //        }
    //    }
    //    file.close();

    //    if(gbk == "GB18030"){       //变长,一个汉字:1-3byte
    //        GBK_ba = myHelper::ChinesetoGB18030(out_ba.data());
    //    }else if(gbk == "GB2312"){  //一个汉字:2byte
    //        GBK_ba = myHelper::ChinesetoGB2312(out_ba.data(), -1);
    //    }else{
    //        GBK_ba = 0;
    //    }
    //    qDebug()<<QString().sprintf("读取大小:%d, 转化大小:%d", out_ba.size(), GBK_ba.size());
    //    return GBK_ba;
}

void SmartWin::SendUserTime(const char *buffer){
    if(NULL == buffer){
        return;
    }

    int nBusinessNum = (buffer[2] & 0xFF) | ((buffer[3] << 8) & 0xFF);
    Time stTime;
    Tool::getTime(stTime);
    ServerAgreementManager::getInstance()->sendData(nBusinessNum, stTime);
}

void SmartWin::saveSerialCfg()
{
    IniFile configFile;

    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + CONFIG_INI_FILE);

    setFasId(m_FasListCbx->currentData().toInt());

    configFile.WriteInteger("SerialCfg", "FasID", m_FasListCbx->currentData().toInt());
    configFile.WriteString("SerialCfg", "FasName", m_FasListCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "Serial", m_serialCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "BaudRate", m_baudRateCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "DataBit", m_dataBitCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "Parity", m_parityCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "StopBit", m_stopCbx->currentText().toStdString());
    configFile.WriteString("SerialCfg", "openSerial", (m_isOpenSerial ? "1":"0"));
}

void SmartWin::readSerialCfg()
{
    IniFile configFile;

    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + CONFIG_INI_FILE);

    std::vector<IniFile::T_LineConf> vecSection;
    configFile.ReadSection("SerialCfg", vecSection);

    for (auto& v : vecSection) {
        if (v.Key == "FasID") {
            int d = m_FasListCbx->findData(QVariant(std::stoi(v.Value)));
            m_FasListCbx->setCurrentIndex(d > -1 ? d : 0);
            setFasId(m_FasListCbx->currentData().toInt());
        } else if (v.Key == "Serial") {
            int d = m_serialCbx->findText(QString::fromStdString(v.Value));
            m_serialCbx->setCurrentIndex(d > -1 ? d : 0);
        } else if (v.Key == "BaudRate") {
            int d = m_baudRateCbx->findText(QString::fromStdString(v.Value));
            m_baudRateCbx->setCurrentIndex(d > -1 ? d : 0);
        } else if (v.Key == "DataBit") {
            int d = m_dataBitCbx->findText(QString::fromStdString(v.Value));
            m_dataBitCbx->setCurrentIndex(d > -1 ? d : 0);
        } else if (v.Key == "Parity") {
            int d = m_parityCbx->findText(QString::fromStdString(v.Value));
            m_parityCbx->setCurrentIndex(d > -1 ? d : 0);
        } else if (v.Key == "StopBit") {
            int d = m_stopCbx->findText(QString::fromStdString(v.Value));
            m_stopCbx->setCurrentIndex(d > -1 ? d : 0);
        } else if (v.Key == "openSerial") {
            m_autoStartSerial = std::stoi(v.Value) == 1 ? true : false;
        }
    }
}

void SmartWin::readGateWayIni(std::vector<IniFile::T_LineConf>& vecSection)
{
    IniFile configFile;

    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + CONFIG_INI_FILE);

    configFile.ReadSection("GatewayServerConfiguration", vecSection);
}

void SmartWin::readGateWayIni()
{
    std::vector<IniFile::T_LineConf> vecSection;
    readGateWayIni(vecSection);

    bool _isUserIp = false;
    std::string _domain = "";

    for (auto& v : vecSection) {
        if (v.Key == "Server_IP") {
            strServerIP = v.Value;
        } else if (v.Key == "ServerPort") {
            nPort = std::stoi(v.Value);
        } else if (v.Key == "localPort") {
            nLocalPort = std::stoi(v.Value);
        } else if (v.Key == "Server_addr") {
            _domain = v.Value;
        } else if (v.Key == "b_userIP_chk") {
            _isUserIp = std::stoi(v.Value) == 1 ? true : false;
        } else if(v.Key == "CommunicationType"){
            m_Network_Protocol = std::stoi(v.Value);
            qDebug()<<"m_Network_Protocol"<<m_Network_Protocol;
        }  else if (v.Key == "gatewayID") {
            m_gatewayId = std::stoull(v.Value);
        } else if (v.Key == "handleShake") {

        } else if (v.Key == "com3G") {

        } else if (v.Key == "b_use3G_chk") {

        } else if (v.Key == "b_serverConnect_chk") {
            m_autoConnect = std::stoi(v.Value) == 1 ? true : false;
        } else if (v.Key == "showSerialMsg") {
            m_isDisplaySerialMsg = std::stoi(v.Value) == 1 ? true : false;
        } else if (v.Key == "showServerMsg") {
            m_isDisplayServerMsg = std::stoi(v.Value) == 1 ? true : false;
        }
    }

    if (!_isUserIp) {
        char ip[128] = {0};
        strServerIP = std::string(Tool::getIpByDomain(_domain.c_str(), ip));
    }

}
void SmartWin::saveTxtBtn()
{
    if(m_saveMutex->tryLock()){
        try{
            saveSerialTxt();
        }
        catch(...)
        {
            m_saveMutex->unlock();
        }

    }
    m_saveMutex->unlock();
}
void SmartWin::saveSerialTxt()
{

    QFileDialog * saveFileDlg = new QFileDialog();

    QString path = qApp->applicationDirPath();
    saveFileDlg->setDirectory(path);

    QString fileNames;
    fileNames = saveFileDlg->getSaveFileName(NULL,tr("Save"),path,tr("txt(*.txt)"));

    QFile file(fileNames);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Append | QIODevice::Text)){
        return;
    }

    QStringList strList = msgLisModel.stringList();
    QTextStream out(&file);
    foreach( const QString &str, strList){
        out << str <<endl;
    }

    file.close();
    delete saveFileDlg ;
}
void SmartWin::delResources()
{
    delete m_saveMutex;
}
