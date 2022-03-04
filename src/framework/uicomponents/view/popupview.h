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

#ifndef MU_UICOMPONENTS_POPUPVIEW_H
#define MU_UICOMPONENTS_POPUPVIEW_H

#include <QQuickItem>
#include <QQmlParserStatus>

#include "ret.h"
#include "modularity/ioc.h"
#include "ui/imainwindow.h"
#include "ui/iuiconfiguration.h"
#include "ui/inavigationcontroller.h"
#include "ui/view/navigationcontrol.h"
#include "popupwindow/ipopupwindow.h"

class QQuickCloseEvent;

namespace mu::uicomponents {
class PopupView : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QQuickItem * parent READ parentItem WRITE setParentItem NOTIFY parentItemChanged)
    Q_PROPERTY(QQuickItem * contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged)
    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(QWindow * window READ window NOTIFY windowChanged)

    //! NOTE Local, related parent
    Q_PROPERTY(qreal x READ localX WRITE setLocalX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ localY WRITE setLocalY NOTIFY yChanged)

    Q_PROPERTY(bool showArrow READ showArrow WRITE setShowArrow NOTIFY showArrowChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

    Q_PROPERTY(QQuickItem * anchorItem READ anchorItem WRITE setAnchorItem NOTIFY anchorItemChanged)
    Q_PROPERTY(bool opensUpward READ opensUpward NOTIFY opensUpwardChanged)
    Q_PROPERTY(int arrowX READ arrowX WRITE setArrowX NOTIFY arrowXChanged)
    Q_PROPERTY(Qt::AlignmentFlag cascadeAlign READ cascadeAlign WRITE setCascadeAlign NOTIFY cascadeAlignChanged)

    Q_PROPERTY(bool isOpened READ isOpened NOTIFY isOpenedChanged)
    Q_PROPERTY(ClosePolicy closePolicy READ closePolicy WRITE setClosePolicy NOTIFY closePolicyChanged)

    //! NOTE We use QObject (instead ui::NavigationControl) for avoid add  UI module dependency at link time.
    //! Itself not bad for uicomponents, but we have dependency uicomponents - instruments - libmscore - imports - tests
    //! So, add ui dependency is bad and problems with compilation
    Q_PROPERTY(QObject * navigationParentControl
               READ navigationParentControl
               WRITE setNavigationParentControl
               NOTIFY navigationParentControlChanged
               )

    //! NOTE Used for dialogs, but be here so that dialogs and just popups have one api
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString objectId READ objectId WRITE setObjectId NOTIFY objectIdChanged)
    Q_PROPERTY(bool modal READ modal WRITE setModal NOTIFY modalChanged)
    Q_PROPERTY(bool resizable READ resizable WRITE setResizable NOTIFY resizableChanged)
    Q_PROPERTY(QVariantMap ret READ ret WRITE setRet NOTIFY retChanged)

    Q_ENUMS(ClosePolicy)

    INJECT(uicomponents, ui::IMainWindow, mainWindow)
    INJECT(uicomponents, ui::IUiConfiguration, uiConfiguration)
    INJECT(uicomponents, ui::INavigationController, navigationController)

public:

    explicit PopupView(QQuickItem* parent = nullptr);
    ~PopupView() override = default;

    enum ClosePolicy {
        NoAutoClose = 0,
        CloseOnPressOutsideParent
    };

    QQuickItem* parentItem() const;
    QQuickItem* contentItem() const;

    QWindow* window() const;

    qreal localX() const;
    qreal localY() const;
    QRect geometry() const;

    Q_INVOKABLE void forceActiveFocus();

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void toggleOpened();

    Q_INVOKABLE void setParentWindow(QWindow* window);

    ClosePolicy closePolicy() const;
    QObject* navigationParentControl() const;

    bool isOpened() const;

    QString objectId() const;
    QString title() const;
    bool modal() const;
    bool resizable() const;
    QVariantMap ret() const;

    bool opensUpward() const;
    int arrowX() const;
    Qt::AlignmentFlag cascadeAlign() const;
    int padding() const;
    bool showArrow() const;
    QQuickItem* anchorItem() const;

    int contentWidth() const;
    void setContentWidth(int newContentWidth);

    int contentHeight() const;
    void setContentHeight(int newContentHeight);

public slots:
    void setParentItem(QQuickItem* parent);
    void setContentItem(QQuickItem* content);
    void setLocalX(qreal x);
    void setLocalY(qreal y);
    void setClosePolicy(ClosePolicy closePolicy);
    void setNavigationParentControl(QObject* parentNavigationControl);
    void setObjectId(QString objectId);
    void setTitle(QString title);
    void setModal(bool modal);
    void setResizable(bool resizable);
    void setRet(QVariantMap ret);

    void setOpensUpward(bool opensUpward);
    void setArrowX(int arrowX);
    void setCascadeAlign(Qt::AlignmentFlag cascadeAlign);
    void setPadding(int padding);
    void setShowArrow(bool showArrow);
    void setAnchorItem(QQuickItem* anchorItem);

signals:
    void parentItemChanged();
    void contentItemChanged();
    void windowChanged();
    void xChanged(qreal x);
    void yChanged(qreal y);
    void closePolicyChanged(ClosePolicy closePolicy);
    void navigationParentControlChanged(QObject* navigationParentControl);
    void objectIdChanged(QString objectId);
    void titleChanged(QString title);
    void modalChanged(bool modal);
    void resizableChanged(bool resizable);
    void retChanged(QVariantMap ret);

    void isOpenedChanged();
    void opened();
    void aboutToClose(QQuickCloseEvent* closeEvent);
    void closed();

    void opensUpwardChanged(bool opensUpward);
    void arrowXChanged(int arrowX);
    void cascadeAlignChanged(Qt::AlignmentFlag cascadeAlign);
    void paddingChanged(int padding);
    void showArrowChanged(bool showArrow);
    void anchorItemChanged(QQuickItem* anchorItem);

    void contentWidthChanged();
    void contentHeightChanged();

private slots:
    void onApplicationStateChanged(Qt::ApplicationState state);

protected:
    virtual bool isDialog() const;
    void classBegin() override;
    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    void doFocusOut();

    bool isMouseWithinBoundaries(const QPoint& mousePos) const;

    QWindow* qWindow() const;
    virtual void beforeShow();
    virtual void onHidden();

    void repositionWindowIfNeed();

    void setErrCode(Ret::Code code);

    QRect currentScreenGeometry() const;
    void updatePosition();
    void updateContentPosition();

    QQuickItem* parentPopupContentItem() const;
    Qt::AlignmentFlag parentCascadeAlign(const QQuickItem* parent) const;

    QRectF anchorGeometry() const;

    IPopupWindow* m_window = nullptr;

    QQuickItem* m_contentItem = nullptr;
    int m_contentWidth = 0;
    int m_contentHeight = 0;

    QQuickItem* m_anchorItem = nullptr;

    QPointF m_localPos;
    QPointF m_globalPos;
    ClosePolicy m_closePolicy = ClosePolicy::CloseOnPressOutsideParent;
    QObject* m_navigationParentControl = nullptr;
    QString m_objectId;
    QString m_title;
    bool m_modal = true;
    bool m_resizable = false;
    QVariantMap m_ret;
    bool m_opensUpward = false;
    int m_arrowX = 0;
    Qt::AlignmentFlag m_cascadeAlign = Qt::AlignmentFlag::AlignRight;
    int m_padding = 0;
    bool m_showArrow = false;
};
}

#endif // MU_UICOMPONENTS_POPUPVIEW_H
