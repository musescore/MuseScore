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

#include "editdrumset.h"
#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/text.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "musescore.h"
#include "seq.h"
#include "selinstrument.h"
#include "texteditor.h"

namespace Ms {

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      orgStaff = s;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      Part* part        = orgStaff->part();
      instrument        = *part->instr();
      Score* score      = part->score();
      staff             = new Staff(score);
      staff->setSmall(orgStaff->small());
      staff->setInvisible(orgStaff->invisible());
      staff->setUserDist(orgStaff->userDist());
      staff->setColor(orgStaff->color());
      staff->setStaffType(orgStaff->staffType());
      staff->setPart(part);
      staff->setNeverHide(orgStaff->neverHide());
      staff->setShowIfEmpty(orgStaff->showIfEmpty());
      staff->setUserMag(orgStaff->userMag());

      // hide string data controls if instrument has no strings
      stringDataFrame->setVisible(instrument.stringData() && instrument.stringData()->strings() > 0);
      // set dlg controls
      spinExtraDistance->setValue(s->userDist() / score->spatium());
      invisible->setChecked(staff->invisible());
      small->setChecked(staff->small());
      color->setColor(s->color());
      partName->setText(part->partName());
      neverHide->setChecked(staff->neverHide());
      showIfEmpty->setChecked(staff->showIfEmpty());
      mag->setValue(staff->userMag() * 100.0);
      updateStaffType();
      updateInstrument();

      connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument,     SIGNAL(clicked()),            SLOT(showInstrumentDialog()));
      connect(changeStaffType,      SIGNAL(clicked()),            SLOT(showStaffTypeDialog()));
//      connect(editShortName,        SIGNAL(clicked()),            SLOT(editShortNameClicked()));
//      connect(editLongName,         SIGNAL(clicked()),            SLOT(editLongNameClicked()));
      connect(minPitchASelect,      SIGNAL(clicked()),            SLOT(minPitchAClicked()));
      connect(maxPitchASelect,      SIGNAL(clicked()),            SLOT(maxPitchAClicked()));
      connect(minPitchPSelect,      SIGNAL(clicked()),            SLOT(minPitchPClicked()));
      connect(maxPitchPSelect,      SIGNAL(clicked()),            SLOT(maxPitchPClicked()));
      connect(editStringData,       SIGNAL(clicked()),            SLOT(editStringDataClicked()));
      connect(lines,                SIGNAL(valueChanged(int)),    SLOT(numOfLinesChanged()));
      connect(lineDistance,         SIGNAL(valueChanged(double)), SLOT(lineDistanceChanged()));
      connect(showClef,             SIGNAL(clicked()),            SLOT(showClefChanged()));
      connect(showTimesig,          SIGNAL(clicked()),            SLOT(showTimeSigChanged()));
      connect(showBarlines,         SIGNAL(clicked()),            SLOT(showBarlinesChanged()));
      }

//---------------------------------------------------------
//   updateStaffType
//---------------------------------------------------------

void EditStaff::updateStaffType()
      {
      StaffType* staffType = staff->staffType();
      lines->setValue(staffType->lines());
      lineDistance->setValue(staffType->lineDistance().val());
      showClef->setChecked(staffType->genClef());
      showTimesig->setChecked(staffType->genTimesig());
      showBarlines->setChecked(staffType->showBarlines());
      staffGroupName->setText(qApp->translate("Staff type group name", staffType->groupName()));
      }

//---------------------------------------------------------
//   updateInstrument
//---------------------------------------------------------

void EditStaff::updateInstrument()
      {
      updateInterval(instrument.transpose());

      QList<StaffName>& snl = instrument.shortNames();
      QString df = snl.isEmpty() ? "" : snl[0].name;
      shortName->setPlainText(df);

      QList<StaffName>& lnl = instrument.longNames();
      df = lnl.isEmpty() ? "" : lnl[0].name;

      longName->setPlainText(df);

      if (partName->text() == instrumentName->text())    // Updates part name if no custom name has been set before
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
//   updateInterval
//---------------------------------------------------------

void EditStaff::updateInterval(const Interval& iv)
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
            qDebug("EditStaff: unknown interval %d %d", diatonic, chromatic);
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
                  if (staff != nullptr)
                        delete staff;
                  break;

            default:
                  qDebug("EditStaff: unknown button %d", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStaff::apply()
      {
      Score* score  = orgStaff->score();
      Part* part    = orgStaff->part();

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
      Text text(0);
//      instrument.setShortName(text.convertFromHtml(shortName->toHtml()));
      instrument.setShortName(shortName->toPlainText());
//      instrument.setLongName(text.convertFromHtml(longName->toHtml()));
      instrument.setLongName(longName->toPlainText());

      bool s         = small->isChecked();
      bool inv       = invisible->isChecked();
      qreal userDist = spinExtraDistance->value();
      QColor col     = color->color();
      bool nhide     = neverHide->isChecked();
      bool ifEmpty   = showIfEmpty->isChecked();
      qreal scale    = mag->value() / 100.0;

      QString newPartName = partName->text().simplified();
      if (!(instrument == *part->instr()) || part->partName() != newPartName) {
            Interval v1 = instrument.transpose();
            Interval v2 = part->instr()->transpose();

            score->undo(new ChangePart(part, instrument, newPartName));
            emit instrumentChanged();

            if (v1 != v2)
                  score->transpositionChanged(part);
            }

      if (s != staff->small()
         || inv != staff->invisible()
         || userDist != staff->userDist()
         || col != staff->color()
         || nhide != staff->neverHide()
         || ifEmpty != staff->showIfEmpty()
         || scale != staff->userMag()
         ) {
            score->undo(new ChangeStaff(orgStaff, s, inv, userDist * score->spatium(), col, nhide, ifEmpty, scale));
            }

      if ( !(*orgStaff->staffType() == *staff->staffType()) ) {
            // updateNeeded |= (orgStaff->staffGroup() == StaffGroup::TAB || staff->staffGroup() == StaffGroup::TAB);
            score->undo()->push(new ChangeStaffType(orgStaff, *staff->staffType()));
            }

      score->update();
      score->updateChannel();
      }

//---------------------------------------------------------
//   <Pitch>Clicked
//---------------------------------------------------------

void EditStaff::minPitchAClicked()
      {
      int         newCode;
      EditPitch* ep = new EditPitch(this, instrument.minPitchA() );
      ep->setWindowModality(Qt::WindowModal);
      if ( (newCode=ep->exec()) != -1) {
            minPitchA->setText(midiCodeToStr(newCode));
            _minPitchA = newCode;
            }
      }

void EditStaff::maxPitchAClicked()
      {
      int         newCode;
      EditPitch* ep = new EditPitch(this, instrument.maxPitchA() );
      ep->setWindowModality(Qt::WindowModal);
      if ( (newCode=ep->exec()) != -1) {
            maxPitchA->setText(midiCodeToStr(newCode));
            _maxPitchA = newCode;
            }
      }

void EditStaff::minPitchPClicked()
      {
      int         newCode;
      EditPitch* ep = new EditPitch(this, instrument.minPitchP() );
      ep->setWindowModality(Qt::WindowModal);
      if ( (newCode=ep->exec()) != -1) {
            minPitchP->setText(midiCodeToStr(newCode));
            _minPitchP = newCode;
            }
      }

void EditStaff::maxPitchPClicked()
      {
      int         newCode;
      EditPitch* ep = new EditPitch(this, instrument.maxPitchP() );
      ep->setWindowModality(Qt::WindowModal);
      if ( (newCode=ep->exec()) != -1) {
            maxPitchP->setText(midiCodeToStr(newCode));
            _maxPitchP = newCode;
            }
      }

//---------------------------------------------------------
//   StaffType props slots
//---------------------------------------------------------

void EditStaff::lineDistanceChanged()
      {
      staff->staffType()->setLineDistance(Spatium(lineDistance->value()));
      }

void EditStaff::numOfLinesChanged()
      {
      staff->staffType()->setLines(lines->value());
      }

void EditStaff::showClefChanged()
      {
      staff->staffType()->setGenClef(showClef->checkState() == Qt::Checked);
      }

void EditStaff::showTimeSigChanged()
      {
      staff->staffType()->setGenTimesig(showTimesig->checkState() == Qt::Checked);
      }

void EditStaff::showBarlinesChanged()
      {
      staff->staffType()->setShowBarlines(showBarlines->checkState() == Qt::Checked);
      }

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(instrument, this);
      si.setWindowModality(Qt::WindowModal);
      if (si.exec()) {
            instrument = Instrument::fromTemplate(si.instrTemplate());
            updateInstrument();
            }
      }

//---------------------------------------------------------
//   editStringDataClicked
//---------------------------------------------------------

void EditStaff::editStringDataClicked()
      {
      int                     frets = instrument.stringData()->frets();
      QList<instrString>      stringList = instrument.stringData()->stringList();

      EditStringData* esd = new EditStringData(this, &stringList, &frets);
      esd->setWindowModality(Qt::WindowModal);
      if (esd->exec()) {
            StringData stringData(frets, stringList);

            // update instrument pitch ranges as necessary
            if (stringList.size() > 0) {
                  // get new string range bottom and top
                  // as we have to choose an int size, INT16 are surely beyond midi pitch limits
                  int oldHighestStringPitch     = INT16_MIN;
                  int highestStringPitch        = INT16_MIN;
                  int lowestStringPitch         = INT16_MAX;
                  for (const instrString& str : stringList) {
                        if (str.pitch > highestStringPitch) highestStringPitch = str.pitch;
                        if (str.pitch < lowestStringPitch)  lowestStringPitch  = str.pitch;
                        }
                  // get old string range bottom
                  for (const instrString& str : instrument.stringData()->stringList())
                        if (str.pitch > oldHighestStringPitch) oldHighestStringPitch = str.pitch;
                  // if there were no string, arbitrarely set old top to maxPitchA
                  if (oldHighestStringPitch == INT16_MIN)
                        oldHighestStringPitch = instrument.maxPitchA();

                  // range bottom is surely the pitch of the lowest string
                  instrument.setMinPitchA(lowestStringPitch);
                  instrument.setMinPitchP(lowestStringPitch);
                  // range top should keep the same interval with the highest string it has now
                  instrument.setMaxPitchA(instrument.maxPitchA() + highestStringPitch - oldHighestStringPitch);
                  instrument.setMaxPitchP(instrument.maxPitchP() + highestStringPitch - oldHighestStringPitch);
                  // update dlg controls
                  minPitchA->setText(midiCodeToStr(instrument.minPitchA()));
                  maxPitchA->setText(midiCodeToStr(instrument.maxPitchA()));
                  minPitchP->setText(midiCodeToStr(instrument.minPitchP()));
                  maxPitchP->setText(midiCodeToStr(instrument.maxPitchP()));
                  // if no longer there is any string, leave everything as it is now
                  }

            // update instrument data and dlg controls
            instrument.setStringData(stringData);
            numOfStrings->setText(QString::number(stringData.strings()));
            }
      }

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------

static const char* g_cNoteName[] = {
      QT_TRANSLATE_NOOP("editstaff", "C"),
      QT_TRANSLATE_NOOP("editstaff", "C#"),
      QT_TRANSLATE_NOOP("editstaff", "D"),
      QT_TRANSLATE_NOOP("editstaff", "Eb"),
      QT_TRANSLATE_NOOP("editstaff", "E"),
      QT_TRANSLATE_NOOP("editstaff", "F"),
      QT_TRANSLATE_NOOP("editstaff", "F#"),
      QT_TRANSLATE_NOOP("editstaff", "G"),
      QT_TRANSLATE_NOOP("editstaff", "Ab"),
      QT_TRANSLATE_NOOP("editstaff", "A"),
      QT_TRANSLATE_NOOP("editstaff", "Bb"),
      QT_TRANSLATE_NOOP("editstaff", "B")
      };

QString EditStaff::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(qApp->translate("editstaff", g_cNoteName[midiCode % 12])).arg(midiCode / 12 - 1);
      }

//---------------------------------------------------------
//   showStaffTypeDialog
//---------------------------------------------------------

void EditStaff::showStaffTypeDialog()
      {
      EditStaffType editor(this, staff);
      editor.setWindowModality(Qt::WindowModal);
      if (editor.exec()) {
            staff->setStaffType(editor.getStaffType());
            updateStaffType();
            }
      }

}

