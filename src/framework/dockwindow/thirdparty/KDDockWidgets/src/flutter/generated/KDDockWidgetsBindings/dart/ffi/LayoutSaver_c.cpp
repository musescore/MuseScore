/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/
#include "LayoutSaver_c.h"


#include <iostream>

#include <cassert>


namespace Dartagnan {

typedef int (*CleanupCallback)(void *thisPtr);
static CleanupCallback s_cleanupCallback = nullptr;

template<typename T>
struct ValueWrapper
{
    T value;
};

}
namespace KDDockWidgetsBindings_wrappersNS {
LayoutSaver_wrapper::LayoutSaver_wrapper()
    : ::KDDockWidgets::LayoutSaver()
{
}
bool LayoutSaver_wrapper::restoreFromFile(const QString &jsonFilename)
{
    return ::KDDockWidgets::LayoutSaver::restoreFromFile(jsonFilename);
}
bool LayoutSaver_wrapper::restoreInProgress()
{
    return ::KDDockWidgets::LayoutSaver::restoreInProgress();
}
bool LayoutSaver_wrapper::saveToFile(const QString &jsonFilename)
{
    return ::KDDockWidgets::LayoutSaver::saveToFile(jsonFilename);
}
LayoutSaver_wrapper::~LayoutSaver_wrapper()
{
}

}
static KDDockWidgets::LayoutSaver *fromPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgets::LayoutSaver *>(ptr);
}
static KDDockWidgetsBindings_wrappersNS::LayoutSaver_wrapper *fromWrapperPtr(void *ptr)
{
    return reinterpret_cast<KDDockWidgetsBindings_wrappersNS::LayoutSaver_wrapper *>(ptr);
}
extern "C" {
void c_KDDockWidgets__LayoutSaver_Finalizer(void *cppObj)
{
    delete reinterpret_cast<KDDockWidgetsBindings_wrappersNS::LayoutSaver_wrapper *>(cppObj);
}
void *c_KDDockWidgets__LayoutSaver__constructor()
{
    auto ptr = new KDDockWidgetsBindings_wrappersNS::LayoutSaver_wrapper();
    return reinterpret_cast<void *>(ptr);
}
// restoreFromFile(const QString & jsonFilename)
bool c_KDDockWidgets__LayoutSaver__restoreFromFile_QString(void *thisObj, const char *jsonFilename_)
{
    const auto jsonFilename = QString::fromUtf8(jsonFilename_);
    const auto &result = fromPtr(thisObj)->restoreFromFile(jsonFilename);
    free(( char * )jsonFilename_);
    return result;
}
// restoreInProgress()
bool c_static_KDDockWidgets__LayoutSaver__restoreInProgress()
{
    const auto &result = KDDockWidgetsBindings_wrappersNS::LayoutSaver_wrapper::restoreInProgress();
    return result;
}
// saveToFile(const QString & jsonFilename)
bool c_KDDockWidgets__LayoutSaver__saveToFile_QString(void *thisObj, const char *jsonFilename_)
{
    const auto jsonFilename = QString::fromUtf8(jsonFilename_);
    const auto &result = fromPtr(thisObj)->saveToFile(jsonFilename);
    free(( char * )jsonFilename_);
    return result;
}
void c_KDDockWidgets__LayoutSaver__destructor(void *thisObj)
{
    delete fromPtr(thisObj);
}
}
