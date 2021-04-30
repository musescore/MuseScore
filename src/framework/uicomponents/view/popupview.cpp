/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "popupview.h"

#include <functional>
#include <QQuickView>
#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QMainWindow>
#include <QTimer>

#include "log.h"

namespace mu::uicomponents {
// the private part of PopupView implementation
class PopupWindow : public QObject
{
    INJECT(uicomponents, ui::IMainWindow, mainWindow)
    public:
        PopupWindow(QQmlEngine* engine, std::shared_ptr<ui::IUiConfiguration> uiConfiguration, bool isDialogMode);
    ~PopupWindow();

    void setContent(QQuickItem* item);

    void show(QPoint p);
    void hide();

    QWindow* qWindow() const;
    bool isVisible() const;
    const QRect& geometry() const;

    void forceActiveFocus();

    std::function<void()> onHidden;

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    void syncSizes();

    QQuickView* m_view = nullptr;
    QWidget* m_widget = nullptr;
    QTimer m_sizesSynchronizer;
};

PopupWindow::PopupWindow(QQmlEngine* engine, std::shared_ptr<ui::IUiConfiguration> uiConfiguration, bool isDialogMode)
    : QObject()
{
    setObjectName("PopupWindow");

    m_view = new QQuickView(engine, mainWindow()->qWindow());
    m_view->setObjectName("PopupQuickWindow");

    m_view->setResizeMode(QQuickView::SizeRootObjectToView);

    Qt::WindowFlags windowFlags;
    QColor bgColor;

    // dialog
    if (isDialogMode) {
        windowFlags = Qt::Dialog;

        QString bgColorStr = uiConfiguration->currentTheme().values.value(ui::BACKGROUND_PRIMARY_COLOR).toString();
        bgColor = QColor(bgColorStr);

    }
    // popup
    else {
        windowFlags = Qt::Dialog                // The most appropriate behavior for us on all platforms
                | Qt::FramelessWindowHint       // Without border
                | Qt::NoDropShadowWindowHint    // Without system shadow
                | Qt::BypassWindowManagerHint;  // Otherwise, it does not work correctly on Gnome (Linux) when resizing

        bgColor = QColor(0, 0, 0, 0); // transparent
    }

    m_view->setFlags(windowFlags);
    m_view->setColor(bgColor);
    m_view->installEventFilter(this);

    //! HACK The shortcut system expects
    //! the active window should be a QWidget window or QWidget Popup,
    //! else shortcut not working, even with Application Shortcut context.
    //! So, added QWidget Window for shortcuts to work
    //! see https://code.woboq.org/qt5/qtbase/src/widgets/kernel/qshortcut.cpp.html#_Z29qWidgetShortcutContextMatcherP7QObjectN2Qt15ShortcutContextE

    m_widget = QWidget::createWindowContainer(m_view, nullptr, windowFlags);
    m_widget->setObjectName("PopupWidgetWindow");

    // dialog
    if (isDialogMode) {
        m_widget->setAutoFillBackground(true);

        QPalette pal = m_widget->palette();
        pal.setColor(QPalette::Window, bgColor);
        m_widget->setPalette(pal);
    }
    // popup
    else {
        m_widget->setAutoFillBackground(false);
        m_widget->setAttribute(Qt::WA_TranslucentBackground);
    }

    m_widget->installEventFilter(this);

    //! HACK Resizing the popup window is overhead and a slow operation,
    //! if it is doing for each cycle of animation of changing the size of the content (usually the height),
    //! then graphic artifacts may appear and it looks awful.
    //! So, when changing the size of the content,
    //! we resize the window itself twice as large, and as soon as the changes are over,
    //! we will synchronize the sizes.

    if (!isDialogMode) {
        m_sizesSynchronizer.setSingleShot(true);
        m_sizesSynchronizer.setInterval(1000);
        connect(&m_sizesSynchronizer, &QTimer::timeout, [this]() {
            syncSizes();
        });
    }
}

PopupWindow::~PopupWindow()
{
    delete m_widget;
}

void PopupWindow::syncSizes()
{
    QQuickItem* item = m_view->rootObject();
    if (item) {
        m_widget->resize(item->implicitWidth(), item->implicitHeight());
    }
}

void PopupWindow::setContent(QQuickItem* item)
{
    m_view->setContent(QUrl(), nullptr, item);

    m_widget->resize(item->width(), item->height());

    connect(item, &QQuickItem::widthChanged, [this, item]() {
        if (item->width() > m_widget->width()) {
            m_widget->resize(m_widget->width() * 2, m_widget->height());
        }
        m_sizesSynchronizer.start();
    });

    connect(item, &QQuickItem::heightChanged, [this, item]() {
        if (item->height() > m_widget->height()) {
            m_widget->resize(m_widget->width(), m_widget->height() * 2);
        }
        m_sizesSynchronizer.start();
    });
}

void PopupWindow::forceActiveFocus()
{
    if (!m_view->contentItem()->hasActiveFocus()) {
        m_widget->setFocus();
    }
}

void PopupWindow::show(QPoint p)
{
    m_widget->move(p);
    m_widget->show();

    syncSizes();
    m_widget->activateWindow();
    m_widget->setFocus();
}

void PopupWindow::hide()
{
    m_widget->hide();
}

QWindow* PopupWindow::qWindow() const
{
    return m_widget->windowHandle();
}

bool PopupWindow::isVisible() const
{
    return m_widget->isVisible();
}

const QRect& PopupWindow::geometry() const
{
    return m_widget->geometry();
}

bool PopupWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Please, don't remove
    //#define POPUPWINDOW_DEBUG_EVENTS_ENABLED
#ifdef POPUPWINDOW_DEBUG_EVENTS_ENABLED
    static QMetaEnum typeEnum = QMetaEnum::fromType<QEvent::Type>();
    static QList<QEvent::Type> excludeLoggingTypes = { QEvent::MouseMove };
    const char* typeStr = typeEnum.key(event->type());
    if (!excludeLoggingTypes.contains(event->type())) {
        LOGI() << (watched ? watched->objectName() : "null") << " event: " << (typeStr ? typeStr : "unknown");
    }

    static QList<QEvent::Type> trackEvents = { QEvent::Hide, QEvent::Show };
    if (trackEvents.contains(event->type())) {
        int k = 1;
    }

    if (QString(typeStr) == "WindowDeactivate") {
        int k = 1;
    }
#endif

    // QQuickView events
    if (watched == m_view) {
        if (event->type() == QEvent::FocusIn) {
            m_view->contentItem()->forceActiveFocus();
        }

        if (event->type() == QEvent::MouseButtonPress) {
            forceActiveFocus();
        }
    }

    // QWidgetContainer events
    if (watched == m_widget) {
        if (event->type() == QEvent::Hide) {
            if (onHidden) {
                onHidden();
            }
        }
    }

    return QObject::eventFilter(watched, event);
}
}

using namespace mu::uicomponents;

PopupView::PopupView(QQuickItem* parent)
    : QObject(parent)
{
    setObjectName("PopupView");
    setErrCode(Ret::Code::Ok);

    qApp->installEventFilter(this);
    connect(qApp, &QApplication::applicationStateChanged, this, &PopupView::onApplicationStateChanged);
}

QQuickItem* PopupView::parentItem() const
{
    return qobject_cast<QQuickItem*>(parent());
}

void PopupView::setParentItem(QQuickItem* parent)
{
    if (parentItem() == parent) {
        return;
    }

    QObject::setParent(parent);
    emit parentItemChanged();
}

void PopupView::forceActiveFocus()
{
    IF_ASSERT_FAILED(m_window) {
        return;
    }
    m_window->forceActiveFocus();
}

bool PopupView::isDialog() const
{
    return false;
}

void PopupView::classBegin()
{
}

void PopupView::componentComplete()
{
    QQmlEngine* engine = qmlEngine(this);
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_window = new PopupWindow(engine, uiConfiguration(), isDialog());
    m_window->onHidden = [this]() { onHidden(); };
    m_window->setContent(m_contentItem);
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::MouseButtonPress == event->type()) {
        mousePressEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::MouseButtonRelease == event->type()) {
        mouseReleaseEvent(static_cast<QMouseEvent*>(event));
    } else if (QEvent::Close == event->type() && watched == mainWindow()->qMainWindow()) {
        close();
    }

    return QObject::eventFilter(watched, event);
}

QWindow* PopupView::qWindow() const
{
    return m_window ? m_window->qWindow() : nullptr;
}

void PopupView::beforeShow()
{
}

void PopupView::open()
{
    if (isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    beforeShow();

    if (m_globalPos.isNull()) {
        QQuickItem* prn = parentItem();
        IF_ASSERT_FAILED(prn) {
            return;
        }

        m_globalPos = prn->mapToGlobal(m_localPos);
    }

    m_window->show(m_globalPos.toPoint());

    QWindow* qWindow = m_window->qWindow();
    IF_ASSERT_FAILED(qWindow) {
        return;
    }
    qWindow->setTitle(m_title);
    qWindow->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);

    const QRect& winRect = m_window->geometry();
    qWindow->setMinimumSize(winRect.size());
    if (!m_resizable) {
        qWindow->setMaximumSize(winRect.size());
    }

    if (!m_navigationParentControl) {
        ui::INavigationControl* ctrl = navigationController()->activeControl();
        //! NOTE At the moment we have only qml navigation controls
        ui::NavigationControl* qmlCtrl = dynamic_cast<ui::NavigationControl*>(ctrl);
        setNavigationParentControl(qmlCtrl);
    }

    emit isOpenedChanged();
    emit opened();
}

void PopupView::onHidden()
{
    emit isOpenedChanged();
    emit closed();
}

void PopupView::close()
{
    if (!isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    m_window->hide();
}

void PopupView::toggleOpened()
{
    if (isOpened()) {
        close();
    } else {
        open();
    }
}

bool PopupView::isOpened() const
{
    return m_window ? m_window->isVisible() : false;
}

PopupView::ClosePolicy PopupView::closePolicy() const
{
    return m_closePolicy;
}

mu::ui::NavigationControl* PopupView::navigationParentControl() const
{
    return m_navigationParentControl;
}

void PopupView::setNavigationParentControl(mu::ui::NavigationControl* navigationParentControl)
{
    if (m_navigationParentControl == navigationParentControl) {
        return;
    }

    m_navigationParentControl = navigationParentControl;
    emit navigationParentControlChanged(m_navigationParentControl);
}

void PopupView::setContentItem(QQuickItem* content)
{
    if (m_contentItem == content) {
        return;
    }

    m_contentItem = content;
    emit contentItemChanged();
}

QQuickItem* PopupView::contentItem() const
{
    return m_contentItem;
}

qreal PopupView::localX() const
{
    return m_localPos.x();
}

qreal PopupView::localY() const
{
    return m_localPos.y();
}

const QRect& PopupView::geometry() const
{
    return m_window->geometry();
}

void PopupView::setLocalX(qreal x)
{
    if (qFuzzyCompare(m_localPos.x(), x)) {
        return;
    }

    m_localPos.setX(x);
    emit xChanged(x);
}

void PopupView::setLocalY(qreal y)
{
    if (qFuzzyCompare(m_localPos.y(), y)) {
        return;
    }

    m_localPos.setY(y);
    emit yChanged(y);
}

void PopupView::setClosePolicy(ClosePolicy closePolicy)
{
    if (m_closePolicy == closePolicy) {
        return;
    }

    m_closePolicy = closePolicy;
    emit closePolicyChanged(closePolicy);
}

void PopupView::onApplicationStateChanged(Qt::ApplicationState state)
{
    if (m_closePolicy == NoAutoClose) {
        return;
    }

    if (state != Qt::ApplicationActive) {
        close();
    }
}

void PopupView::mousePressEvent(QMouseEvent* event)
{
    if (!isOpened()) {
        return;
    }

    if (m_closePolicy == ClosePolicy::CloseOnPressOutsideParent) {
        if (!isMouseWithinBoundaries(event->globalPos())) {
            close();
        }
    }
}

void PopupView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!isOpened()) {
        return;
    }

    if (m_closePolicy == ClosePolicy::CloseOnReleaseOutsideParent) {
        if (!isMouseWithinBoundaries(event->globalPos())) {
            close();
        }
    }
}

bool PopupView::isMouseWithinBoundaries(const QPoint& mousePos) const
{
    QRect viewRect = m_window->geometry();
    bool contains = viewRect.contains(mousePos);
    if (!contains) {
        //! NOTE We also check the parent because often clicking on the parent should toggle the popup,
        //! but if we don't check a parent here, the popup will be closed and reopened.
        QQuickItem* prn = parentItem();
        QPointF localPos = prn->mapFromGlobal(mousePos);
        QRectF parentRect = QRectF(prn->x(), prn->y(), prn->width(), prn->height());
        contains = parentRect.contains(localPos);
    }

    return contains;
}

void PopupView::setObjectID(QString objectID)
{
    if (m_objectID == objectID) {
        return;
    }

    m_objectID = objectID;
    emit objectIDChanged(m_objectID);
}

QString PopupView::objectID() const
{
    return m_objectID;
}

QString PopupView::title() const
{
    return m_title;
}

void PopupView::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    if (qWindow()) {
        qWindow()->setTitle(title);
    }

    emit titleChanged(m_title);
}

bool PopupView::modal() const
{
    return m_modal;
}

void PopupView::setModal(bool modal)
{
    if (m_modal == modal) {
        return;
    }

    m_modal = modal;

    if (qWindow()) {
        qWindow()->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);
    }

    emit modalChanged(m_modal);
}

void PopupView::setResizable(bool resizable)
{
    if (m_resizable == resizable) {
        return;
    }

    m_resizable = resizable;
    emit resizableChanged(m_resizable);
}

bool PopupView::resizable() const
{
    return m_resizable;
}

void PopupView::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

QVariantMap PopupView::ret() const
{
    return m_ret;
}

void PopupView::setErrCode(Ret::Code code)
{
    QVariantMap ret;
    ret["errcode"] = static_cast<int>(code);
    setRet(ret);
}
