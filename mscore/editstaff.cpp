//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstaff.cpp 5149 2011-12-29 08:38:43Z wschweer $
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

#include "editstaff.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "editdrumset.h"
#include "libmscore/score.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/text.h"
#include "libmscore/utils.h"
#include "libmscore/instrtemplate.h"
#include "seq.h"
#include "musescore.h"
#include "libmscore/stafftype.h"
#include "selinstrument.h"
#include "texteditor.h"
#include "editpitch.h"
#include "editstringdata.h"
#include "libmscore/tablature.h"

namespace Ms {

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      Part* part = staff->part();
      instrument = *part->instr();
      Score* score = part->score();

      // hide string data controls if instrument has no strings
      stringDataFrame->setVisible(instrument.stringData() && instrument.stringData()->strings() > 0);
      fillStaffTypeCombo();
      small->setChecked(staff->small());
      invisible->setChecked(staff->invisible());
      spinExtraDistance->setValue(s->userDist() / score->spatium());
      partName->setText(part->partName());

      updateInstrument();

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument, SIGNAL(clicked()), SLOT(showInstrumentDialog()));
      connect(editShortName,    SIGNAL(clicked()), SLOT(editShortNameClicked()));
      connect(editLongName,     SIGNAL(clicked()), SLOT(editLongNameClicked()));
      connect(minPitchASelect,  SIGNAL(clicked()), SLOT(minPitchAClicked()));
      connect(maxPitchASelect,  SIGNAL(clicked()), SLOT(maxPitchAClicked()));
      connect(minPitchPSelect,  SIGNAL(clicked()), SLOT(minPitchPClicked()));
      connect(maxPitchPSelect,  SIGNAL(clicked()), SLOT(maxPitchPClicked()));
      connect(editStringData,   SIGNAL(clicked()), SLOT(editStringDataClicked()));
      }

//---------------------------------------------------------
//   fillStaffTypecombo
//---------------------------------------------------------

void EditStaff::fillStaffTypeCombo()
      {
      Score* score   = staff->score();
      int curIdx     = 0;
      int n          = score->staffTypes().size();
      // can this instrument accept tabs or drum set?
      bool canUseTabs = instrument.stringData() && instrument.stringData()->strings() > 0;
      bool canUsePerc = instrument.useDrumset();
      staffType->clear();
      for (int idx = 0; idx < n; ++idx) {
            StaffType* st = score->staffType(idx);
            if ( (canUseTabs && st->group() == TAB_STAFF_GROUP)
                        || ( canUsePerc && st->group() == PERCUSSION_STAFF_GROUP)
                        || (!canUsePerc && st->group() == STANDARD_STAFF_GROUP) ) {
                  staffType->addItem(st->name(), idx);
                  if (st == staff->staffType())
                        curIdx = staffType->count() - 1;
                  }
            }
      staffType->setCurrentIndex(curIdx);
      }

//---------------------------------------------------------
//   updateInstrument
//---------------------------------------------------------

void EditStaff::updateInstrument()
      {
      setInterval(instrument.transpose());

      QList<StaffNameDoc>& nl = instrument.shortNames();
      QTextDocumentFragment df = nl.isEmpty() ? QTextDocumentFragment() : nl[0].name;
      shortName->setHtml(df.toHtml());

      nl = instrument.longNames();
      df = nl.isEmpty() ? QTextDocumentFragment() : nl[0].name;
      longName->setHtml(df.toHtml());

      if (partName->text() == instrumentName->text())    // Updates part name is no custom name has been set before
            partName->setText(instrument.trackName());

      instrumentName->setText(instrument.trackName());

      _minPitchA = instrument.minPitchA();
      _maxPitchA = instrument.maxPitchA();
      _minPitchP = instrument.minPitchP();
      _maxPitchP = instrument.maxPitchP();
      minPitchA->setText(midiCodeToStr(_minPitchA));
      maxPitchA->setText(midiCodeToStr(_maxPitchA));
      minPitchP->setText(midiCodeToStr(_minPitchP));
      maxPitchP->setText(midiCodeToStr(_maxPitchP));

      int numStr = instrument.stringData() ? instrument.stringData()->strings() : 0;
      numOfStrings->setText(QString::number(numStr));
      }

//---------------------------------------------------------
//   setInterval
//---------------------------------------------------------

void EditStaff::setInterval(const Interval& iv)
      {
      int diatonic  = iv.diatonic;
      int chromatic = iv.chromatic;

      int oct = chromatic / 12;
      if (oct < 0)
            oct = -oct;

      bool upFlag = true;
      if (chromatic < 0 || diatonic < 0) {
            upFlag    = false;
            chromatic = -chromatic;
            diatonic  = -diatonic;
            }
      chromatic %= 12;
      diatonic  %= 7;

      int interval = searchInterval(diatonic, chromatic);
      if (interval == -1) {
            qDebug("EditStaff: unknown interval %d %d\n", diatonic, chromatic);
            interval = 0;
            }
      iList->setCurrentIndex(interval);
      up->setChecked(upFlag);
      down->setChecked(!upFlag);
      octave->setValue(oct);
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void EditStaff::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  qDebug("EditStaff: unknown button %d\n", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStaff::apply()
      {
      Score* score  = staff->score();
      Part* part    = staff->part();

      int intervalIdx = iList->currentIndex();
      bool upFlag     = up->isChecked();

      Interval interval  = intervalList[intervalIdx];
      interval.diatonic  += octave->value() * 7;
      interval.chromatic += octave->value() * 12;

      if (!upFlag)
            interval.flip();
      instrument.setTranspose(interval);

      instrument.setMinPitchA(_minPitchA);
      instrument.setMaxPitchA(_maxPitchA);
      instrument.setMinPitchP(_minPitchP);
      instrument.setMaxPitchP(_maxPitchP);
      instrument.setShortName(QTextDocumentFragment(shortName->document()));
      instrument.setLongName(QTextDocumentFragment(longName->document()));

      bool s            = small->isChecked();
      bool inv          = invisible->isChecked();
      qreal userDist    = spinExtraDistance->value();
      int staffIdx      = staffType->itemData(staffType->currentIndex()).toInt();
      StaffType* st     = score->staffType(staffIdx);


      // before changing instrument, check if notes need to be updated
      // true if changing into or away from TAB or from one TAB type to another

      StaffGroup ng = st->group();                          // new staff group
      StaffGroup og = staff->staffType()->group();          // old staff group

      bool updateNeeded = (ng == TAB_STAFF_GROUP && og != TAB_STAFF_GROUP) ||
                          (ng != TAB_STAFF_GROUP && og == TAB_STAFF_GROUP) ||
                          (ng == TAB_STAFF_GROUP && og == TAB_STAFF_GROUP
                             && instrument.stringData() != part->instr()->stringData());

      if (!(instrument == *part->instr()) || part->partName() != partName->text()) {
            score->undo(new ChangePart(part, instrument, partName->text()));
            emit instrumentChanged();
            }

      if (s != staff->small() || inv != staff->invisible() || userDist != staff->userDist() || st  != staff->staffType())
            score->undo(new ChangeStaff(staff, s, inv, userDist * score->spatium(), st));

      if (updateNeeded)
            score->cmdUpdateNotes();

      score->setLayoutAll(true);
      score->update();
      }

//---------------------------------------------------------
//   editDrumsetClicked
//---------------------------------------------------------

void EditStaff::editDrumsetClicked()
      {
      EditDrumset dse(staff->part()->instr()->drumset(), this);
      dse.exec();
      }

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(instrument, this);
      if (si.exec()) {
            instrument = Instrument::fromTemplate(si.instrTemplate());
            updateInstrument();
            fillStaffTypeCombo();
            }
      }

//---------------------------------------------------------
//   editShortNameClicked
//---------------------------------------------------------

void EditStaff::editShortNameClicked()
      {
      QString s = editHtml(shortName->toHtml(), tr("Edit Short Name"));
      shortName->setHtml(s);
      }

//---------------------------------------------------------
//   editLongNameClicked
//---------------------------------------------------------

void EditStaff::editLongNameClicked()
      {
      QString s = editHtml(longName->toHtml(), tr("Edit Long Name"));
      longName->setHtml(s);
      }

//---------------------------------------------------------
//   <Pitch>Clicked
//---------------------------------------------------------

void EditStaff::minPitchAClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.minPitchA() );
      if ( (newCode=ep->exec()) != -1) {
            minPitchA->setText(midiCodeToStr(newCode));
            _minPitchA = newCode;
            }
      }

void EditStaff::maxPitchAClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.maxPitchA() );
      if ( (newCode=ep->exec()) != -1) {
            maxPitchA->setText(midiCodeToStr(newCode));
            _maxPitchA = newCode;
            }
      }

void EditStaff::minPitchPClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.minPitchP() );
      if ( (newCode=ep->exec()) != -1) {
            minPitchP->setText(midiCodeToStr(newCode));
            _minPitchP = newCode;
            }
      }

void EditStaff::maxPitchPClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.maxPitchP() );
      if ( (newCode=ep->exec()) != -1) {
            maxPitchP->setText(midiCodeToStr(newCode));
            _maxPitchP = newCode;
            }
      }

//---------------------------------------------------------
//   editStringDataClicked
//---------------------------------------------------------

void EditStaff::editStringDataClicked()
      {
      int         frets = instrument.stringData()->frets();
      QList<int>  stringList = instrument.stringData()->stringList();

      EditStringData* esd = new EditStringData(this, &stringList, &frets);
      if (esd->exec()) {
            StringData* stringData = new StringData(frets, stringList);
            // detect number of strings going from 0 to !0 or vice versa
            bool redoStaffTypeCombo =
                  (stringList.size() != 0) != (instrument.stringData()->strings() != 0);
            instrument.setStringData(stringData);
            int numStr = stringData ? stringData->strings() : 0;
            numOfStrings->setText(QString::number(numStr));
            if (redoStaffTypeCombo)
                  fillStaffTypeCombo();
            }
      }

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------

static char g_cNoteName[12][4] =
{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

QString EditStaff::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(g_cNoteName[midiCode % 12]).arg(midiCode / 12 - 1);
      }
}

