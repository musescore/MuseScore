#ifndef HAW_LOGGER_H
#define HAW_LOGGER_H

#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "logstream.h"

#undef ERROR
#undef WARN
#undef INFO
#undef DEBUG

namespace haw::logger {
//! Types
using Type = std::string;

enum Level {
    Off     = 0,
    Normal  = 1,
    Debug   = 2,
    Full    = 3
};

enum Color {
    None,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

struct Date
{
    int day = 0;
    int mon = 0;
    int year = 0;

    inline bool operator ==(const Date& d) const
    {
        return day == d.day && mon == d.mon && year == d.year;
    }

    inline bool operator !=(const Date& d) const { return !this->operator ==(d); }
};

struct Time
{
    int msec = 0;
    int sec = 0;
    int min = 0;
    int hour = 0;
};

struct DateTime
{
    Date date;
    Time time;

    static DateTime now();
};

//! Message --------------------------------
class LogMsg
{
public:

    LogMsg() = default;

    LogMsg(const Type& l, const std::string& t)
        : type(l), tag(t), datetime(DateTime::now()),
        thread(std::this_thread::get_id()) {}

    LogMsg(const Type& l, const std::string& t, const Color& color)
        : type(l), tag(t), datetime(DateTime::now()),
        thread(std::this_thread::get_id()), color(color) {}

    LogMsg(const Type& l, const std::string& t, const std::string& m)
        : type(l), tag(t), message(m), datetime(DateTime::now()),
        thread(std::this_thread::get_id()) {}

    Type type;
    std::string tag;
    std::string message;
    DateTime datetime;
    std::thread::id thread;
    Color color = None;
};

//! Layout ---------------------------------
class LogLayout
{
public:
    explicit LogLayout(const std::string& format);
    virtual ~LogLayout();

    struct Pattern {
        std::string pattern;
        std::string beforeStr;
        size_t index = std::string::npos;
        size_t count = 0;
        size_t minWidth = 0;
        Pattern() = default;
    };

    std::string format() const;

    virtual std::string output(const LogMsg& logMsg) const;

    virtual std::string formatPattern(const LogMsg& logMsg, const Pattern& p) const;
    virtual std::string formatDateTime(const DateTime& datetime) const;
    virtual std::string formatDate(const Date& date) const;
    virtual std::string formatTime(const Time& time) const;
    virtual std::string formatThread(const std::thread::id& thID) const;

    static Pattern parcePattern(const std::string& format, const std::string& pattern);
    static std::vector<Pattern> patterns(const std::string& format);

private:
    std::string m_format;
    std::vector<Pattern> m_patterns;
    std::thread::id m_mainThread;
};

//! Destination ----------------------------
class LogDest
{
public:
    explicit LogDest(const LogLayout& l);
    virtual ~LogDest();

    virtual std::string name() const = 0;
    virtual void write(const LogMsg& logMsg) = 0;

    LogLayout layout() const;

protected:
    LogLayout m_layout;
};

//! Logger ---------------------------------
class Logger
{
public:

    static Logger* instance();

    static const Type ERRR;
    static const Type WARN;
    static const Type INFO;
    static const Type DEBG;

    void setupDefault();

    void setLevel(Level level);
    Level level() const;
    inline bool isLevel(Level level) const { return level <= m_level && level != Off; }

    std::vector<Type> types() const;
    void setTypes(const std::vector<Type>& types);
    void setType(const Type& type, bool enb);
    bool isType(const Type& type) const;

    bool isAsseptMsg(const Type& type) const;

#ifdef HAW_LOGGER_QT_SUPPORT
    static void setIsCatchQtMsg(bool arg);
#endif

    void write(const LogMsg& logMsg);

    void addDest(LogDest* dest);
    std::vector<LogDest*> dests() const;
    void clearDests();

private:
    Logger();
    ~Logger();

#ifdef HAW_LOGGER_QT_SUPPORT
    static void logMsgHandler(QtMsgType, const QMessageLogContext&, const QString&);
    static Type qtMsgTypeToString(enum QtMsgType defType);
#endif

    Level m_level = Normal;
    std::vector<LogDest*> m_dests;
    std::vector<Type> m_types;
    std::mutex m_mutex;
};

//! LogInput ---------------------------------
class LogInput
{
public:
    explicit LogInput(const Type& type, const std::string& tag, const std::string& funcInfo, const Color& color);
    ~LogInput();

    Stream& stream();
    Stream& stream(const char* msg, ...);

private:
    LogMsg m_msg;
    std::string m_funcInfo;
    Stream m_stream;
};
}

#endif // HAW_LOGGER_H
