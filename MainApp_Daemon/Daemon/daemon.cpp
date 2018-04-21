#include "daemon.h"

void createPip(HANDLE& h)
{
    h = CreateNamedPipe(
                TEXT(_NAMED_PIPE_NAME),
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                0,
                0,
                NMPWAIT_WAIT_FOREVER,
                NULL);
}

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
