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

#ifndef __TREMOLOBARPROP_H__
#define __TREMOLOBARPROP_H__

#include "libmscore/element.h"
#include "ui_tremolobar.h"
#include "libmscore/pitchvalue.h"

namespace Ms {

class Painter;
class TremoloBar;

//---------------------------------------------------------
//   TremoloBarProperties
//---------------------------------------------------------

class TremoloBarProperties : public QDialog, public Ui::TremoloBarDialog {
      Q_OBJECT
      TremoloBar* bend;
      QButtonGroup* bendTypes;

      virtual void hideEvent(QHideEvent*);
   private slots:
      void bendTypeChanged(int);

   public:
      TremoloBarProperties(TremoloBar*, QWidget* parent = 0);
      const QList<PitchValue>& points() const;
      };
}

#endif

