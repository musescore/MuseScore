//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: hairpin.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "hairpinprop.h"
#include "libmscore/hairpin.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"

//---------------------------------------------------------
//   HairpinProperties
//---------------------------------------------------------

HairpinProperties::HairpinProperties(Hairpin* h, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      hairpin = h;
      veloChange->setValue(hairpin->veloChange());
      int tick1 = static_cast<Segment*>(hairpin->startElement())->tick();
      int velo = hairpin->staff()->velocities().velo(tick1);
      beginVelocity->setValue(velo);
      diagonal->setChecked(h->diagonal());
      }

//---------------------------------------------------------
//   dynamicType
//---------------------------------------------------------

DynamicType HairpinProperties::dynamicType() const
      {
      if (staffButton->isChecked())
            return DYNAMIC_STAFF;
      if (systemButton->isChecked())
            return DYNAMIC_SYSTEM;
      // default:
      return DYNAMIC_PART;
      }

