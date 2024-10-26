/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Platform.h"
#include "kddockwidgets/KDDockWidgets.h"

#include "Window_p.h"
#include "DebugWindow.h"
#include "views/ViewWrapper_p.h"
#include "views/View.h"
#include "core/Platform_p.h"
#include "ViewFactory.h"

#include <QScreen>
#include <QApplication>
#include <QLineEdit>
#include <QAbstractButton>
#include <QStyleFactory>

#include <memory.h>

#if defined(KDDOCKWIDGETS_STATICLIB) || defined(QT_STATIC)
static void initResources()
{
    Q_INIT_RESOURCE(kddockwidgets_resources);
}
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtWidgets;

// clazy:excludeall=missing-qobject-macro

class Platform::GlobalEventFilter : public QObject
{
public:
    GlobalEventFilter()
    {
        if (qGuiApp)
            qGuiApp->installEventFilter(this);
    }

    bool eventFilter(QObject *o, QEvent *ev) override
    {
        // We're only interested in window activation events
        if (ev->type() != QEvent::WindowActivate && ev->type() != QEvent::WindowDeactivate)
            return false;

        // QWindow is not receiving it
        if (!o->isWidgetType())
            return false;

        if (auto w = qobject_cast<QWidget *>(o)) {
            if (w->isWindow()) {
                if (ev->type() == QEvent::WindowActivate) {
                    Platform::instance()->d->windowActivated.emit(
                        QtWidgets::ViewWrapper::create(w));
                }

                if (ev->type() == QEvent::WindowDeactivate) {
                    Platform::instance()->d->windowDeactivated.emit(
                        QtWidgets::ViewWrapper::create(w));
                }
            }
        }

        return false;
    }

    ~GlobalEventFilter() override;
};

Platform::GlobalEventFilter::~GlobalEventFilter() = default;

Platform::Platform()
    : m_globalEventFilter(new GlobalEventFilter())
{
    init();
}

void Platform::init()
{
#if defined(KDDOCKWIDGETS_STATICLIB) || defined(QT_STATIC)
    initResources();
#endif

#ifdef DOCKS_DEVELOPER_MODE
    if (qEnvironmentVariableIntValue("KDDOCKWIDGETS_SHOW_DEBUG_WINDOW") == 1) {
        auto dv = new Debug::DebugWindow();
        dv->show();
    }
#endif

    qGuiApp->connect(qApp, &QGuiApplication::focusObjectChanged, qApp, [this](QObject *obj) {
        d->focusedViewChanged.emit(QtWidgets::ViewWrapper::create(obj));
    });
}

Platform::~Platform()
{
    delete m_globalEventFilter;
}

const char *Platform::name() const
{
    return "qtwidgets";
}

bool Platform::hasActivePopup() const
{
    return qApp->activePopupWidget() != nullptr;
}

std::shared_ptr<Core::View> Platform::qobjectAsView(QObject *obj) const
{
    return QtWidgets::ViewWrapper::create(obj);
}

std::shared_ptr<Core::Window> Platform::windowFromQWindow(QWindow *qwindow) const
{
    Q_ASSERT(qwindow);
    return std::shared_ptr<Core::Window>(new Window(qwindow));
}

Core::ViewFactory *Platform::createDefaultViewFactory()
{
    return new ViewFactory();
}

Core::Window::Ptr Platform::windowAt(QPoint globalPos) const
{
    if (auto qwindow = qGuiApp->QGuiApplication::topLevelAt(globalPos)) {
        auto window = new Window(qwindow);
        return Core::Window::Ptr(window);
    }

    return {};
}

int Platform::screenNumberForView(Core::View *view) const
{
    if (auto widget = QtCommon::View_qt::asQWidget(view)) {
        if (QWindow *qtwindow = widget->window()->windowHandle())
            return screenNumberForQWindow(qtwindow);
    }

    return -1;
}

QSize Platform::screenSizeFor(Core::View *view) const
{
    if (auto widget = QtCommon::View_qt::asQWidget(view)) {
        if (QScreen *screen = widget->screen()) {
            return screen->size();
        }
    }

    return {};
}

int Platform::startDragDistance_impl() const
{
    return QApplication::startDragDistance();
}

Core::View *Platform::createView(Core::Controller *controller, Core::View *parent) const
{
    return new QtWidgets::View<QWidget>(controller, Core::ViewType::None,
                                        QtCommon::View_qt::asQWidget(parent));
}

bool Platform::usesFallbackMouseGrabber() const
{
    // For QtWidgets we just use QWidget::grabMouse()
    return false;
}

bool Platform::inDisallowedDragView(QPoint globalPos) const
{
    QWidget *widget = qApp->widgetAt(globalPos);
    if (!widget)
        return false;

    // User might have a line edit on the toolbar.
    // Not so elegant fix, we should make the user's tabbar implement some virtual method...
    return qobject_cast<QAbstractButton *>(widget) || qobject_cast<QLineEdit *>(widget);
}

void Platform::ungrabMouse()
{
    if (QWidget *grabber = QWidget::mouseGrabber())
        grabber->releaseMouse();
}

#ifdef DOCKS_TESTING_METHODS

inline QCoreApplication *createCoreApplication(int &argc, char **argv, bool defaultToOffscreenQPA)
{
    if (defaultToOffscreenQPA)
        QtCommon::Platform_qt::maybeSetOffscreenQPA(argc, argv);

    return new QApplication(argc, argv);
}

Platform::Platform(int &argc, char **argv, bool defaultToOffscreenQPA)
    : Platform_qt(createCoreApplication(argc, argv, defaultToOffscreenQPA))
    , m_globalEventFilter(new Platform::GlobalEventFilter())
{
    qputenv("KDDOCKWIDGETS_SHOW_DEBUG_WINDOW", "");
    qApp->setStyle(QStyleFactory::create(QStringLiteral("fusion")));
    init();
}

#endif
