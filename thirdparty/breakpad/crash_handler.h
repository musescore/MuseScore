#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#pragma once
#include <QtCore/QString>

#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QCoreApplication>
#include <QMutex>



#include "client/windows/handler/exception_handler.h"


namespace Breakpad {

    int AnnotateCrashReport(const QString& aKey, const QString& aData);
    int PrintMyCrashReport();



    class CrashHandlerPrivate;
    class CrashHandler
    {
    public:

        static CrashHandler* instance();

        void Init(const QString&  reportPath);
        void setReportCrashesToSystem(bool report);
        bool writeMinidump();




    private:
        CrashHandler();
        ~CrashHandler();
        Q_DISABLE_COPY(CrashHandler)
        CrashHandlerPrivate* d;
    };
}

#endif // CRASH_HANDLER_H
