#include "crash_handler.h"




namespace Breakpad {



    /************************************************************************/
    /* CrashHandlerPrivate                                                  */
    /************************************************************************/
    class CrashHandlerPrivate
    {
    public:
        CrashHandlerPrivate()
        {
            pHandler = NULL;
        }

        ~CrashHandlerPrivate()
        {
            delete pHandler;
        }

        void InitCrashHandler(const QString& dumpPath);
        static google_breakpad::ExceptionHandler* pHandler;
        static bool bReportCrashesToSystem;
    };

    google_breakpad::ExceptionHandler* CrashHandlerPrivate::pHandler = NULL;
    bool CrashHandlerPrivate::bReportCrashesToSystem = false;

    QHash<QString, QString> crashTable;
    QMutex mymutex;

    int AnnotateCrashReport(const QString& aKey, const QString& aData){
        mymutex.lock();
        crashTable.insert(aKey,aData);
        mymutex.unlock();
        return 0;
    }

    int PrintMyCrashReport(){
        QHashIterator<QString, QString> i(crashTable);
        while (i.hasNext()) {
            i.next();
            qDebug("%s : %s", qUtf8Printable(i.key()),qUtf8Printable(i.value()) );
        }
        return 0;
    }


    /************************************************************************/
    /* DumpCallback                                                         */
    /************************************************************************/

    bool DumpCallback(const wchar_t* _dump_dir,const wchar_t* _minidump_id,void* context,EXCEPTION_POINTERS* exinfo,MDRawAssertionInfo* assertion,bool success)
    {
        Q_UNUSED(_dump_dir);
        Q_UNUSED(_minidump_id);
        Q_UNUSED(assertion);
        Q_UNUSED(exinfo);

        qDebug("BreakpadQt crash");

        PrintMyCrashReport();

        /*
        NO STACK USE, NO HEAP USE THERE !!!
        Creating QString's, using qDebug, etc. - everything is crash-unfriendly.
        */
        return CrashHandlerPrivate::bReportCrashesToSystem ? success : true;
    }

    void CrashHandlerPrivate::InitCrashHandler(const QString& dumpPath)
    {
        if ( pHandler != NULL )
            return;
        std::wstring pathAsStr = (const wchar_t*)dumpPath.utf16();
        pHandler = new google_breakpad::ExceptionHandler(
            pathAsStr,
            /*FilterCallback*/ 0,
            DumpCallback,
            /*context*/
            0,
            true
            );

    }

    /************************************************************************/
    /* CrashHandler                                                         */
    /************************************************************************/
    CrashHandler* CrashHandler::instance()
    {
        static CrashHandler globalHandler;
        return &globalHandler;
    }

    CrashHandler::CrashHandler()
    {
        d = new CrashHandlerPrivate();
    }

    CrashHandler::~CrashHandler()
    {
        delete d;
    }

    void CrashHandler::setReportCrashesToSystem(bool report)
    {
        d->bReportCrashesToSystem = report;
    }

    bool CrashHandler::writeMinidump()
    {
        bool res = d->pHandler->WriteMinidump();
        if (res) {
            qDebug("BreakpadQt: writeMinidump() success.");
        } else {
            qWarning("BreakpadQt: writeMinidump() failed.");
        }
        return res;
    }

    void CrashHandler::Init( const QString& reportPath )
    {
        d->InitCrashHandler(reportPath);
    }
}
