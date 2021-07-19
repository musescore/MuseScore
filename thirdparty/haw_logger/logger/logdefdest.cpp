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
FileLogDest::FileLogDest(const std::string& filePath, const LogLayout& l)
    : LogDest(l), m_filePath(filePath)
{
    m_file.open(m_filePath, std::ios_base::out | std::ios_base::app);
    if (!m_file.is_open()) {
        std::clog << "failed open log file: " << m_filePath << std::endl;
    }
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

std::string FileLogDest::filePath() const
{
    return m_filePath;
}

void FileLogDest::write(const LogMsg& logMsg)
{
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
