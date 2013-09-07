//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "editstafftype.h"
#include "libmscore/stafftype.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "musescore.h"
#include "navigator.h"
#include "scoreview.h"

namespace Ms {

extern Score::FileError readScore(Score* score, QString name, bool ignoreVersionError);

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

EditStaffType::EditStaffType(QWidget* parent, Staff* st)
   : QDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      staff = st;
      Score* score = staff->score();
      // copy types from score and set as non-built-in
      foreach(StaffType** const st, score->staffTypes())
             staffTypes.append((*st)->clone());
      foreach(StaffType* const st, staffTypes)
             st->setBuiltin(false);
      int idx = 0;
      QListWidgetItem* ci = 0;
      foreach(StaffType* st, staffTypes) {
            QListWidgetItem* item = new QListWidgetItem(st->name());
            item->setData(Qt::UserRole, idx);
            if (st == staff->staffType())
                  ci = item;
            staffTypeList->addItem(item);
            ++idx;
            }

      // init presets
      int numOfPreset = StaffType::numOfPresets();
      for (int i=0; i < numOfPreset; ++i) {
            const StaffType* st = StaffType::preset(i);
            switch (st->group()) {
                  case TAB_STAFF_GROUP:
                        presetTablatureCombo->addItem(st->name(), i);
                        break;
                  case PERCUSSION_STAFF_GROUP:
                        presetPercCombo->addItem(st->name(), i);
                        break;
                  default:
                        break;
                  }
            }
      presetPercCombo->addItem(tr("Custom"), -1);
      presetTablatureCombo->addItem(tr("Custom"), -1);

      // tab page configuration
      tabDetails->hide();                       // start tabulature page in simple mode
      QList<QString> fontNames = StaffTypeTablature::fontNames(false);
      foreach(const QString& name, fontNames)   // fill fret font name combo
            fretFontName->addItem(name);
      fretFontName->setCurrentIndex(0);
      fontNames = StaffTypeTablature::fontNames(true);
      foreach(const QString& name, fontNames)   // fill duration font name combo
            durFontName->addItem(name);
      durFontName->setCurrentIndex(0);
      // load a sample tabulature score in preview
      Score* sc = new Score(MScore::defaultStyle());
      tabPreview = 0;
      if (readScore(sc, QString(":/data/tab_sample.mscx"), false) == Score::FILE_NO_ERROR) {
            // add a preview widget to tabulature page
#ifdef _USE_NAVIGATOR_PREVIEW_
            NScrollArea* sa = new NScrollArea;
            tabPreview = new Navigator(sa, this);
            static_cast<QVBoxLayout*>(groupPreview->layout())->insertWidget(0, sa);
#else
            tabPreview = new ScoreView(this);
            static_cast<QVBoxLayout*>(groupPreview->layout())->insertWidget(0, tabPreview);
#endif
            tabPreview->setScore(sc);
            }

      connect(staffTypeList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(typeChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(newTypePitched,       SIGNAL(clicked()),            SLOT(createNewType()));
      connect(newTypeTablature,     SIGNAL(clicked()),            SLOT(createNewType()));
      connect(newTypePercussion,    SIGNAL(clicked()),            SLOT(createNewType()));
      connect(name,           SIGNAL(textEdited(const QString&)), SLOT(nameEdited(const QString&)));
      connect(presetTablatureCombo, SIGNAL(currentIndexChanged(int)), SLOT(presetTablatureChanged(int)));
      connect(presetPercCombo,      SIGNAL(currentIndexChanged(int)), SLOT(presetPercChanged(int)));

      if (ci)
            staffTypeList->setCurrentItem(ci);

      connect(lines,          SIGNAL(valueChanged(int)),          SLOT(updatePreviews()));
      connect(lineDistance,   SIGNAL(valueChanged(double)),       SLOT(updatePreviews()));
      connect(showBarlines,   SIGNAL(toggled(bool)),              SLOT(updatePreviews()));
      connect(genClef,        SIGNAL(toggled(bool)),              SLOT(updatePreviews()));
      connect(genTimesig,     SIGNAL(toggled(bool)),              SLOT(updatePreviews()));

      connect(genKeysigPercussion,       SIGNAL(toggled(bool)),   SLOT(updatePercPreview()));
      connect(showLedgerLinesPercussion, SIGNAL(toggled(bool)),   SLOT(updatePercPreview()));
      connect(stemlessPercussion,        SIGNAL(toggled(bool)),   SLOT(updatePercPreview()));

      connect(noteValuesSymb, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(noteValuesStems,SIGNAL(toggled(bool)),              SLOT(tabStemsToggled(bool)));
      connect(stemBesideRadio,SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(stemThroughRadio,SIGNAL(toggled(bool)),             SLOT(tabStemThroughToggled(bool)));
      connect(stemAboveRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(stemBelowRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
//    connect(minimNoneRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(minimShortRadio,SIGNAL(toggled(bool)),              SLOT(tabMinimShortToggled(bool)));
      connect(minimSlashedRadio,SIGNAL(toggled(bool)),            SLOT(updateTabPreview()));
      connect(showRests,      SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(durFontName,    SIGNAL(currentIndexChanged(int)),   SLOT(durFontNameChanged(int)));
      connect(durFontSize,    SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(durY,           SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(fretFontName,   SIGNAL(currentIndexChanged(int)),   SLOT(fretFontNameChanged(int)));
      connect(fretFontSize,   SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(fretY,          SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(linesThroughRadio, SIGNAL(toggled(bool)),           SLOT(updateTabPreview()));
      connect(onLinesRadio,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(upsideDown,     SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(numbersRadio,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));

      modified = false;
      }

EditStaffType::~EditStaffType()
      {
      foreach(StaffType* st, staffTypes)
            delete st;
      }

//---------------------------------------------------------
//   saveCurrent
//---------------------------------------------------------

void EditStaffType::saveCurrent(QListWidgetItem* o)
      {
      bool        modif = false;                            // assume no modifications
      int         idx   = o->data(Qt::UserRole).toInt();
      StaffType*  st    = staffTypes[idx];

      // if any of the common properties is modified
      if (name->text()              != st->name()
         || st->lines()             != lines->value()
         || st->lineDistance().val()!= lineDistance->value()
         || st->genClef()           != genClef->isChecked()
         || st->showBarlines()      != showBarlines->isChecked()
         || st->genTimesig()        != genTimesig->isChecked()
         ) {
            modif = true;
            }

      // or if any of the props specific to each group is modified
      switch(st->group()) {
            case STANDARD_STAFF_GROUP:
                  {
                  StaffTypePitched* sp = static_cast<StaffTypePitched*>(st);
                  if (sp->genKeysig()         != genKeysigPitched->isChecked()
                     || sp->showLedgerLines() != showLedgerLinesPitched->isChecked()
                     || st->slashStyle()      != stemlessPitched->isChecked()
                     ) {
                        modif = true;
                        }
                  }
                  break;

            case TAB_STAFF_GROUP:
                  {
                  StaffTypeTablature*  stt = static_cast<StaffTypeTablature*>(st);
                  TablatureMinimStyle minimStyle = minimNoneRadio->isChecked() ? TAB_MINIM_NONE :
                        (minimShortRadio->isChecked() ? TAB_MINIM_SHORTER : TAB_MINIM_SLASHED);
                  if (stt->durationFontName()    != durFontName->currentText()
                     || stt->durationFontSize() != durFontSize->value()
                     || stt->durationFontUserY()!= durY->value()
                     || stt->fretFontName()     != fretFontName->currentText()
                     || stt->fretFontSize()     != fretFontSize->value()
                     || stt->fretFontUserY()    != fretY->value()
                     || stt->linesThrough()     != linesThroughRadio->isChecked()
                     || stt->onLines()          != onLinesRadio->isChecked()
                     || stt->upsideDown()       != upsideDown->isChecked()
                     || stt->useNumbers()       != numbersRadio->isChecked()
                     || ( noteValuesNone->isChecked() && (!stt->slashStyle() ||  stt->genDurations()) )
                     || ( noteValuesSymb->isChecked() && (!stt->slashStyle() || !stt->genDurations()) )
                     // if stems, there are more values to take into account
                     || ( noteValuesStems->isChecked()&& ( stt->slashStyle() ||  stt->genDurations()
                              || stt->stemsDown()     != stemBelowRadio->isChecked()
                              || stt->stemThrough()   != stemThroughRadio->isChecked()
                              || stt->minimStyle()    != minimStyle)
                          )
                     || stt->showRests()        != showRests->isChecked()
                     ) {
                        modif = true;
                        }
                  }
                  break;

            case PERCUSSION_STAFF_GROUP:
                  {
                  StaffTypePercussion* sp = static_cast<StaffTypePercussion*>(st);
                  if (sp->genKeysig()         != genKeysigPercussion->isChecked()
                     || sp->showLedgerLines() != showLedgerLinesPercussion->isChecked()
                     || st->slashStyle()      != stemlessPercussion->isChecked()
                     ) {
                        modif = true;
                        }
                  }
                  break;
            }

      if (modif) {
            // save common properties
            // save-group specific properties
            if(name->text().isEmpty()) {
                  QString n = createUniqueStaffTypeName(st->group());
                  name->setText(n);
                  o->setText(n);
                  }
            switch(st->group()) {
                  case STANDARD_STAFF_GROUP:
                        {
                        StaffTypePitched* stp = static_cast<StaffTypePitched*>(st);
                        stp->setName(name->text());
                        stp->setLines(lines->value());
                        stp->setLineDistance(Spatium(lineDistance->value()));
                        stp->setShowBarlines(showBarlines->isChecked());
                        stp->setGenClef(genClef->isChecked());
                        stp->setGenTimesig(genTimesig->isChecked());
                        stp->setGenKeysig(genKeysigPitched->isChecked());
                        stp->setShowLedgerLines(showLedgerLinesPitched->isChecked());
                        stp->setSlashStyle(stemlessPitched->isChecked());
                        }
                        break;
                  case TAB_STAFF_GROUP:
                        {
                        StaffTypeTablature*  stt = static_cast<StaffTypeTablature*>(st);
                        setTabFromDlg(stt);
                        }
                        break;
                  case PERCUSSION_STAFF_GROUP:
                        {
                        StaffTypePercussion* stp = static_cast<StaffTypePercussion*>(st);
                        setPercFromDlg(stp);
                        }
                        break;
                  }
            modified = true;
            }
      }

//---------------------------------------------------------
//   typeChanged
//---------------------------------------------------------

void EditStaffType::typeChanged(QListWidgetItem* n, QListWidgetItem* o)
      {
      if (n == 0)
            return;
      if (o)
            saveCurrent(o);
      // retrieve staff type corresponding to new current item in type list
      int idx = n->data(Qt::UserRole).toInt();
      StaffType* st = staffTypes[idx];

      // switch to stack page and set props specific to each staff group

      switch(st->group()) {
            case STANDARD_STAFF_GROUP:
                  {
                  StaffTypePitched* ps = static_cast<StaffTypePitched*>(st);
                  stack->setCurrentIndex(0);
                  name->setText(st->name());
                  lines->setValue(st->lines());
                  lineDistance->setValue(st->lineDistance().val());
                  genClef->setChecked(st->genClef());
                  showBarlines->setChecked(st->showBarlines());
                  genTimesig->setChecked(st->genTimesig());
                  genKeysigPitched->setChecked(ps->genKeysig());
                  showLedgerLinesPitched->setChecked(ps->showLedgerLines());
                  stemlessPitched->setChecked(st->slashStyle());
                  }
                  break;

            case TAB_STAFF_GROUP:
                  {
                  StaffTypeTablature* stt = static_cast<StaffTypeTablature*>(st);
                  blockTabPreviewSignals(true);
                  setDlgFromTab(stt);
                  name->setText(stt->name());   // setDlgFromTab() does not copy the name and it shouldn't
                  stack->setCurrentIndex(1);
                  blockTabPreviewSignals(false);
                  }
                  break;

            case PERCUSSION_STAFF_GROUP:
                  {
                  StaffTypePercussion* ps = static_cast<StaffTypePercussion*>(st);
                  blockPercPreviewSignals(true);
                  setDlgFromPerc(ps);
                  name->setText(ps->name());   // setDlgFromPerc() does not copy the name and it shouldn't
                  stack->setCurrentIndex(2);
                  blockPercPreviewSignals(false);
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   createNewType
//---------------------------------------------------------

void EditStaffType::createNewType()
      {
      //
      // initialize new StaffType with current selected one
      //
      int idx       = staffTypeList->currentItem()->data(Qt::UserRole).toInt();
      StaffType* ns = staffTypes[idx]->clone();

      ns->setName(createUniqueStaffTypeName(ns->group()));

      staffTypes.append(ns);
      QListWidgetItem* item = new QListWidgetItem(ns->name());
      item->setData(Qt::UserRole, staffTypes.size() - 1);
      staffTypeList->addItem(item);
      staffTypeList->setCurrentItem(item);
      modified = true;
      }

//---------------------------------------------------------
//   nameEdited
//---------------------------------------------------------

void EditStaffType::nameEdited(const QString& s)
      {
      staffTypeList->currentItem()->setText(s);
      }

//---------------------------------------------------------
//   updatePreviews
//---------------------------------------------------------

void EditStaffType::updatePreviews()
      {
      updatePercPreview();
      updateTabPreview();
      }

//=========================================================
//   PERCUSSION PAGE METHODS
//=========================================================

//---------------------------------------------------------
//   Precussion preset clicked
//---------------------------------------------------------

void EditStaffType::presetPercChanged(int idx)
      {
      int presetIdx = presetPercCombo->itemData(idx).toInt();
      if(presetIdx >= 0) {                      // ignore setting to "Custom"
            blockPercPreviewSignals(true);       // do not redraw preview for every value we change!
            const StaffTypePercussion* st = static_cast<const StaffTypePercussion*>(StaffType::preset(presetIdx));
            setDlgFromPerc(st);
            blockPercPreviewSignals(false);
            }
      }

//---------------------------------------------------------
//   setDlgFromPerc
//
//    initializes dlg controls from a StaffTypePercussion
//---------------------------------------------------------

void EditStaffType::setDlgFromPerc(const StaffTypePercussion * st)
      {
//      name->setText(st->name());             // keep existing name: presets should not overwrite type name
      lines->setValue(st->lines());
      lineDistance->setValue(st->lineDistance().val());
      genClef->setChecked(st->genClef());
      showBarlines->setChecked(st->showBarlines());
      genTimesig->setChecked(st->genTimesig());
      genKeysigPercussion->setChecked(st->genKeysig());
      showLedgerLinesPercussion->setChecked(st->showLedgerLines());
      stemlessPercussion->setChecked(st->slashStyle());
      updatePercPreview();
      }

//---------------------------------------------------------
//   setPercFromDlg
//
//    initializes a StaffTypePercussion from dlg controls
//---------------------------------------------------------

void EditStaffType::setPercFromDlg(StaffTypePercussion * st)
{
      st->setName(name->text());
      st->setLines(lines->value());
      st->setLineDistance(Spatium(lineDistance->value()));
      st->setGenClef(genClef->isChecked());
      st->setShowBarlines(showBarlines->isChecked());
      st->setGenTimesig(genTimesig->isChecked());
      st->setGenKeysig(genKeysigPercussion->isChecked());
      st->setShowLedgerLines(showLedgerLinesPercussion->isChecked());
      st->setSlashStyle(stemlessPercussion->isChecked());
}

//---------------------------------------------------------
//   Update percussion preview
///   update percussion-related fields
//---------------------------------------------------------

void EditStaffType::updatePercPreview()
      {
      // if current type is not a PERC type, do nothing
      if(staffTypes[staffTypeList->currentItem()->data(Qt::UserRole).toInt()]->group() != PERCUSSION_STAFF_GROUP)
            return;
      // create a new staff type from dlg settings
      StaffTypePercussion* st = new StaffTypePercussion();
      setPercFromDlg(st);
      // set preset combo: check stt has the same structure as one of the presets
      // if none matches, set as custom
      int idx;
      int numOfPresets = presetPercCombo->count()-1;  // do not count the final "Custom" item
      for(idx=0; idx < numOfPresets; idx++) {
            int presetIdx = presetPercCombo->itemData(idx).toInt();
            if(st->isSameStructure(*StaffType::preset(presetIdx)) )
                  break;
            }
      presetPercCombo->blockSignals(true);
      presetPercCombo->setCurrentIndex(idx);
      presetPercCombo->blockSignals(false);
      }

//---------------------------------------------------------
//   Block percussion preview signals
//---------------------------------------------------------

void EditStaffType::blockPercPreviewSignals(bool block)
      {
      lines->blockSignals(block);
      lineDistance->blockSignals(block);
      genClef->blockSignals(block);
      genTimesig->blockSignals(block);
      showBarlines->blockSignals(block);
      showLedgerLinesPercussion->blockSignals(block);
      genKeysigPercussion->blockSignals(block);
      stemlessPercussion->blockSignals(block);
      }

//=========================================================
//   TABULATURE PAGE METHODS
//=========================================================

//---------------------------------------------------------
//   Tabulature FullConfig/QuickConfig clicked
//---------------------------------------------------------

void EditStaffType::on_pushFullConfig_clicked()
      {
      tabPresets->hide();
      tabDetails->show();
      }

void EditStaffType::on_pushQuickConfig_clicked()
      {
      tabDetails->hide();
      tabPresets->show();
      }

//---------------------------------------------------------
//   Tabulature preset clicked
//---------------------------------------------------------

void EditStaffType::presetTablatureChanged(int idx)
      {
      int presetIdx = presetTablatureCombo->itemData(idx).toInt();
      if(presetIdx >= 0) {                      // ignore setting to "Custom"
            blockTabPreviewSignals(true);       // do not redraw preview for every value we change!
            const StaffTypeTablature* st = static_cast<const StaffTypeTablature*>(StaffType::preset(presetIdx));
            setDlgFromTab(st);
            blockTabPreviewSignals(false);
            }
      }

//---------------------------------------------------------
//   Tabulature duration / fret font name changed
//
//    set depending parameters
//---------------------------------------------------------

void EditStaffType::durFontNameChanged(int idx)
      {
      qreal size, yOff;
      if (StaffTypeTablature::fontData(true, idx, 0, 0, &size, &yOff)) {
            durFontSize->setValue(size);
            durY->setValue(yOff);
            }
      updateTabPreview();
      }

void EditStaffType::fretFontNameChanged(int idx)
      {
      qreal size, yOff;
      if (StaffTypeTablature::fontData(false, idx, 0, 0, &size, &yOff)) {
            fretFontSize->setValue(size);
            fretY->setValue(yOff);
            }
      updateTabPreview();
      }

//---------------------------------------------------------
//   Tabulature note stems toggled
//
//    enable / disable all controls related to stems
//---------------------------------------------------------

void EditStaffType::tabStemsToggled(bool checked)
      {
      tabStemsCompatibility(checked);
      updateTabPreview();
      }

//---------------------------------------------------------
//   Tabulature "minim short" toggled
//
//    contra-toggle "stems through"
//---------------------------------------------------------

void EditStaffType::tabMinimShortToggled(bool checked)
      {
      tabMinimShortCompatibility(checked);
      updateTabPreview();
      }

//---------------------------------------------------------
//   Tabulature "stems through" toggled
//---------------------------------------------------------

void EditStaffType::tabStemThroughToggled(bool checked)
      {
      tabStemThroughCompatibility(checked);
      updateTabPreview();
      }

//---------------------------------------------------------
//   setDlgFromTab
//
//    initializes dlg controls from a StaffTypeTablature
//---------------------------------------------------------

void EditStaffType::setDlgFromTab(const StaffTypeTablature * stt)
      {
//      name->setText(stt->name());             // keep existing name: presets should not overwrite type name
      lines->setValue(stt->lines());
      lineDistance->setValue(stt->lineDistance().val());
      genClef->setChecked(stt->genClef());
      showBarlines->setChecked(stt->showBarlines());
      genTimesig->setChecked(stt->genTimesig());
      upsideDown->setChecked(stt->upsideDown());
      int idx = fretFontName->findText(stt->fretFontName(), Qt::MatchFixedString);
      if(idx == -1)     idx = 0;          // if name not found, use first name
      fretFontName->setCurrentIndex(idx);
      fretFontSize->setValue(stt->fretFontSize());
      fretY->setValue(stt->fretFontUserY());
      numbersRadio->setChecked(stt->useNumbers());
      lettersRadio->setChecked(!stt->useNumbers());
      onLinesRadio->setChecked(stt->onLines());
      aboveLinesRadio->setChecked(!stt->onLines());
      linesThroughRadio->setChecked(stt->linesThrough());
      linesBrokenRadio->setChecked(!stt->linesThrough());
      idx = durFontName->findText(stt->durationFontName(), Qt::MatchFixedString);
      if(idx == -1)     idx = 0;          // if name not found, use first name
      durFontName->setCurrentIndex(idx);
      durFontSize->setValue(stt->durationFontSize());
      durY->setValue(stt->durationFontUserY());
      // convert combined values of genDurations and slashStyle into noteValuesx radio buttons
      // Sbove/Below, Beside/Through and minim are only used if stems-and-beams
      // but set them from stt values anyway, to ensure preset matching
      stemAboveRadio->setChecked(!stt->stemsDown());
      stemBelowRadio->setChecked(stt->stemsDown());
      stemBesideRadio->setChecked(!stt->stemThrough());
      stemThroughRadio->setChecked(stt->stemThrough());
      TablatureMinimStyle minimStyle = stt->minimStyle();
      minimNoneRadio->setChecked(minimStyle == TAB_MINIM_NONE);
      minimShortRadio->setChecked(minimStyle == TAB_MINIM_SHORTER);
      minimSlashedRadio->setChecked(minimStyle == TAB_MINIM_SLASHED);
      if(stt->genDurations()) {
            noteValuesNone->setChecked(false);
            noteValuesSymb->setChecked(true);
            noteValuesStems->setChecked(false);
            }
      else {
            if(stt->slashStyle()) {
                  noteValuesNone->setChecked(true);
                  noteValuesSymb->setChecked(false);
                  noteValuesStems->setChecked(false);
                  }
            else {
                  noteValuesNone->setChecked(false);
                  noteValuesSymb->setChecked(false);
                  noteValuesStems->setChecked(true);
                  }
            }
      showRests->setChecked(stt->showRests());
      // adjust compatibility across different settings
      tabStemThroughCompatibility(stemThroughRadio->isChecked());
      tabMinimShortCompatibility(minimShortRadio->isChecked());
      tabStemsCompatibility(noteValuesStems->isChecked());
      updateTabPreview();
      }

//---------------------------------------------------------
//   setTabFromDlg
//
//    initializes a StaffTypeTablature from dlg controls
//---------------------------------------------------------

void EditStaffType::setTabFromDlg(StaffTypeTablature * stt)
{
      stt->setName(name->text());
      stt->setLines(lines->value());
      stt->setLineDistance(Spatium(lineDistance->value()));
      stt->setGenClef(genClef->isChecked());
      stt->setShowBarlines(showBarlines->isChecked());
      stt->setGenTimesig(genTimesig->isChecked());
      stt->setDurationFontName(durFontName->currentText());
      stt->setDurationFontSize(durFontSize->value());
      stt->setDurationFontUserY(durY->value());
      stt->setFretFontName(fretFontName->currentText());
      stt->setFretFontSize(fretFontSize->value());
      stt->setFretFontUserY(fretY->value());
      stt->setLinesThrough(linesThroughRadio->isChecked());
      stt->setMinimStyle(minimNoneRadio->isChecked() ? TAB_MINIM_NONE :
            (minimShortRadio->isChecked() ? TAB_MINIM_SHORTER : TAB_MINIM_SLASHED));
      stt->setOnLines(onLinesRadio->isChecked());
      stt->setShowRests(showRests->isChecked());
      stt->setUpsideDown(upsideDown->isChecked());
      stt->setUseNumbers(numbersRadio->isChecked());
      //note values
      stt->setStemsDown(stemBelowRadio->isChecked());
      stt->setStemsThrough(stemThroughRadio->isChecked());
      stt->setSlashStyle(true);                 // assume no note values
      stt->setGenDurations(false);              //    "     "
      if (noteValuesSymb->isChecked())
            stt->setGenDurations(true);
      if (noteValuesStems->isChecked()) {
            stt->setSlashStyle(false);
            }
}

//---------------------------------------------------------
//   Block tabulature preview signals
//---------------------------------------------------------

void EditStaffType::blockTabPreviewSignals(bool block)
      {
      lines->blockSignals(block);
      lineDistance->blockSignals(block);
      showBarlines->blockSignals(block);
      genClef->blockSignals(block);
      genTimesig->blockSignals(block);
      noteValuesSymb->blockSignals(block);
      noteValuesStems->blockSignals(block);
      durFontName->blockSignals(block);
      durFontSize->blockSignals(block);
      durY->blockSignals(block);
      fretFontName->blockSignals(block);
      fretFontSize->blockSignals(block);
      fretY->blockSignals(block);
      linesThroughRadio->blockSignals(block);
      onLinesRadio->blockSignals(block);
      upsideDown->blockSignals(block);
      numbersRadio->blockSignals(block);
      stemAboveRadio->blockSignals(block);
      stemBelowRadio->blockSignals(block);
      stemBesideRadio->blockSignals(block);
      stemThroughRadio->blockSignals(block);
      minimShortRadio->blockSignals(block);
      minimSlashedRadio->blockSignals(block);
      showRests->blockSignals(block);
      }

//---------------------------------------------------------
//   Tabulature note stems compatibility
//
//    Enable / disable all stem-related controls according to "Stems and beams" is checked/unchecked
//---------------------------------------------------------

void EditStaffType::tabStemsCompatibility(bool checked)
      {
      stemAboveRadio->setEnabled(checked && !stemThroughRadio->isChecked());
      stemBelowRadio->setEnabled(checked && !stemThroughRadio->isChecked());
      stemBesideRadio->setEnabled(checked);
      stemThroughRadio->setEnabled(checked && !minimShortRadio->isChecked());
      minimNoneRadio->setEnabled(checked);
      minimShortRadio->setEnabled(checked && !stemThroughRadio->isChecked());
      minimSlashedRadio->setEnabled(checked);
      }

//---------------------------------------------------------
//   Tabulature "minim short" compatibility
//
//    Setting "short minim" stems is incompatible with "stems through":
//    if checked and "stems through" is checked, move check to "stems beside"
//---------------------------------------------------------

void EditStaffType::tabMinimShortCompatibility(bool checked)
      {
      if(checked) {
            if(stemThroughRadio->isChecked()) {
                  stemThroughRadio->setChecked(false);
                  stemBesideRadio->setChecked(true);
                  }
            }
      // disable / enable "stems through" according "minim short" is checked / unchecked
      stemThroughRadio->setEnabled(!checked && noteValuesStems->isChecked());
      }

//---------------------------------------------------------
//   Tabulature "stems through" compatibility
//
//    Setting "stems through" is incompatible with "minim short":
//    if checking and "minim short" is checked, move check to "minim slashed"
//    It also make "Stems above" and "Stems below" meaningless: disable them
//---------------------------------------------------------

void EditStaffType::tabStemThroughCompatibility(bool checked)
      {
      if(checked) {
            if(minimShortRadio->isChecked()) {
                  minimShortRadio->setChecked(false);
                  minimSlashedRadio->setChecked(true);
                  }
            }
      // disable / enable "minim short" and "stems above/below" according "stems through" is checked / unchecked
      bool enab = !checked && noteValuesStems->isChecked();
      minimShortRadio->setEnabled(enab);
      stemAboveRadio->setEnabled(enab);
      stemBelowRadio->setEnabled(enab);
      }

//---------------------------------------------------------
//   Update tabulature preview
///   update tabulature staff type in preview score
//---------------------------------------------------------

void EditStaffType::updateTabPreview()
      {
      // if no preview or current type is not a TAB type, do nothing
      if(!tabPreview ||
                  staffTypes[staffTypeList->currentItem()->data(Qt::UserRole).toInt()]->group() != TAB_STAFF_GROUP)
            return;
      // create a new staff type from dlg settings and set it into the preview score
      StaffTypeTablature* stt = new StaffTypeTablature();
      setTabFromDlg(stt);
      tabPreview->score()->addStaffType(TAB_6COMMON_STAFF_TYPE, stt);

      tabPreview->score()->doLayout();
#ifdef _USE_NAVIGATOR_PREVIEW_
      tabPreview->layoutChanged();
#endif
      tabPreview->updateAll();
      // set preset combo: check stt has the same structure as one of the presets
      // if none matches, set as custom
      int idx;
      int numOfPresets = presetTablatureCombo->count() - 1; // do not count last "Custom" item
      for(idx=0; idx < numOfPresets; idx++) {
            int presetIdx = presetTablatureCombo->itemData(idx).toInt();
            if(stt->isSameStructure(*StaffType::preset(presetIdx)) )
                  break;
            }
      presetTablatureCombo->blockSignals(true);
      presetTablatureCombo->setCurrentIndex(idx);
      presetTablatureCombo->blockSignals(false);
      }


//---------------------------------------------------------
//   createUniqueStaffTypeName
///  create unique new name for StaffType
//---------------------------------------------------------

QString EditStaffType::createUniqueStaffTypeName(StaffGroup group)
      {
      // count how many types there are already of the same group of the new type
      int idx       = staffTypeList->currentItem()->data(Qt::UserRole).toInt();
      for (int i = idx = 0; i < staffTypes.count(); i++)
            if (staffTypes[i]->group() == group)
                  idx++;
      QString name;
      switch (group)
            {
            case STANDARD_STAFF_GROUP:
                  name = QString("Standard-%1 [*]").arg(idx);
                  break;
            case PERCUSSION_STAFF_GROUP:
                  name = QString("Perc-%1 [*]").arg(idx);
                  break;
            case TAB_STAFF_GROUP:
                  name = QString("Tab-%1 [*]").arg(idx);
                  break;
            }
      return name;
      }


//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStaffType::accept()
      {
      QListWidgetItem* item = staffTypeList->currentItem();
      if (item)
            saveCurrent(item);
      QDialog::accept();
      }
}

