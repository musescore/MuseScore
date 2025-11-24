/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include <qqmlintegration.h>

#include "windowview.h"

#include "internal/popupviewclosecontroller.h"

namespace muse::uicomponents {
namespace PopupPosition {
Q_NAMESPACE;
QML_ELEMENT;

enum Type: int {
    Left = 0x01,
    Right = 0x02,
    Horizontal = 0x03,
    Bottom = 0x04,
    Top = 0x08,
    Vertical = 0x0C,
};

Q_ENUM_NS(Type)
}

class PopupView : public WindowView
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(ClosePolicies closePolicies READ closePolicies WRITE setClosePolicies NOTIFY closePoliciesChanged)

    Q_PROPERTY(QQuickItem * anchorItem READ anchorItem WRITE setAnchorItem NOTIFY anchorItemChanged)
    Q_PROPERTY(PlacementPolicies placementPolicies READ placementPolicies WRITE setPlacementPolicies NOTIFY placementPoliciesChanged)
    Q_PROPERTY(muse::uicomponents::PopupPosition::Type popupPosition READ popupPosition WRITE setPopupPosition NOTIFY popupPositionChanged)

    // Relative to parentItem
    Q_PROPERTY(qreal x READ localX WRITE setLocalX NOTIFY xChanged)
    Q_PROPERTY(qreal y READ localY WRITE setLocalY NOTIFY yChanged)

    Q_PROPERTY(bool showArrow READ showArrow WRITE setShowArrow NOTIFY showArrowChanged)
    Q_PROPERTY(int arrowX READ arrowX WRITE setArrowX NOTIFY arrowXChanged)
    Q_PROPERTY(int arrowY READ arrowY WRITE setArrowY NOTIFY arrowYChanged)
    Q_PROPERTY(int padding READ padding WRITE setPadding NOTIFY paddingChanged)

public:
    explicit PopupView(QQuickItem* parent = nullptr);
    ~PopupView() override;

    enum class ClosePolicy {
        NoAutoClose = 0x00000000,
        CloseOnPressOutsideParent = 0x00000001,
    };
    Q_DECLARE_FLAGS(ClosePolicies, ClosePolicy)
    Q_FLAG(ClosePolicies)

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

    void setParentItem(QQuickItem* parent) override;

    ClosePolicies closePolicies() const;
    void setClosePolicies(ClosePolicies closePolicies);

    QQuickItem* anchorItem() const;
    void setAnchorItem(QQuickItem* anchorItem);
    Q_INVOKABLE QRectF anchorGeometry() const;

    PlacementPolicies placementPolicies() const;
    void setPlacementPolicies(PlacementPolicies placementPolicies);

    PopupPosition::Type popupPosition() const;
    void setPopupPosition(PopupPosition::Type position);

    qreal localX() const;
    qreal localY() const;
    void setLocalX(qreal x);
    void setLocalY(qreal y);

    bool showArrow() const;
    void setShowArrow(bool showArrow);

    int arrowX() const;
    void setArrowX(int arrowX);

    int arrowY() const;
    void setArrowY(int arrowY);

    int padding() const;
    void setPadding(int padding);

signals:
    void closePoliciesChanged(muse::uicomponents::PopupView::ClosePolicies closePolicies);

    void anchorItemChanged(QQuickItem* anchorItem);
    void placementPoliciesChanged(muse::uicomponents::PopupView::PlacementPolicies placementPolicies);
    void popupPositionChanged(muse::uicomponents::PopupPosition::Type position);

    void xChanged(qreal x);
    void yChanged(qreal y);

    void showArrowChanged(bool showArrow);
    void arrowXChanged(int arrowX);
    void arrowYChanged(int arrowY);
    void paddingChanged(int padding);

protected:
    void initView() override;

    void beforeOpen() override;
    void onHidden() override;

    bool eventFilter(QObject* watched, QEvent* event) override;

    void initCloseController();

    void repositionWindowIfNeed() override;
    void updateGeometry() override;
    virtual void updateContentPosition();

    QPointF m_localPos;

private:
    ClosePolicies m_closePolicies = { ClosePolicy::CloseOnPressOutsideParent };

    QQuickItem* m_anchorItem = nullptr;
    PlacementPolicies m_placementPolicies = { PlacementPolicy::Default };
    PopupPosition::Type m_popupPosition = { PopupPosition::Bottom };

    bool m_showArrow = false;
    int m_arrowX = 0;
    int m_arrowY = 0;
    int m_padding = 0;

    PopupViewCloseController* m_closeController = nullptr;
};
}
