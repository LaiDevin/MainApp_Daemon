#include <thread>
#include "daemon.h"

static bool s_threadRuning = true;

void killProcess(const char *faultExe, const char *target)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

        //现在我们获得了所有进程的信息。
        //将从hSnapShot中抽取数据到一个PROCESSENTRY32结构中
        //这个结构代表了一个进程，是ToolHelp32 API的一部分。
        //抽取数据靠Process32First()和Process32Next()这两个函数。
        //这里我们仅用Process32Next()，他的原形是：
        //BOOL WINAPI Process32Next(HANDLE hSnapshot,LPPROCESSENTRY32 lppe);
        //我们程序的代码中加入：
        PROCESSENTRY32* processInfo=new PROCESSENTRY32;
        // 必须设置PROCESSENTRY32的dwSize成员的值 ;
        processInfo->dwSize=sizeof(PROCESSENTRY32);
        int index=0;
        //这里我们将快照句柄和PROCESSENTRY32结构传给Process32Next()。
        //执行之后，PROCESSENTRY32 结构将获得进程的信息。我们循环遍历，直到函数返回FALSE。
        int ID = 0;
        while(Process32Next(hSnapShot,processInfo)!=FALSE)
        {
            index++;
            int size=WideCharToMultiByte(CP_ACP,0,processInfo->szExeFile,-1,NULL,0,NULL,NULL);
            char *ch=new char[size+1];
            if(WideCharToMultiByte(CP_ACP,0,processInfo->szExeFile,-1,ch,size,NULL,NULL))
            {
                //使用这段代码的时候只需要改变"WerFault.exe" "SmartGateway.exe".将其改成你要结束的进程名就可以了。
                if(strstr(ch, faultExe) || strstr(ch, target))
                {
                    ID = processInfo->th32ProcessID;
                    HANDLE hProcess;
                    // 现在我们用函数 TerminateProcess()终止进程，这里我们用PROCESS_ALL_ACCESS
                    hProcess=OpenProcess(PROCESS_ALL_ACCESS,TRUE,ID);
                    TerminateProcess(hProcess,0);
                    CloseHandle(hProcess);
                }
            }
        }
        CloseHandle(hSnapShot);
        delete processInfo;
}

void startProcess(LPCWSTR target)
{
    SHELLEXECUTEINFO e1;
    memset(&e1,0,sizeof(e1));
    e1.cbSize=sizeof(e1);
    e1.hwnd=NULL;
    e1.lpFile=target;
    e1.nShow=1;
    e1.fMask=SEE_MASK_NOCLOSEPROCESS;
    ShellExecuteEx(&e1);
}

void startExternalExe()
{
    LPCWSTR _target = L"Daemon";
#ifdef QT_DEBUG
    _target = L"Daemond";
#endif

    startProcess(_target);
}

Daemon::Daemon() : m_timeId(-1), m_connectd(false), hPipe(INVALID_HANDLE_VALUE)
{
    startExternalExe();
    m_timeId = startTimer(5000);
    linkToDaemon();
}

Daemon::~Daemon()
{
    s_threadRuning = false;
    CloseHandle(hPipe);
}

void Daemon::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timeId) {

        if (!m_connectd) {
            m_connectd = WaitNamedPipe(TEXT(_NAMED_PIPE_NAME), NMPWAIT_WAIT_FOREVER);
            createPip(m_connectd, hPipe);
        } else {
            if (INVALID_HANDLE_VALUE == hPipe) {
                cout << "create file failed\n" << endl;
                createPip(m_connectd, hPipe);
            }
        }
    }
}


void Daemon::createPip(bool isConnected, HANDLE &h)
{   if (INVALID_HANDLE_VALUE != h) return;

    if (isConnected) {
        h = CreateFile(TEXT(_NAMED_PIPE_NAME),
                        GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
}

void Daemon::linkToDaemon()
{
    std::thread t([&]() {

        while (s_threadRuning) {

            if ( INVALID_HANDLE_VALUE != hPipe ) {
                char buf[256] = {0};
                DWORD len = 0;
                sprintf(buf, "%s%d", buf, rand() % 1000);

                if (WriteFile(hPipe, buf, sizeof(buf), &len, NULL) == FALSE) {
                    cout << "WRITE to pipe failed\n" << endl;
                    break;
                } else {

                    printf("send data to server,data=%s, size=%d\n", buf, len);

                    char rbuf[256] = {0};
                    DWORD rlen = 0;
                    ReadFile(hPipe, rbuf, sizeof(rbuf), &rlen, NULL);

                    printf("read data from server,data=%s, size=%d\n", rbuf, rlen);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        }
    });

    t.detach();
}
