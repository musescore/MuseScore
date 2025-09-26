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
class WindowView : public QObject, public QQmlParserStatus, public Injectable, public async::Asyncable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QQuickItem * parent READ parentItem WRITE setParentItem NOTIFY parentItemChanged)

    Q_PROPERTY(QQuickItem * contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(QWindow * window READ window NOTIFY windowChanged)
    Q_PROPERTY(QWindow * parentWindow READ parentWindow WRITE setParentWindow NOTIFY parentWindowChanged FINAL)

    Q_PROPERTY(bool isOpened READ isOpened NOTIFY isOpenedChanged)
    Q_PROPERTY(OpenPolicies openPolicies READ openPolicies WRITE setOpenPolicies NOTIFY openPoliciesChanged)

    Q_PROPERTY(bool isContentReady READ isContentReady WRITE setIsContentReady NOTIFY isContentReadyChanged)

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

    explicit WindowView(QQuickItem* parent = nullptr);
    ~WindowView() override;

    enum class OpenPolicy {
        Default = 0x00000000,
        NoActivateFocus = 0x00000001,
        OpenOnContentReady = 0x00000002
    };
    Q_DECLARE_FLAGS(OpenPolicies, OpenPolicy)
    Q_FLAG(OpenPolicies)

    enum class FocusPolicy {
        TabFocus = 0x00000001,
        ClickFocus = 0x00000002,
        DefaultFocus = FocusPolicy::TabFocus | FocusPolicy::ClickFocus,
        NoFocus = 0
    };
    Q_DECLARE_FLAGS(FocusPolicies, FocusPolicy)
    Q_FLAG(FocusPolicies)

    QQuickItem* parentItem() const;

    QQuickItem* contentItem() const;
    int contentWidth() const;
    int contentHeight() const;

    QWindow* window() const;

    QRect geometry() const;

    Q_INVOKABLE void forceActiveFocus();

    void init();

    Q_INVOKABLE void open();
    Q_INVOKABLE void close(bool force = false);
    Q_INVOKABLE void toggleOpened();

    OpenPolicies openPolicies() const;

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

    bool isContentReady() const;
    void setIsContentReady(bool ready);

public slots:
    void setParentItem(QQuickItem* parent);
    void setEngine(QQmlEngine* engine);
    void setComponent(QQmlComponent* component);
    void setContentItem(QQuickItem* content);
    void setContentWidth(int contentWidth);
    void setContentHeight(int contentHeight);
    void setOpenPolicies(muse::uicomponents::WindowView::OpenPolicies openPolicies);
    void setNavigationParentControl(muse::ui::INavigationControl* parentNavigationControl);
    void setObjectId(QString objectId);
    void setTitle(QString title);
    void setModal(bool modal);
    void setFrameless(bool frameless);
    void setResizable(bool resizable);
    void setAlwaysOnTop(bool alwaysOnTop);
    void setRet(QVariantMap ret);

    void setActivateParentOnClose(bool activateParentOnClose);
    void setFocusPolicies(const muse::uicomponents::WindowView::FocusPolicies& policies);

signals:
    void parentItemChanged();
    void contentItemChanged();
    void contentWidthChanged();
    void contentHeightChanged();
    void windowChanged();
    void openPoliciesChanged(muse::uicomponents::WindowView::OpenPolicies openPolicies);
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

    void activateParentOnCloseChanged(bool activateParentOnClose);
    void focusPoliciesChanged();

    void isContentReadyChanged();

    void parentWindowChanged();

protected:
    virtual bool isDialog() const = 0;
    void classBegin() override;
    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    virtual void initCloseController() {}

    void doFocusOut();
    void windowMoveEvent();

    virtual void beforeOpen();
    void doOpen();

    QWindow* qWindow() const;
    virtual void onHidden();

    virtual void repositionWindowIfNeed() {}

    void setErrCode(Ret::Code code);

    QWindow* parentWindow() const;
    void setParentWindow(QWindow* window);
    void resolveParentWindow();

    virtual QScreen* resolveScreen() const;
    QRect currentScreenGeometry() const;
    virtual void updateGeometry() = 0;

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

    QPointF m_globalPos;

    OpenPolicies m_openPolicies = { OpenPolicy::Default };
    bool m_shouldOpenOnReady = false;
    bool m_isContentReady = false;

    FocusPolicies m_focusPolicies = { FocusPolicy::DefaultFocus };

    bool m_activateParentOnClose = true;
    ui::INavigationControl* m_navigationParentControl = nullptr;
    QString m_objectId;
    QString m_title;
    bool m_modal = true;
    bool m_frameless = false;
    bool m_resizable = false;
    bool m_alwaysOnTop = false;
    QVariantMap m_ret;

    PopupViewCloseController* m_closeController = nullptr;
    bool m_forceClosed = false;
};
}
