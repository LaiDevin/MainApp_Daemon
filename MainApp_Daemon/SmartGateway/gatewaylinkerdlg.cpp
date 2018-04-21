#include "gatewaylinkerdlg.h"
#include "ui_gatewaylinkerdlg.h"
#include "Com.h"
#include "IniFile.h"

GatewayLinkerDlg::GatewayLinkerDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GatewayLinkerDlg)
{
    ui->setupUi(this);
    setAttrTheme();
}

GatewayLinkerDlg::~GatewayLinkerDlg()
{
    delete ui;
}
void GatewayLinkerDlg::setAttrTheme()
{
    setWindowFlags(Qt::Dialog|Qt::WindowCloseButtonHint);
    setWindowTitle(tr("GatewayLinker"));

    Com com;
    std::vector<std::wstring> serialName = com.GetAllSerialPorts();
    ui->serialPortList->clear();
    for(int i = 0; i < serialName.size();i++){
        QString name = QString::fromStdWString(serialName[i]);
        ui->serialPortList->addItem(name);
    }
    ui->linkerIdList->setRange(0,65535);
}

void GatewayLinkerDlg::on_cancelBtn_clicked()
{
    this->close();
}

void GatewayLinkerDlg::on_confirmBtn_clicked()
{
    IniFile configFile;
    QString path = qApp->applicationDirPath() + "/";

    configFile.LoadFile(path.toStdString() + "config.ini");
    QString serialPort = ui->serialPortList->currentText();

    configFile.WriteString("GatewayLinkerConfigure","LinkerSerialPort",serialPort.toStdString());

    int linkerIdValue = ui->linkerIdList->value();
    configFile.WriteInteger("GatewayLinkerConfigure","LinkerIdValue",linkerIdValue);

    bool checkFlag = ui->linkerOpenFlag->isChecked();
    if(checkFlag){

        configFile.WriteInteger("GatewayLinkerConfigure","CheckFlag",TRUE);
    }
    else {

        configFile.WriteInteger("GatewayLinkerConfigure","CheckFlag",FALSE);
    }

    this->close();
}
void GatewayLinkerDlg::readConfigText()
{
    IniFile configFile;
    QString path = qApp->applicationDirPath() + "/";
    configFile.LoadFile(path.toStdString() + "config.ini");
    std::string linkerSerialPort = configFile.ReadString("GatewayLinkerConfigure","LinkerSerialPort");
    ui->serialPortList->setCurrentText( QString::fromStdString(linkerSerialPort));

    int linkerIdValue = configFile.ReadInteger("GatewayLinkerConfigure","LinkerIdValue");
    ui->linkerIdList->setValue(linkerIdValue);

    int checkFlag = configFile.ReadInteger("GatewayLinkerConfigure","CheckFlag");
    if( checkFlag){

        ui->linkerOpenFlag->setChecked(TRUE);
    }
    else
    {
        ui->linkerOpenFlag->setChecked(FALSE);
    }

}
void GatewayLinkerDlg::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    readConfigText();
}
