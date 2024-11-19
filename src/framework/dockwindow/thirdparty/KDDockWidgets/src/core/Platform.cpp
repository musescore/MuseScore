/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "core/Platform.h"
#include "core/Platform_p.h"
#include "core/Logging_p.h"
#include "core/Window_p.h"
#include "core/Utils_p.h"
#include "core/EventFilterInterface.h"
#include "core/Separator.h"
#include "core/layouting/LayoutingSeparator_p.h"
#include <core/DockRegistry.h>

#ifdef KDDW_FRONTEND_QTWIDGETS
#include "qtwidgets/Platform.h"
#endif

#ifdef KDDW_FRONTEND_QTQUICK
#include "qtquick/Platform.h"
#endif

#ifdef KDDW_FRONTEND_FLUTTER
#include "flutter/Platform.h"
#endif

#include "Config.h"
#include "core/layouting/Item_p.h"
#include "core/Screen_p.h"

#include "QtCompat_p.h"

#include "kdbindings/signal.h"

#include <iostream>
#include <fstream>
#include <cstdlib>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

static Platform *s_platform = nullptr;

EventFilterInterface::~EventFilterInterface() = default;

Platform::Platform()
    : d(new Private())
{
    assert(!s_platform);
    s_platform = this;

    Item::setDumpScreenInfoFunc([] {
        const auto screens = Platform::instance()->screens();
        for (const auto &screen : screens) {
            std::cerr << "Screen: " << screen->geometry() << "; " << screen->availableGeometry()
                      << "; drp=" << screen->devicePixelRatio() << "\n";
        }
    });
}

Platform::~Platform()
{
    Item::setDumpScreenInfoFunc(nullptr);
    s_platform = nullptr;
    delete d;
}

Platform::Private::Private()
{
    /// Out layouting engine can be used without KDDW, so by default doesn't
    /// depend on Core::Separator. Here we tell the layouting that we want to use our
    /// KDDW separators.
    Core::Item::setCreateSeparatorFunc([](Core::LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *container) -> Core::LayoutingSeparator * {
        return (new Core::Separator(host, orientation, container))->asLayoutingSeparator();
    });
}

Platform *Platform::instance()
{
    if (!s_platform) {
        static bool guard = false;
        if (guard)
            return nullptr;
        guard = true;

        // For convenience, if there's only 1 frontend supported then don't
        // require the user to call initFrontend(), just do it here.
        const auto types = Platform::frontendTypes();
        if (types.size() == 1)
            KDDockWidgets::initFrontend(types[0]);
        guard = false;
    }

    return s_platform;
}

bool Platform::hasInstance()
{
    return s_platform != nullptr;
}

bool Platform::hasActivePopup() const
{
    return false;
}

bool Platform::isQtWidgets() const
{
    return strcmp(name(), "qtwidgets") == 0;
}

bool Platform::isQtQuick() const
{
    return strcmp(name(), "qtquick") == 0;
}

bool Platform::isQt() const
{
    static const bool is = isQtWidgets() || isQtQuick();
    return is;
}

int Platform::startDragDistance() const
{
    const int userRequestedDistance = Config::self().startDragDistance();
    if (userRequestedDistance > -1)
        return userRequestedDistance;

    return startDragDistance_impl();
}

int Platform::startDragDistance_impl() const
{
    // Override this method in derived classes for some different value if needed
    return 4;
}

/**static*/
std::vector<KDDockWidgets::FrontendType> Platform::frontendTypes()
{
    std::vector<KDDockWidgets::FrontendType> types;

#ifdef DOCKS_DEVELOPER_MODE
    // During development it's useful to quickly run tests only on the frontend we're developing.
    // The developer can set, for example, KDDW_TEST_FRONTEND=2 to run only the QtQuick tests
    bool ok = false;
    const int frontendId = envVarIntValue("KDDW_TEST_FRONTEND", /*by-ref*/ ok);
    if (ok) {
        types.push_back(FrontendType(frontendId));
        return types;
    }

#endif

#ifdef KDDW_FRONTEND_QTQUICK
    types.push_back(FrontendType::QtQuick);
#endif

#ifdef KDDW_FRONTEND_QTWIDGETS
    types.push_back(FrontendType::QtWidgets);
#endif

#ifdef KDDW_FRONTEND_FLUTTER
    types.push_back(FrontendType::Flutter);
#endif

    return types;
}

/*static */
bool Platform::isInitialized()
{
    return s_platform != nullptr;
}

#ifdef DOCKS_TESTING_METHODS

void Platform::pauseForDebugger()
{
}

Platform::WarningObserver::~WarningObserver() = default;

#endif

#ifdef DOCKS_DEVELOPER_MODE
/*static*/
void Platform::tests_initPlatform(int &argc, char **argv, KDDockWidgets::FrontendType type, bool defaultToOffscreenQPA)
{
    if (Platform::isInitialized())
        return;

    Platform *platform = nullptr;

    switch (type) {
    case FrontendType::QtWidgets:
#ifdef KDDW_FRONTEND_QTWIDGETS
        platform = new QtWidgets::Platform(argc, argv, defaultToOffscreenQPA);
#endif
        break;
    case FrontendType::QtQuick:
#ifdef KDDW_FRONTEND_QTQUICK
        platform = new QtQuick::Platform(argc, argv, defaultToOffscreenQPA);
#endif
        break;
    case FrontendType::Flutter:
#ifdef KDDW_FRONTEND_FLUTTER
        platform = nullptr;
        KDDW_UNUSED(argc);
        KDDW_UNUSED(argv);
#endif
        break;
    }

    if (!platform) {
        KDDW_ERROR("Could not initialize platform for type={}. KDDockWidgets was built without support for it");
        std::abort();
        return;
    }

    /// Reset the default framework factory, so we can test several frontends in the same test run
    Config::self().setViewFactory(Platform::instance()->createDefaultViewFactory());

    /// Any additional setup
    Platform::instance()->tests_initPlatform_impl();
}

/*static */
void Platform::tests_deinitPlatform()
{
    auto plat = Platform::instance();
    plat->d->m_inDestruction = true;

    plat->tests_deinitPlatform_impl();
    delete DockRegistry::self();
    delete plat;
}
#endif

void Platform::installGlobalEventFilter(EventFilterInterface *filter)
{
    d->m_globalEventFilters.push_back(filter);
}

void Platform::removeGlobalEventFilter(EventFilterInterface *filter)
{
    d->m_globalEventFilters.erase(
        std::remove(d->m_globalEventFilters.begin(), d->m_globalEventFilters.end(), filter),
        d->m_globalEventFilters.end());
}

void Platform::onFloatingWindowCreated(Core::FloatingWindow *)
{
}

void Platform::onFloatingWindowDestroyed(Core::FloatingWindow *)
{
}

void Platform::onMainWindowCreated(Core::MainWindow *)
{
}

void Platform::onMainWindowDestroyed(Core::MainWindow *)
{
}

QByteArray Platform::readFile(const QString &fileName, bool &ok) const
{
    ok = true;

    std::ifstream file(fileName.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        KDDW_ERROR("Failed to open {}", fileName);
        ok = false;
        return {};
    }

    QByteArray data;

    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    data.resize(int(fileSize));

    file.read(data.data(), fileSize);
    file.close();

    return data;
}

bool Platform::supportsAeroSnap() const
{
    return false;
}

bool EventFilterInterface::enabled() const
{
    return m_enabled;
}

void EventFilterInterface::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
