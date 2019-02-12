//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#ifndef __STYLEDSLIDER_H__
#define __STYLEDSLIDER_H__

#include <QWidget>
#include <QIcon>


namespace Awl {

class StyledSlider : public QWidget
      {
      Q_OBJECT

      QList<double> marks;

      double _minValue = 0;
      double _maxValue = 127;
      double _value;
      double _barThickness = 4;
      double _margin = 20;
      int _numMajorTicks = 10;
      int _numMinorTicks = 4;
      double _majorTickWidth = 30;
      double _minorTickWidth = 10;

      QColor _backgroundColor = QColor(10, 10, 10);
      QColor _hilightColor = QColor(0, 255, 0);
      QColor _tickColor = QColor(150, 150, 150);

      bool draggingMouse;
      QPoint mouseDownPos;
      double mouseDownVal;

      QIcon _sliderHeadIcon;

public:
      explicit StyledSlider(QWidget *parent = nullptr);
      virtual QSize sizeHint() const { return QSize(50, 50); }
      virtual void paintEvent(QPaintEvent *ev);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void keyPressEvent(QKeyEvent*);

      double maxValue() const { return _maxValue; }
      double minValue() const { return _minValue; }
      double value() const { return _value; }
      double barThickness() const { return _barThickness; }
      double margin() const { return _margin; }
      double numMajorTicks() const { return _numMajorTicks; }
      int numMinorTicks() const { return _numMinorTicks; }
      double majorTickWidth() const { return _majorTickWidth; }
      double minorTickWidth() const { return _minorTickWidth; }
      QColor backgroundColor() const { return _backgroundColor; }
      QColor hilightColor() const { return _hilightColor; }
      QColor tickColor() const { return _tickColor; }
      QIcon sliderHeadIcon() const { return _sliderHeadIcon; }
      void addMark(double value) { marks.append(value); }

signals:
      void valueChanged(double);
      void minValueChanged(double);
      void maxValueChanged(double);
      void sliderPressed();

public slots:
      void setValue(double v);
      void setMinValue(double v);
      void setMaxValue(double v);
      void setBarThickness(double v);
      void setMargin(double v);
      void setNumMajorTicks(int v);
      void setNumMinorTicks(int v);
      void setMajorTickWidth(double v);
      void setMinorTickWidth(double v);

      void setBackgroundColor(QColor v);
      void setHilightColor(QColor v);
      void setTickColor(QColor v);
      void setSliderHeadIcon(QIcon v);
      };

}
#endif // __STYLEDSLIDER_H__
