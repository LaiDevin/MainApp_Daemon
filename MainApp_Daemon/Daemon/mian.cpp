
/***
 * author: Devin
 * date:  2018/04/21
 * brief: @
***/

#include <iostream>
#include <windows.h>
#include <thread>
#include <mutex>
#include "daemon.h"


using namespace std;

static bool _pipeConnected = false;
static unsigned int _index = 0;


int main(int argc, char *argv[])
{

    char    buf[256] = "";

    DWORD   rlen = 0;
    HANDLE  hPipe;

    createPip(hPipe);

    if (INVALID_HANDLE_VALUE == hPipe) {
        std::cout << "created pipe failed!\n" << std::endl;
        exit(-1);
    }

    cout << "waiting connecting pipe\n" << endl;

    std::thread t_1([&]() {

        while (1) {
            if ( !_pipeConnected )   {

                if (ConnectNamedPipe(hPipe, NULL) == FALSE) {
                    cout << "connect pipe failed\n" << endl;
                    _pipeConnected = false;
                } else {
                    cout << "connect pipe success\n" << endl;
                    _pipeConnected = true;
                    mutex tx;
                    tx.lock();
                    _index = 0;
                    tx.unlock();
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        }

    });

    std::thread t_2([&]() {

        while (1) {

            if ( _pipeConnected ) {

                if (ReadFile(hPipe, buf, 256, &rlen, NULL) == FALSE) {
                    cout << "read data from pipe failed\n" << endl;

                    mutex tx;
                    tx.lock();

                    _index++;
                    if (_index > 3) {
                        _index = 0;
                        tx.unlock();
                        DisconnectNamedPipe(hPipe);
                        _pipeConnected = false;

                        killProcess(_WINDOWS_FAULT_EXE, _DAEMON_TARGET);
                        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                        startProcess(_DAEMON_START);
                    }

                } else {
                    printf("from pipe read data=%s, size=%d\n", buf, rlen);

                    char wbuf[256] = "";
                    sprintf(wbuf, "%s%d", wbuf, rand() % 1000);

                    DWORD wlen = 0;
                    WriteFile(hPipe, wbuf, sizeof(wbuf), &wlen, NULL);
                    _index = 0;
                }

            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        }
    });

    t_1.join();
    t_2.join();


    CloseHandle(hPipe);
    system("PAUSE");
    return 0;
}
