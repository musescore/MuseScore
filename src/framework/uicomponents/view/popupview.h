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
#pragma once

#include <QQuickItem>
#include <QQmlParserStatus>

#include "types/ret.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iuiconfiguration.h"
#include "ui/inavigationcontroller.h"

#include "popupwindow/ipopupwindow.h"
#include "internal/popupviewclosecontroller.h"

Q_MOC_INCLUDE(< QWindow >)

class QQuickCloseEvent;

namespace muse::ui {
class INavigationControl;
}

namespace muse::uicomponents {
class PopupPosition
{
    Q_GADGET

public:
    enum Type: int {
        Left = 0x01,
        Right = 0x02,
        Horizontal = 0x03,
        Bottom = 0x04,
        Top = 0x08,
        Vertical = 0x0C,
    };

    Q_ENUM(Type)
};

class PopupView : public QObject, public QQmlParserStatus, public Injectable, public async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QQuickItem * parent READ parentItem WRITE setParentItem NOTIFY parentItemChanged)

    Q_PROPERTY(QQuickItem * contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(QWindow * window READ window NOTIFY windowChanged)
    Q_PROPERTY(QWindow * parentWindow READ parentWindow WRITE setParentWindow NOTIFY parentWindowChanged FINAL)

    //! NOTE Local, related parent
    Q_PROPERTY(qreal x READ localX WRITE setLocalX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ localY WRITE setLocalY NOTIFY yChanged)

    Q_PROPERTY(bool showArrow READ showArrow WRITE setShowArrow NOTIFY showArrowChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)
    Q_PROPERTY(PlacementPolicies placementPolicies READ placementPolicies WRITE setPlacementPolicies NOTIFY placementPoliciesChanged)

    Q_PROPERTY(QQuickItem * anchorItem READ anchorItem WRITE setAnchorItem NOTIFY anchorItemChanged)
    Q_PROPERTY(PopupPosition::Type popupPosition READ popupPosition WRITE setPopupPosition NOTIFY popupPositionChanged)

    Q_PROPERTY(int arrowX READ arrowX WRITE setArrowX NOTIFY arrowXChanged)
    Q_PROPERTY(int arrowY READ arrowY WRITE setArrowY NOTIFY arrowYChanged)

    Q_PROPERTY(bool isOpened READ isOpened NOTIFY isOpenedChanged)
    Q_PROPERTY(OpenPolicies openPolicies READ openPolicies WRITE setOpenPolicies NOTIFY openPoliciesChanged)

    Q_PROPERTY(bool isContentReady READ isContentReady WRITE setIsContentReady NOTIFY isContentReadyChanged)

    Q_PROPERTY(ClosePolicies closePolicies READ closePolicies WRITE setClosePolicies NOTIFY closePoliciesChanged)

    Q_PROPERTY(
        bool activateParentOnClose READ activateParentOnClose WRITE setActivateParentOnClose NOTIFY activateParentOnCloseChanged)

    Q_PROPERTY(FocusPolicies focusPolicies READ focusPolicies WRITE setFocusPolicies NOTIFY focusPoliciesChanged)

    //! NOTE Used for dialogs, but be here so that dialogs and just popups have one api
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString objectId READ objectId WRITE setObjectId NOTIFY objectIdChanged)
    Q_PROPERTY(bool modal READ modal WRITE setModal NOTIFY modalChanged)
    Q_PROPERTY(bool frameless READ frameless WRITE setFrameless NOTIFY framelessChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(bool alwaysOnTop READ alwaysOnTop WRITE setAlwaysOnTop NOTIFY alwaysOnTopChanged)
    Q_PROPERTY(QVariantMap ret READ ret WRITE setRet NOTIFY retChanged)

public:
    Inject<ui::IMainWindow> mainWindow = { this };
    Inject<ui::IUiConfiguration> uiConfiguration = { this };
    Inject<ui::INavigationController> navigationController = { this };

public:

    explicit PopupView(QQuickItem* parent = nullptr);
    ~PopupView() override;

    enum class OpenPolicy {
        Default = 0x00000000,
        NoActivateFocus = 0x00000001,
        OpenOnContentReady = 0x00000002
    };
    Q_DECLARE_FLAGS(OpenPolicies, OpenPolicy)
    Q_FLAG(OpenPolicies)

    enum class ClosePolicy {
        NoAutoClose = 0x00000000,
        CloseOnPressOutsideParent = 0x00000001,
    };
    Q_DECLARE_FLAGS(ClosePolicies, ClosePolicy)
    Q_FLAG(ClosePolicies)

    enum class FocusPolicy {
        TabFocus = 0x00000001,
        ClickFocus = 0x00000002,
        DefaultFocus = FocusPolicy::TabFocus | FocusPolicy::ClickFocus,
        NoFocus = 0
    };
    Q_DECLARE_FLAGS(FocusPolicies, FocusPolicy)
    Q_FLAG(FocusPolicies)

    enum class PlacementPolicy {
        Default = 0x00000000,
        PreferBelow = 0x00000001,
        PreferAbove = 0x00000002,
        PreferLeft = 0x00000004,
        PreferRight = 0x00000008,
        IgnoreFit = 0x0000000F,
    };
    Q_DECLARE_FLAGS(PlacementPolicies, PlacementPolicy)
    Q_FLAG(PlacementPolicies)

    QQuickItem* parentItem() const;

    QQuickItem* contentItem() const;
    int contentWidth() const;
    int contentHeight() const;

    QWindow* window() const;

    qreal localX() const;
    qreal localY() const;
    QRect geometry() const;

    Q_INVOKABLE void forceActiveFocus();

    void init();

    Q_INVOKABLE void open();
    Q_INVOKABLE void close(bool force = false);
    Q_INVOKABLE void toggleOpened();

    Q_INVOKABLE QRectF anchorGeometry() const;

    OpenPolicies openPolicies() const;
    ClosePolicies closePolicies() const;
    PlacementPolicies placementPolicies() const;

    bool activateParentOnClose() const;
    FocusPolicies focusPolicies() const;

    ui::INavigationControl* navigationParentControl() const;

    bool isOpened() const;

    QString objectId() const;
    QString title() const;
    bool modal() const;
    bool frameless() const;
    bool resizable() const;
    bool alwaysOnTop() const;
    QVariantMap ret() const;

    PopupPosition::Type popupPosition() const;
    int arrowX() const;
    int arrowY() const;
    int padding() const;
    bool showArrow() const;
    QQuickItem* anchorItem() const;

    bool isContentReady() const;
    void setIsContentReady(bool ready);

public slots:
    void setParentItem(QQuickItem* parent);
    void setEngine(QQmlEngine* engine);
    void setComponent(QQmlComponent* component);
    void setContentItem(QQuickItem* content);
    void setContentWidth(int contentWidth);
    void setContentHeight(int contentHeight);
    void setLocalX(qreal x);
    void setLocalY(qreal y);
    void setOpenPolicies(muse::uicomponents::PopupView::OpenPolicies openPolicies);
    void setClosePolicies(muse::uicomponents::PopupView::ClosePolicies closePolicies);
    void setPlacementPolicies(muse::uicomponents::PopupView::PlacementPolicies placementPolicies);
    void setNavigationParentControl(muse::ui::INavigationControl* parentNavigationControl);
    void setObjectId(QString objectId);
    void setTitle(QString title);
    void setModal(bool modal);
    void setFrameless(bool frameless);
    void setResizable(bool resizable);
    void setAlwaysOnTop(bool alwaysOnTop);
    void setRet(QVariantMap ret);

    void setPopupPosition(PopupPosition::Type position);
    void setArrowX(int arrowX);
    void setArrowY(int arrowY);
    void setPadding(int padding);
    void setShowArrow(bool showArrow);
    void setAnchorItem(QQuickItem* anchorItem);

    void setActivateParentOnClose(bool activateParentOnClose);
    void setFocusPolicies(const muse::uicomponents::PopupView::FocusPolicies& policies);

signals:
    void parentItemChanged();
    void contentItemChanged();
    void contentWidthChanged();
    void contentHeightChanged();
    void windowChanged();
    void xChanged(qreal x);
    void yChanged(qreal y);
    void openPoliciesChanged(muse::uicomponents::PopupView::OpenPolicies openPolicies);
    void closePoliciesChanged(muse::uicomponents::PopupView::ClosePolicies closePolicies);
    void placementPoliciesChanged(muse::uicomponents::PopupView::PlacementPolicies placementPolicies);
    void navigationParentControlChanged(muse::ui::INavigationControl* navigationParentControl);
    void objectIdChanged(QString objectId);
    void titleChanged(QString title);
    void modalChanged(bool modal);
    void framelessChanged(bool frameless);
    void resizableChanged(bool resizable);
    void alwaysOnTopChanged();
    void retChanged(QVariantMap ret);

    void isOpenedChanged();
    void opened();
    void aboutToClose(QQuickCloseEvent* closeEvent);
    void closed(bool force);

    void popupPositionChanged(PopupPosition::Type position);
    void arrowXChanged(int arrowX);
    void arrowYChanged(int arrowY);
    void paddingChanged(int padding);
    void showArrowChanged(bool showArrow);
    void anchorItemChanged(QQuickItem* anchorItem);

    void activateParentOnCloseChanged(bool activateParentOnClose);
    void focusPoliciesChanged();

    void isContentReadyChanged();

    void parentWindowChanged();

protected:
    virtual bool isDialog() const;
    void classBegin() override;
    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    void initCloseController();

    void doFocusOut();
    void windowMoveEvent();

    bool isMouseWithinBoundaries(const QPointF& mousePos) const;

    virtual void beforeOpen();
    void doOpen();

    QWindow* qWindow() const;
    virtual void onHidden();

    void repositionWindowIfNeed();

    void setErrCode(Ret::Code code);

    QWindow* parentWindow() const;
    void setParentWindow(QWindow* window);
    void resolveParentWindow();

    virtual QScreen* resolveScreen() const;
    QRect currentScreenGeometry() const;
    virtual void updateGeometry();
    virtual void updateContentPosition();

    virtual QRect viewGeometry() const;

    void resolveNavigationParentControl();
    void activateNavigationParentControl();

    QQmlEngine* engine() const;

    QWindow* m_parentWindow = nullptr;
    IPopupWindow* m_window = nullptr;

    QQmlComponent* m_component = nullptr;
    QQmlEngine* m_engine = nullptr;

    QQuickItem* m_contentItem = nullptr;
    int m_contentWidth = 0;
    int m_contentHeight = 0;

    QQuickItem* m_anchorItem = nullptr;

    QPointF m_localPos;
    QPointF m_globalPos;

    OpenPolicies m_openPolicies = { OpenPolicy::Default };
    bool m_shouldOpenOnReady = false;
    bool m_isContentReady = false;

    ClosePolicies m_closePolicies = { ClosePolicy::CloseOnPressOutsideParent };
    FocusPolicies m_focusPolicies = { FocusPolicy::DefaultFocus };

    PlacementPolicies m_placementPolicies = { PlacementPolicy::Default };

    bool m_activateParentOnClose = true;
    ui::INavigationControl* m_navigationParentControl = nullptr;
    QString m_objectId;
    QString m_title;
    bool m_modal = true;
    bool m_frameless = false;
    bool m_resizable = false;
    bool m_alwaysOnTop = false;
    QVariantMap m_ret;
    PopupPosition::Type m_popupPosition = PopupPosition::Bottom;
    int m_arrowX = 0;
    int m_arrowY = 0;
    int m_padding = 0;
    bool m_showArrow = false;

    PopupViewCloseController* m_closeController = nullptr;
    bool m_forceClosed = false;
};
}
