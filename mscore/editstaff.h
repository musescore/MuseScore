//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "libmscore/stafftype.h"

namespace Ms {

class Staff;
class InstrumentTemplate;

//---------------------------------------------------------
//   EditStaff
//    edit staff and part properties
//---------------------------------------------------------

class EditStaff : public QDialog, private Ui::EditStaffBase {
      Q_OBJECT

      Staff*      staff;
      Staff*      orgStaff;
      Instrument  instrument;
      int         _minPitchA, _maxPitchA, _minPitchP, _maxPitchP;
      int         _tickStart, _tickEnd;

      virtual void hideEvent(QHideEvent*);
      void apply();
      void setStaff(Staff*);
      void updateInterval(const Interval&);
      void updateStaffType();
      void updateInstrument();
      void updateNextPreviousButtons();

   protected:
      QString midiCodeToStr(int midiCode);

   private slots:
      void bboxClicked(QAbstractButton* button);
      void editStringDataClicked();
      void showInstrumentDialog();
      void showStaffTypeDialog();
      void minPitchAClicked();
      void maxPitchAClicked();
      void minPitchPClicked();
      void maxPitchPClicked();
      void lineDistanceChanged();
      void numOfLinesChanged();
      void showClefChanged();
      void showTimeSigChanged();
      void showBarlinesChanged();
      void gotoNextStaff();
      void gotoPreviousStaff();

   signals:
      void instrumentChanged();

   public:
      EditStaff(Staff*, int tick, QWidget* parent = 0);
      };


} // namespace Ms
#endif

