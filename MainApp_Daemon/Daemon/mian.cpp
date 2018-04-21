#include <iostream>
#include <windows.h>


using namespace std;

#define _NAMED_PIPE_NAME "\\\\.\\Pipe\\zdst_smart_gateway"


int main()
{

    char buf[256] = "";

        DWORD rlen = 0;

        HANDLE hPipe = CreateNamedPipe(
        TEXT(_NAMED_PIPE_NAME),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            0,
            0,
            NMPWAIT_WAIT_FOREVER,
            NULL);

        if (INVALID_HANDLE_VALUE == hPipe)
        {
            std::cout << "created pipe failed!\n" << std::endl;
        }
        else {
            cout << "waiting connecting pipe\n" << endl;
            if (ConnectNamedPipe(hPipe, NULL) == FALSE) {
                cout << "connect pipe failed\n" << endl;
            }
            else {
                cout << "connect pipe success\n" << endl;
            }

            while (1)
            {
                if (ReadFile(hPipe, buf, 256, &rlen, NULL) == FALSE)
                {
                    cout << "read data from pipe failed\n" << endl;
                    //break;
                }
                else
                {
                    //cout << "read data from pipe success\n" << endl;
                    printf("from pipe read data=%s, size=%d\n", buf, rlen);

                    char wbuf[256] = "";
                    sprintf(wbuf, "%s%d", wbuf, rand() % 1000);

                    DWORD wlen = 0;
                    WriteFile(hPipe, wbuf, sizeof(wbuf), &wlen, NULL);
                    Sleep(3000);
                }
            }

            CloseHandle(hPipe);
        }
    system("PAUSE");
    return 0;
}
