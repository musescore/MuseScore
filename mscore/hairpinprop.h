//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: hairpin.h -1   $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __HAIRPINPROP_H__
#define __HAIRPINPROP_H__

#include "libmscore/mscore.h"
#include "ui_hairpinproperties.h"

class Hairpin;

//---------------------------------------------------------
//   HairpinProperties
//---------------------------------------------------------

class HairpinProperties : public QDialog, public Ui::HairpinProperties {
      Q_OBJECT
      Hairpin* hairpin;

   public:
      HairpinProperties(Hairpin*, QWidget* parent = 0);
      int changeVelo() const { return veloChange->value(); }
      DynamicType dynamicType() const;
      bool allowDiagonal() const { return diagonal->isChecked(); }
      };

#endif

