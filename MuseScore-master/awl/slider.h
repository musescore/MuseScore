//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
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

#ifndef __AWLSLIDER_H__
#define __AWLSLIDER_H__

#include "aslider.h"

namespace Awl {

//---------------------------------------------------------
//    Slider
//
//!   Base class of all slider type input widgets.
//
//!   Inherits from AwlKnob
//!
//---------------------------------------------------------

class Slider : public AbstractSlider {
      Q_OBJECT

      Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)
      Q_PROPERTY(QSize sliderSize READ sliderSize WRITE setSliderSize)

      Qt::Orientation orient;
      QSize _sliderSize;

      QPoint startDrag;
      bool dragMode;
      int dragppos;

      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void paintEvent(QPaintEvent*);
      void init();
      void updateKnob();

   protected:
      QPainterPath* points;
      virtual void mousePressEvent(QMouseEvent*);

   signals:
      void sliderPressed(int);
      void sliderReleased(int);

   public:
      Slider(QWidget* parent = 0);
      Slider(Qt::Orientation orientation, QWidget* parent = 0);
      ~Slider();

      virtual void setOrientation(Qt::Orientation);
      Qt::Orientation orientation() const    { return orient; }

      QSize sliderSize() const           { return _sliderSize; }
      void setSliderSize(const QSize& s);

      virtual void setInvertedAppearance(bool val);
      virtual QSize sizeHint() const;
      };
}

#endif

