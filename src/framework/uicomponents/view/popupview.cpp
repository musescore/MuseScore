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

#include <QQuickView>
#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QMainWindow>

#include "log.h"

namespace mu::uicomponents {
// the private part of PopupView implementation
class PopupWindow : public QObject
{
public:
    PopupWindow(QQmlEngine* engine);
    ~PopupWindow();

    void setContent(QQuickItem* item);

    void show(QPoint p);
    void hide();
    bool isVisible() const;
    const QRect& geometry() const;

    void forceActiveFocus();

private:
    bool eventFilter(QObject* watched, QEvent* event) override;

    QQuickView* m_view = nullptr;
    QWidget* m_widget = nullptr;
};

PopupWindow::PopupWindow(QQmlEngine* engine)
    : QObject()
{
    setObjectName("PopupWindow");

    m_view = new QQuickView(engine, nullptr);
    m_view->setObjectName("PopupQuickWindow");
    m_view->setFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_view->setResizeMode(QQuickView::SizeViewToRootObject);
    m_view->setColor(QColor(0, 0, 0, 0)); // transparent

    m_view->installEventFilter(this);

    //! HACK The shortcut system expects
    //! the active window should be a QWidget window or QWidget Popup,
    //! else shortcut not working, even with Application Shortcut context.
    //! So, added QWidget Window for shortcuts to work
    /* From Qt source
     * https://code.woboq.org/qt5/qtbase/src/widgets/kernel/qshortcut.cpp.html#_Z29qWidgetShortcutContextMatcherP7QObjectN2Qt15ShortcutContextE
    bool qWidgetShortcutContextMatcher(QObject *object, Qt::ShortcutContext context)
    {
        QWidget *active_window = QApplication::activeWindow();

        // popups do not become the active window,
        // so we fake it here to get the correct context
        // for the shortcut system.
        if (QApplication::activePopupWidget())
            active_window = QApplication::activePopupWidget();

        if (!active_window) {
            QWindow *qwindow = QGuiApplication::focusWindow();
            if (qwindow && qwindow->isActive()) {
                while (qwindow) {
                    if (auto widgetWindow = qobject_cast<QWidgetWindow *>(qwindow)) {
                        active_window = widgetWindow->widget();
                        break;
                    }
                    qwindow = qwindow->parent();
                }
            }
        }

        if (!active_window)
            return false;
        ...
    }
    */

    m_widget = QWidget::createWindowContainer(m_view, nullptr, Qt::Window | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_widget->setObjectName("PopupWidgetWindow");
    m_widget->setAutoFillBackground(false);
    m_widget->setAttribute(Qt::WA_TranslucentBackground);

    m_widget->installEventFilter(this);

    connect(m_view, &QQuickView::widthChanged, [this](int arg) {
        m_widget->resize(arg, m_widget->height());
    });

    connect(m_view, &QQuickView::heightChanged, [this](int arg) {
        m_widget->resize(m_widget->width(), arg);
    });
}

PopupWindow::~PopupWindow()
{
    delete m_widget;
}

void PopupWindow::setContent(QQuickItem* item)
{
    m_view->setContent(QUrl(), nullptr, item);
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
    m_widget->setFocus();
}

void PopupWindow::hide()
{
    m_widget->hide();
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
    //    static QMetaEnum typeEnum = QMetaEnum::fromType<QEvent::Type>();
    //    LOGI() << watched->objectName() << " event: " << typeEnum.key(event->type());

    // QQuickView events
    if (watched == m_view) {
        if (event->type() == QEvent::FocusIn) {
            m_view->contentItem()->forceActiveFocus();
        }

        if (event->type() == QEvent::MouseButtonPress) {
            forceActiveFocus();
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

void PopupView::open()
{
    if (isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    QQuickItem* prn = parentItem();
    IF_ASSERT_FAILED(prn) {
        return;
    }

    QPointF pos = prn->position();
    pos.setY(pos.y() + m_localPos.y());
    pos.setX(pos.x() + m_localPos.x());

    QPointF global = prn->mapToGlobal(pos);
    m_window->show(global.toPoint());

    emit isOpenedChanged();
    emit opened();
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

    emit isOpenedChanged();
    emit closed();
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

qreal PopupView::x() const
{
    return m_localPos.x();
}

qreal PopupView::y() const
{
    return m_localPos.y();
}

void PopupView::setX(qreal x)
{
    if (qFuzzyCompare(m_localPos.x(), x)) {
        return;
    }

    m_localPos.setX(x);
    emit xChanged(x);
}

void PopupView::setY(qreal y)
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

void PopupView::classBegin()
{
}

void PopupView::componentComplete()
{
    QQmlEngine* engine = qmlEngine(this);
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_window = new PopupWindow(engine);
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

void PopupView::onApplicationStateChanged(Qt::ApplicationState state)
{
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
    return viewRect.contains(mousePos);
}
