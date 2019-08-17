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

namespace Ms {

class Accidental;
class Clef;
class FretDiagram;
enum class FretDotType : signed char;

//---------------------------------------------------------
//   FretCanvas
//---------------------------------------------------------

class FretCanvas : public QFrame {
      Q_OBJECT

      FretDiagram* diagram;
      int cstring;
      int cfret;

      bool _automaticDotType   { true };
      FretDotType _currentDtype;
      bool _barreMode    { false };
      bool _multidotMode { false };

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);

      void paintDotSymbol(QPainter& p, QPen& pen, qreal y, qreal x, qreal dotd, FretDotType dtype);
      void getPosition(const QPointF& pos, int* string, int* fret);

   public:
      FretCanvas(QWidget* parent = 0);
      void setFretDiagram(FretDiagram* fd);

      void setAutomaticDotType(bool v)              { _automaticDotType = v; }
      void setCurrentDotType(FretDotType t)         { _currentDtype = t; }
      void setBarreMode(bool v)                     { _barreMode = v; }
      void setMultidotMode(bool v)                  { _multidotMode = v; }
      void clear();
      
      };


} // namespace Ms
#endif

