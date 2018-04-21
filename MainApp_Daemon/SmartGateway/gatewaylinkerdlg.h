#ifndef GATEWAYLINKERDLG_H
#define GATEWAYLINKERDLG_H

#include <QDialog>

namespace Ui {
class GatewayLinkerDlg;
}

class GatewayLinkerDlg : public QDialog
{
    Q_OBJECT

public:
    explicit GatewayLinkerDlg(QWidget *parent = 0);
    ~GatewayLinkerDlg();

private slots:
    void on_cancelBtn_clicked();

    void on_confirmBtn_clicked();
protected slots:

    void showEvent(QShowEvent* event);

private:
    void setAttrTheme();

    void readConfigText();

private:
    Ui::GatewayLinkerDlg *ui;
};

#endif // GATEWAYLINKERDLG_H
