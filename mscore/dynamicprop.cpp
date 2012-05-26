//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: dynamic.cpp -1   $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "libmscore/dynamic.h"
#include "dynamicprop.h"

//---------------------------------------------------------
//   DynamicProperties
//---------------------------------------------------------

DynamicProperties::DynamicProperties(Dynamic* d, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      dynamic = d;
      velocity->setValue(dynamic->velocity());
      switch(dynamic->dynType()) {
            case DYNAMIC_STAFF:     staffButton->setChecked(true); break;
            case DYNAMIC_PART:      partButton->setChecked(true); break;
            case DYNAMIC_SYSTEM:    systemButton->setChecked(true); break;
            }
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void DynamicProperties::accept()
      {
      dynamic->setVelocity(velocity->value());
      if (staffButton->isChecked())
            dynamic->setDynType(DYNAMIC_STAFF);
      else if (partButton->isChecked())
            dynamic->setDynType(DYNAMIC_PART);
      else if (systemButton->isChecked())
            dynamic->setDynType(DYNAMIC_SYSTEM);
      QDialog::accept();
      }

