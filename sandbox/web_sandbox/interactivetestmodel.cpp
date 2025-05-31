#include "interactivetestmodel.h"

#include <QApplication>

#include <emscripten.h>

#include "log.h"

InteractiveTestModel::InteractiveTestModel() {}

void InteractiveTestModel::openAsyncDialog()
{
    LOGDA() << "before open dialog";
    //interactive()->info(std::string("Title"), std::string("Info"));
    interactive()->open("muse://interactive/sample?sync=false");
    LOGDA() << "after open dialog";
}

void InteractiveTestModel::openSyncDialog()
{
    LOGDA() << "before open dialog";
    //interactive()->info(std::string("Title"), std::string("Info"));
    interactive()->open("muse://interactive/sample?sync=true");
    LOGDA() << "after open dialog";
}

QString InteractiveTestModel::runLoop()
{
    LOGDA() << "start loop";

    m_loop.exec();

    LOGDA() << "end loop";

    return QString("end loop");
}

void InteractiveTestModel::exitLoop()
{
    m_loop.quit();
}

QString InteractiveTestModel::runSleep()
{
    LOGDA() << "start sleep";
    m_running = true;
    while (m_running) {
        emscripten_sleep(100);
    }

    LOGDA() << "end sleep";

    return QString("end sleep");
}

void InteractiveTestModel::exitSleep()
{
    m_running = false;
}
