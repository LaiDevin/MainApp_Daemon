#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include<QDebug>
#include "smartwin.h"

#define TRANS_FILE qApp->applicationDirPath() + "/translations/gateway_zh.qm"

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator trans;
    trans.load(TRANS_FILE);
    qApp->installTranslator(&trans);

    SmartWin w;
    w.show();
    return a.exec();
}
