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

#include "popupview.h"

#include <QTimer>

namespace muse::uicomponents {
class MenuView : public PopupView
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int contentWidth READ contentWidth WRITE setContentWidth NOTIFY contentWidthChanged)
    Q_PROPERTY(int contentHeight READ contentHeight WRITE setContentHeight NOTIFY contentHeightChanged)

    Q_PROPERTY(Qt::AlignmentFlag cascadeAlign READ cascadeAlign WRITE setCascadeAlign NOTIFY cascadeAlignChanged)

    // Amazon triangle properties for event filtering
    Q_PROPERTY(QPointF triangleP1 READ triangleP1 WRITE setTriangleP1 NOTIFY triangleP1Changed)
    Q_PROPERTY(QPointF triangleP2 READ triangleP2 WRITE setTriangleP2 NOTIFY triangleP2Changed)
    Q_PROPERTY(QPointF triangleP3 READ triangleP3 WRITE setTriangleP3 NOTIFY triangleP3Changed)
    Q_PROPERTY(bool amazonTriangleActive READ amazonTriangleActive WRITE setAmazonTriangleActive NOTIFY amazonTriangleActiveChanged)

public:
    explicit MenuView(QQuickItem* parent = nullptr);
    ~MenuView() override = default;

    Q_INVOKABLE int viewVerticalMargin() const;

    Qt::AlignmentFlag cascadeAlign() const;

    int contentWidth() const;
    void setContentWidth(int newContentWidth);

    int contentHeight() const;
    void setContentHeight(int newContentHeight);

    // Amazon triangle accessors
    QPointF triangleP1() const;
    void setTriangleP1(const QPointF& point);

    QPointF triangleP2() const;
    void setTriangleP2(const QPointF& point);

    QPointF triangleP3() const;
    void setTriangleP3(const QPointF& point);

    bool amazonTriangleActive() const;
    void setAmazonTriangleActive(bool active);

    // Amazon triangle management
    Q_INVOKABLE void setSubMenuGeometry(const QRectF& geometry);
    Q_INVOKABLE void clearSubMenuGeometry();
    Q_INVOKABLE void onMouseMove(const QPointF& position);
    Q_INVOKABLE bool isMouseInsideTriangle(const QPointF& mousePos) const;

public slots:
    void setCascadeAlign(Qt::AlignmentFlag cascadeAlign);

signals:
    void cascadeAlignChanged(Qt::AlignmentFlag cascadeAlign);

    void contentWidthChanged();
    void contentHeightChanged();

    void triangleP1Changed();
    void triangleP2Changed();
    void triangleP3Changed();
    void amazonTriangleActiveChanged();

private:
    void componentComplete() override;

    void updateGeometry() override;
    void updateContentPosition() override;

    QRect viewGeometry() const override;

    Qt::AlignmentFlag parentCascadeAlign(const QQuickItem* parent) const;

    QQuickItem* parentMenuContentItem() const;

private slots:
    void onMouseStoppedTimer();

protected:
    void initCloseController() override;

private:
    void updateCloseControllerTriangle();
    void updateTriangle(const QPointF& mousePos);
    void calculateTriangleVertices(const QPointF& p1);

    Qt::AlignmentFlag m_cascadeAlign = Qt::AlignmentFlag::AlignRight;

    int m_contentWidth = -1;
    int m_contentHeight = -1;

    // Amazon triangle coordinates
    QPointF m_triangleP1;
    QPointF m_triangleP2;
    QPointF m_triangleP3;
    bool m_amazonTriangleActive = false;

    // Mouse tracking for amazon triangle
    QPointF m_lastMousePos;
    qint64 m_lastMouseMoveTime = 0;
    QTimer* m_mouseStoppedTimer = nullptr;
    bool m_mouseStopped = false;

    // Submenu geometry for triangle calculation
    QRectF m_subMenuGeometry;
    bool m_hasSubMenuOpen = false;
};
}
