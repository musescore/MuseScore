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

#include <QQmlEngine>
#include <QQuickView>
#include <QScreen>
#include <QTimer>
#include <QUrl>

#include "ui/inavigation.h"

#include "log.h"

using namespace muse::uicomponents;

WindowView::WindowView(QQuickItem* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
}

void WindowView::classBegin()
{
}

void WindowView::componentComplete()
{
    init();
}

void WindowView::init()
{
    IF_ASSERT_FAILED(engine()) {
        return;
    }

    initView();
    setViewContent(m_contentItem);

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

        if (navigationPanel->window() == m_view && !hasActiveFocus()) {
            forceActiveFocus();
        }
    });

    emit windowChanged();
}

void WindowView::initView()
{
    //! NOTE: do not set the window when constructing the view
    //! This causes different bugs on different OS (e.g., no transparency for popups on windows)
    m_view = new QQuickView(engine(), nullptr);

    //! NOTE: We must set the parent
    //! Otherwise, the garbage collector may take ownership of the view and destroy it when we don't expect it
    m_view->QObject::setParent(this);

    m_view->setObjectName(objectName() + "_WindowView_QQuickView");
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);

    //! NOTE It is important that there is a connection to this signal with an error,
    //! otherwise the default action will be performed - displaying a message and terminating.
    //! We will not be able to switch to another backend.
    QObject::connect(m_view, &QQuickWindow::sceneGraphError, this, [this](QQuickWindow::SceneGraphError, const QString& msg) {
        LOGE() << "[" << objectName() << "] scene graph error: " << msg;
    });

    QObject::connect(m_view, &QQuickView::closing, this, &WindowView::aboutToClose);

    m_view->installEventFilter(this);
}

void WindowView::setViewContent(QQuickItem* item)
{
    if (!m_view || !item) {
        return;
    }

    m_view->setContent(QUrl(), nullptr, item);

    connect(item, &QQuickItem::implicitWidthChanged, this, [this, item]() {
        if (!m_view->isVisible()) {
            return;
        }
        if (item->implicitWidth() != m_view->width()) {
            updateSize(QSize(item->implicitWidth(), item->implicitHeight()));
        }
    });

    connect(item, &QQuickItem::implicitHeightChanged, this, [this, item]() {
        if (!m_view->isVisible()) {
            return;
        }
        if (item->implicitHeight() != m_view->height()) {
            updateSize(QSize(item->implicitWidth(), item->implicitHeight()));
        }
    });
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

    IF_ASSERT_FAILED(m_view) {
        return;
    }

    resolveParentWindow();
    updateGeometry();

    beforeOpen();

    resolveNavigationParentControl();

    showView();

    m_globalPos = QPointF(); // invalidate

    emit isOpenedChanged();
    emit opened();
}

void WindowView::showView()
{
    if (!m_view) {
        return;
    }

    const QRect geometry = viewGeometry();
    m_view->setGeometry(geometry);

    QScreen* screen = resolveScreen();
    m_view->setScreen(screen);

    if (m_openPolicies & OpenPolicy::NoActivateFocus) {
        m_view->setFlag(Qt::WindowDoesNotAcceptFocus);
    }

    assert(m_parentWindow);
    m_view->setTransientParent(m_parentWindow);

    connect(m_parentWindow, &QWindow::visibleChanged, this, [this](){
        if (!m_view->transientParent() || !m_view->transientParent()->isVisible()) {
            close();
        }
    });

    m_view->show();

    const QQuickItem* item = m_view->rootObject();
    if (item) {
        updateSize(QSize(item->implicitWidth(), item->implicitHeight()));
    }

    if (!(m_openPolicies & OpenPolicy::NoActivateFocus)) {
        QTimer::singleShot(0, this, [this]() {
            forceActiveFocus();
        });
    }
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

    IF_ASSERT_FAILED(m_view) {
        return;
    }

    m_forceClosed = force;
    m_view->close();
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
    return m_view ? m_view->isVisible() : false;
}

bool WindowView::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_view) {
        switch (event->type()) {
        case QEvent::Hide:
            onHidden();
            break;
        case QEvent::FocusIn:
            if (m_view->rootObject()) {
                m_view->rootObject()->forceActiveFocus();
            }
            break;
        case QEvent::FocusOut:
            if (!(m_focusPolicies & FocusPolicy::ClickFocus)) {
                // Undo what's done in TextInput{Field,Area}.qml ensureActiveFocus
                m_view->setFlag(Qt::WindowDoesNotAcceptFocus);
            }
            break;
        case QEvent::MouseButtonPress:
            if (m_focusPolicies & FocusPolicy::ClickFocus) {
                forceActiveFocus();
            }
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

QQmlEngine* WindowView::engine() const
{
    if (m_engine) {
        return m_engine;
    }

    return qmlEngine(this);
}

void WindowView::setEngine(QQmlEngine* engine)
{
    if (m_engine == engine) {
        return;
    }

    m_engine = engine;
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

QQuickItem* WindowView::contentItem() const
{
    return m_contentItem;
}

void WindowView::setContentItem(QQuickItem* content)
{
    if (m_contentItem == content) {
        return;
    }

    m_contentItem = content;
    emit contentItemChanged();
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
    return m_view;
}

QRect WindowView::geometry() const
{
    return m_view ? m_view->geometry() : QRect();
}

bool WindowView::hasActiveFocus()
{
    return m_view && m_view->activeFocusItem() != nullptr;
}

void WindowView::forceActiveFocus()
{
    if (!m_view) {
        return;
    }

    m_view->setFlag(Qt::WindowDoesNotAcceptFocus, false);
    m_view->requestActivate();

    QQuickItem* rootObject = m_view->rootObject();
    if (!rootObject) {
        return;
    }

    if (!rootObject->hasActiveFocus()) {
        rootObject->forceActiveFocus();
    }
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
    setParentWindow(interactiveProvider()->topWindow());
}

QScreen* WindowView::resolveScreen() const
{
    QScreen* screen = m_parentWindow ? m_parentWindow->screen() : nullptr;

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

void WindowView::updateSize(const QSize& newSize)
{
    if (!m_view) {
        return;
    }

    if (m_resizable) {
        m_view->setMinimumSize(newSize);
        m_view->setMaximumSize(QSize(16777215, 16777215));
        m_view->resize(m_view->size().expandedTo(newSize));
    } else {
        m_view->setMinimumSize(newSize);
        m_view->setMaximumSize(newSize);
        m_view->resize(newSize);
    }
}

WindowView::OpenPolicies WindowView::openPolicies() const
{
    return m_openPolicies;
}

void WindowView::setOpenPolicies(WindowView::OpenPolicies openPolicies)
{
    if (m_openPolicies == openPolicies) {
        return;
    }

    m_openPolicies = openPolicies;
    emit openPoliciesChanged(m_openPolicies);
}

WindowView::FocusPolicies WindowView::focusPolicies() const
{
    return m_focusPolicies;
}

void WindowView::setFocusPolicies(const FocusPolicies& policies)
{
    if (m_focusPolicies == policies) {
        return;
    }

    m_focusPolicies = policies;
    emit focusPoliciesChanged();
}

bool WindowView::activateParentOnClose() const
{
    return m_activateParentOnClose;
}

void WindowView::setActivateParentOnClose(bool activateParentOnClose)
{
    if (m_activateParentOnClose == activateParentOnClose) {
        return;
    }

    m_activateParentOnClose = activateParentOnClose;
    emit activateParentOnCloseChanged(m_activateParentOnClose);
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
