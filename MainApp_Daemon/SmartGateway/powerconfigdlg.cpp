#include "powerconfigdlg.h"
#include "ui_powerconfigdlg.h"
#include "Com.h"
#include "IniFile.h"
#include <vector>
#include <QDebug>

PowerConfigDlg::PowerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PowerConfigDlg)
{
    ui->setupUi(this);
    setAttrTheme();
}

PowerConfigDlg::~PowerConfigDlg()
{
    delete ui;
}
void PowerConfigDlg::setAttrTheme()
{
     setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint);
     setWindowTitle(tr("PowerConfigure"));

     Com com;
     std::vector<std::wstring> serialName = com.GetAllSerialPorts();
     ui->serialPortList->clear();
     for(int i = 0; i < serialName.size();i++){
         QString name = QString::fromStdWString(serialName[i]);
         ui->serialPortList->addItem(name);
     }

     ui->voltageNumList->setRange(0,65535);


}

void PowerConfigDlg::on_cancelBtn_clicked()
{
     this->close();
}

void PowerConfigDlg::on_confirmBtn_clicked()
{

    IniFile configFile;
    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + "config.ini");
    QString serialPort = ui->serialPortList->currentText();

    configFile.WriteString("PowerConfigure","PowerBoardSerialPort",serialPort.toStdString());

    int shutdownVoltage = ui->voltageNumList->value();
    configFile.WriteInteger("PowerConfigure","ShutdownVoltage",shutdownVoltage);

    bool checkFlag = ui->pulseOnFlag->isChecked();
    if(checkFlag){

        configFile.WriteInteger("PowerConfigure","CheckFlag",TRUE);
    }
    else {

        configFile.WriteInteger("PowerConfigure","CheckFlag",FALSE);
    }

    this->close();
}
void PowerConfigDlg::readConfigText()
{
    IniFile configFile;
    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + "config.ini");

    std::string linkerSerialPort = configFile.ReadString("PowerConfigure","PowerBoardSerialPort");
    ui->serialPortList->setCurrentText( QString::fromStdString(linkerSerialPort));

    int shutdownVoltage = configFile.ReadInteger("PowerConfigure","ShutdownVoltage");
    ui->voltageNumList->setValue(shutdownVoltage);

    int checkFlag = configFile.ReadInteger("PowerConfigure","CheckFlag");
    if( checkFlag){

        ui->pulseOnFlag->setChecked(TRUE);
    }
    else
    {
        ui->pulseOnFlag->setChecked(FALSE);
    }

}
void PowerConfigDlg::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    readConfigText();
}

