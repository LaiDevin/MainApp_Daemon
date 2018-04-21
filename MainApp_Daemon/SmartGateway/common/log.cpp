#include "log.h"
#ifdef WIN32
#include <direct.h>
#endif
#ifdef linux
#include <dir.h>
#endif

const int BUFFER_SIZE = 4096;

const char* Logging::s_LevelStr[LALL + 1] = {
    "RAW", "INFO", "WARN", "ERROR", "FATAL", "DEBUG", "TRACE", "ALL"
};

const char* Logging::s_fmt = "[%04d/%02d/%02d-%02d:%02d:%02d]: %s";

Logging& Logging::logger()
{
    static Logging logger;
    return logger;
}

void Logging::write(int level, const char *file, const char *fmt, ...)
{
    if (level <= getLogLevel()) {
        char buffer[BUFFER_SIZE] = {0};
        va_list a_list;
        va_start(a_list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, a_list);
        va_end(a_list);

        string str(buffer);

        ofstream _file;
        _file.open(file, ios::app);

        if (_file.is_open()) {

#if PRINT_LINE_FILE
            _file << s_LevelStr[level] << ":" <<str.c_str() << ", (File=" << __FILE__
                  << ", Line=" << __LINE__ <<")" <<endl;
#else
            _file << s_LevelStr[level] << ":" <<str.c_str() <<endl;
#endif
        }

        _file.close();

    }
}

void Logging::writeMsg(int level, const char *file, const char *msg)
{
    if (level <= getLogLevel()) {
        char buffer[BUFFER_SIZE] = {0};

        auto tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct tm *t = localtime(&tp);

        snprintf(buffer, sizeof(buffer), s_fmt,
                 t->tm_year + 1900,
                 t->tm_mon + 1,
                 t->tm_mday,
                 t->tm_hour,
                 t->tm_min,
                 t->tm_sec, msg);

        string str(buffer);

        ofstream _file;
        _file.open(file, ios::app);

        if (_file.is_open()) {

#if PRINT_LINE_FILE
            _file << s_LevelStr[level] << ":" <<str.c_str() << ", (File=" << __FILE__
                  << ", Line=" << __LINE__ <<")" <<endl;
#else
            _file << s_LevelStr[level] << ":" <<str.c_str() <<endl;
#endif
        }

        _file.close();

    }
}

void Logging::console(int level, const char *fmt, ...)
{
    if (level <= getLogLevel()) {
        char buffer[BUFFER_SIZE] = {0};
        va_list a_list;
        va_start(a_list, fmt);
        vsnprintf(buffer, sizeof(buffer), fmt, a_list);
        va_end(a_list);

        string str(buffer);
#if PRINT_LINE_FILE
        cout << s_LevelStr[level] << ":" <<str.c_str() << ", (File=" << __FILE__
             << ", Line=" << __LINE__ <<")" <<endl;
#else
        cout << s_LevelStr[level] << ":" <<str.c_str() <<endl;
#endif
    }
}

void Logging::consoleMsg(int level, const char *msg)
{
    if (level <= getLogLevel()) {
        char buffer[BUFFER_SIZE] = {0};

        auto tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct tm *t = localtime(&tp);

        snprintf(buffer, sizeof(buffer), s_fmt,
                 t->tm_year + 1900,
                 t->tm_mon + 1,
                 t->tm_mday,
                 t->tm_hour,
                 t->tm_min,
                 t->tm_sec, msg);

        string str(buffer);
#if PRINT_LINE_FILE
        cout << s_LevelStr[level] << ":" <<str.c_str() << ", (File=" << __FILE__
             << ", Line=" << __LINE__ <<")" <<endl;
#else
        cout << s_LevelStr[level] << ":" <<str.c_str() <<endl;
#endif
    }
}

void Logging::setDateTimeFileName()
{
    mkdir("log");

    auto tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *t = localtime(&tp);
    char buffer[120];
    snprintf(buffer, sizeof(buffer), "log/%04d-%02d-%02d.log",  t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    setFileName(string(buffer));
}

void Logging::log(int level, const char *msg)
{
    setLogLevel(static_cast<LogLevel>(level));

#if WRITE_TO_FILE
    setDateTimeFileName();
    writeMsg(level, fileName().c_str(), msg);
#else
    consoleMsg(level, msg);
#endif
}

string Logging::parseDateMsg(const char *msg)
{
    auto tp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *t = localtime(&tp);

    char buffer[BUFFER_SIZE] = {0};
    snprintf(buffer, sizeof(buffer), s_fmt,
             t->tm_year + 1900,
             t->tm_mon + 1,
             t->tm_mday,
             t->tm_hour,
             t->tm_min,
             t->tm_sec, msg);

    string strMsg(buffer);
    return strMsg;
}
