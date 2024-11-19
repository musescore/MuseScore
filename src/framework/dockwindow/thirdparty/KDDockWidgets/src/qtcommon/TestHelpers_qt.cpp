/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Platform.h"
#include "qtcommon/Window_p.h"
#include "qtcommon/View.h"
#include "core/ViewGuard.h"
#include "core/Utils_p.h"

#include <QString>
#include <QTimer>
#include <QDebug>
#include <QEventLoop>
#include <QWindow>
#include <QGuiApplication>
#include <QElapsedTimer>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtCommon;

namespace KDDockWidgets::Tests {

static QtMessageHandler s_original = nullptr;

static bool shouldBlacklistWarning(const QString &msg, const QString &category)
{
    if (category == QLatin1String("qt.qpa.xcb"))
        return true;

    return msg.contains(QLatin1String("QSocketNotifier: Invalid socket"))
        || msg.contains(QLatin1String("QWindowsWindow::setGeometry"))
        || msg.contains(QLatin1String("This plugin does not support"))
        || msg.contains(QLatin1String("Note that Qt no longer ships fonts"))
        || msg.contains(QLatin1String("Another dock KDDockWidgets::DockWidget"))
        || msg.contains(
            QLatin1String("There's multiple MainWindows, not sure what to do about parenting"))
        || msg.contains(QLatin1String("Testing::"))
        || msg.contains(QLatin1String("outside any known screen, using primary screen"))
        || msg.contains(QLatin1String("Populating font family aliases took"))
        || msg.contains(QLatin1String("QSGThreadedRenderLoop: expose event received for window"))
        || msg.contains(QLatin1String("Group.qml:0: ReferenceError: parent is not defined"))

        // Ignore benign warning in Material style when deleting a dock widget. Should be fixed in
        // Qt.
        || (msg.contains(QLatin1String("TypeError: Cannot read property"))
            && msg.contains(QLatin1String("Material")));
}

static void fatalWarningsMessageHandler(QtMsgType t, const QMessageLogContext &context,
                                        const QString &msg)
{
    if (shouldBlacklistWarning(msg, QLatin1String(context.category)))
        return;
    s_original(t, context, msg);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // There's a ton of benign warnings in Qt5 QML when app exits, which add maintenance burden.
    // Please, jump to Qt 6
    if (Core::Platform::instance()->isQtQuick())
        return;
#endif

    if (t == QtWarningMsg) {
        const std::string expectedWarning = Core::Platform::instance()->m_expectedWarning;
        if (!expectedWarning.empty() && msg.contains(QString::fromStdString(expectedWarning)))
            return;

        if (!Platform_qt::isGammaray() && !qEnvironmentVariableIsSet("NO_FATAL")) {
            std::terminate();
        }
    }
}

static void sleepWithEventLoop(int ms)
{
    if (!ms)
        return;

    QEventLoop loop;
    QTimer::singleShot(ms, &loop, [&loop] {
        loop.exit();
    });
    loop.exec();
}

template<typename Func>
static bool waitFor(Func func, int timeout)
{

    QElapsedTimer timer;
    timer.start();

    while (!func()) {
        if (timer.elapsed() >= timeout)
            return false;

        sleepWithEventLoop(100);
    }

    return true;
}


/// @brief Helper class to help us with tests
class EventFilter : public QObject
{
public:
    EventFilter(QEvent::Type type)
        : m_type(type)
    {
    }
    ~EventFilter() override;
    bool eventFilter(QObject *, QEvent *e) override
    {
        if (e->type() == m_type)
            m_got = true;

        return false;
    }

    const QEvent::Type m_type;
    bool m_got = false;
};

EventFilter::~EventFilter() = default;

}

bool Platform_qt::tests_waitForWindowActive(Core::Window::Ptr window, int timeout) const
{
    Q_ASSERT(window);
    auto windowqt = static_cast<Window *>(window.get());
    QWindow *qwindow = windowqt->qtWindow();

    return Tests::waitFor([qwindow] {
        return qwindow && qwindow->isActive();
    },
                          timeout);
}

bool Platform_qt::tests_waitForEvent(Core::Object *w, QEvent::Type type, int timeout) const
{
    Tests::EventFilter filter(type);
    w->installEventFilter(&filter);
    QElapsedTimer time;
    time.start();

    while (!filter.m_got && time.elapsed() < timeout) {
        qGuiApp->processEvents();
        Tests::sleepWithEventLoop(50);
    }

    return filter.m_got;
}

bool Platform_qt::tests_waitForEvent(Core::View *view, QEvent::Type type, int timeout) const
{
    return tests_waitForEvent(QtCommon::View_qt::asQObject(view), type, timeout);
}

bool Platform_qt::tests_waitForResize(Core::View *view, int timeout) const
{
    return tests_waitForEvent(QtCommon::View_qt::asQObject(view), QEvent::Resize, timeout);
}

bool Platform_qt::tests_waitForResize(Core::Controller *c, int timeout) const
{
    return tests_waitForResize(c->view(), timeout);
}

bool Platform_qt::tests_waitForEvent(std::shared_ptr<Core::Window> window, QEvent::Type type,
                                     int timeout) const
{
    auto windowqt = static_cast<Window *>(window.get());
    return tests_waitForEvent(windowqt->qtWindow(), type, timeout);
}

bool Platform_qt::tests_waitForDeleted(Core::View *view, int timeout) const
{
    QObject *o = view ? QtCommon::View_qt::asQObject(view) : nullptr;
    if (!o)
        return true;

    QPointer<QObject> ptr = o;
    QElapsedTimer time;
    time.start();

    while (ptr && time.elapsed() < timeout) {
        qGuiApp->processEvents();
        Tests::sleepWithEventLoop(50);
    }

    const bool wasDeleted = !ptr;
    return wasDeleted;
}


bool Platform_qt::tests_waitForDeleted(Core::Controller *o, int timeout) const
{
    if (!o)
        return true;

    QPointer<QObject> ptr = o;
    QElapsedTimer time;
    time.start();

    while (ptr && time.elapsed() < timeout) {
        qGuiApp->processEvents();
        Tests::sleepWithEventLoop(50);
    }

    const bool wasDeleted = !ptr;
    return wasDeleted;
}

void Platform_qt::tests_doubleClickOn(QPoint globalPos, Core::View *receiver)
{
    QCursor::setPos(globalPos);
    tests_pressOn(globalPos, receiver); // double-click involves an initial press

    MouseEvent ev(Event::MouseButtonDblClick, receiver->mapFromGlobal(globalPos),
                  receiver->rootView()->mapFromGlobal(globalPos), globalPos, Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);

    if (Platform::instance()->isQtQuick()) {
        // QtQuick case, we need to send the event to the mouse area
        auto actualReceiver = static_cast<KDDockWidgets::QtCommon::View_qt *>(receiver)->viewProperty("titleBarMouseArea").value<QObject *>();
        qGuiApp->sendEvent(actualReceiver, &ev);
    } else {
        // QtWidgets case
        Platform::instance()->sendEvent(receiver, &ev);
    }
}

void Platform_qt::installMessageHandler()
{
    Tests::s_original = qInstallMessageHandler(Tests::fatalWarningsMessageHandler);
}

void Platform_qt::uninstallMessageHandler()
{
    if (!Tests::s_original)
        qWarning() << Q_FUNC_INFO
                   << "No message handler was installed or the fatalWarningsMessageHandler was "
                      "already uninstalled!";
    qInstallMessageHandler(Tests::s_original);
    Tests::s_original = nullptr;
}

void Platform_qt::tests_initPlatform_impl()
{
    qGuiApp->setOrganizationName(QStringLiteral("KDAB"));
    qGuiApp->setApplicationName(QStringLiteral("dockwidgets-unit-tests"));
}

void Platform_qt::tests_deinitPlatform_impl()
{
    delete qGuiApp;
}

/*static*/
QT_BEGIN_NAMESPACE
extern quintptr Q_CORE_EXPORT qtHookData[];
QT_END_NAMESPACE
bool Platform_qt::isGammaray()
{
    static bool is = qtHookData[3] != 0;
    return is;
}

bool Platform_qt::tests_wait(int ms) const
{
    Tests::sleepWithEventLoop(ms);
    return true;
}

void Platform_qt::maybeSetOffscreenQPA(int argc, char **argv)
{
    bool qpaPassed = false;

    for (int i = 1; i < argc; ++i) {
        if (qstrcmp(argv[i], "-platform") == 0) {
            qpaPassed = true;
            break;
        }
    }

    if (!qpaPassed && !qEnvironmentVariableIsSet("KDDW_NO_OFFSCREEN")) {
        // Use offscreen by default as it's less annoying, doesn't create visible windows
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
}

void Platform_qt::tests_pressOn(QPoint globalPos, Core::View *receiver)
{
    setCursorPos(globalPos);
    MouseEvent ev(Event::MouseButtonPress, receiver->mapFromGlobal(globalPos),
                  receiver->rootView()->mapFromGlobal(globalPos), globalPos, Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);
    sendEvent(receiver, &ev);
}

void Platform_qt::tests_pressOn(QPoint globalPos, std::shared_ptr<Core::Window> receiver)
{
    Platform::instance()->setCursorPos(globalPos);
    MouseEvent ev(Event::MouseButtonPress, receiver->mapFromGlobal(globalPos),
                  receiver->mapFromGlobal(globalPos), globalPos, Qt::LeftButton, Qt::LeftButton,
                  Qt::NoModifier);
    qGuiApp->sendEvent(static_cast<Window *>(receiver.get())->qtWindow(), &ev);
}

bool Platform_qt::tests_releaseOn(QPoint globalPos, Core::View *receiver)
{
    MouseEvent ev(Event::MouseButtonRelease, receiver->mapFromGlobal(globalPos),
                  receiver->rootView()->mapFromGlobal(globalPos), globalPos, Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);
    Platform::instance()->sendEvent(receiver, &ev);

    return true;
}

void Platform_qt::tests_doubleClickOn(QPoint globalPos, std::shared_ptr<Core::Window> receiver)
{
    Platform::instance()->setCursorPos(globalPos);
    MouseEvent ev(Event::MouseButtonDblClick, receiver->mapFromGlobal(globalPos),
                  receiver->mapFromGlobal(globalPos), globalPos, Qt::LeftButton, Qt::LeftButton,
                  Qt::NoModifier);

    tests_pressOn(globalPos, receiver); // double-click involves an initial press
    qGuiApp->sendEvent(static_cast<Window *>(receiver.get())->qtWindow(), &ev);
}

bool Platform_qt::tests_mouseMove(QPoint globalPos, Core::View *receiver)
{
    Core::ViewGuard receiverP = receiver;
    Platform::instance()->setCursorPos(globalPos);
    MouseEvent ev(Event::MouseMove, receiver->mapFromGlobal(globalPos),
                  receiver->rootView()->mapFromGlobal(globalPos), globalPos, Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);

    if (!receiverP) {
        qWarning() << "Receiver was deleted";
        return false;
    }
    Platform::instance()->sendEvent(receiver, &ev);
    Platform::instance()->tests_wait(2);

    return true;
}
