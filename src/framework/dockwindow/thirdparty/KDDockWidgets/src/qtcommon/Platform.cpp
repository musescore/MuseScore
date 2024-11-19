/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Platform.h"
#include "Window_p.h"
#include "kddockwidgets/KDDockWidgets.h"
#include "kddockwidgets/core/EventFilterInterface.h"
#include "core/Platform_p.h"
#include "core/Utils_p.h"
#include "core/Logging_p.h"
#include "core/DelayedCall_p.h"
#include "qtcommon/View.h"

#include <QWindow>
#include <QDebug>
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QScreen>
#include <QTimer>
#include <QFile>
#include <QOperatingSystemVersion>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtCommon;
using namespace KDDockWidgets::Core;

// clazy:excludeall=missing-qobject-macro

class Platform_qt::GlobalEventFilter : public QObject
{
public:
    explicit GlobalEventFilter(Platform_qt *qq)
        : q(qq)
    {
        if (qGuiApp) {
            qGuiApp->installEventFilter(this);
        } else {
            qWarning() << Q_FUNC_INFO << "Expected a qGuiApp!";
        }
    }

    bool eventFilter(QObject *o, QEvent *ev) override
    {
        if (ev->type() == QEvent::Expose)
            return handleExpose(o);
        else if (QMouseEvent *me = mouseEvent(ev))
            return handleMouseEvent(o, me);
        else if (isDnDEvent(ev))
            return handleDnDEvent(o, ev);
        else if (ev->type() == QEvent::Move)
            return handleMoveEvent(o, ev);
        else if (ev->type() != QEvent::Quit || m_isProcessingAppQuitEvent)
            return false;

        auto view = Platform_qt::instance()->qobjectAsView(o);
        if (!view)
            return false;

        m_isProcessingAppQuitEvent = true;
        qGuiApp->sendEvent(qApp, ev);
        m_isProcessingAppQuitEvent = false;
        return true;
    }

    bool handleMoveEvent(QObject *o, QEvent *)
    {
        if (q->d->m_globalEventFilters.empty())
            return false;

        auto view = Platform_qt::instance()->qobjectAsView(o);
        for (EventFilterInterface *filter : std::as_const(q->d->m_globalEventFilters)) {
            if (filter->onMoveEvent(view.get()))
                return true;
        }

        return false;
    }

    bool handleDnDEvent(QObject *o, QEvent *ev)
    {
        if (q->d->m_globalEventFilters.empty())
            return false;

        if (auto view = Platform_qt::instance()->qobjectAsView(o)) {
            for (EventFilterInterface *filter : std::as_const(q->d->m_globalEventFilters)) {
                if (filter->onDnDEvent(view.get(), ev))
                    return true;
            }
        }

        return false;
    }

    bool handleExpose(QObject *o)
    {
        if (q->d->m_globalEventFilters.empty())
            return false;

        auto window = Platform_qt::instance()->qobjectAsWindow(o);
        if (!window)
            return false;

        for (EventFilterInterface *filter : std::as_const(q->d->m_globalEventFilters)) {
            if (filter->enabled() && filter->onExposeEvent(window))
                return true;
        }

        return false;
    }

    bool handleMouseEvent(QObject *watched, QMouseEvent *ev)
    {
        if (q->d->m_globalEventFilters.empty())
            return false;

        auto view = Platform_qt::instance()->qobjectAsView(watched);

        // Make a copy, as there could be reentrancy and filters getting removed while event being
        // processed
        const auto &filters = std::as_const(q->d->m_globalEventFilters);

        for (EventFilterInterface *filter : filters) {
            // Filter might have been deleted meanwhile
            if (std::find(q->d->m_globalEventFilters.cbegin(), q->d->m_globalEventFilters.cend(),
                          filter)
                == q->d->m_globalEventFilters.cend())
                continue;

            if (!filter->enabled())
                continue;

            if (filter->onMouseEvent(view.get(), ev))
                return true;

            switch (ev->type()) {
            case QEvent::MouseButtonPress:
                if (filter->onMouseButtonPress(view.get(), ev))
                    return true;
                break;
            case QEvent::MouseButtonRelease:
                if (filter->onMouseButtonRelease(view.get(), ev))
                    return true;
                break;
            case QEvent::MouseMove:
                if (filter->onMouseButtonMove(view.get(), ev))
                    return true;
                break;
            case QEvent::MouseButtonDblClick:
                if (filter->onMouseDoubleClick(view.get(), ev))
                    return true;
                break;
            default:
                break;
            }
        }

        return false;
    }

    ~GlobalEventFilter() override;
    bool m_isProcessingAppQuitEvent = false;
    Platform_qt *const q;
};

Platform_qt::GlobalEventFilter::~GlobalEventFilter() = default;

Platform_qt::Platform_qt()
    : m_globalEventFilter(new GlobalEventFilter(this))
{
    if (!qGuiApp)
        qWarning() << "Please call KDDockWidgets::initPlatform() after QGuiApplication";
}

#ifdef DOCKS_DEVELOPER_MODE

// ctor used by the tests only
Platform_qt::Platform_qt(QCoreApplication *)
    : m_globalEventFilter(new GlobalEventFilter(this))
{
    // We want stability during tests.
    // QMainWindow uses the factor for its margins, we don't want tests failing due
    // to off by 1 or 2 pixels. Use 96dpi everywhere.
    View_qt::s_logicalDpiFactorOverride = 1;
}

#endif

Platform_qt::~Platform_qt()
{
    delete m_globalEventFilter;
}

std::shared_ptr<View> Platform_qt::focusedView() const
{
    return qobjectAsView(qGuiApp->focusObject());
}

Core::Window::List Platform_qt::windows() const
{
    Core::Window::List windows;
    const auto qtwindows = qGuiApp->topLevelWindows();
    windows.reserve(qtwindows.size());
    for (QWindow *qtwindow : qtwindows) {
        windows << windowFromQWindow(qtwindow);
    }

    return windows;
}

std::shared_ptr<Core::Window> Platform_qt::qobjectAsWindow(QObject *obj) const
{
    if (auto window = qobject_cast<QWindow *>(obj))
        return windowFromQWindow(window);
    return nullptr;
}

int Platform_qt::screenNumberForWindow(std::shared_ptr<Core::Window> window) const
{
    if (!window)
        return -1;

    return screenNumberForQWindow(static_cast<Window *>(window.get())->qtWindow());
}

int Platform_qt::screenNumberForQWindow(QWindow *window) const
{
    if (QScreen *screen = window->screen()) {
        return qGuiApp->screens().indexOf(screen);
    }

    return -1;
}

void Platform_qt::sendEvent(View *view, QEvent *ev) const
{
    qGuiApp->sendEvent(QtCommon::View_qt::asQObject(view), ev);
}

bool Platform_qt::isProcessingAppQuitEvent() const
{
    return m_globalEventFilter->m_isProcessingAppQuitEvent;
}

QString Platform_qt::applicationName() const
{
    return qGuiApp->applicationName();
}

void Platform_qt::setMouseCursor(Qt::CursorShape shape, bool discardLast)
{
    if (discardLast) {
        qGuiApp->changeOverrideCursor(shape);
    } else {
        qGuiApp->setOverrideCursor(shape);
    }
}

void Platform_qt::restoreMouseCursor()
{
    qGuiApp->restoreOverrideCursor();
}

QPoint Platform_qt::cursorPos() const
{
    return QCursor::pos();
}

void Platform_qt::setCursorPos(QPoint pos)
{
    QCursor::setPos(pos);
}

Core::Platform::DisplayType Platform_qt::displayType() const
{
    if (qGuiApp->platformName() == QLatin1String("wayland"))
        return DisplayType::Wayland;

    if (qGuiApp->platformName() == QLatin1String("offscreen"))
        return DisplayType::QtOffscreen;

    if (qGuiApp->platformName() == QLatin1String("xcb"))
        return DisplayType::X11;

    if (qGuiApp->platformName() == QLatin1String("eglfs"))
        return DisplayType::QtEGLFS;

    if (qGuiApp->platformName() == QLatin1String("windows"))
        return DisplayType::Windows;

    return DisplayType::Other;
}

bool Platform_qt::isLeftMouseButtonPressed() const
{
    return qGuiApp->mouseButtons() & Qt::LeftButton;
}

/*static*/
Platform_qt *Platform_qt::instance()
{
    return static_cast<Platform_qt *>(Platform::instance());
}

Screen::List Platform_qt::screens() const
{
    const auto qscreens = qGuiApp->screens();
    Screen::List screens;
    screens.reserve(qscreens.size());
    for (auto qscreen : qscreens) {
        screens.push_back(std::make_shared<Screen_qt>(qscreen));
    }
    return screens;
}

Screen::Ptr Platform_qt::primaryScreen() const
{
    return std::make_shared<Screen_qt>(qGuiApp->primaryScreen());
}

void Platform_qt::runDelayed(int ms, Core::DelayedCall *c)
{
    QTimer::singleShot(ms, qGuiApp, [c] { c->call(); delete c; });
}

QByteArray Platform_qt::readFile(const QString &fileName, bool &ok) const
{
    QFile f(fileName);
    ok = f.open(QIODevice::ReadOnly);
    if (!ok) {
        KDDW_ERROR("Failed to open {}, error={}", fileName, f.errorString());
        return {};
    }

    return f.readAll();
}

bool Platform_qt::supportsAeroSnap() const
{
#if defined(Q_OS_WIN)
    // Aero-snap requires Windows 10
    return QOperatingSystemVersion::current().majorVersion() >= 10;
#endif
    return false;
}
