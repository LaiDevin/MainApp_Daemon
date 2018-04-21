#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <IniFile.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_btnOpen_clicked();

    void on_btnOpen_2_clicked();

    void on_btnOpen_3_clicked();

    void on_btn_gatewayServerSet_clicked();

private:
    Ui::MainWindow *ui;
    IniFile *config;
};

#endif // MAINWINDOW_H
