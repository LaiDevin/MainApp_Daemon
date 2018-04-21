
/***
 * author: Devin
 * date:  2018/04/21
 * brief: @
***/

#ifndef DAEMON_H
#define DAEMON_H
#include <iostream>
#include <windows.h>
#include <Tlhelp32.h>
#include <stdio.h>
#include <winnt.h>
#include <shellapi.h>

#define _NAMED_PIPE_NAME    "\\\\.\\Pipe\\zdst_smart_gateway"
#define _WINDOWS_FAULT_EXE  "WerFault.exe"
#define _DAEMON_TARGET      "SmartGateway.exe"
#define _DAEMON_START       L"SmartGateway.exe"

void createPip(HANDLE &h);
void killProcess(const char* faultExe, const char* target);
void startProcess(LPCWSTR target);

#endif // DAEMON_H
