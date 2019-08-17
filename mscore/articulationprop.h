//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __ARTICULATIONPROP_H__
#define __ARTICULATIONPROP_H__

#include "ui_articulation.h"

class Articulation;

namespace Ms {

//---------------------------------------------------------
//   ArticulationProperties
//    Dialog
//---------------------------------------------------------

class ArticulationProperties : public QDialog, public Ui::ArticulationProperties {
      Q_OBJECT

      Articulation* articulation;
      virtual void hideEvent(QHideEvent*);

   private slots:
      void saveValues();

   public:
      ArticulationProperties(Articulation*, QWidget* parent = 0);
      };


} // namespace Ms
#endif

