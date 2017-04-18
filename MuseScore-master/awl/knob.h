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

#ifndef __AWLKNOB_H__
#define __AWLKNOB_H__

#include "aslider.h"
// #include <QtDesigner/QDesignerExportWidget>

namespace Awl {

//---------------------------------------------------------
//    Knob
//
//!   Base class of all dialer type input widgets.
//
//!   xxxxxxxxx
//!
//---------------------------------------------------------

class Q_DECL_EXPORT Knob : public AbstractSlider {
      Q_OBJECT
      Q_PROPERTY(int scaleSize READ scaleSize WRITE setScaleSize)
      Q_PROPERTY(int markSize READ markSize WRITE setMarkSize)
      Q_PROPERTY(int border READ border WRITE setBorder)
      Q_PROPERTY(QString text READ text WRITE setText)

      int _scaleSize;         //! scale size in degrees
      int _markSize;
      int _border;
      QPainterPath* points;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);

   protected:
      int startY;
      QString _text;

   signals:
      void sliderPressed(int);
      void sliderReleased(int);

   public:
      Knob(QWidget* parent = 0);
      ~Knob();
      virtual QSize sizeHint() const { return QSize(50, 50); }
      virtual int heightForWidth(int w) const { return w; }

      //! return text decoration
      QString text() const           { return _text; }
      void setText(const QString& s);

      //! return scale size in degrees
      int scaleSize() const          { return _scaleSize; }
      void setScaleSize(int val);
      int markSize() const           { return _markSize; }
      void setMarkSize(int val);
      int border() const             { return _border; }
      void setBorder(int val);
      };

}

#endif

