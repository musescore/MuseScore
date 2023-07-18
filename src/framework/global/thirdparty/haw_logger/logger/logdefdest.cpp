#include "logdefdest.h"

#include <iostream>
#include <cassert>
#include <unordered_map>

#ifdef _WIN32
#include <Windows.h>
#endif

using namespace haw::logger;

static const std::unordered_map<Color, std::string> COLOR_CODES = {
    { None,    "\033[0m" },
    { Black,   "\033[1;30m" },
    { Red,     "\033[1;31m" },
    { Green,   "\033[1;32m" },
    { Yellow,  "\033[1;33m" },
    { Blue,    "\033[1;34m" },
    { Magenta, "\033[1;35m" },
    { Cyan,    "\033[1;36m" },
    { White,   "\033[1;37m" }
};

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
    std::string msg;
    msg.reserve(100);
    msg.append(COLOR_CODES.at(logMsg.color));
    msg.append(m_layout.output(logMsg));
    msg.append("\033[0m");
    #ifdef _WIN32
    std::wstring temp = std::wstring(msg.begin(), msg.end());
    OutputDebugString(temp.c_str());
    OutputDebugString(L"\n");
    #else
    std::cout << msg << std::endl;
    #endif
}
