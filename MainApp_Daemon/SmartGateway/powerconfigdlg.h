#ifndef POWERCONFIGDLG_H
#define POWERCONFIGDLG_H

#include <QDialog>

namespace Ui {
class PowerConfigDlg;
}

class PowerConfigDlg : public QDialog
{
    Q_OBJECT

public:
    explicit PowerConfigDlg(QWidget *parent = 0);
    ~PowerConfigDlg();

private slots:
    void on_cancelBtn_clicked();

    void on_confirmBtn_clicked();

protected:

    void showEvent(QShowEvent* event);

private:
    void setAttrTheme();

    void readConfigText();


private:
    Ui::PowerConfigDlg *ui;
};

#endif // POWERCONFIGDLG_H
