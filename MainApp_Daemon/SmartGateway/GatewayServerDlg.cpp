#include "GatewayServerDlg.h"
#include "ui_GatewayServerDlg.h"
#include "IniFile.h"
#include "Com.h"
#include <QDebug>

GatewayServerDlg::GatewayServerDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GatewayServerDlg),m_CommunicationType(1)
{
    ui->setupUi(this);
    initData();
    setAttrTheme();
    connect(ui->udpBtn,SIGNAL(clicked(bool)),this,SLOT(udp_Btn_click()));
    connect(ui->tcpBtn,SIGNAL(clicked(bool)),this,SLOT(tcp_Btn_click()));
}

GatewayServerDlg::~GatewayServerDlg()
{
    delete ui;
}

void GatewayServerDlg::initData()
{
    Com com;
    std::vector<std::wstring> vctTemp = com.GetAllSerialPorts();
    ui->comboBox_3G_COM->clear();
    m_CommunicationType = UDP;
    for (auto &item : vctTemp) {
        ui->comboBox_3G_COM->addItem( QString::fromStdWString(item));
    }
}

void GatewayServerDlg::on_btn_ok_clicked()
{
    QString  Server_addr = ui->lineEdit_domainName->text();
    QString  Server_IP = ui->lineEdit_IP->text();
    bool  b_userIP_chk =  ui->checkBox_useIP->checkState();   // false -- un checked
    QString ServerPort =  ui->spinBox_serverPort->text();
    QString localPort = ui->spinBox_localPort->text();
    QString gatewayID = ui->spinBox_gatewayID->text();
    QString handleShake  = ui->spinBox_handshake->text();
    QString  com3G = ui->comboBox_3G_COM->currentText();
    bool b_use3G_chk = ui->checkBox_use3G->checkState();
    bool b_serverConnect_chk  = ui->checkBox_startConnect->checkState();

    Server_addr = Server_addr.remove(QRegExp("\\s"));
    Server_IP = Server_IP.remove(QRegExp("\\s"));

    if(Server_addr.isEmpty() || Server_IP.isEmpty())   // check
        return;

    qDebug()<<"check number:"<<Server_addr<<Server_IP<<b_userIP_chk<<ServerPort
           <<localPort<<gatewayID<<handleShake<<com3G<<b_use3G_chk<<b_serverConnect_chk;

    IniFile configFile;
    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + "config.ini");
    qDebug()<<"print path:"<<path<< "config.ini";

    std::string  GatewaySer_sec=   configFile.ReadSection("GatewayServerConfiguration");
    configFile.WriteString("GatewayServerConfiguration","Server_addr",Server_addr.toStdString());
    configFile.WriteString("GatewayServerConfiguration","Server_IP",Server_IP.toStdString());
    configFile.WriteInteger("GatewayServerConfiguration","b_userIP_chk",(b_userIP_chk ? 1:0));
    configFile.WriteInteger("GatewayServerConfiguration","CommunicationType",m_CommunicationType);
    configFile.WriteString("GatewayServerConfiguration","ServerPort",ServerPort.toStdString());
    configFile.WriteString("GatewayServerConfiguration","localPort",localPort.toStdString());
    configFile.WriteString("GatewayServerConfiguration","gatewayID",gatewayID.toStdString());
    configFile.WriteString("GatewayServerConfiguration","handleShake",handleShake.toStdString());
    configFile.WriteString("GatewayServerConfiguration","com3G",com3G.toStdString());
    configFile.WriteInteger("GatewayServerConfiguration","b_use3G_chk",(b_use3G_chk ? 1:0));
    configFile.WriteInteger("GatewayServerConfiguration","b_serverConnect_chk",(b_serverConnect_chk ? 1:0));

    QDialog::accept();
    emit clickOkSig();
    this->close();
}

void GatewayServerDlg::udp_Btn_click(){
    m_CommunicationType = UDP;
    ui->udpBtn->setChecked(true);
}

void GatewayServerDlg::tcp_Btn_click(){
    m_CommunicationType = TCP;
    ui->tcpBtn->setChecked(true);
}

void GatewayServerDlg::on_btn_cancel_clicked()
{
    QDialog::reject();
    this->close();
}
void GatewayServerDlg::setAttrTheme()
{
    setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint);
    setWindowTitle(tr("GatewayServerConfigure"));
}

void GatewayServerDlg::readGateWayIni(std::vector<IniFile::T_LineConf> &vecSection)
{
    for (auto& v : vecSection) {
        if (v.Key == "Server_IP") {
            ui->lineEdit_IP->setText(QString::fromStdString(v.Value));
        } else if (v.Key == "ServerPort") {
            ui->spinBox_serverPort->setValue(std::stoi(v.Value));
        } else if (v.Key == "localPort") {
            ui->spinBox_localPort->setValue(std::stoi(v.Value));
        } else if (v.Key == "Server_addr") {
            ui->lineEdit_domainName->setText(QString::fromStdString(v.Value));
        } else if (v.Key == "b_userIP_chk") {
            ui->checkBox_useIP->setChecked(std::stoi(v.Value) == 1 ? true : false);
        } else if(v.Key == "CommunicationType"){
            int  nType = std::stoi(v.Value);
            if(nType){
                ui->udpBtn->setChecked(true);
            }else {
                ui->tcpBtn->setCheckable(true);
            }
        }  else if (v.Key == "gatewayID") {
            ui->spinBox_gatewayID->setValue(std::stoi(v.Value));
        } else if (v.Key == "handleShake") {
            ui->spinBox_handshake->setValue(std::stoi(v.Value));
        } else if (v.Key == "com3G") {
            int index = ui->comboBox_3G_COM->findText(QString::fromStdString(v.Value));
            ui->comboBox_3G_COM->setCurrentIndex(index > -1 ? index : 0);
        } else if (v.Key == "b_use3G_chk") {
            ui->checkBox_use3G->setChecked(std::stoi(v.Value) == 1 ? true : false);
        } else if (v.Key == "b_serverConnect_chk") {
            ui->checkBox_startConnect->setChecked(std::stoi(v.Value) == 1 ? true : false);
        }
    }
}
