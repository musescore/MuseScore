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

#include "colorlabel.h"

namespace Awl {

//---------------------------------------------------------
//   ColorLabel
//---------------------------------------------------------

ColorLabel::ColorLabel(QWidget* parent)
   : QFrame (parent)
      {
      _color  = Qt::blue;
      _pixmap = 0;
      }

ColorLabel::~ColorLabel()
      {
      delete _pixmap;
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void ColorLabel::setColor(const QColor& c)
      {
      _color = c;
      update();
      }

//---------------------------------------------------------
//   setPixmap
//---------------------------------------------------------

void ColorLabel::setPixmap(QPixmap* pm)
      {
      delete _pixmap;
      _pixmap = pm;
      update();
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize ColorLabel::sizeHint() const
      {
      return QSize(30, 20);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ColorLabel::paintEvent(QPaintEvent* ev)
      {
      {
      QPainter p(this);
      int fw = frameWidth();
      QRect r(frameRect().adjusted(fw, fw, -2*fw, -2*fw));
      if (_pixmap)
            p.drawTiledPixmap(r, *_pixmap);
      else
            p.fillRect(r, _color);
      }
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ColorLabel::mousePressEvent(QMouseEvent*)
      {
      if (_pixmap)
            return;
      QColor c = QColorDialog::getColor(_color, this,
         tr("MuseScore: Select Color"),
         QColorDialog::ShowAlphaChannel
         );
      if (c.isValid()) {
            if (_color != c) {
                  _color = c;
                  emit colorChanged(_color);
                  update();
                  }
            }
      }

} // namespace Awl

