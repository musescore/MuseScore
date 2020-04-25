//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "mssplashscreen.h"

namespace Ms {

//---------------------------------------------------------
//   drawContents
//---------------------------------------------------------

void MsSplashScreen::drawContents(QPainter* painter)
      {
      qreal width  = static_cast<qreal>(QWidget::width());
      qreal height = static_cast<qreal>(QWidget::height());
      QRectF rect  = QRectF(0.0, 0.65 * height, width, 0.35 * height);

      painter->setPen(QColor(255, 255, 255, 255 * 0.8));
      painter->drawText(rect, Qt::AlignTop | Qt::AlignHCenter, _message);
      }

//---------------------------------------------------------
//   showMessage
//---------------------------------------------------------

void MsSplashScreen::showMessage(const QString& message)
      {
      _message = message;
      // The align flags and color don't matter here as drawContents() is overwritten
      QSplashScreen::showMessage(message, Qt::AlignTop | Qt::AlignHCenter, QColor(255, 255, 255, 255 * 0.8));
      }

}
