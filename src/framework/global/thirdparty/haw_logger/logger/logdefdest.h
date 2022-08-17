#ifndef HAW_DEFLOGDEST_H
#define HAW_DEFLOGDEST_H

#include <fstream>
#include <ctime>

#include "logger.h"

namespace haw::logger {
class MemLogDest : public LogDest
{
public:
    MemLogDest(const LogLayout& l);

    std::string name() const;
    void write(const LogMsg& logMsg);

    std::string content() const;

private:
    std::stringstream m_stream;
};

class FileLogDest : public LogDest
{
public:
    FileLogDest(const std::string& filePath, const LogLayout& l);
    ~FileLogDest();

    std::string name() const;
    void write(const LogMsg& logMsg);

    std::string filePath() const;

private:

    std::ofstream m_file;
    std::string m_filePath;
};

class ConsoleLogDest : public LogDest
{
public:
    explicit ConsoleLogDest(const LogLayout& l);

    std::string name() const;
    void write(const LogMsg& logMsg);
};
}

#endif // HAW_DEFLOGDEST_H
