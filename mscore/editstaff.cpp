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

#include "editstaff.h"

#include "editdrumset.h"
#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"
#include "icons.h"
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

namespace Ms {

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, const Fraction& tick, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("EditStaff");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      staff = nullptr;
      setStaff(s, tick);

      MuseScore::restoreGeometry(this);

      connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument,     SIGNAL(clicked()),            SLOT(showInstrumentDialog()));
      connect(changeStaffType,      SIGNAL(clicked()),            SLOT(showStaffTypeDialog()));
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
      connect(invisible,            SIGNAL(clicked()),            SLOT(invisibleChanged()));
      connect(nextButton,           SIGNAL(clicked()),            SLOT(gotoNextStaff()));
      connect(previousButton,       SIGNAL(clicked()),            SLOT(gotoPreviousStaff()));

      connect(iList,                SIGNAL(currentIndexChanged(int)),  SLOT(transpositionChanged()));

      nextButton->setIcon(*icons[int(Icons::arrowDown_ICON)]);
      previousButton->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      minPitchASelect->setIcon(*icons[int(Icons::edit_ICON)]);
      maxPitchASelect->setIcon(*icons[int(Icons::edit_ICON)]);
      minPitchPSelect->setIcon(*icons[int(Icons::edit_ICON)]);
      maxPitchPSelect->setIcon(*icons[int(Icons::edit_ICON)]);

      addAction(getAction("help"));  // why is this needed?
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void EditStaff::setStaff(Staff* s, const Fraction& tick)
      {
      if (staff != nullptr)
            delete staff;

      orgStaff = s;
      Part* part        = orgStaff->part();
      instrument        = *part->instrument(tick);
      Score* score      = part->score();
      staff             = new Staff(score);
      StaffType* stt = staff->setStaffType(Fraction(0,1), *orgStaff->staffType(Fraction(0,1)));
      stt->setSmall(orgStaff->staffType(Fraction(0,1))->isSmall());
      stt->setInvisible(orgStaff->staffType(Fraction(0,1))->invisible());
      staff->setUserDist(orgStaff->userDist());
      stt->setColor(orgStaff->staffType(Fraction(0,1))->color());
      staff->setPart(part);
      staff->setCutaway(orgStaff->cutaway());
      staff->setHideWhenEmpty(orgStaff->hideWhenEmpty());
      staff->setShowIfEmpty(orgStaff->showIfEmpty());
      stt->setUserMag(orgStaff->staffType(Fraction(0,1))->userMag());
      staff->setHideSystemBarLine(orgStaff->hideSystemBarLine());
      staff->setMergeMatchingRests(orgStaff->mergeMatchingRests());

      // get tick range for instrument
      auto i = part->instruments()->upper_bound(tick.ticks());
      if (i == part->instruments()->end())
            _tickEnd = Fraction(-1,1);
      else
            _tickEnd = Fraction::fromTicks(i->first);
      --i;
      if (i == part->instruments()->begin())
            _tickStart = Fraction(-1, 1);
      else
            _tickStart = Fraction::fromTicks(i->first);

      // set dlg controls
      spinExtraDistance->setValue(s->userDist() / score->spatium());
      invisible->setChecked(staff->invisible(Fraction(0,1)));
      isSmallCheckbox->setChecked(stt->isSmall());
      color->setColor(stt->color());
      partName->setText(part->partName());
      cutaway->setChecked(staff->cutaway());
      hideMode->setCurrentIndex(int(staff->hideWhenEmpty()));
      showIfEmpty->setChecked(staff->showIfEmpty());
      hideSystemBarLine->setChecked(staff->hideSystemBarLine());
      mergeMatchingRests->setChecked(staff->mergeMatchingRests());
      mag->setValue(stt->userMag() * 100.0);
      updateStaffType();
      updateInstrument();
      updateNextPreviousButtons();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void EditStaff::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }

//---------------------------------------------------------
//   updateStaffType
//---------------------------------------------------------

void EditStaff::updateStaffType()
      {
      const StaffType* staffType = staff->staffType(Fraction(0,1));
      lines->setValue(staffType->lines());
      lineDistance->setValue(staffType->lineDistance().val());
      showClef->setChecked(staffType->genClef());
      showTimesig->setChecked(staffType->genTimesig());
      showBarlines->setChecked(staffType->showBarlines());
      invisible->setChecked(staffType->invisible());
      staffGroupName->setText(qApp->translate("Staff type group name", staffType->groupName()));
      }

//---------------------------------------------------------
//   updateInstrument
//---------------------------------------------------------

void EditStaff::updateInstrument()
      {
      updateInterval(instrument.transpose());

      QList<StaffName>& snl = instrument.shortNames();
      QString df = snl.isEmpty() ? "" : snl[0].name();
      shortName->setPlainText(df);

      QList<StaffName>& lnl = instrument.longNames();
      df = lnl.isEmpty() ? "" : lnl[0].name();

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
      color_2->setColor(instrument.getNameColor());
      singleNoteDynamics->setChecked(instrument.singleNoteDynamics());

      // only show string data controls if instrument has strings
      int numStr = instrument.stringData() ? instrument.stringData()->strings() : 0;
      stringDataFrame->setVisible(numStr > 0);
      numOfStrings->setText(QString::number(numStr));

      // show transp_PreferSharpFlat if instrument isn't non-transposing or octave-transposing
      bool showPreferSharpFlat = (iList->currentIndex() != 0) && (iList->currentIndex() != 25);
      transp_PreferSharpFlat->setVisible(showPreferSharpFlat);
      preferSharpFlat->setCurrentIndex(int(orgStaff->part()->preferSharpFlat()));
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
//   updateNextPreviousButtons
//---------------------------------------------------------

void EditStaff::updateNextPreviousButtons()
      {
      int staffIdx = orgStaff->idx();

      nextButton->setEnabled(staffIdx < (orgStaff->score()->nstaves() - 1));
      previousButton->setEnabled(staffIdx != 0);
      }

//---------------------------------------------------------
//   gotoNextStaff
//---------------------------------------------------------

void EditStaff::gotoNextStaff()
      {
      Staff* nextStaff = orgStaff->score()->staff(orgStaff->idx() + 1);
      if (nextStaff)
            {
            setStaff(nextStaff, _tickStart);
            }
      }

//---------------------------------------------------------
//   gotoPreviousStaff
//---------------------------------------------------------

void EditStaff::gotoPreviousStaff()
      {
      Staff* prevStaff = orgStaff->score()->staff(orgStaff->idx() - 1);
      if (prevStaff)
            {
            setStaff(prevStaff, _tickStart);
            }
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

      QString sn = shortName->toPlainText();
      QString ln = longName->toPlainText();
      if (!Text::validateText(sn) || !Text::validateText(ln)) {
            QMessageBox msgBox;
            msgBox.setText(tr("The instrument name is invalid."));
            msgBox.exec();
            return;
            }
      shortName->setPlainText(sn);  // show the fixed text
      longName->setPlainText(ln);

      int intervalIdx = iList->currentIndex();
      bool upFlag     = up->isChecked();

      Interval interval  = intervalList[intervalIdx];
      interval.diatonic  += octave->value() * 7;
      interval.chromatic += octave->value() * 12;

      if (!upFlag)
            interval.flip();
      instrument.setTranspose(interval);

      bool preferSharpFlatChanged = (part->preferSharpFlat() != PreferSharpFlat(preferSharpFlat->currentIndex()));
      // instrument becomes non/octave-transposing, preferSharpFlat isn't useful anymore
      if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25))
            part->undoChangeProperty(Pid::PREFER_SHARP_FLAT, int(PreferSharpFlat::DEFAULT));
      else
            part->undoChangeProperty(Pid::PREFER_SHARP_FLAT, int(PreferSharpFlat(preferSharpFlat->currentIndex())));

      instrument.setMinPitchA(_minPitchA);
      instrument.setMaxPitchA(_maxPitchA);
      instrument.setMinPitchP(_minPitchP);
      instrument.setMaxPitchP(_maxPitchP);

      instrument.setShortName(sn);
      instrument.setLongName(ln);
      instrument.setNameColor(color_2->color());

      instrument.setSingleNoteDynamics(singleNoteDynamics->isChecked());

      bool inv       = invisible->isChecked();
      ClefTypeList clefType = orgStaff->defaultClefType();
      qreal userDist = spinExtraDistance->value();
      bool ifEmpty   = showIfEmpty->isChecked();
      bool hideSystemBL = hideSystemBarLine->isChecked();
      bool mergeRests = mergeMatchingRests->isChecked();
      bool cutAway      = cutaway->isChecked();
      Staff::HideMode hideEmpty = Staff::HideMode(hideMode->currentIndex());

      QString newPartName = partName->text().simplified();

      bool instrumentFieldChanged = !(instrument == *part->instrument(_tickStart));
      if (instrumentFieldChanged && _tickStart == Fraction(-1, 1))
            clefType = instrument.clefType(orgStaff->rstaff());

      Interval v1 = instrument.transpose();
      Interval v2 = part->instrument(_tickStart)->transpose();

      if (instrumentFieldChanged || part->partName() != newPartName) {
            // instrument has changed

            if (_tickStart == Fraction(-1, 1)) {
                  // change instrument and part name globally
                  score->undo(new ChangePart(part, new Instrument(instrument), newPartName));
                  }
            else {
                  // change part name globally, instrument locally if possible
                  if (part->partName() != newPartName)
                        score->undo(new ChangePart(part, new Instrument(*part->instrument()), newPartName));  //tick?
                  if (instrumentFieldChanged) {
                        Segment* s = score->tick2segment(_tickStart, true, SegmentType::ChordRest);
                        const std::vector<Element*> elist = s ? s->findAnnotations(ElementType::INSTRUMENT_CHANGE, part->startTrack(), part->endTrack() - 1) : std::vector<Element*>();
                        if (elist.size())
                              for (Element* e : elist) // Change instrument in all Instrument Changes (for linked staves)
                                    score->undo(new ChangeInstrument(toInstrumentChange(e), new Instrument(instrument)));
                        else
                              score->undo(new ChangePart(part, new Instrument(instrument), newPartName));
                        }
                  }
            emit instrumentChanged();

            if (v1 != v2)
                  score->transpositionChanged(part, v2, _tickStart, _tickEnd);
            }

      if (preferSharpFlatChanged)
            score->transpositionChanged(part, v2, _tickStart, _tickEnd);

      orgStaff->undoChangeProperty(Pid::MAG, mag->value() / 100.0);
      orgStaff->undoChangeProperty(Pid::STAFF_COLOR, color->color());
      orgStaff->undoChangeProperty(Pid::SMALL, isSmallCheckbox->isChecked());

      if (inv != orgStaff->invisible(Fraction(0,1))
         || clefType != orgStaff->defaultClefType()
         || userDist != orgStaff->userDist()
         || cutAway != orgStaff->cutaway()
         || hideEmpty != orgStaff->hideWhenEmpty()
         || ifEmpty != orgStaff->showIfEmpty()
         || hideSystemBL != orgStaff->hideSystemBarLine()
         || mergeRests != orgStaff->mergeMatchingRests()
         ) {
            score->undo(new ChangeStaff(orgStaff, inv, clefType, userDist * score->spatium(), hideEmpty, ifEmpty, cutAway, hideSystemBL, mergeRests));
            }

      if ( !(*orgStaff->staffType(Fraction(0,1)) == *staff->staffType(Fraction(0,1))) ) {
            // updateNeeded |= (orgStaff->staffGroup() == StaffGroup::TAB || staff->staffGroup() == StaffGroup::TAB);
            score->undo(new ChangeStaffType(orgStaff, *staff->staffType(Fraction(0,1))));
            }

      score->update();
      score->masterScore()->updateChannel();
      staff->score()->masterScore()->rebuildAndUpdateExpressive(MuseScore::synthesizer("Fluid"));
      }

//---------------------------------------------------------
//   Slots
//---------------------------------------------------------

void EditStaff::minPitchAClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.minPitchA());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            minPitchA->setText(midiCodeToStr(newCode));
            _minPitchA = newCode;
            }
      }

void EditStaff::maxPitchAClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.maxPitchA());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            maxPitchA->setText(midiCodeToStr(newCode));
            _maxPitchA = newCode;
            }
      }

void EditStaff::minPitchPClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.minPitchP());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            minPitchP->setText(midiCodeToStr(newCode));
            _minPitchP = newCode;
            }
      }

void EditStaff::maxPitchPClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.maxPitchP());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            maxPitchP->setText(midiCodeToStr(newCode));
            _maxPitchP = newCode;
            }
      }

void EditStaff::lineDistanceChanged()
      {
      staff->staffType(Fraction(0,1))->setLineDistance(Spatium(lineDistance->value()));
      }

void EditStaff::numOfLinesChanged()
      {
      staff->staffType(Fraction(0,1))->setLines(lines->value());
      }

void EditStaff::showClefChanged()
      {
      staff->staffType(Fraction(0,1))->setGenClef(showClef->checkState() == Qt::Checked);
      }

void EditStaff::showTimeSigChanged()
      {
      staff->staffType(Fraction(0,1))->setGenTimesig(showTimesig->checkState() == Qt::Checked);
      }

void EditStaff::showBarlinesChanged()
      {
      staff->staffType(Fraction(0,1))->setShowBarlines(showBarlines->checkState() == Qt::Checked);
      }

void EditStaff::invisibleChanged()
      {
      staff->staffType(Fraction(0,1))->setInvisible(invisible->checkState() == Qt::Checked);
      }

void EditStaff::transpositionChanged()
      {
      // non-transposing or octave-transposing instrument
      // don't show transp_preferSharpFlat
      if ((iList->currentIndex() == 0) || (iList->currentIndex() == 25))
            transp_PreferSharpFlat->setVisible(false);
      else
            transp_PreferSharpFlat->setVisible(true);
      }

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(&instrument, this);
      si.setWindowModality(Qt::WindowModal);
      if (si.exec()) {
            instrument = Instrument::fromTemplate(si.instrTemplate());
            const StaffType* staffType = si.instrTemplate()->staffTypePreset;
            if (!staffType)
                  staffType = StaffType::getDefaultPreset(StaffGroup::STANDARD);
            staff->setStaffType(Fraction(0,1), *staffType);
            updateStaffType();
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
      QT_TRANSLATE_NOOP("editstaff", "C♯"),
      QT_TRANSLATE_NOOP("editstaff", "D"),
      QT_TRANSLATE_NOOP("editstaff", "E♭"),
      QT_TRANSLATE_NOOP("editstaff", "E"),
      QT_TRANSLATE_NOOP("editstaff", "F"),
      QT_TRANSLATE_NOOP("editstaff", "F♯"),
      QT_TRANSLATE_NOOP("editstaff", "G"),
      QT_TRANSLATE_NOOP("editstaff", "A♭"),
      QT_TRANSLATE_NOOP("editstaff", "A"),
      QT_TRANSLATE_NOOP("editstaff", "B♭"),
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
            staff->setStaffType(Fraction(0,1), *editor.getStaffType());
            updateStaffType();
            }
      }
}

