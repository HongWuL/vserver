#ifndef __VS_LOG_H
#define __VS_LOG_H

#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <functional>
#include "Utils.h"
#include "Singleton.h"
#include "Asynclogging.h"

#define LOG_LEVEL(logger, level) \  
        if (logger->getLevel() <= level) \
            vs::LogManager(std::shared_ptr<vs::LogEvent>(new vs::LogEvent(    \
                logger, level, __FILE__, __LINE__, vs::GetThreadId(), vs::GetFiberId(), time(0)) \
            )).getSS()

#define LOG_DEBUG(logger) LOG_LEVEL(logger, vs::LogLevel::Level::DEBUG)
#define LOG_INFO(logger)  LOG_LEVEL(logger, vs::LogLevel::Level::INFO)
#define LOG_WARN(logger) LOG_LEVEL(logger, vs::LogLevel::Level::WARN)
#define LOG_ERROR(logger) LOG_LEVEL(logger, vs::LogLevel::Level::ERROR)
#define LOG_FATAL(logger) LOG_LEVEL(logger, vs::LogLevel::Level::FATAL)

#define GLOG_DEBUG LOG_LEVEL(GLOGER, vs::LogLevel::Level::DEBUG)
#define GLOG_INFO  LOG_LEVEL(GLOGER, vs::LogLevel::Level::INFO)
#define GLOG_WARN LOG_LEVEL(GLOGER, vs::LogLevel::Level::WARN)
#define GLOG_ERROR LOG_LEVEL(GLOGER, vs::LogLevel::Level::ERROR)
#define GLOG_FATAL LOG_LEVEL(GLOGER, vs::LogLevel::Level::FATAL)

#define GLOGER vs::GlobalRootLogger::getInstance()

namespace vs {

class Logger;
class LogAppender;
class StdoutLogAppender;
class FileLogAppender;
class LogFormatter;

typedef SingletonPtr<Logger> GlobalRootLogger;

class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };
    static const char* toString(LogLevel::Level level);
};

class LogEvent {
public:
    LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t thread_id, 
            uint32_t fiber_id, uint64_t time);
    
    std::shared_ptr<Logger> getLogger() {return m_logger; };
    LogLevel::Level getLevel() {return m_level; };
    const char* getFile() { return m_file; };
    int32_t getLine() { return m_line; };
    uint32_t getThreadId() { return m_threadId; };
    int32_t getFiberId() { return m_fiberId; };
    uint64_t getTime() { return m_time; };
    std::string getContent() {return m_ss.str(); };
    std::stringstream& getSS() { return m_ss; };

private:
    std::shared_ptr<Logger> m_logger;   //日志器
    LogLevel::Level m_level;            //日志等级
    const char* m_file = nullptr;  //文件名
    int32_t m_line = 0;            //行号
    uint32_t m_threadId = 0;       //线程id
    uint32_t m_fiberId = 0;        //协程id
    uint64_t m_time = 0;           //时间戳
    std::stringstream m_ss;         //消息
};

class Logger {
public:

    Logger(std::string name = "system", 
        LogLevel::Level level = LogLevel::Level::INFO, 
        std::string default_appender = "file");
        
    void addLogAppender(std::shared_ptr<LogAppender> LogAppender);
    void delLogAppender(std::shared_ptr<LogAppender> LogAppender);

    void setFormatter(const std::string& fmtr);
    
    void log(std::shared_ptr<LogEvent> event);

    LogLevel::Level getLevel () { return m_level; };
    std::string getName () { return m_name; };
    std::shared_ptr<LogFormatter> getFormatter() { return m_fomatter; }

private:
    LogLevel::Level m_level;
    std::string m_name;
    std::list<std::shared_ptr<LogAppender>> m_appenders;
    std::shared_ptr<LogFormatter> m_fomatter;
    bool m_use_default_app;   // 当前使用的是否是默认的appender
};


class LogManager {
public:
    LogManager(std::shared_ptr<LogEvent> event);
    ~LogManager();

    std::stringstream& getSS();

private:
    std::shared_ptr<LogEvent> m_event;
};

class LogFormatter {
public:
    /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     *  %m 消息
     *  %p 日志级别
     *  %c 日志名称
     *  %t 线程id
     *  %n 换行
     *  %d 时间
     *  %f 文件名
     *  %l 行号
     *  %T 制表符
     *  %F 协程id
     *  %N 线程名称
     *
     *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
     */
    LogFormatter(const std::string& pattern);

    std::string format(std::shared_ptr<LogEvent> event);
    std::ostream& format(std::ostream& ofs, std::shared_ptr<LogEvent> event);
    
    void init();

    class FormatItem {
    public:
        /**
         * @brief 析构函数
         */
        virtual ~FormatItem() {}
        /**
         * @brief 格式化日志到流
         * @param[in, out] os 日志输出流
         * @param[in] logger 日志器
         * @param[in] event 日志事件
         */
        virtual void format(std::ostream& os, std::shared_ptr<LogEvent> event) = 0;
    };

private:
    std::string m_pattern;
    bool m_error = false;
    std::vector<std::shared_ptr<FormatItem>> m_items;

};

class LogAppender {
public:
    
    virtual ~LogAppender() {};
    virtual void log(std::shared_ptr<LogEvent> event) = 0;

};

class StdoutLogAppender : public LogAppender {

    void log(std::shared_ptr<LogEvent> event) override;

};

class FileLogAppender : public LogAppender {
public:

    FileLogAppender(const std::string& filename);
    ~FileLogAppender();

    void log(std::shared_ptr<LogEvent> event) override;

    bool reopen();

protected:
    std::string m_filename;
    std::ofstream m_filestream;
};

class AsyncStdoutLogAppender : public StdoutLogAppender {
public:

    AsyncStdoutLogAppender(const std::string& name, long rollSize, int flushInterval = 3);
    void log(std::shared_ptr<LogEvent> event) override;

private:
    AsyncLogging m_asylogging;
};

class AsyncFileLogAppender : public FileLogAppender {
public:

    AsyncFileLogAppender(const std::string& filename, long rollSize, int flushInterval = 3);
    void log(std::shared_ptr<LogEvent> event) override;

private:
    AsyncLogging m_asylogging;
};


}

#endif