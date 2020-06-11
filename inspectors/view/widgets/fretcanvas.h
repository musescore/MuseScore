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

namespace Ms {

class Accidental;
class Clef;

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

class FretCanvas : public QQuickPaintedItem {
      Q_OBJECT

      Q_PROPERTY(QVariant diagram READ diagram WRITE setFretDiagram NOTIFY diagramChanged)
      Q_PROPERTY(bool isBarreModeOn READ isBarreModeOn WRITE setIsBarreModeOn NOTIFY isBarreModeOnChanged)
      Q_PROPERTY(bool isMultipleDotsModeOn READ isMultipleDotsModeOn WRITE setIsMultipleDotsModeOn NOTIFY isMultipleDotsModeOnChanged)
      Q_PROPERTY(int currentFretDotType READ currentFretDotType WRITE setCurrentFretDotType NOTIFY currentFretDotTypeChanged)

      int cstring          { 0 };
      int cfret            { 0 };

      bool _automaticDotType    { false };
      FretDotType _currentDtype = FretDotType::NORMAL;
      bool _barreMode    { false };
      bool _multidotMode { false };

      void draw(QPainter* painter);
      void mousePressEvent(QMouseEvent*) override;
      void mouseMoveEvent(QMouseEvent*) override;

      void paintDotSymbol(QPainter *p, QPen& pen, qreal y, qreal x, qreal dotd, FretDotType dtype);
      void getPosition(const QPointF& pos, int* string, int* fret);

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
      FretDiagram* m_diagram = nullptr;
};

} // namespace Ms

#endif

