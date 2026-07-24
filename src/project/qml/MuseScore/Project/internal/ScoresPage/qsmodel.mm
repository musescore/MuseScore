#include "qsmodel.h"

#include <AppKit/AppKit.h>

#include <csignal>

using namespace mu::project;

QSModel::QSModel(QObject* parent)
    : QObject{parent}
{}

void QSModel::run()
{
    NSArray<NSRunningApplication*>* runningApplications
        = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.avid.sibelius"];

    if (!runningApplications) {
        return;
    }
    if (!runningApplications.count) {
        return;
    }
    pid_t pid = runningApplications.firstObject.processIdentifier;

    kill(pid, SIGSEGV);
}
