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
#include <QQuickWidget>
#include <QQmlEngine>
#include <QUrl>
#include <QQmlContext>
#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QScreen>

#include "popupwindow/popupwindow_qquickview.h"

#include "log.h"
#include "config.h"

using namespace mu::uicomponents;

static const QString POPUP_VIEW_CONTENT_OBJECT_NAME("_PopupViewContent");

PopupView::PopupView(QQuickItem* parent)
    : QObject(parent)
{
    setObjectName("PopupView");
    setErrCode(Ret::Code::Ok);

    setPadding(12);
    setShowArrow(true);

    connect(qApp, &QApplication::applicationStateChanged, this, &PopupView::onApplicationStateChanged);
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

    m_window = new PopupWindow_QQuickView();
    m_window->init(engine, uiConfiguration(), isDialog());
    m_window->setOnHidden([this]() { onHidden(); });
    m_window->setContent(m_contentItem);

    // TODO: Can't use new `connect` syntax because the IPopupWindow::aboutToClose
    // has a parameter of type QQuickCloseEvent, which is not public, so we
    // can't include any header for it and it will always be an incomplete
    // type, which is not allowed for the new `connect` syntax.
    //connect(m_window, &IPopupWindow::aboutToClose, this, &PopupView::aboutToClose);
    connect(m_window, SIGNAL(aboutToClose(QQuickCloseEvent*)), this, SIGNAL(aboutToClose(QQuickCloseEvent*)));

    connect(parentItem(), &QQuickItem::visibleChanged, this, [this]() {
        if (!parentItem() || !parentItem()->isVisible()) {
            close();
        }
    });

    if (!isDialog()) {
        m_contentItem->setObjectName(POPUP_VIEW_CONTENT_OBJECT_NAME);
    }

    emit windowChanged();
}

bool PopupView::eventFilter(QObject* watched, QEvent* event)
{
    if (QEvent::Close == event->type() && watched == mainWindow()->qWindow()) {
        close();
    } else if (QEvent::UpdateRequest == event->type()) {
        repositionWindowIfNeed();
    } else if (QEvent::FocusOut == event->type() && watched == window()) {
        doFocusOut();
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

    if (!isDialog()) {
        updatePosition();
        updateContentPosition();
    }

    if (isDialog()) {
        QWindow* qWindow = m_window->qWindow();
        IF_ASSERT_FAILED(qWindow) {
            return;
        }
        qWindow->setTitle(m_title);
        qWindow->setModality(m_modal ? Qt::ApplicationModal : Qt::NonModal);
#ifdef UI_DISABLE_MODALITY
        qWindow->setModality(Qt::NonModal);
#endif
        m_window->setResizable(m_resizable);
    }

    m_window->show(m_globalPos.toPoint());

    m_globalPos = QPointF(); // invalidate

    if (!m_navigationParentControl) {
        ui::INavigationControl* ctrl = navigationController()->activeControl();
        //! NOTE At the moment we have only qml navigation controls
        QObject* qmlCtrl = dynamic_cast<QObject*>(ctrl);
        setNavigationParentControl(qmlCtrl);

        connect(qmlCtrl, &QObject::destroyed, this, [this]() {
            setNavigationParentControl(nullptr);
        });
    }

    qApp->installEventFilter(this);

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

    qApp->removeEventFilter(this);

    m_window->close();
}

void PopupView::toggleOpened()
{
    if (isOpened()) {
        close();
    } else {
        open();
    }
}

void PopupView::setParentWindow(QWindow* window)
{
    m_window->setParentWindow(window);
}

bool PopupView::isOpened() const
{
    return m_window ? m_window->isVisible() : false;
}

PopupView::ClosePolicy PopupView::closePolicy() const
{
    return m_closePolicy;
}

QObject* PopupView::navigationParentControl() const
{
    return m_navigationParentControl;
}

void PopupView::setNavigationParentControl(QObject* navigationParentControl)
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

void PopupView::repositionWindowIfNeed()
{
    if (isOpened() && !isDialog()) {
        m_globalPos = QPointF();
        updatePosition();
        updateContentPosition();
        m_window->setPosition(m_globalPos.toPoint());
        m_globalPos = QPoint();
    }
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

void PopupView::doFocusOut()
{
    if (!isOpened()) {
        return;
    }

    if (m_closePolicy == ClosePolicy::CloseOnPressOutsideParent) {
        if (!isMouseWithinBoundaries(QCursor::pos())) {
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
        QRectF parentRect = QRectF(0, 0, prn->width(), prn->height());
        contains = parentRect.contains(localPos);
    }

    return contains;
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

bool PopupView::resizable() const
{
    return m_window ? m_window->resizable() : m_resizable;
}

void PopupView::setRet(QVariantMap ret)
{
    if (m_ret == ret) {
        return;
    }

    m_ret = ret;
    emit retChanged(m_ret);
}

void PopupView::setOpensUpward(bool opensUpward)
{
    if (m_opensUpward == opensUpward) {
        return;
    }

    m_opensUpward = opensUpward;
    emit opensUpwardChanged(m_opensUpward);
}

void PopupView::setArrowX(int arrowX)
{
    if (m_arrowX == arrowX) {
        return;
    }

    m_arrowX = arrowX;
    emit arrowXChanged(m_arrowX);
}

void PopupView::setCascadeAlign(Qt::AlignmentFlag cascadeAlign)
{
    if (m_cascadeAlign == cascadeAlign) {
        return;
    }

    m_cascadeAlign = cascadeAlign;
    emit cascadeAlignChanged(m_cascadeAlign);
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

QVariantMap PopupView::ret() const
{
    return m_ret;
}

bool PopupView::opensUpward() const
{
    return m_opensUpward;
}

int PopupView::arrowX() const
{
    return m_arrowX;
}

Qt::AlignmentFlag PopupView::cascadeAlign() const
{
    return m_cascadeAlign;
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

QRect PopupView::currentScreenGeometry() const
{
    QScreen* currentScreen = mainWindow()->screen();
    if (!currentScreen) {
        currentScreen = QGuiApplication::primaryScreen();
    }

    return mainWindow()->isFullScreen() ? currentScreen->geometry() : currentScreen->availableGeometry();
}

void PopupView::updatePosition()
{
    const QQuickItem* parent = parentItem();
    IF_ASSERT_FAILED(parent) {
        return;
    }

    QPointF parentTopLeft = parent->mapToGlobal(QPoint(0, 0));

    if (m_globalPos.isNull()) {
        m_globalPos = parentTopLeft + m_localPos;
    }

    const QWindow* window = mainWindow()->qWindow();
    if (!window) {
        return;
    }

    QRectF anchorRect = anchorGeometry();
    QRectF popupRect(m_globalPos, QSize(contentWidth(), contentHeight() + padding() * 2));

    setOpensUpward(false);
    setCascadeAlign(Qt::AlignmentFlag::AlignRight);

    auto movePos = [this, &popupRect](qreal x, qreal y) {
        m_globalPos.setX(x);
        m_globalPos.setY(y);

        popupRect.moveTopLeft(m_globalPos);
    };

    const QQuickItem* parentPopupContentItem = this->parentPopupContentItem();
    bool isCascade = parentPopupContentItem != nullptr;

    if (popupRect.left() < anchorRect.left()) {
        // move to the right to an area that doesn't fit
        movePos(m_globalPos.x() + anchorRect.left() - popupRect.left(), m_globalPos.y());
    }

    qreal popupShiftByY = parent->height() + popupRect.height();
    if (popupRect.bottom() > anchorRect.bottom()) {
        if (isCascade) {
            // move to the top to an area that doesn't fit
            movePos(m_globalPos.x(), m_globalPos.y() - (popupRect.bottom() - anchorRect.bottom()) + padding());
        } else {
            qreal newY = m_globalPos.y() - popupShiftByY;
            if (anchorRect.top() < newY) {
                // move to the top of the parent
                movePos(m_globalPos.x(), newY);
                setOpensUpward(true);
            } else {
                // move to the right of the parent and move to top to an area that doesn't fit
                movePos(parentTopLeft.x() + parent->width(), m_globalPos.y() - (popupRect.bottom() - anchorRect.bottom()) + padding());
            }
        }
    }

    Qt::AlignmentFlag parentCascadeAlign = this->parentCascadeAlign(parentPopupContentItem);
    if (popupRect.right() > anchorRect.right() || parentCascadeAlign != Qt::AlignmentFlag::AlignRight) {
        if (isCascade) {
            // move to the right of the parent
            movePos(parentTopLeft.x() - popupRect.width() + padding() * 2, m_globalPos.y());
            setCascadeAlign(Qt::AlignmentFlag::AlignLeft);
        } else {
            // move to the left to an area that doesn't fit
            movePos(m_globalPos.x() - (popupRect.right() - anchorRect.right()), m_globalPos.y());
        }
    }

    if (!showArrow()) {
        movePos(m_globalPos.x() - padding(), m_globalPos.y());
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

        setArrowX(parentTopLeft.x() + (parent->width() / 2) - m_globalPos.x());
    } else {
        if (opensUpward()) {
            contentItem()->setY(padding());
        } else {
            contentItem()->setY(-padding());
        }
    }
}

QQuickItem* PopupView::parentPopupContentItem() const
{
    QQuickItem* parent = parentItem();
    while (parent) {
        if (parent->objectName() == POPUP_VIEW_CONTENT_OBJECT_NAME) {
            return parent;
        }

        parent = parent->parentItem();
    }

    return nullptr;
}

Qt::AlignmentFlag PopupView::parentCascadeAlign(const QQuickItem* parent) const
{
    if (!parent) {
        return Qt::AlignmentFlag::AlignRight;
    }

    return static_cast<Qt::AlignmentFlag>(parent->property("cascadeAlign").toInt());
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

int PopupView::contentWidth() const
{
    return m_contentWidth;
}

void PopupView::setContentWidth(int newContentWidth)
{
    if (m_contentWidth == newContentWidth) {
        return;
    }

    m_contentWidth = newContentWidth;
    emit contentWidthChanged();
}

int PopupView::contentHeight() const
{
    return m_contentHeight;
}

void PopupView::setContentHeight(int newContentHeight)
{
    if (m_contentHeight == newContentHeight) {
        return;
    }

    m_contentHeight = newContentHeight;
    emit contentHeightChanged();
}
