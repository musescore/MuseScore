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

#ifndef MU_INSPECTOR_FRETCANVAS_H
#define MU_INSPECTOR_FRETCANVAS_H

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVariant>

#include "context/iglobalcontext.h"
#include "fret.h"

namespace mu::inspector {
class FretCanvas : public QQuickPaintedItem
{
    INJECT(instruments, context::IGlobalContext, globalContext)

    Q_OBJECT

    Q_PROPERTY(QVariant diagram READ diagram WRITE setFretDiagram NOTIFY diagramChanged)
    Q_PROPERTY(bool isBarreModeOn READ isBarreModeOn WRITE setIsBarreModeOn NOTIFY isBarreModeOnChanged)
    Q_PROPERTY(bool isMultipleDotsModeOn READ isMultipleDotsModeOn WRITE setIsMultipleDotsModeOn NOTIFY isMultipleDotsModeOnChanged)
    Q_PROPERTY(int currentFretDotType READ currentFretDotType WRITE setCurrentFretDotType NOTIFY currentFretDotTypeChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit FretCanvas(QQuickItem* parent = nullptr);

    Q_INVOKABLE void clear();

    void setFretDiagram(QVariant fd);

    void paint(QPainter* painter) override;

    QVariant diagram() const;

    int currentFretDotType() const;
    bool isBarreModeOn() const;
    bool isMultipleDotsModeOn() const;

    QColor color() const;

public slots:
    void setCurrentFretDotType(int currentFretDotType);
    void setIsBarreModeOn(bool isBarreModeOn);
    void setIsMultipleDotsModeOn(bool isMultipleDotsModeOn);
    void setColor(QColor color);

signals:
    void diagramChanged(QVariant diagram);

    void currentFretDotTypeChanged(int currentFretDotType);
    void isBarreModeOnChanged(bool isBarreModeOn);
    void isMultipleDotsModeOnChanged(bool isMultipleDotsModeOn);

    void colorChanged(QColor color);

private:
    void draw(QPainter* painter);
    void mousePressEvent(QMouseEvent*) override;
    void hoverMoveEvent(QHoverEvent*) override;

    void paintDotSymbol(QPainter* p, QPen& pen, qreal y, qreal x, qreal dotd, Ms::FretDotType dtype);
    void getPosition(const QPointF& pos, int* string, int* fret);

    Ms::FretDiagram* m_diagram = nullptr;

    int m_cstring = 0;
    int m_cfret = 0;

    bool m_automaticDotType = false;
    Ms::FretDotType m_currentDtype = Ms::FretDotType::NORMAL;
    bool m_barreMode = false;
    bool m_multidotMode = false;

    QColor m_color;
};
}

#endif // MU_INSPECTOR_FRETCANVAS_H
