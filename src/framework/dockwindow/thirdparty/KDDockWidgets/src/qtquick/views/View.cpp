/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "View.h"
#include "core/Utils_p.h"
#include "core/View_p.h"
#include "ViewWrapper_p.h"
#include "core/layouting/Item_p.h"
#include "core/ScopedValueRollback_p.h"
#include "qtquick/Window_p.h"
#include "qtquick/Platform.h"
#include "core/Group.h"

#include <QtQuick/private/qquickitem_p.h>
#include <qpa/qplatformwindow.h>
#include <QtGui/private/qhighdpiscaling_p.h>
#include <QGuiApplication>
#include <QFile>
#include <QQmlContext>

using namespace KDDockWidgets;
using namespace KDDockWidgets::QtQuick;

namespace KDDockWidgets::QtQuick {

/**
 * @brief Event filter which redirects mouse events from one QObject to another.
 * Needed for QtQuick to redirect the events from MouseArea to our KDDW classes which derive from
 * Draggable. For QtWidgets it's not needed, as the Draggables are QWidgets themselves.
 */
class MouseEventRedirector : public QObject
{
    Q_OBJECT
public:
    explicit MouseEventRedirector(QQuickItem *eventSource, View *eventTarget)
        : QObject(eventTarget)
        , m_eventSource(eventSource)
        , m_eventTarget(eventTarget)
    {
        eventSource->installEventFilter(this);

        // Each source can only have one MouseEventRedirector
        auto oldRedirector = s_mouseEventRedirectors.take(eventSource);
        if (oldRedirector) {
            eventSource->removeEventFilter(oldRedirector);
            oldRedirector->deleteLater();
        }

        s_mouseEventRedirectors.insert(eventSource, this);
    }

    static MouseEventRedirector *redirectorForSource(QObject *eventSource)
    {
        return s_mouseEventRedirectors.value(eventSource);
    }

    ~MouseEventRedirector() override;

    bool eventFilter(QObject *source, QEvent *ev) override
    {
        if (QHoverEvent *hev = hoverEvent(ev)) {
            /// Don't block the hover events. In case users want to style tabs differently in QML
            m_eventTarget->onHoverEvent(
                hev, m_eventSource->mapToGlobal(Qt5Qt6Compat::eventPos(hev)).toPoint());
            return false;
        }

        QMouseEvent *me = mouseEvent(ev);
        if (!me)
            return false;

        // MouseArea.enable is different from Item.enabled. The former still lets the events
        // go through event loops. So query MouseArea.enable here and bail out if false.
        const QVariant v = source->property("enabled");
        if (v.isValid() && !v.toBool())
            return false;

        // Finally send the event
        m_eventTarget->setProperty("cursorPosition", m_eventSource->property("cursorPosition"));
        QPointer<QObject> lifeGuard(this);
        qGuiApp->sendEvent(m_eventTarget, me);

        if (!lifeGuard) {
            // m_eventTarget was deleted, and so was "this"
            return true;
        }

        m_eventTarget->setProperty("cursorPosition", CursorPosition_Undefined);

        return false;
    }

    QQuickItem *const m_eventSource;
    View *const m_eventTarget;
    static QHash<QObject *, MouseEventRedirector *> s_mouseEventRedirectors;
};

QHash<QObject *, MouseEventRedirector *> MouseEventRedirector::s_mouseEventRedirectors = {};

MouseEventRedirector::~MouseEventRedirector()
{
    s_mouseEventRedirectors.remove(m_eventSource);
}

}

static bool flagsAreTopLevelFlags(Qt::WindowFlags flags)
{
    return flags & (Qt::Window | Qt::Tool);
}

static QQuickItem *actualParentItem(QQuickItem *candidateParentItem, Qt::WindowFlags flags)
{
    // When we have a top-level, such as FloatingWindow, we only want to set QObject parentship
    // and not parentItem.
    return flagsAreTopLevelFlags(flags) ? nullptr : candidateParentItem;
}

View::View(Core::Controller *controller, Core::ViewType type, QQuickItem *parent,
           Qt::WindowFlags flags)
    : QQuickItem(actualParentItem(parent, flags))
    , View_qt(controller, type, this)
    , m_windowFlags(flags)
{
    if (parent && flagsAreTopLevelFlags(flags)) {
        // See comment in actualParentItem(). We set only the QObject parent. Mimics QWidget
        // behaviour
        QObject::setParent(parent);
    }

    connect(this, &QQuickItem::widthChanged, this, [this] {
        if (!Core::View::d->aboutToBeDestroyed() && !m_inDtor) { // If Window is being destroyed we don't bother
            onResize(Core::View::size());
            updateGeometry();
        }
    });

    connect(this, &QQuickItem::heightChanged, this, [this] {
        if (!Core::View::d->aboutToBeDestroyed() && !m_inDtor) { // If Window is being destroyed we don't bother
            onResize(Core::View::size());
            updateGeometry();
        }
    });

    _setSize({ 800, 800 });
}

void View::setGeometry(QRect rect)
{
    setSize(rect.width(), rect.height());
    Core::View::move(rect.topLeft());
}

QQuickItem *View::createItem(QQmlEngine *engine, const QString &filename, QQmlContext *context)
{
    QQmlComponent component(engine, filename);
    QObject *obj = component.create(context);
    if (!obj) {
        qWarning() << Q_FUNC_INFO << component.errorString();
        return nullptr;
    }

    return qobject_cast<QQuickItem *>(obj);
}

void View::redirectMouseEvents(QQuickItem *source)
{
    if (auto existingRedirector = MouseEventRedirector::redirectorForSource(source)) {
        if (existingRedirector->m_eventTarget == this) {
            // Nothing to do. The specified event source is already redirecting to this instance
            return;
        }
    }

    new MouseEventRedirector(source, this);
}

void View::sendVisibleChangeEvent()
{
    if (Core::View::d->freed())
        return;

    if (m_inSetParent) {
        // Setting parent to nullptr will emit visible true in QtQuick
        // which we don't want, as we're going to hide it (as we do with QtWidgets)
        return;
    }

    QEvent ev(isVisible() ? QEvent::Show : QEvent::Hide);
    event(&ev);
}

void View::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &data)
{
    if (m_inDtor)
        return;

    QQuickItem::itemChange(change, data);

    // Emulate the QWidget behaviour as QQuickItem doesn't receive some QEvents.
    if (change == QQuickItem::ItemVisibleHasChanged)
        sendVisibleChangeEvent();
}

void View::updateNormalGeometry()
{
    QWindow *window = QQuickItem::window();
    if (!window) {
        return;
    }

    QRect normalGeometry;
    if (const QPlatformWindow *pw = window->handle()) {
        normalGeometry = QHighDpi::fromNativePixels(pw->normalGeometry(), pw->window());
    }

    if (!normalGeometry.isValid() && isNormalWindowState(WindowState(window->windowState()))) {
        normalGeometry = window->geometry();
    }

    if (normalGeometry.isValid()) {
        setNormalGeometry(normalGeometry);
    }
}

void View::move(int x, int y)
{
    if (isRootView()) {
        if (QWindow *w = QQuickItem::window()) {
            w->setPosition(x, y);
            return;
        }
    }

    QQuickItem::setX(x);
    QQuickItem::setY(y);
    enableAttribute(Qt::WA_Moved);
}

bool View::event(QEvent *ev)
{
    if (ev->type() == QEvent::Close)
        Core::View::d->requestClose(static_cast<QCloseEvent *>(ev));

    return QQuickItem::event(ev);
}

bool View::eventFilter(QObject *watched, QEvent *ev)
{
    if (qobject_cast<QWindow *>(watched)) {
        if (m_mouseTrackingEnabled) {
            switch (ev->type()) {
            case QEvent::MouseMove:
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
                ev->ignore();
                qGuiApp->sendEvent(this, ev);
                // qDebug() << "Mouse event" << ev;
                if (ev->isAccepted())
                    return true;
                break;
            default:
                break;
            }
        }

        if (ev->type() == QEvent::Resize || ev->type() == QEvent::Move) {
            updateNormalGeometry();
        } else if (ev->type() == QEvent::WindowStateChange) {
            onWindowStateChangeEvent(static_cast<QWindowStateChangeEvent *>(ev));
        }
    }

    return QQuickItem::eventFilter(watched, ev);
}

bool View::close(QQuickItem *item)
{
    if (auto viewqtquick = qobject_cast<View *>(item)) {
        QCloseEvent ev;
        viewqtquick->Core::View::d->closeRequested.emit(&ev);

        if (ev.isAccepted()) {
            viewqtquick->setVisible(false);
            return true;
        }
    }

    return false;
}

bool View::close()
{
    return close(this);
}

void View::QQUICKITEMgeometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    // Send a few events manually, since QQuickItem doesn't do it for us.
    QQuickItem::QQUICKITEMgeometryChanged(newGeometry, oldGeometry);

    // Not calling event() directly, otherwise it would skip event filters

    if (newGeometry.size() != oldGeometry.size()) {
        QEvent ev(QEvent::Resize);
        qGuiApp->sendEvent(this, &ev);
    }

    if (newGeometry.topLeft() != oldGeometry.topLeft()) {
        QEvent ev(QEvent::Move);
        qGuiApp->sendEvent(this, &ev);
    }

    Q_EMIT itemGeometryChanged();
}

bool View::isVisible() const
{
    if (QWindow *w = QQuickItem::window()) {
        if (!w->isVisible())
            return false;
    }

    // QQuickItemPrivate::explicitVisible is the actual visible state independently from the parent
    // being visible or not. This is what we want to save with LayoutSaver.
    // For now, we have no use case to know what is the effectiveVisible state. If we need it, then we'll
    // need to have more API for this.

    auto priv = QQuickItemPrivate::get(this);
    return priv->explicitVisible;
}

void View::setVisible(bool is)
{
    if (isRootView()) {
        if (QWindow *w = QQuickItem::window()) {
            if (is && !w->isVisible()) {
                w->show();
            } else if (!is && w->isVisible()) {
                w->hide();
            }
        }
    }

    QQuickItem::setVisible(is);

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // Workaround QTBUG-112838, QQuickItem::itemChanged() isn't called anymore if there's no parent
    if (is && !parentItem() && !QQuickItem::isVisible()) {
        sendVisibleChangeEvent();
    }
#endif
}

bool View::isExplicitlyHidden() const
{
    auto priv = QQuickItemPrivate::get(this);
    return !priv->explicitVisible;
}

void View::setSize(int w, int h)
{
    const auto newSize = QSize(w, h).expandedTo(minSize());
    _setSize(newSize);
}

void View::_setSize(QSize newSize)
{
    // Like setSize() but does not honour minSize()

    if (isRootView()) {
        if (QWindow *window = QQuickItem::window()) {

            if (window->size() != newSize) {
                QRect windowGeo = window->geometry();
                windowGeo.setSize(newSize);
                window->setGeometry(windowGeo);
            }
        }
    }

    QQuickItem::setSize(newSize);
}

std::shared_ptr<Core::View> View::rootView() const
{
    if (Core::Window::Ptr window = View::window())
        return window->rootView();

    auto thisNonConst = const_cast<View *>(this);
    return thisNonConst->asWrapper();
}

void View::makeItemFillParent(QQuickItem *item)
{
    if (!item) {
        qWarning() << Q_FUNC_INFO << "Invalid item";
        return;
    }

    QQuickItem *parentItem = item->parentItem();
    if (!parentItem) {
        qWarning() << Q_FUNC_INFO << "Invalid parentItem for" << item;
        return;
    }

    QObject *anchors = item->property("anchors").value<QObject *>();
    if (!anchors) {
        qWarning() << Q_FUNC_INFO << "Invalid anchors for" << item;
        return;
    }

    anchors->setProperty("fill", QVariant::fromValue(parentItem));
}

void View::enableAttribute(Qt::WidgetAttribute attr, bool enable)
{
    if (enable)
        m_widgetAttributes |= attr;
    else
        m_widgetAttributes &= ~attr;
}

bool View::hasAttribute(Qt::WidgetAttribute attr) const
{
    return m_widgetAttributes & attr;
}

void View::setFlag(Qt::WindowType f, bool on)
{
    if (on) {
        m_windowFlags |= f;
    } else {
        m_windowFlags &= ~f;
    }
}

Qt::WindowFlags View::flags() const
{
    return m_windowFlags;
}

QSize View::minSize() const
{
    const QSize min = property("kddockwidgets_min_size").toSize();
    return min.expandedTo(Core::Item::hardcodedMinimumSize);
}

QSize View::maxSizeHint() const
{
    const QSize max = property("kddockwidgets_max_size").toSize();
    return max.isEmpty() ? Core::Item::hardcodedMaximumSize
                         : max.boundedTo(Core::Item::hardcodedMaximumSize);
}

QRect View::geometry() const
{
    if (isRootView()) {
        if (QWindow *w = QQuickItem::window()) {
            return w->geometry();
        }
    }

    return QRect(QPointF(QQuickItem::x(), QQuickItem::y()).toPoint(), QQuickItem::size().toSize());
}

QRect View::normalGeometry() const
{
    return m_normalGeometry;
}

void View::setNormalGeometry(QRect geo)
{
    m_normalGeometry = geo;
}

void View::setMaximumSize(QSize sz)
{
    if (maxSizeHint() != sz) {
        setProperty("kddockwidgets_max_size", sz);
        updateGeometry();
        Core::View::d->layoutInvalidated.emit();
    }
}

void View::setWidth(int w)
{
    QQuickItem::setWidth(w);
}

void View::setHeight(int h)
{
    QQuickItem::setHeight(h);
}

void View::setFixedWidth(int w)
{
    setWidth(w);
    setMinimumSize({ w, minSize().height() });
    setMaximumSize({ w, maxSizeHint().height() });
}

void View::setFixedHeight(int h)
{
    setHeight(h);
    setMinimumSize({ minSize().width(), h });
    setMaximumSize({ maxSizeHint().width(), h });
}

void View::setFixedSize(Size sz)
{
    setFixedWidth(sz.width());
    setFixedHeight(sz.height());
}

void View::show()
{
    setVisible(true);
}

void View::hide()
{
    setVisible(false);
}

void View::updateGeometry()
{
    Q_EMIT geometryUpdated();
}

void View::update()
{
    // Nothing to do for QtQuick
}

void View::setParent(QQuickItem *parentItem)
{
    {
        ScopedValueRollback guard(m_inSetParent, true);
        QQuickItem::setParent(parentItem);
        QQuickItem::setParentItem(parentItem);
    }

    // Mimic QWidget::setParent(), hide widget when setting parent
    // Only if no parent item though, as that causes binding loops. Since it's benign we won't
    // bother making it strictly like qtwidgets.
    if (!parentItem && !m_inDtor)
        setVisible(false);
}

void View::setParent(Core::View *parent)
{
    setParent(QtQuick::asQQuickItem(parent));
}

void View::raiseAndActivate(QQuickItem *item)
{
    if (QWindow *w = item->window()) {
        w->raise();
        w->requestActivate();
    }
}

void View::raiseAndActivate()
{
    raiseAndActivate(this);
}

void View::activateWindow()
{
    if (QWindow *w = QQuickItem::window())
        w->requestActivate();
}

void View::raise()
{
    if (isRootView()) {
        if (QWindow *w = QQuickItem::window())
            w->raise();
    } else if (auto p = QQuickItem::parentItem()) {
        // It's not a top-level, so just increase its Z-order
        const auto siblings = p->childItems();
        QQuickItem *last = siblings.last();
        if (last != this)
            stackAfter(last);
    }
}

/*static*/ bool View::isRootView(const QQuickItem *item)
{
    QQuickItem *parent = item->parentItem();
    if (!parent)
        return true;

    if (auto *w = qobject_cast<QQuickWindow *>(item->window())) {
        if (parent == w->contentItem() || item == w->contentItem())
            return true;
        if (auto *qv = qobject_cast<QQuickView *>(item->window())) {
            if (parent == qv->rootObject() || item == qv->rootObject())
                return true;
        }
    }

    return false;
}

bool View::isRootView() const
{
    return View::isRootView(this);
}

QQuickView *View::quickView() const
{
    return qobject_cast<QQuickView *>(QQuickItem::window());
}

QPoint View::mapToGlobal(QPoint localPt) const
{
    return QQuickItem::mapToGlobal(localPt).toPoint();
}

QPoint View::mapFromGlobal(QPoint globalPt) const
{
    return QQuickItem::mapFromGlobal(globalPt).toPoint();
}

QPoint View::mapTo(Core::View *parent, QPoint pos) const
{
    if (!parent)
        return {};

    auto parentItem = asQQuickItem(parent);
    return parentItem->mapFromGlobal(QQuickItem::mapToGlobal(pos)).toPoint();
}

void View::setWindowOpacity(double v)
{
    if (QWindow *w = QQuickItem::window())
        w->setOpacity(v);
}

void View::setWindowTitle(const QString &title)
{
    if (QWindow *w = QQuickItem::window())
        w->setTitle(title);
}

void View::setWindowIcon(const QIcon &icon)
{
    if (QWindow *w = QQuickItem::window())
        w->setIcon(icon);
}

bool View::isActiveWindow() const
{
    if (QWindow *w = QQuickItem::window())
        return w->isActive();

    return false;
}

void View::showNormal()
{
    if (QWindow *w = QQuickItem::window())
        w->showNormal();
}

void View::showMinimized()
{
    if (QWindow *w = QQuickItem::window())
        w->showMinimized();
}

void View::showMaximized()
{
    if (QWindow *w = QQuickItem::window())
        w->showMaximized();
}

bool View::isMinimized() const
{
    if (QWindow *w = QQuickItem::window())
        return w->windowStates() & Qt::WindowMinimized;

    return false;
}

bool View::isMaximized() const
{
    if (QWindow *w = QQuickItem::window())
        return w->windowStates() & Qt::WindowMaximized;

    return false;
}

int View::zOrder() const
{
    // Returns the zOrder so we can unit test that raising works in MDI mode.
    // This is unrelated to QQuickItem::z(), which is always 0 for us.
    if (auto p = parentItem()) {
        const auto siblings = p->childItems();
        return siblings.indexOf(const_cast<QtQuick::View *>(this));
    }

    return 0;
}

std::shared_ptr<Core::Window> View::window() const
{
    if (QWindow *w = QQuickItem::window()) {
        auto windowqtquick = new QtQuick::Window(w);
        return std::shared_ptr<Core::Window>(windowqtquick);
    }

    return {};
}

std::shared_ptr<Core::View> View::childViewAt(QPoint p) const
{
    auto child = QQuickItem::childAt(p.x(), p.y());
    return child ? asQQuickWrapper(child) : nullptr;
}

/*static*/
std::shared_ptr<Core::View> View::parentViewFor(const QQuickItem *item)
{
    auto p = item->parentItem();
    if (QQuickWindow *window = item->window()) {
        if (p == window->contentItem()) {
            // For our purposes, the root view is the one directly below QQuickWindow::contentItem
            return nullptr;
        }
    }

    return p ? asQQuickWrapper(p) : nullptr;
}

/* static */
std::shared_ptr<Core::View> View::asQQuickWrapper(QQuickItem *item)
{
    return ViewWrapper::create(item);
}

std::shared_ptr<Core::View> View::parentView() const
{
    return parentViewFor(this);
}

std::shared_ptr<Core::View> View::asWrapper()
{
    return ViewWrapper::create(this);
}

void View::grabMouse()
{
    QQuickItem::grabMouse();
}

void View::releaseMouse()
{
    QQuickItem::ungrabMouse();
}

void View::releaseKeyboard()
{
    // Not needed for QtQuick
}

void View::setFocus(Qt::FocusReason reason)
{
    QQuickItem::setFocus(true, reason);
    forceActiveFocus(reason);
}

bool View::hasFocus() const
{
    return QQuickItem::hasActiveFocus();
}

Qt::FocusPolicy View::focusPolicy() const
{
    return m_focusPolicy;
}

void View::setFocusPolicy(Qt::FocusPolicy policy)
{
    m_focusPolicy = policy;
}

QString View::viewName() const
{
    return QQuickItem::objectName();
}

void View::setMinimumSize(QSize sz)
{
    if (minSize() != sz) {
        setProperty("kddockwidgets_min_size", sz);
        updateGeometry();
        Core::View::d->layoutInvalidated.emit();
    }
}

void View::render(QPainter *painter)
{
    if (QQuickWindow *w = QQuickItem::window()) {
        const QImage image = w->grabWindow();

        const QPoint pos = mapToScene({ 0, 0 }).toPoint();
        const QRect sourceRect { pos * image.devicePixelRatio(), painter->window().size() * image.devicePixelRatio() };
        painter->drawImage(painter->window(), image, sourceRect);
    }
}

void View::setCursor(Qt::CursorShape shape)
{
    QQuickItem::setCursor(shape);
}

void View::setMouseTracking(bool enable)
{
    m_mouseTrackingEnabled = enable;
}

QVector<std::shared_ptr<Core::View>> View::childViews() const
{
    QVector<std::shared_ptr<Core::View>> result;
    const auto childItems = QQuickItem::childItems();
    result.reserve(childItems.size());
    for (QQuickItem *child : childItems) {
        result << asQQuickWrapper(child);
    }

    return result;
}

void View::onWindowStateChangeEvent(QWindowStateChangeEvent *)
{
    if (QWindow *window = QQuickItem::window()) {
        m_oldWindowState = window->windowState();
    }
}

bool View::isFixedWidth() const
{
    return m_controller && m_controller->isFixedWidth();
}

bool View::isFixedHeight() const
{
    return m_controller && m_controller->isFixedHeight();
}

namespace KDDockWidgets {
inline QString cleanQRCFilename(const QString &filename)
{
    // QFile doesn't understand qrc:/ only :/

    if (filename.startsWith(QStringLiteral("qrc:/")))
        return filename.right(filename.size() - 3);

    return filename;
}
}

QQuickItem *View::createItem(const QString &filename, QQuickItem *parent, QQmlContext *ctx)
{
    auto p = parent;
    QQmlEngine *engine = nullptr;
    while (p && !engine) {
        engine = qmlEngine(p);
        p = p->parentItem();
    }

    if (!engine)
        engine = QtQuick::Platform::instance()->qmlEngine();

    if (!engine) {
        qWarning() << Q_FUNC_INFO << "No engine found";
        return nullptr;
    }

    if (!QFile::exists(cleanQRCFilename(filename))) {
        qWarning() << Q_FUNC_INFO << "File not found" << filename;
        return nullptr;
    }

    QQmlComponent component(engine, filename);
    auto qquickitem = qobject_cast<QQuickItem *>(component.create(ctx));
    if (!qquickitem) {
        qWarning() << Q_FUNC_INFO << component.errorString();
        return nullptr;
    }

    qquickitem->setParentItem(parent);
    qquickitem->QObject::setParent(parent);

    return qquickitem;
}

void View::setZOrder(int z)
{
    QQuickItem::setZ(z);
}

QQuickItem *View::visualItem() const
{
    qWarning() << Q_FUNC_INFO
               << "Base class called, please implement in your derived class if needed";
    return nullptr;
}

QQmlContext *QtQuick::qmlContextFor(QQuickItem *item)
{
    while (item) {
        if (auto ctx = qmlContext(item))
            return ctx;
        item = item->parentItem();
    }

    return nullptr;
}

#include "View.moc"
