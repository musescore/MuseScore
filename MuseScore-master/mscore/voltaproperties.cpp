//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: voltaproperties.cpp 1840 2009-05-20 11:57:51Z wschweer $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "voltaproperties.h"

namespace Ms {

//---------------------------------------------------------
//   VoltaProperties
//---------------------------------------------------------

VoltaProperties::VoltaProperties(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      }

//---------------------------------------------------------
//   getEndings
//---------------------------------------------------------

QList<int> VoltaProperties::getEndings() const
      {
      QList<int> il;
      QString s = repeat->text();
      QStringList sl = s.split(",", QString::SkipEmptyParts);
      foreach(const QString& l, sl) {
            int i = l.simplified().toInt();
            il.append(i);
            }
      return il;
      }

//---------------------------------------------------------
//   setEndings
//---------------------------------------------------------

void VoltaProperties::setEndings(const QList<int>& l)
      {
      QString s;
      foreach(int i, l) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      repeat->setText(s);
      }
}

