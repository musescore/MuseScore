//=============================================================================
//  MusE
//  Linux Music Editor
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

#include "simplebutton.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   SimpleButton
//---------------------------------------------------------

SimpleButton::SimpleButton(const QString& on, const QString& off, QWidget* parent)
   : QToolButton(parent)
      {
      setAutoRaise(true);
      QIcon icon(off);
      icon.addFile(on,  QSize(), QIcon::Normal, QIcon::On);
      icon.addFile(on,  QSize(), QIcon::Active, QIcon::On);
      icon.addFile(off, QSize(), QIcon::Normal, QIcon::Off);
      QAction* a = new QAction(this);
      a->setIcon(icon);
      setDefaultAction(a);
      }

//---------------------------------------------------------
//   SimpleButton
//---------------------------------------------------------

SimpleButton::SimpleButton(QPixmap* on, QPixmap* off, QWidget* parent)
   : QToolButton(parent)
      {
      setAutoRaise(true);
      QIcon icon(*off);
      icon.addPixmap(*on, QIcon::Normal, QIcon::On);
      QAction* a = new QAction(this);
      a->setIcon(icon);
      setDefaultAction(a);
      }

//---------------------------------------------------------
//   SimpleButton
//---------------------------------------------------------

SimpleButton::SimpleButton(const QString& s, QWidget* parent)
   : QToolButton(parent)
      {
      setAutoRaise(false);
      setText(s);
      }
}

