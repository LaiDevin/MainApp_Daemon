#ifndef GATEWAYSERVERDLG_H
#define GATEWAYSERVERDLG_H

#include <QDialog>
#include "IniFile.h"

namespace Ui {
class GatewayServerDlg;
}

class GatewayServerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit GatewayServerDlg(QWidget *parent = 0);
    ~GatewayServerDlg();

    void readGateWayIni(std::vector<IniFile::T_LineConf> &vecSection);

private:
    void setAttrTheme();
    void initData();

signals:
    void clickOkSig();

private slots:
    void on_btn_ok_clicked();
    void on_btn_cancel_clicked();
    void udp_Btn_click();
    void tcp_Btn_click();

private:
    Ui::GatewayServerDlg *ui;
    enum COMMUNICATION_TYPE{
        TCP,
        UDP
    };
    int m_CommunicationType;
};

#endif // GATEWAYSERVERDLG_H
