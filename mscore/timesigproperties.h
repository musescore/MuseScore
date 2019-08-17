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

#ifndef __TIMESIGPROPERTIES_H__
#define __TIMESIGPROPERTIES_H__

#include "ui_timesigproperties.h"

namespace Ms {

class TimeSig;

//---------------------------------------------------------
//   TimeSigProperties
//---------------------------------------------------------

class TimeSigProperties : public QDialog, public Ui::TimeSigProperties {
      Q_OBJECT

      TimeSig* timesig;
      virtual void hideEvent(QHideEvent*);
   public slots:
      virtual void accept();

   public:
      TimeSigProperties(TimeSig*, QWidget* parent = 0);
      };
}

#endif
