//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __BENDPROPERTIES_H__
#define __BENDPROPERTIES_H__

#include "ui_bend.h"
#include "libmscore/pitchvalue.h"

namespace Ms {

class Bend;

//---------------------------------------------------------
//   BendProperties
//---------------------------------------------------------

class BendProperties : public QDialog, public Ui::BendDialog {
      Q_OBJECT
      Bend* bend;
      QButtonGroup* bendTypes;

      virtual void hideEvent(QHideEvent*);

   private slots:
      void bendTypeChanged(int);

   public:
      BendProperties(Bend*, QWidget* parent = 0);
      const QList<PitchValue>& points() const;
      };


} // namespace Ms
#endif

