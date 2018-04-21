/***
 * author: Devin
 * date:  2018/04/21
 * brief: @
***/

#ifndef DAEMON_H
#define DAEMON_H
#include <QObject>
#include <QTimerEvent>
#include <iostream>
#include <windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <winnt.h>
#include <shellapi.h>

using namespace  std;
#define _NAMED_PIPE_NAME    "\\\\.\\Pipe\\zdst_smart_gateway"

#define _WINDOWS_FAULT_EXE  "WerFault.exe"

#ifdef QT_DEBUG
#define _DAEMON_TARGET "Daemond"
#else
#define _DAEMON_TARGET "Daemon"
#endif

void killProcess(const char* faultExe, const char* target);
void startProcess(LPCWSTR target);
void startExternalExe();

class Daemon : public QObject
{
  Q_OBJECT

public:
    Daemon();
    ~Daemon();

protected:
    void timerEvent(QTimerEvent* e);

private:
    int m_timeId;
    bool m_connectd;

    HANDLE hPipe;

    void createPip(bool isConnected, HANDLE& h);
    void linkToDaemon();
};

#endif // DAEMON_H
