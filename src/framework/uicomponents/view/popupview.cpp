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
#include <QTimer>
#include <QScreen>

#include "ui/inavigation.h"

#include "popupwindow/popupwindow_qquickview.h"

#if defined(Q_OS_MAC)
#include "internal/platform/macos/macospopupviewclosecontroller.h"
#elif defined(Q_OS_WIN)
#include "internal/platform/win/winpopupviewclosecontroller.h"
#endif

#include "log.h"

using namespace muse::uicomponents;

PopupView::PopupView(QQuickItem* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{
    setObjectName("PopupView");
    setErrCode(Ret::Code::Ok);

    setPadding(12);
    setShowArrow(true);
}

PopupView::~PopupView()
{
    if (m_window) {
        m_window->setOnHidden(std::function<void()>());
    }

    if (m_closeController) {
        delete m_closeController;
    }
}

QQuickItem* PopupView::parentItem() const
{
    if (!parent()) {
        return nullptr;
    }

    return qobject_cast<QQuickItem*>(parent());
}

void PopupView::setParentItem(QQuickItem* parent)
{
    if (parentItem() == parent) {
        return;
    }

    QObject::setParent(parent);

    if (m_closeController) {
        m_closeController->setParentItem(parent);
    }

    emit parentItemChanged();
}

void PopupView::setEngine(QQmlEngine* engine)
{
    if (m_engine == engine) {
        return;
    }

    m_engine = engine;
}

void PopupView::setComponent(QQmlComponent* component)
{
    if (m_component == component) {
        return;
    }

    m_component = component;
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

void PopupView::init()
{
    QQmlEngine* engine = this->engine();
    IF_ASSERT_FAILED(engine) {
        return;
    }

    m_window = new PopupWindow_QQuickView(muse::iocCtxForQmlEngine(engine), this);
    m_window->init(engine, isDialog(), frameless());
    m_window->setOnHidden([this]() { onHidden(); });
    m_window->setContent(m_component, m_contentItem);
    m_window->setTakeFocusOnClick(m_focusPolicies & FocusPolicy::ClickFocus);

    // TODO: Can't use new `connect` syntax because the IPopupWindow::aboutToClose
    // has a parameter of type QQuickCloseEvent, which is not public, so we
    // can't include any header for it and it will always be an incomplete
    // type, which is not allowed for the new `connect` syntax.
    //connect(m_window, &IPopupWindow::aboutToClose, this, &PopupView::aboutToClose);
    connect(m_window, SIGNAL(aboutToClose(QQuickCloseEvent*)), this, SIGNAL(aboutToClose(QQuickCloseEvent*)));

    connect(this, &PopupView::isContentReadyChanged, this, [this]() {
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

void PopupView::initCloseController()
{
#if defined(Q_OS_MAC)
    m_closeController = new MacOSPopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#elif defined(Q_OS_WIN)
    m_closeController = new WinPopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#else
    m_closeController = new PopupViewCloseController(muse::iocCtxForQmlEngine(this->engine()));
#endif

    m_closeController->init();

    m_closeController->setParentItem(parentItem());
    m_closeController->setWindow(window());
    m_closeController->setIsCloseOnPressOutsideParent(m_closePolicies & ClosePolicy::CloseOnPressOutsideParent);
    m_closeController->setCanClosed(!m_closePolicies.testFlag(ClosePolicy::NoAutoClose));

    m_closeController->closeNotification().onNotify(this, [this]() {
        close(true);
    });
}

void PopupView::componentComplete()
{
    init();
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::UpdateRequest == event->type()
        || (event->type() == QEvent::Move && watched == mainWindow()->qWindow())) {
        repositionWindowIfNeed();
    }

    return QObject::eventFilter(watched, event);
}

QWindow* PopupView::qWindow() const
{
    return m_window ? m_window->qWindow() : nullptr;
}

void PopupView::open()
{
    if ((m_openPolicies & OpenPolicy::OpenOnContentReady) && !m_isContentReady) {
        m_shouldOpenOnReady = true;
        return;
    }

    doOpen();
}

void PopupView::beforeOpen()
{
}

void PopupView::doOpen()
{
    if (isOpened()) {
        repositionWindowIfNeed();
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    beforeOpen();

    resolveParentWindow();

    updateGeometry();

    if (!isDialog()) {
        updateContentPosition();
    }

    if (isDialog()) {
        QWindow* qWindow = m_window->qWindow();
        IF_ASSERT_FAILED(qWindow) {
            return;
        }

        qWindow->setTitle(m_title);

        if (m_alwaysOnTop) {
#ifdef Q_OS_MAC
            auto updateStayOnTopHint = [this]() {
                bool stay = qApp->applicationState() == Qt::ApplicationActive;
                m_window->qWindow()->setFlag(Qt::WindowStaysOnTopHint, stay);
            };
            updateStayOnTopHint();
            connect(qApp, &QApplication::applicationStateChanged, this, updateStayOnTopHint);
#endif
        } else {
            qWindow->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);
        }

        qWindow->setFlag(Qt::FramelessWindowHint, m_frameless);
#ifdef MUSE_MODULE_UI_DISABLE_MODALITY
        qWindow->setModality(Qt::NonModal);
#endif
        m_window->setResizable(m_resizable);
    }

    resolveNavigationParentControl();

    QScreen* screen = resolveScreen();
    m_window->show(screen, viewGeometry(), !(m_openPolicies & OpenPolicy::NoActivateFocus));

    m_globalPos = QPointF(); // invalidate

    if (!isDialog()) {
        if (!m_closeController) {
            initCloseController();
        }

        m_closeController->setActive(true);
    }

    qApp->installEventFilter(this);

    emit isOpenedChanged();
    emit opened();
}

void PopupView::onHidden()
{
    emit isOpenedChanged();
    emit closed(m_forceClosed);
}

void PopupView::close(bool force)
{
    if (!isOpened()) {
        return;
    }

    IF_ASSERT_FAILED(m_window) {
        return;
    }

    if (m_closeController) {
        m_closeController->setCanClosed(true);
        m_closeController->setActive(false);
    }

    qApp->removeEventFilter(this);

    m_forceClosed = force;
    m_window->close();

    activateNavigationParentControl();
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

PopupView::OpenPolicies PopupView::openPolicies() const
{
    return m_openPolicies;
}

PopupView::ClosePolicies PopupView::closePolicies() const
{
    return m_closePolicies;
}

PopupView::PlacementPolicies PopupView::placementPolicies() const
{
    return m_placementPolicies;
}

bool PopupView::activateParentOnClose() const
{
    return m_activateParentOnClose;
}

PopupView::FocusPolicies PopupView::focusPolicies() const
{
    return m_focusPolicies;
}

muse::ui::INavigationControl* PopupView::navigationParentControl() const
{
    return m_navigationParentControl;
}

void PopupView::setNavigationParentControl(ui::INavigationControl* navigationParentControl)
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

int PopupView::contentWidth() const
{
    return m_contentWidth;
}

void PopupView::setContentWidth(int contentWidth)
{
    if (m_contentWidth == contentWidth) {
        return;
    }

    m_contentWidth = contentWidth;
    emit contentWidthChanged();
}

int PopupView::contentHeight() const
{
    return m_contentHeight;
}

void PopupView::setContentHeight(int contentHeight)
{
    if (m_contentHeight == contentHeight) {
        return;
    }

    m_contentHeight = contentHeight;
    emit contentHeightChanged();
}

QWindow* PopupView::window() const
{
    return qWindow();
}

qreal PopupView::localX() const
{
    return m_localPos.x();
}

qreal PopupView::localY() const
{
    return m_localPos.y();
}

QRect PopupView::geometry() const
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

    repositionWindowIfNeed();
}

void PopupView::setLocalY(qreal y)
{
    if (qFuzzyCompare(m_localPos.y(), y)) {
        return;
    }

    m_localPos.setY(y);
    emit yChanged(y);

    repositionWindowIfNeed();
}

void PopupView::setOpenPolicies(PopupView::OpenPolicies openPolicies)
{
    if (m_openPolicies == openPolicies) {
        return;
    }

    m_openPolicies = openPolicies;
    emit openPoliciesChanged(m_openPolicies);
}

void PopupView::repositionWindowIfNeed()
{
    if (isOpened() && !isDialog()) {
        m_globalPos = QPointF();
        updateGeometry();
        updateContentPosition();
        m_window->setPosition(m_globalPos.toPoint());
        m_globalPos = QPoint();
    }
}

void PopupView::setClosePolicies(ClosePolicies closePolicies)
{
    if (m_closePolicies == closePolicies) {
        return;
    }

    m_closePolicies = closePolicies;

    if (m_closeController) {
        m_closeController->setIsCloseOnPressOutsideParent(closePolicies & ClosePolicy::CloseOnPressOutsideParent);
    }

    emit closePoliciesChanged(closePolicies);
}

void PopupView::setPlacementPolicies(muse::uicomponents::PopupView::PlacementPolicies placementPolicies)
{
    if (m_placementPolicies == placementPolicies) {
        return;
    }

    m_placementPolicies = placementPolicies;
    emit placementPoliciesChanged(placementPolicies);
}

void PopupView::setObjectId(QString objectId)
{
    if (m_objectId == objectId) {
        return;
    }

    m_objectId = objectId;
    emit objectIdChanged(m_objectId);
}

QString PopupView::objectId() const
{
    return m_objectId;
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
    emit modalChanged(m_modal);
}

bool PopupView::frameless() const
{
    return m_frameless;
}

void PopupView::setFrameless(bool frameless)
{
    if (m_frameless == frameless) {
        return;
    }

    m_frameless = frameless;
    emit framelessChanged(m_frameless);
}

bool PopupView::resizable() const
{
    return m_window ? m_window->resizable() : m_resizable;
}

void PopupView::setResizable(bool resizable)
{
    if (this->resizable() == resizable) {
        return;
    }

    m_resizable = resizable;
    if (m_window) {
        m_window->setResizable(m_resizable);
    }
    emit resizableChanged(m_resizable);
}

bool PopupView::alwaysOnTop() const
{
    return m_alwaysOnTop;
}

void PopupView::setAlwaysOnTop(bool alwaysOnTop)
{
    if (m_alwaysOnTop == alwaysOnTop) {
        return;
    }

    m_alwaysOnTop = alwaysOnTop;
    emit alwaysOnTopChanged();
}

void PopupView::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

void PopupView::setArrowX(int arrowX)
{
    if (m_arrowX == arrowX) {
        return;
    }

    m_arrowX = arrowX;
    emit arrowXChanged(m_arrowX);
}

void PopupView::setArrowY(int arrowY)
{
    if (m_arrowY == arrowY) {
        return;
    }

    m_arrowY = arrowY;
    emit arrowYChanged(m_arrowY);
}

void PopupView::setPopupPosition(PopupPosition::Type position)
{
    if (m_popupPosition == position) {
        return;
    }

    m_popupPosition = position;
    emit popupPositionChanged(m_popupPosition);
}

void PopupView::setPadding(int padding)
{
    if (m_padding == padding) {
        return;
    }

    m_padding = padding;
    emit paddingChanged(m_padding);
}

void PopupView::setShowArrow(bool showArrow)
{
    if (m_showArrow == showArrow) {
        return;
    }

    m_showArrow = showArrow;
    emit showArrowChanged(m_showArrow);
}

void PopupView::setAnchorItem(QQuickItem* anchorItem)
{
    if (m_anchorItem == anchorItem) {
        return;
    }

    m_anchorItem = anchorItem;
    emit anchorItemChanged(m_anchorItem);
}

void PopupView::setActivateParentOnClose(bool activateParentOnClose)
{
    if (m_activateParentOnClose == activateParentOnClose) {
        return;
    }

    m_activateParentOnClose = activateParentOnClose;
    emit activateParentOnCloseChanged(m_activateParentOnClose);
}

void PopupView::setFocusPolicies(const FocusPolicies& policies)
{
    if (m_focusPolicies == policies) {
        return;
    }

    m_focusPolicies = policies;
    emit focusPoliciesChanged();
}

QVariantMap PopupView::ret() const
{
    return m_ret;
}

PopupPosition::Type PopupView::popupPosition() const
{
    return m_popupPosition;
}

int PopupView::arrowX() const
{
    return m_arrowX;
}

int PopupView::arrowY() const
{
    return m_arrowY;
}

int PopupView::padding() const
{
    return m_padding;
}

bool PopupView::showArrow() const
{
    return m_showArrow;
}

QQuickItem* PopupView::anchorItem() const
{
    return m_anchorItem;
}

void PopupView::setErrCode(Ret::Code code)
{
    QVariantMap ret;
    ret["errcode"] = static_cast<int>(code);
    setRet(ret);
}

QWindow* PopupView::parentWindow() const
{
    return m_window->parentWindow();
}

void PopupView::setParentWindow(QWindow* window)
{
    m_window->setParentWindow(window);
}

void PopupView::resolveParentWindow()
{
    if (QQuickItem* parent = parentItem()) {
        if (QWindow* window = parent->window()) {
            setParentWindow(window);
            return;
        }
    }
    setParentWindow(mainWindow()->qWindow());
}

QScreen* PopupView::resolveScreen() const
{
    const QWindow* parentWindow = this->parentWindow();
    QScreen* screen = parentWindow ? parentWindow->screen() : nullptr;

    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    return screen;
}

QRect PopupView::currentScreenGeometry() const
{
    QScreen* screen = resolveScreen();
    return mainWindow()->isFullScreen() ? screen->geometry() : screen->availableGeometry();
}

void PopupView::updateGeometry()
{
    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    if (m_globalPos.isNull()) {
        m_globalPos = parentTopLeft + m_localPos;
    }

    QRectF anchorRect = anchorGeometry();
    QRectF viewRect = viewGeometry();

    auto movePos = [this, &viewRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        viewRect.moveTopLeft(m_globalPos);
    };

    bool ignoreFit = m_placementPolicies.testFlag(PlacementPolicy::IgnoreFit);
    bool canFitAbove = !ignoreFit ? viewRect.height() < parentTopLeft.y() : true;
    bool canFitBelow = !ignoreFit ? viewRect.bottom() < anchorRect.bottom() : true;
    bool canFitLeft = !ignoreFit ? viewRect.width() < parentTopLeft.x() : true;
    bool canFitRight = !ignoreFit ? viewRect.right() < anchorRect.right() : true;

    auto moveBelow = [&]() {
        movePos(m_globalPos.x(), parentTopLeft.y() + parent->height());
        setPopupPosition(PopupPosition::Bottom);
    };

    auto moveAbove = [&]() {
        movePos(m_globalPos.x(), parentTopLeft.y() - viewRect.height());
        setPopupPosition(PopupPosition::Top);
    };

    auto moveLeft = [&]() {
        movePos(parentTopLeft.x() - viewRect.width(), m_globalPos.y());
        setPopupPosition(PopupPosition::Left);
    };

    auto moveRight = [&]() {
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y());
        setPopupPosition(PopupPosition::Right);
    };

    bool placementDefault = m_placementPolicies.testFlag(PlacementPolicy::Default);
    bool preferBelow = m_placementPolicies.testFlag(PlacementPolicy::PreferBelow);
    bool preferAbove = m_placementPolicies.testFlag(PlacementPolicy::PreferAbove);
    bool preferLeft = m_placementPolicies.testFlag(PlacementPolicy::PreferLeft);
    bool preferRight = m_placementPolicies.testFlag(PlacementPolicy::PreferRight);

    if ((preferBelow || placementDefault) && canFitBelow) {
        moveBelow();
    } else if ((preferAbove || placementDefault) && canFitAbove) {
        moveAbove();
    } else if (preferLeft && canFitLeft) {
        moveLeft();
    } else if (preferRight && canFitRight) {
        moveRight();
    } else if (!canFitBelow && canFitAbove && (preferBelow || placementDefault)) {
        moveAbove();
    } else if (!canFitAbove && canFitBelow && (preferAbove || placementDefault)) {
        moveBelow();
    } else if (!canFitLeft && canFitRight && preferLeft) {
        moveRight();
    } else if (!canFitRight && canFitLeft && preferRight) {
        moveLeft();
    } else {
        // move to the right of the parent and move to top to an area that doesn't fit
        movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (viewRect.bottom() - anchorRect.bottom()) + padding());
        setPopupPosition(PopupPosition::Right);
    }

    if (viewRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - viewRect.left(), m_globalPos.y());
    }

    if (viewRect.right() > anchorRect.right()) {
        // move to the left to an area that doesn't fit
        movePos(m_globalPos.x() - (viewRect.right() - anchorRect.right()), m_globalPos.y());
    }

    if (!showArrow()) {
        if (popupPosition() == PopupPosition::Bottom || popupPosition() == PopupPosition::Top) {
            movePos(m_globalPos.x() - padding(), m_globalPos.y());
        } else if (popupPosition() == PopupPosition::Left || popupPosition() == PopupPosition::Right) {
            movePos(m_globalPos.x(), m_globalPos.y() - padding());
        }
    }
}

void PopupView::updateContentPosition()
{
    if (showArrow()) {
        const QQuickItem* parent = parentItem();
        IF_ASSERT_FAILED(parent) {
            return;
        }

        QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

        QRect viewGeometry = this->viewGeometry();
        QPointF viewTopLeft = QPointF(viewGeometry.x(), viewGeometry.y());
        QPointF viewTopRight = QPointF(viewGeometry.x() + viewGeometry.width(), viewGeometry.y());

        if (parentTopLeft.x() < viewTopLeft.x() || parentTopLeft.x() > viewTopRight.x()) {
            setArrowX(viewGeometry.width() / 2);
        } else {
            setArrowX(parentTopLeft.x() + (parent->width() / 2) - m_globalPos.x());
        }

        if (parentTopLeft.y() < viewTopLeft.y() || parentTopLeft.y() > viewGeometry.bottom()) {
            setArrowY(viewGeometry.height() / 2);
        } else {
            setArrowY(parentTopLeft.y() + (parent->height() / 2) - m_globalPos.y());
        }
    }
}

QRect PopupView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), contentItem()->size().toSize());
}

QRectF PopupView::anchorGeometry() const
{
    QRectF geometry = currentScreenGeometry();
    if (m_anchorItem) {
        QPointF anchorItemTopLeft = m_anchorItem->mapToGlobal(QPoint(0, 0));
        geometry &= QRectF(anchorItemTopLeft, m_anchorItem->size());
    }

    return geometry;
}

void PopupView::resolveNavigationParentControl()
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

void PopupView::activateNavigationParentControl()
{
    if (m_activateParentOnClose && m_navigationParentControl) {
        m_navigationParentControl->requestActive();
    }
}

QQmlEngine* PopupView::engine() const
{
    if (m_engine) {
        return m_engine;
    }

    return qmlEngine(this);
}

bool PopupView::isContentReady() const
{
    return m_isContentReady;
}

void PopupView::setIsContentReady(bool ready)
{
    if (m_isContentReady == ready) {
        return;
    }

    m_isContentReady = ready;
    emit isContentReadyChanged();
}
