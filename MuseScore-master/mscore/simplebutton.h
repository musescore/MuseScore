//=============================================================================
//  MusE
//  Linux Music Editor
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SIMPLE_BUTTON_H__
#define __SIMPLE_BUTTON_H__

namespace Ms {

//---------------------------------------------------------
//   SimpleButton
//---------------------------------------------------------

class SimpleButton : public QToolButton {
      Q_OBJECT

      virtual QSize minimumSizeHint() const { return QSize(0, 0); }

   public:
      SimpleButton(QPixmap* on, QPixmap* off, QWidget* parent = 0);
      SimpleButton(const QString& on, const QString& off, QWidget* parent = 0);
      SimpleButton(const QString& s, QWidget* parent = 0);
      };
}

#endif

