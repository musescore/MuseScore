#include "logger.h"

#include <chrono>
#include <ctime>
#include <algorithm>
#include <cassert>
#include <cstdarg>
#include <map>

#include "logdefdest.h"
#include "helpful.h"

using namespace haw::logger;

const Type Logger::ERRR("ERROR");
const Type Logger::WARN("WARN");
const Type Logger::INFO("INFO");
const Type Logger::DEBG("DEBUG");

static const std::map<Type, Color> TYPES_COLOR = {
    { Logger::ERRR, Color::Red },
    { Logger::WARN, Color::Yellow },
    { Logger::INFO, Color::Green },
    { Logger::DEBG, Color::None },
};

// Layout ---------------------------------

static const std::string DATETIME_PATTERN("${datetime}");
static const std::string TIME_PATTERN("${time}");
static const std::string TYPE_PATTERN("${type}");
static const std::string TAG_PATTERN("${tag}");
static const std::string THREAD_PATTERN("${thread}");
static const std::string MESSAGE_PATTERN("${message}");

static const char ZERO('0');
static const char COLON(':');
static const char DOT('.');
static const char HYPEN('-');
static const char T('T');

static const std::string MAIN_THREAD("main_thread");

static std::string leftJustified(const std::string& in, size_t width)
{
    std::string out = in;
    if (width > out.size()) {
        out.resize(width, ' ');
    }
    return out;
}

DateTime DateTime::now()
{
    using namespace std::chrono;
    milliseconds ms_d = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    std::time_t sec = static_cast<std::time_t>(ms_d.count() / 1000);
    std::tm* tm = std::localtime(&sec);
    assert(tm);
    if (!tm) {
        return DateTime();
    }

    DateTime dt;
    dt.date.year = tm->tm_year + 1900;
    dt.date.mon = tm->tm_mon + 1;
    dt.date.day = tm->tm_mday;
    dt.time.hour = tm->tm_hour;
    dt.time.min = tm->tm_min;
    dt.time.sec = tm->tm_sec;
    dt.time.msec = static_cast<int>(ms_d.count() - (sec * 1000LL));

    return dt;
}

LogLayout::LogLayout(const std::string& format)
    : m_format(format)
{
    m_mainThread = std::this_thread::get_id();
    m_patterns = patterns(format);
}

LogLayout::~LogLayout()
{
}

std::vector<LogLayout::Pattern> LogLayout::patterns(const std::string& format)
{
    std::vector<std::string> ps = {
        DATETIME_PATTERN,
        TIME_PATTERN,
        TYPE_PATTERN,
        TAG_PATTERN,
        THREAD_PATTERN,
        MESSAGE_PATTERN
    };

    std::vector<LogLayout::Pattern> patterns;
    for (const std::string& pstr : ps) {
        Pattern p = parcePattern(format, pstr);
        if (p.index != std::string::npos) {
            patterns.push_back(std::move(p));
        }
    }

    std::sort(patterns.begin(), patterns.end(), [](const LogLayout::Pattern& f, const LogLayout::Pattern& s) {
        return f.index < s.index;
    });

    return patterns;
}

LogLayout::Pattern LogLayout::parcePattern(const std::string& format, const std::string& pattern)
{
    Pattern p;
    p.pattern = pattern;

    std::string beginPattern(pattern.substr(0, pattern.size() - 1));
    std::string::size_type beginPatternIndex = format.find(beginPattern);
    if (beginPatternIndex != std::string::npos) {
        p.index = beginPatternIndex;
        std::string::size_type last = beginPatternIndex + beginPattern.size();

        std::string::size_type filterIndex = std::string::npos;
        std::string::size_type endPatternIndex = std::string::npos;
        while (1) {
            if (!(last < format.size())) {
                break;
            }

            char c = format.at(last);
            if (c == '|') {
                filterIndex = last;
            } else if (c == '}') {
                endPatternIndex = last;
                break;
            }

            ++last;
        }

        if (filterIndex != std::string::npos) {
            std::string filter = format.substr(filterIndex + 1, endPatternIndex - filterIndex - 1);
            p.minWidth = std::stoi(filter);
        }

        p.count = endPatternIndex - beginPatternIndex + 1;

        std::string::size_type beforeEndPatternIndex = std::string::npos;
        std::string::size_type beforeBeginPatterIndex = format.find_last_of("${", p.index - 1);
        if (beforeBeginPatterIndex != std::string::npos) {
            beforeEndPatternIndex = format.find_first_of('}', beforeBeginPatterIndex) + 1;
        }

        p.beforeStr = format.substr(beforeEndPatternIndex, p.index - beforeEndPatternIndex);
    }

    return p;
}

std::string LogLayout::output(const LogMsg& logMsg) const
{
    std::string str;
    str.reserve(100);
    for (const Pattern& p : m_patterns) {
        str.append(p.beforeStr).append(formatPattern(logMsg, p));
    }
    return str;
}

std::string LogLayout::formatPattern(const LogMsg& logMsg, const Pattern& p) const
{
    if (DATETIME_PATTERN == p.pattern) {
        return leftJustified(formatDateTime(logMsg.datetime), p.minWidth);
    } else if (TIME_PATTERN == p.pattern) {
        return leftJustified(formatTime(logMsg.datetime.time), p.minWidth);
    } else if (TYPE_PATTERN == p.pattern) {
        return leftJustified(logMsg.type, p.minWidth);
    } else if (TAG_PATTERN == p.pattern) {
        return leftJustified(logMsg.tag, p.minWidth);
    } else if (THREAD_PATTERN == p.pattern) {
        return leftJustified(formatThread(logMsg.thread), p.minWidth);
    } else if (MESSAGE_PATTERN == p.pattern) {
        return leftJustified(logMsg.message, p.minWidth);
    }

    return p.pattern;
}

std::string LogLayout::formatDateTime(const DateTime& dt) const
{
    std::string str;
    str.reserve(22);
    str.append(formatDate(dt.date));
    str.push_back(T);
    str.append(formatTime(dt.time));
    return str;
}

std::string LogLayout::formatDate(const Date& d) const
{
    std::string str;
    str.reserve(10);

    str.append(std::to_string(d.year)).push_back(HYPEN);

    if (d.mon < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(d.mon)).push_back(HYPEN);

    if (d.day < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(d.day));

    return str;
}

std::string LogLayout::formatTime(const Time& t) const
{
    std::string str;
    str.reserve(12);

    if (t.hour < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(t.hour)).push_back(COLON);

    if (t.min < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(t.min)).push_back(COLON);

    if (t.sec < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(t.sec)).push_back(DOT);

    if (t.msec < 100) {
        str.push_back(ZERO);
    }

    if (t.msec < 10) {
        str.push_back(ZERO);
    }
    str.append(std::to_string(t.msec));

    return str;
}

std::string LogLayout::formatThread(const std::thread::id& thID) const
{
    if (m_mainThread == thID) {
        return MAIN_THREAD;
    }
    std::ostringstream ss;
    ss << thID;
    return ss.str();
}

std::string LogLayout::format() const
{
    return m_format;
}

// LogDest ---------------------------------

LogDest::LogDest(const LogLayout& l)
    : m_layout(l)
{}

LogDest::~LogDest()
{}

LogLayout LogDest::layout() const
{
    return m_layout;
}

// Logger ---------------------------------

Color Logger::colorForType(const Type& type)
{
    auto it = TYPES_COLOR.find(type);
    if (it != TYPES_COLOR.end()) {
        return it->second;
    }
    return Color::None;
}

Logger* Logger::instance()
{
    static Logger l;
    return &l;
}

Logger::Logger()
{
    setupDefault();
}

Logger::~Logger()
{
#ifdef HAW_LOGGER_QT_SUPPORT
    setIsCatchQtMsg(false);
#endif
    clearDests();
}

void Logger::setupDefault()
{
    clearDests();
    addDest(new ConsoleLogDest(LogLayout("${time} | ${type|5} | ${thread} | ${tag|10} | ${message}")));

    m_level = Normal;

    m_types.clear();
    m_types.push_back(ERRR);
    m_types.push_back(WARN);
    m_types.push_back(INFO);
    m_types.push_back(DEBG);

#ifdef HAW_LOGGER_QT_SUPPORT
    setIsCatchQtMsg(true);
#endif
}

void Logger::write(const LogMsg& logMsg)
{
    std::lock_guard<std::mutex> locker(m_mutex);
    if (isAsseptMsg(logMsg.type)) {
        for (LogDest* dest : m_dests) {
            dest->write(logMsg);
        }
    }
}

bool Logger::isAsseptMsg(const Type& type) const
{
    return m_level == Full || m_level == Normal || isType(type);
}

bool Logger::isType(const Type& type) const
{
    if (std::find(m_types.begin(), m_types.end(), type) != m_types.end()) {
        return true;
    }
    return false;
}

void Logger::addDest(LogDest* dest)
{
    assert(dest);
    m_dests.push_back(dest);
}

std::vector<LogDest*> Logger::dests() const
{
    return m_dests;
}

void Logger::clearDests()
{
    for (LogDest* d : m_dests) {
        delete d;
    }
    m_dests.clear();
}

void Logger::setLevel(const Level level)
{
    m_level = level;
}

Level Logger::level() const
{
    return m_level;
}

std::vector<Type> Logger::types() const
{
    return m_types;
}

void Logger::setTypes(const std::vector<Type>& types)
{
    m_types = types;
}

void Logger::setType(const Type& type, bool enb)
{
    if (enb) {
        if (!isType(type)) {
            m_types.push_back(type);
        }
    } else {
        m_types.erase(std::remove(m_types.begin(), m_types.end(), type), m_types.end());
    }
}

#ifdef HAW_LOGGER_QT_SUPPORT
void Logger::logMsgHandler(QtMsgType type, const QMessageLogContext& ctx, const QString& s)
{
    if (type == QtDebugMsg && !Logger::instance()->isLevel(Debug)) {
        return;
    }

    static std::string Qt("Qt");

    std::string msg;
    if (ctx.function) {
        std::string sig = ctx.function;
        msg = Helpful::methodName(sig) + ": " + s.toStdString();
    } else {
        msg = s.toStdString();
    }

    LogMsg logMsg(qtMsgTypeToString(type), Qt, msg);
    logMsg.color = Logger::colorForType(logMsg.type);

    Logger::instance()->write(logMsg);
}

Type Logger::qtMsgTypeToString(enum QtMsgType defType)
{
    switch (defType) {
    case QtDebugMsg: return DEBG;
    case QtWarningMsg: return WARN;
    case QtCriticalMsg: return ERRR;
    case QtFatalMsg: return ERRR;
    default: return INFO;
    }
}

void Logger::setIsCatchQtMsg(bool arg)
{
    QtMessageHandler h = arg ? logMsgHandler : 0;
    qInstallMessageHandler(h);
}

#endif

LogInput::LogInput(const Type& type, const std::string& tag, const std::string& funcInfo, const Color& color)
    : m_msg(type, tag, color), m_funcInfo(funcInfo)
{
}

LogInput::~LogInput()
{
    m_msg.message = m_funcInfo + m_stream.str();
    Logger::instance()->write(m_msg);
}

Stream& LogInput::stream()
{
    return m_stream;
}

Stream& LogInput::stream(const char* msg, ...)
{
    static const int BUFFER_SIZE = 2048;

    va_list args;
    va_start(args, msg);
    char buffer[BUFFER_SIZE];
    vsnprintf(buffer, BUFFER_SIZE, msg, args);
    m_stream << buffer;
    va_end(args);

    return m_stream;
}
