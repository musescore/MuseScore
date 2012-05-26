//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstaff.h 4953 2011-11-04 13:04:28Z wschweer $
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

#ifndef __EDITSTAFF_H__
#define __EDITSTAFF_H__

#include "ui_editstaff.h"
#include "ui_selectinstr.h"
#include "libmscore/instrument.h"

class Staff;
struct InstrumentTemplate;

//---------------------------------------------------------
//   EditStaff
//    edit staff and part properties
//---------------------------------------------------------

class EditStaff : public QDialog, private Ui::EditStaffBase {
      Q_OBJECT

      Staff*      staff;
      Instrument  instrument;
      int         _minPitchA, _maxPitchA, _minPitchP, _maxPitchP;

      void apply();
      void setInterval(const Interval&);
      void updateInstrument();

   protected:
      QString midiCodeToStr(int midiCode);

   private slots:
      void bboxClicked(QAbstractButton* button);
      void editDrumsetClicked();
      void editStringDataClicked();
      void showInstrumentDialog();
      void showEditStaffType();
      void editShortNameClicked();
      void editLongNameClicked();
      void minPitchAClicked();
      void maxPitchAClicked();
      void minPitchPClicked();
      void maxPitchPClicked();

   signals:
      void instrumentChanged();

   public:
      EditStaff(Staff*, QWidget* parent = 0);
      };

#endif

