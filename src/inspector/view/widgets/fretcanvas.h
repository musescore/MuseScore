//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __FRETCANVAS_H__
#define __FRETCANVAS_H__

#include <QQuickPaintedItem>
#include <QPainter>
#include <QVariant>

#include "fret.h"

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

class FretCanvas : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant diagram READ diagram WRITE setFretDiagram NOTIFY diagramChanged)
    Q_PROPERTY(bool isBarreModeOn READ isBarreModeOn WRITE setIsBarreModeOn NOTIFY isBarreModeOnChanged)
    Q_PROPERTY(bool isMultipleDotsModeOn READ isMultipleDotsModeOn WRITE setIsMultipleDotsModeOn NOTIFY isMultipleDotsModeOnChanged)
    Q_PROPERTY(int currentFretDotType READ currentFretDotType WRITE setCurrentFretDotType NOTIFY currentFretDotTypeChanged)

public:
    explicit FretCanvas(QQuickItem* parent = nullptr);

    Q_INVOKABLE void clear();

    void setFretDiagram(QVariant fd);

    void paint(QPainter* painter) override;

    QVariant diagram() const;

    int currentFretDotType() const;
    bool isBarreModeOn() const;
    bool isMultipleDotsModeOn() const;

public slots:
    void setCurrentFretDotType(int currentFretDotType);
    void setIsBarreModeOn(bool isBarreModeOn);
    void setIsMultipleDotsModeOn(bool isMultipleDotsModeOn);

signals:
    void diagramChanged(QVariant diagram);

    void currentFretDotTypeChanged(int currentFretDotType);
    void isBarreModeOnChanged(bool isBarreModeOn);
    void isMultipleDotsModeOnChanged(bool isMultipleDotsModeOn);

private:
    void draw(QPainter* painter);
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

    void paintDotSymbol(QPainter* p, QPen& pen, qreal y, qreal x, qreal dotd, Ms::FretDotType dtype);
    void getPosition(const QPointF& pos, int* string, int* fret);

    Ms::FretDiagram* m_diagram = nullptr;

    int m_cstring = 0;
    int m_cfret = 0;

    bool m_automaticDotType = false;
    Ms::FretDotType m_currentDtype = Ms::FretDotType::NORMAL;
    bool m_barreMode = false;
    bool m_multidotMode = false;
};

#endif
