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

#ifndef __TUPLETDIALOG_H__
#define __TUPLETDIALOG_H__

#include "libmscore/duration.h"
#include "ui_tupletdialog.h"

namespace Ms {

//---------------------------------------------------------
//   TupletDialog
//---------------------------------------------------------

class TupletDialog : public QDialog, Ui::TupletDialog {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);
   public:
      TupletDialog(QWidget* parent = 0);
      void setupTuplet(Tuplet* tuplet);
      int getNormalNotes() const { return normalNotes->value(); }
      };
}

#endif

