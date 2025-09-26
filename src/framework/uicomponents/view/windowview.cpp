/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "windowview.h"

#include <functional>
#include <QQuickView>
#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QTimer>
#include <QScreen>

#include "ui/inavigation.h"

#include "popupwindow/popupwindow_qquickview.h"

#include "log.h"

using namespace muse::uicomponents;

WindowView::WindowView(QQuickItem* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

WindowView::~WindowView()
{
    if (m_window) {
        m_window->setOnHidden(std::function<void()>());
    }
}

QQuickItem* WindowView::parentItem() const
{
    if (!parent()) {
        return nullptr;
    }

    return qobject_cast<QQuickItem*>(parent());
}

void WindowView::setParentItem(QQuickItem* parent)
{
    if (parentItem() == parent) {
        return;
    }

    QObject::setParent(parent);
    emit parentItemChanged();
}

void WindowView::setEngine(QQmlEngine* engine)
{
    if (m_engine == engine) {
        return;
    }

    m_engine = engine;
}

void WindowView::setComponent(QQmlComponent* component)
{
    if (m_component == component) {
        return;
    }

    m_component = component;
}

void WindowView::forceActiveFocus()
{
    IF_ASSERT_FAILED(m_window) {
        return;
    }
    m_window->forceActiveFocus();
}

void WindowView::classBegin()
{
}

void WindowView::init()
{
    QQmlEngine* engine = this->engine();
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_window = new PopupWindow_QQuickView(muse::iocCtxForQmlEngine(engine), this);
    initWindow();
    m_window->setOnHidden([this]() { onHidden(); });
    m_window->setParentWindow(m_parentWindow);
    m_window->setContent(m_component, m_contentItem);
    m_window->setTakeFocusOnClick(m_focusPolicies & FocusPolicy::ClickFocus);

    // TODO: Can't use new `connect` syntax because the IPopupWindow::aboutToClose
    // has a parameter of type QQuickCloseEvent, which is not public, so we
    // can't include any header for it and it will always be an incomplete
    // type, which is not allowed for the new `connect` syntax.
    //connect(m_window, &IPopupWindow::aboutToClose, this, &PopupView::aboutToClose);
    connect(m_window, SIGNAL(aboutToClose(QQuickCloseEvent*)), this, SIGNAL(aboutToClose(QQuickCloseEvent*)));

    connect(this, &WindowView::isContentReadyChanged, this, [this]() {
        if (isContentReady() && m_shouldOpenOnReady) {
            doOpen();
        }
    });

    navigationController()->navigationChanged().onNotify(this, [this]() {
        if (!(m_focusPolicies & FocusPolicy::TabFocus)) {
            return;
        }

        ui::INavigationPanel* navigationPanel = navigationController()->activePanel();
        if (!navigationPanel) {
            return;
        }

        if (navigationPanel->window() == m_window->qWindow() && !m_window->hasActiveFocus()) {
            m_window->forceActiveFocus();
        }
    });

    emit windowChanged();
}

void WindowView::componentComplete()
{
    init();
}

QWindow* WindowView::qWindow() const
{
    return m_window ? m_window->qWindow() : nullptr;
}

void WindowView::open()
{
    if ((m_openPolicies & OpenPolicy::OpenOnContentReady) && !m_isContentReady) {
        m_shouldOpenOnReady = true;
        return;
    }

    doOpen();
}

void WindowView::beforeOpen()
{
}

void WindowView::doOpen()
{
    if (isOpened()) {
        repositionWindowIfNeed();
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    resolveParentWindow();
    updateGeometry();

    beforeOpen();

    resolveNavigationParentControl();

    QScreen* screen = resolveScreen();
    m_window->show(screen, viewGeometry(), !(m_openPolicies & OpenPolicy::NoActivateFocus));

    m_globalPos = QPointF(); // invalidate

    emit isOpenedChanged();
    emit opened();
}

void WindowView::onHidden()
{
    emit isOpenedChanged();
    emit closed(m_forceClosed);

    activateNavigationParentControl();
}

void WindowView::close(bool force)
{
    if (!isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    m_forceClosed = force;
    m_window->close();
}

void WindowView::toggleOpened()
{
    if (isOpened()) {
        close();
    } else {
        open();
    }
}

bool WindowView::isOpened() const
{
    return m_window ? m_window->isVisible() : false;
}

WindowView::OpenPolicies WindowView::openPolicies() const
{
    return m_openPolicies;
}

bool WindowView::activateParentOnClose() const
{
    return m_activateParentOnClose;
}

WindowView::FocusPolicies WindowView::focusPolicies() const
{
    return m_focusPolicies;
}

muse::ui::INavigationControl* WindowView::navigationParentControl() const
{
    return m_navigationParentControl;
}

void WindowView::setNavigationParentControl(ui::INavigationControl* navigationParentControl)
{
    if (m_navigationParentControl == navigationParentControl) {
        return;
    }

    m_navigationParentControl = navigationParentControl;
    emit navigationParentControlChanged(m_navigationParentControl);
}

void WindowView::setContentItem(QQuickItem* content)
{
    if (m_contentItem == content) {
        return;
    }

    m_contentItem = content;
    emit contentItemChanged();
}

QQuickItem* WindowView::contentItem() const
{
    return m_contentItem;
}

int WindowView::contentWidth() const
{
    return m_contentWidth;
}

void WindowView::setContentWidth(int contentWidth)
{
    if (m_contentWidth == contentWidth) {
        return;
    }

    m_contentWidth = contentWidth;
    emit contentWidthChanged();
}

int WindowView::contentHeight() const
{
    return m_contentHeight;
}

void WindowView::setContentHeight(int contentHeight)
{
    if (m_contentHeight == contentHeight) {
        return;
    }

    m_contentHeight = contentHeight;
    emit contentHeightChanged();
}

QWindow* WindowView::window() const
{
    return qWindow();
}

QRect WindowView::geometry() const
{
    return m_window->geometry();
}

void WindowView::setOpenPolicies(WindowView::OpenPolicies openPolicies)
{
    if (m_openPolicies == openPolicies) {
        return;
    }

    m_openPolicies = openPolicies;
    emit openPoliciesChanged(m_openPolicies);
}

void WindowView::setActivateParentOnClose(bool activateParentOnClose)
{
    if (m_activateParentOnClose == activateParentOnClose) {
        return;
    }

    m_activateParentOnClose = activateParentOnClose;
    emit activateParentOnCloseChanged(m_activateParentOnClose);
}

void WindowView::setFocusPolicies(const FocusPolicies& policies)
{
    if (m_focusPolicies == policies) {
        return;
    }

    m_focusPolicies = policies;
    emit focusPoliciesChanged();
}

QWindow* WindowView::parentWindow() const
{
    return m_parentWindow;
}

void WindowView::setParentWindow(QWindow* window)
{
    if (m_parentWindow == window) {
        return;
    }

    if (m_window) {
        m_window->setParentWindow(window);
    }
    m_parentWindow = window;

    emit parentWindowChanged();
}

void WindowView::resolveParentWindow()
{
    if (m_parentWindow) {
        return;
    }

    if (QQuickItem* parent = parentItem()) {
        if (QWindow* window = parent->window()) {
            setParentWindow(window);
            return;
        }
    }
    setParentWindow(mainWindow()->qWindow());
}

QScreen* WindowView::resolveScreen() const
{
    const QWindow* parentWindow = this->parentWindow();
    QScreen* screen = parentWindow ? parentWindow->screen() : nullptr;

    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    return screen;
}

QRect WindowView::currentScreenGeometry() const
{
    QScreen* screen = resolveScreen();
    return mainWindow()->isFullScreen() ? screen->geometry() : screen->availableGeometry();
}

QRect WindowView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), contentItem()->size().toSize());
}

void WindowView::resolveNavigationParentControl()
{
    ui::INavigationControl* ctrl = navigationController()->activeControl();
    setNavigationParentControl(ctrl);

    //! NOTE At the moment we have only qml navigation controls
    QObject* qmlCtrl = dynamic_cast<QObject*>(ctrl);

    if (qmlCtrl) {
        connect(qmlCtrl, &QObject::destroyed, this, [this]() {
            setNavigationParentControl(nullptr);
        });
    }
}

void WindowView::activateNavigationParentControl()
{
    if (m_activateParentOnClose && m_navigationParentControl) {
        m_navigationParentControl->requestActive();
    }
}

QQmlEngine* WindowView::engine() const
{
    if (m_engine) {
        return m_engine;
    }

    return qmlEngine(this);
}

bool WindowView::isContentReady() const
{
    return m_isContentReady;
}

void WindowView::setIsContentReady(bool ready)
{
    if (m_isContentReady == ready) {
        return;
    }

    m_isContentReady = ready;
    emit isContentReadyChanged();
}
