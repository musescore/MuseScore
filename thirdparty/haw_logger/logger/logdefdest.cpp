#include "logdefdest.h"

#include <iostream>
#include <cassert>

#ifdef HAW_LOGGER_QT_SUPPORT
    #define USE_QT_DIR
    #include <QDir>
#else
//    #if __has_include(<filesystem>)
//        #include <filesystem>
//        namespace fs = std::filesystem;
//    #elif __has_include(<experimental/filesystem>)
//        #include <experimental/filesystem>
//        namespace fs = std::experimental::filesystem;
//    #else
//        #error compiler must either support c++17
//    #endif
#error At moment supported only with Qt
#endif

using namespace haw::logger;

MemLogDest::MemLogDest(const LogLayout& l)
    : LogDest(l)
{
}

std::string MemLogDest::name() const
{
    return "MemLogDest";
}

void MemLogDest::write(const LogMsg& logMsg)
{
    m_stream << m_layout.output(logMsg) << std::endl;
}

std::string MemLogDest::content() const
{
    return m_stream.str();
}

// FileLogDest
FileLogDest::FileLogDest(const std::string& path, const std::string& name, const std::string& ext, const LogLayout& l)
    : LogDest(l), m_path(path), m_name(name), m_ext(ext)
{
    rotate();
}

FileLogDest::~FileLogDest()
{
    if (m_file.is_open()) {
        m_file.close();
    }
}

std::string FileLogDest::name() const
{
    return "FileLogDest";
}

void FileLogDest::rotate()
{
    if (m_file.is_open()) {
        m_file.close();
    }

#ifdef USE_QT_DIR
    QString path = QString::fromStdString(m_path);
    if (!QDir(path).exists()) {
        bool ok = QDir().mkpath(path);
        assert(ok);
        if (!ok) {
            std::clog << "failed create dir: " << path.toStdString() << std::endl;
            return;
        }
    }
#else
    fs::path path = m_path;
    if (!fs::exists(path)) {
        bool ok = fs::create_directories(path);
        assert(ok);
        if (!ok) {
            std::clog << "failed create dir: " << path << std::endl;
            return;
        }
    }
#endif

    auto formatDate = [](const Date& d) {
                          std::string str;
                          str.reserve(10);

                          str.append(std::to_string(d.year + 1900));

                          if (d.mon < 11) {
                              str.push_back('0');
                          }
                          str.append(std::to_string(d.mon + 1));

                          if (d.day < 10) {
                              str.push_back('0');
                          }
                          str.append(std::to_string(d.day));

                          return str;
                      };

    m_rotateDate = DateTime::now().date;
    std::string dateStr = formatDate(m_rotateDate);
    std::string filePath = m_path + "/" + m_name + '_' + dateStr + '.' + m_ext;

    m_file.open(filePath, std::ios_base::out | std::ios_base::app);
    if (!m_file.is_open()) {
        std::clog << "failed open log file: " << m_path << std::endl;
    }
}

void FileLogDest::write(const LogMsg& logMsg)
{
    if (m_rotateDate != logMsg.datetime.date) {
        rotate();
    }

    m_file << m_layout.output(logMsg) << std::endl;
    m_file.flush();
}

// OutputDest
ConsoleLogDest::ConsoleLogDest(const LogLayout& l)
    : LogDest(l)
{
}

std::string ConsoleLogDest::name() const
{
    return "ConsoleLogDest";
}

void ConsoleLogDest::write(const LogMsg& logMsg)
{
    std::clog << m_layout.output(logMsg) << std::endl;
}
