#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "FasManager.h"
#include "Com.h"
#include <QDebug>
#include "GatewayServerDlg.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("智慧网关v1.01");
    setWindowFlags(Qt::Dialog | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setFixedSize(this->width(), this->height());

    FasManager *pobjFasManager = FasManager::getInstance();
    vector<FasInfo> stFasInfoList;
    pobjFasManager->getFasList(stFasInfoList);
    QStringList strFasList;
    for(size_t i = 0; i < stFasInfoList.size(); ++ i){
        //qDebug() <<stFasInfoList[i].nID <<":"<< QString::fromLocal8Bit(stFasInfoList[i].strName);
        strFasList << QString::fromLocal8Bit(stFasInfoList[i].strName);
    }
    ui->fasList->addItems(strFasList);

    Com com;
    std::vector<std::wstring> vctTemp = com.GetAllSerialPorts();
    ui->comboBox_serialNum->clear();
    for(int i = 0; i < vctTemp.size();i++){
        QString name = QString::fromStdWString(vctTemp[i]);
        ui->comboBox_serialNum->addItem(name);
    }

    config = new IniFile();

}

MainWindow::~MainWindow()
{
    delete config;
    delete ui;
}


void MainWindow::on_btnOpen_clicked()
{
    if(ui->btnOpen->text() == tr("打开")){
        ui->btnOpen->setText(tr("关闭"));
        ui->fasList->setEnabled(false);
    }else {
        ui->btnOpen->setText(tr("打开"));
        ui->fasList->setEnabled(true);
    }
}

void MainWindow::on_btnOpen_2_clicked()
{
    QString path = qApp->applicationDirPath() + "/";

    config->LoadFile(path.toStdString() + "config.ini");

    std::string section = config->ReadSection("network");
    config->ReadInteger("network","server_port");
}

void MainWindow::on_btnOpen_3_clicked()
{
    config->WriteString("network","server_port","8080");
}

void MainWindow::on_btn_gatewayServerSet_clicked()
{
    GatewayServerDlg dlg;
    dlg.exec();
}
