#include "Log.h"

namespace vs {

void defaultOutput(const char* msg, int len) {
    fwrite(msg, 1, len, stdout);
}

void defaultFlush() {
    fflush(stdout);
}


const char* LogLevel::toString(LogLevel::Level level) {
    
    switch (level) {
    
#define XX(name) \
case LogLevel::name: \
    return #name; \
    break;
    
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);

#undef XX

    default:
        return "UNKNOW";
    }
    return "UNKNOW";
}

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
            ,const char* file, int32_t line, uint32_t thread_id, 
            uint32_t fiber_id, uint64_t time): 
            m_logger(logger),
            m_level(level),
            m_file(file),
            m_line(line),
            m_threadId(thread_id),
            m_fiberId(fiber_id),
            m_time(time)  {
}


Logger::Logger(std::string name, LogLevel::Level level, std::string default_appender):
    m_name(name),
    m_level(level) {
    
    m_fomatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c] %f:%l %T%m%n"));
    if(default_appender == "cout") {
        m_appenders.push_back(std::make_shared<StdoutLogAppender>());
    }
    else if (default_appender == "file") {
        struct tm tm;
        time_t t = time(0);
        localtime_r(&t, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
        std::stringstream ss;
        ss << buf ;

        m_appenders.push_back(std::make_shared<FileLogAppender>(name + "_" + ss.str() + ".log"));
    }
}

void Logger::log(std::shared_ptr<LogEvent> event) {
    // todo
    for(auto &ap: m_appenders) {
        ap->log(event);
    }
}

void Logger::addLogAppender(std::shared_ptr<LogAppender> LogAppender) {
    m_appenders.push_back(LogAppender);
}

void Logger::delLogAppender(std::shared_ptr<LogAppender> LogAppender) {
    if(m_appenders.size())
        m_appenders.remove(LogAppender);
}

void Logger::setFormatter(const std::string& fmtr) {
    m_fomatter = std::shared_ptr<LogFormatter>(new LogFormatter(fmtr));
}

LogManager::LogManager(std::shared_ptr<LogEvent> event) {
    m_event = event;
}

LogManager::~LogManager() {
    // logger output
    m_event->getLogger()->log(m_event);
}

std::stringstream& LogManager::getSS() {
    return m_event->getSS();
}

void StdoutLogAppender::log(std::shared_ptr<LogEvent> event) {
    event->getLogger()->getFormatter()->format(std::cout, event);
}

FileLogAppender::FileLogAppender(const std::string& filename):
    m_filename(filename) {
        reopen();
}

FileLogAppender::~FileLogAppender() {
    if(m_filestream) m_filestream.close();
}

bool FileLogAppender::reopen() {
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);

    return !!m_filestream;
}

void FileLogAppender::log(std::shared_ptr<LogEvent> event) {
    event->getLogger()->getFormatter()->format(m_filestream, event);
}

AsyncStdoutLogAppender::AsyncStdoutLogAppender(
    const std::string& name, 
    long rollSize, 
    int flushInterval):
    m_asylogging(name, std::cout, rollSize, flushInterval) {
        m_asylogging.start();
}

void AsyncStdoutLogAppender::log(std::shared_ptr<LogEvent> event) {
    std::string s = event->getLogger()->getFormatter()->format(event);
    m_asylogging.append(std::move(s.c_str()), s.size());
}

AsyncFileLogAppender::AsyncFileLogAppender(
    const std::string& filename, 
    long rollSize, 
    int flushInterval):
    FileLogAppender(filename),
    m_asylogging(filename, m_filestream, rollSize, flushInterval) {

    m_asylogging.start();
}

void AsyncFileLogAppender::log(std::shared_ptr<LogEvent> event) {
    std::string s = event->getLogger()->getFormatter()->format(event);
    m_asylogging.append(std::move(s.c_str()), s.size());
}


std::string LogFormatter::format(std::shared_ptr<LogEvent> event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<LogEvent> event) {
    for(auto& i : m_items) {
        i->format(ofs, event);
    }
    return ofs;
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getContent();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << LogLevel::toString(event->getLevel());
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getLogger()->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getFiberId();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getThreadId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << std::endl;
    }
};


class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        :m_string(str) {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, std::shared_ptr<LogEvent> event) override {
        os << "\t";
    }
private:
    std::string m_string;
};


LogFormatter::LogFormatter(const std::string& pattern):
    m_pattern(pattern) {
        init();
}

void LogFormatter::init() {
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;
    for(size_t i = 0; i < m_pattern.size(); ++i) {
        if(m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if((i + 1) < m_pattern.size()) {
            if(m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while(n < m_pattern.size()) {
            if(!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{'
                    && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if(fmt_status == 0) {
                if(m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    //std::cout << "*" << str << std::endl;
                    fmt_status = 1; //开始解析特殊格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if(fmt_status == 1) {
                if(m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    //std::cout << "#" << fmt << std::endl;
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if(n == m_pattern.size()) {
                if(str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if(fmt_status == 0) {
            if(!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        } else if(fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

        if(!nstr.empty()) {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

    // static std::map<std::string, std::function<std::shared_ptr<FormatItem>(const std::string& str)> > s_format_items = {
    //     {"m", [](const std::string& fmt) { return  std::shared_ptr<FormatItem>(new MessageFormatItem()); }}},
    // };   //将对象包装成函数

    static std::map<std::string, std::function<std::shared_ptr<FormatItem>(const std::string& str)> > s_format_items = {

#define XX(str, C) \
        {#str, [](const std::string& fmt) { return std::shared_ptr<FormatItem>(new C(fmt));}}

        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FilenameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
#undef XX
    };

    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {
            m_items.push_back(std::shared_ptr<FormatItem>(new StringFormatItem(std::get<0>(i))));
        } else {
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                m_items.push_back(std::shared_ptr<FormatItem>(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

    }
}


}