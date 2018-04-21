#ifndef LOG_H
#define LOG_H


/***
 * author: Devin
 * date:  2018/03/29
 * brief: @
***/

#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <stdarg.h>

#define PRINT_LINE_FILE 0 // which line and which file, can be printed

#undef WRITE_TO_FILE //flag: write to file, 1 or 0

#if defined(LOG)
#define WRITE_TO_FILE 1
#else WRITE_TO_FILE 0
#endif

#if WRITE_TO_FILE
#define LOG_FORMAT(level,fmt,...) \
    Logging::logger().write(level, \
    fmt,Logging::logger().fmt(), __VA_ARGS__);
#else
#define LOG_FORMAT(level,fmt,...) \
    Logging::logger().console(level,fmt,__VA_ARGS__);
#endif

#define LOG(L,MSG) Logging::logger().log(L, MSG)

#define LOG_RAW(MSG) LOG(Logging::LRAW,MSG)
#define LOG_INFO(MSG) LOG(Logging::LINFO,MSG)
#define LOG_WARN(MSG) LOG(Logging::LWARN,MSG)
#define LOG_ERROR(MSG) LOG(Logging::LERROR,MSG)
#define LOG_FATAL(MSG) LOG(Logging::LFATAL,MSG)
#define LOG_DEBUG(MSG) LOG(Logging::LDEBUG,MSG)
#define LOG_TRACE(MSG) LOG(Logging::LTRACE,MSG)
#define LOG_ALL(MSG) LOG(Logging::LALL,MSG)

#define PARSE_MSG(MSG) Logging::parseDateMsg(MSG)

using namespace std;

struct NoCopyable
{
    NoCopyable() {}
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable& operator = (const NoCopyable&) = delete;
};

struct Logging : private NoCopyable
{

    enum LogLevel {LRAW, LINFO, LWARN, LERROR,
                   LFATAL, LDEBUG, LTRACE, LALL} ;

    Logging(): m_level(LRAW) {}
    ~Logging() {}

    void setLogLevel(LogLevel level) {m_level = min(LALL, max(LRAW, level));}
    LogLevel getLogLevel() { return m_level; }

    void setFileName(const string &file) { m_fileName = file; }
    const string fileName() const { return m_fileName; }

    const char* getLevelStr() const { return s_LevelStr[m_level];}

    void write(int level, const char* file, const char* fmt, ...);
    void writeMsg(int level, const char* file, const char* msg);

    void console(int level, const char* fmt, ...);
    void consoleMsg(int level, const char* msg);

    void log(int level, const char *msg);

    const char* fmt() const { return s_fmt; }

    void setDateTimeFileName();

    static Logging& logger();

    static string parseDateMsg(const char* msg);

private:
    string m_fileName;
    LogLevel m_level;

    static const char* s_LevelStr[LALL + 1];
    static const char* s_fmt;
};
#endif // LOG_H
