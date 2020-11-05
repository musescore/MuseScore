#include <iostream>
#include <thread>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include <QDebug>

#include "logdefdest.h"
#include "log.h"

class Example
{
public:
    Example() = default;

    void example()
    {
        using namespace haw::logger;

        Logger* logger = Logger::instance();
        logger->setupDefault();

        //! Default output to console, catch Qt messages

        LOGE() << "This is error";
        LOGW() << "This is warning";
        LOGI() << "This is info";
        LOGD() << "This is debug"; //! NOTE Default not output

        std::thread t1([]() { LOGI() << "From thread"; });
        t1.join();

        qCritical() << "This is qCritical";
        qWarning() << "This is qWarning";
        qDebug() << "This is qDebug"; //! NOTE Default not output

        /*
        15:21:53.454 | ERROR | main_thread     | Example    | example: This is error
        15:21:53.454 | WARN  | main_thread     | Example    | example: This is warning
        15:21:53.454 | INFO  | main_thread     | Example    | example: This is info
        15:21:53.454 | INFO  | 139855304345344 | Example    | example: From thread
        15:21:53.455 | ERROR | main_thread     | Qt         | example: This is qCritical
        15:21:53.455 | WARN  | main_thread     | Qt         | example: This is qWarning
        */

        //! Set tag (default class name)

        #undef LOG_TAG
        #define LOG_TAG "MYTAG"

        LOGI() << "This is info with tag";
        /*
        15:21:53.455 | INFO  | main_thread     | MYTAG      | example: This is info with tag
        */

        //! Set log level
        logger->setLevel(haw::logger::Debug);

        LOGD() << "This is debug";
        qDebug() << "This is qDebug";

        /*
        15:34:54.257 | DEBUG | main_thread     | MYTAG      | example: This is debug
        15:34:54.257 | DEBUG | main_thread     | Qt         | example: This is qDebug
        */

        //! --- Setup logger ---
        //! Destination and format
        logger->clearDests();

        //! Console
        logger->addDest(new ConsoleLogDest(LogLayout("${time} | ${type|5} | ${thread} | ${tag|10} | ${message}")));

        std::string pwd = fs::current_path();
        //! File,this creates a file named "apppath/logs/myapp_yyMMdd.log"
        std::string logPath = pwd + "/logs";
        logger->addDest(new FileLogDest(logPath, "myapp", "log",
                                        LogLayout("${datetime} | ${type|5} | ${thread} | ${tag|10} | ${message}")));

        /** NOTE Layout have a tags
        "${datetime}"   - yyyy-MM-ddThh:mm:ss.zzz
        "${time}"       - hh:mm:ss.zzz
        "${type}"       - type
        "${tag}"        - tag
        "${thread}"     - thread, the main thread output as "main" otherwise hex
        "${message}"    - message
        |N - min field width
         */

        //! Level
        logger->setLevel(haw::logger::Debug);

        //! Catch Qt message
        logger->setIsCatchQtMsg(true);

        //! Custom types
        logger->setType("MYTRACE", true);

        //! Add to log.h
        #define MYTRACE() IF_LOGLEVEL(haw::logger::Debug) LOG("MYTRACE", LOG_TAG)

        MYTRACE() << "This my trace";

        /*
        15:34:54.257 | MYTRACE | main_thread     | MYTAG      | example: This my trace
        */

        //! That type does not output
        logger->setType("MYTRACE", false); //! NOTE Type must be a debug level

        MYTRACE() << "This my trace"; //! NOTE Not output

        //! Custom LogLayout - inherits of the LogLayout and override method "output"
        //! Custom LogDest - inherits of the LogDest and override method "write"
        //! Custom log macro - see log.h
    }
};

int main(int argc, char* argv[])
{
    std::clog << "Hello World, I am Logger\n";

    Example t;
    t.example();

#undef LOG_TAG
#define LOG_TAG CLASSNAME(FUNC_INFO)

    LOGI() << "Good bye!";
}
