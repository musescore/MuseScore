//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2007 by Werner Schweer and others
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

#ifndef __AWLCOLORLABEL_H__
#define __AWLCOLORLABEL_H__

namespace Awl {

//---------------------------------------------------------
//   ColorLabel
//---------------------------------------------------------

class ColorLabel : public QFrame {
      Q_OBJECT
      Q_PROPERTY(QColor color READ color WRITE setColor)

      QColor _color;
      QPixmap* _pixmap;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);

   signals:
      void colorChanged(QColor);

   public:
      ColorLabel(QWidget* parent = 0);
      ~ColorLabel();
      void setColor(const QColor& c);
      virtual QSize sizeHint() const;
      void setPixmap(QPixmap*);
      QColor color() const     { return _color; }
      QPixmap* pixmap() const  { return _pixmap;  }
      };

}  // namespace Awl
#endif

