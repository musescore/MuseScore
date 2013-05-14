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

#include "slurproperties.h"

namespace Ms {

//---------------------------------------------------------
//   SlurProperties
//---------------------------------------------------------

SlurProperties::SlurProperties(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      }

//---------------------------------------------------------
//   getLineType
//---------------------------------------------------------

int SlurProperties::getLineType() const
      {
         if (dotted->isChecked())
              return 1;
         else if (dashed->isChecked())
              return 2;
         else
              return 0;
      }

//---------------------------------------------------------
//   setLineType
//---------------------------------------------------------

void SlurProperties::setLineType(int val)
      {
      switch (val) {
            case 1:
                  dotted->setChecked(true);
                  break;
            case 2:
                  dashed->setChecked(true);
                  break;
            default:
                  solid->setChecked(true);
            }
      }
}

