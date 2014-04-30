//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2014 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "editstafftype.h"
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

      staff     = st;
      staffType = *staff->staffType();

      // init preset list
      int idx = 0;
      for (const StaffType& st : StaffType::presets()) {
            QListWidgetItem* item = new QListWidgetItem(st.name());
            item->setData(Qt::UserRole, idx);
            staffTypeList->addItem(item);
            ++idx;
            }
      staffTypeList->setCurrentRow(0);

      groupCombo->clear();
      groupCombo->addItem(StaffType::groupName(STANDARD_STAFF_GROUP), STANDARD_STAFF_GROUP);
      groupCombo->addItem(StaffType::groupName(PERCUSSION_STAFF_GROUP), PERCUSSION_STAFF_GROUP);
      groupCombo->addItem(StaffType::groupName(TAB_STAFF_GROUP), TAB_STAFF_GROUP);

      // tab page configuration
      QList<QString> fontNames = StaffType::fontNames(false);
      foreach (const QString& name, fontNames)   // fill fret font name combo
            fretFontName->addItem(name);
      fretFontName->setCurrentIndex(0);
      fontNames = StaffType::fontNames(true);
      foreach(const QString& name, fontNames)   // fill duration font name combo
            durFontName->addItem(name);
      durFontName->setCurrentIndex(0);
      // load a sample tabulature score in preview
      Score* sc = new Score(MScore::defaultStyle());
      if (readScore(sc, QString(":/data/tab_sample.mscx"), false) == Score::FILE_NO_ERROR) {
            // add a preview widget to tabulature page
            preview = new ScoreView(this);
            static_cast<QVBoxLayout*>(groupPreview->layout())->insertWidget(0, preview);
            preview->setScore(sc);
            }

      setValues();

      connect(name,           SIGNAL(textEdited(const QString&)), SLOT(nameEdited(const QString&)));
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

      connect(save,             SIGNAL(clicked()), SLOT(savePresets()));
      connect(load,             SIGNAL(clicked()), SLOT(loadPresets()));
      connect(loadFromTemplate, SIGNAL(clicked()), SLOT(loadFromTemplateClicked()));
      connect(addToTemplates,   SIGNAL(clicked()), SLOT(addToTemplatesClicked()));
      connect(groupCombo,       SIGNAL(currentIndexChanged(int)), SLOT(staffGroupChanged(int)));
      }

//---------------------------------------------------------
//   staffGroupChanged
//---------------------------------------------------------

void EditStaffType::staffGroupChanged(int n)
      {
      StaffGroup group = StaffGroup(n);
      staffType = *StaffType::getDefaultPreset(group); // overwrite with default
      setValues();
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStaffType::setValues()
      {
      StaffGroup group = staffType.group();
      int idx = int(group);
      stack->setCurrentIndex(idx);
      groupCombo->setCurrentIndex(idx);

      name->setText(staffType.name());
      lines->setValue(staffType.lines());
      lineDistance->setValue(staffType.lineDistance().val());
      genClef->setChecked(staffType.genClef());
      showBarlines->setChecked(staffType.showBarlines());
      genTimesig->setChecked(staffType.genTimesig());

      switch (group) {
            case STANDARD_STAFF_GROUP:
                  genKeysigPitched->setChecked(staffType.genKeysig());
                  showLedgerLinesPitched->setChecked(staffType.showLedgerLines());
                  stemlessPitched->setChecked(staffType.slashStyle());
                  break;

            case TAB_STAFF_GROUP:
                  {
                  blockTabPreviewSignals(true);
                  upsideDown->setChecked(staffType.upsideDown());
                  int idx = fretFontName->findText(staffType.fretFontName(), Qt::MatchFixedString);
                  if (idx == -1)
                        idx = 0;          // if name not found, use first name
                  fretFontName->setCurrentIndex(idx);
                  fretFontSize->setValue(staffType.fretFontSize());
                  fretY->setValue(staffType.fretFontUserY());
                  numbersRadio->setChecked(staffType.useNumbers());
                  lettersRadio->setChecked(!staffType.useNumbers());
                  onLinesRadio->setChecked(staffType.onLines());
                  aboveLinesRadio->setChecked(!staffType.onLines());
                  linesThroughRadio->setChecked(staffType.linesThrough());
                  linesBrokenRadio->setChecked(!staffType.linesThrough());
                  idx = durFontName->findText(staffType.durationFontName(), Qt::MatchFixedString);
                  if (idx == -1)
                        idx = 0;          // if name not found, use first name
                  durFontName->setCurrentIndex(idx);
                  durFontSize->setValue(staffType.durationFontSize());
                  durY->setValue(staffType.durationFontUserY());
                  // convert combined values of genDurations and slashStyle into noteValuesx radio buttons
                  // Sbove/Below, Beside/Through and minim are only used if stems-and-beams
                  // but set them from stt values anyway, to ensure preset matching
                  stemAboveRadio->setChecked(!staffType.stemsDown());
                  stemBelowRadio->setChecked(staffType.stemsDown());
                  stemBesideRadio->setChecked(!staffType.stemThrough());
                  stemThroughRadio->setChecked(staffType.stemThrough());
                  TablatureMinimStyle minimStyle = staffType.minimStyle();
                  minimNoneRadio->setChecked(minimStyle == TAB_MINIM_NONE);
                  minimShortRadio->setChecked(minimStyle == TAB_MINIM_SHORTER);
                  minimSlashedRadio->setChecked(minimStyle == TAB_MINIM_SLASHED);
                  if (staffType.genDurations()) {
                        noteValuesNone->setChecked(false);
                        noteValuesSymb->setChecked(true);
                        noteValuesStems->setChecked(false);
                        }
                  else {
                        if (staffType.slashStyle()) {
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
                  showRests->setChecked(staffType.showRests());
                  // adjust compatibility across different settings
                  tabStemThroughCompatibility(stemThroughRadio->isChecked());
                  tabMinimShortCompatibility(minimShortRadio->isChecked());
                  tabStemsCompatibility(noteValuesStems->isChecked());
                  updateTabPreview();

                  blockTabPreviewSignals(false);
                  }
                  break;

            case PERCUSSION_STAFF_GROUP:
                  blockPercPreviewSignals(true);
                  genKeysigPercussion->setChecked(staffType.genKeysig());
                  showLedgerLinesPercussion->setChecked(staffType.showLedgerLines());
                  stemlessPercussion->setChecked(staffType.slashStyle());
                  updatePercPreview();
                  blockPercPreviewSignals(false);
                  break;
            }
      }

//---------------------------------------------------------
//   nameEdited
//---------------------------------------------------------

void EditStaffType::nameEdited(const QString& s)
      {
//      staffTypeList->currentItem()->setText(s);
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
//   setPercFromDlg
//
//    initializes a StaffTypePercussion from dlg controls
//---------------------------------------------------------

void EditStaffType::setPercFromDlg()
      {
      staffType.setName(name->text());
      staffType.setLines(lines->value());
      staffType.setLineDistance(Spatium(lineDistance->value()));
      staffType.setGenClef(genClef->isChecked());
      staffType.setShowBarlines(showBarlines->isChecked());
      staffType.setGenTimesig(genTimesig->isChecked());
      staffType.setGenKeysig(genKeysigPercussion->isChecked());
      staffType.setShowLedgerLines(showLedgerLinesPercussion->isChecked());
      staffType.setSlashStyle(stemlessPercussion->isChecked());
      }

//---------------------------------------------------------
//   Update percussion preview
///   update percussion-related fields
//---------------------------------------------------------

void EditStaffType::updatePercPreview()
      {
      setPercFromDlg();
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
//   Tabulature duration / fret font name changed
//
//    set depending parameters
//---------------------------------------------------------

void EditStaffType::durFontNameChanged(int idx)
      {
      qreal size, yOff;
      if (StaffType::fontData(true, idx, 0, 0, &size, &yOff)) {
            durFontSize->setValue(size);
            durY->setValue(yOff);
            }
      updateTabPreview();
      }

void EditStaffType::fretFontNameChanged(int idx)
      {
      qreal size, yOff;
      if (StaffType::fontData(false, idx, 0, 0, &size, &yOff)) {
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
//   setTabFromDlg
//
//    initializes a StaffTypeTablature from dlg controls
//---------------------------------------------------------

void EditStaffType::setTabFromDlg()
      {
      staffType.setName(name->text());
      staffType.setLines(lines->value());
      staffType.setLineDistance(Spatium(lineDistance->value()));
      staffType.setGenClef(genClef->isChecked());
      staffType.setShowBarlines(showBarlines->isChecked());
      staffType.setGenTimesig(genTimesig->isChecked());
      staffType.setDurationFontName(durFontName->currentText());
      staffType.setDurationFontSize(durFontSize->value());
      staffType.setDurationFontUserY(durY->value());
      staffType.setFretFontName(fretFontName->currentText());
      staffType.setFretFontSize(fretFontSize->value());
      staffType.setFretFontUserY(fretY->value());
      staffType.setLinesThrough(linesThroughRadio->isChecked());
      staffType.setMinimStyle(minimNoneRadio->isChecked() ? TAB_MINIM_NONE :
            (minimShortRadio->isChecked() ? TAB_MINIM_SHORTER : TAB_MINIM_SLASHED));
      staffType.setOnLines(onLinesRadio->isChecked());
      staffType.setShowRests(showRests->isChecked());
      staffType.setUpsideDown(upsideDown->isChecked());
      staffType.setUseNumbers(numbersRadio->isChecked());
      //note values
      staffType.setStemsDown(stemBelowRadio->isChecked());
      staffType.setStemsThrough(stemThroughRadio->isChecked());
      staffType.setSlashStyle(true);                 // assume no note values
      staffType.setGenDurations(false);              //    "     "
      if (noteValuesSymb->isChecked())
            staffType.setGenDurations(true);
      if (noteValuesStems->isChecked())
            staffType.setSlashStyle(false);
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
//    updateTabPreview
///   update tabulature staff type in preview score
//---------------------------------------------------------

void EditStaffType::updateTabPreview()
      {
      setTabFromDlg();
      if (preview) {
            preview->score()->staff(0)->setStaffType(&staffType);
            preview->score()->cmdUpdateNotes();
            preview->score()->doLayout();
            preview->updateAll();
            }
      }

//---------------------------------------------------------
//   createUniqueStaffTypeName
///  create unique new name for StaffType
//---------------------------------------------------------

QString EditStaffType::createUniqueStaffTypeName(StaffGroup group)
      {
      QString name;
      for (int idx = 1; ; ++idx) {
            switch (group) {
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
            bool found = false;
            for (const StaffType& st : StaffType::presets()) {
                  if (st.name() == name) {
                        found = true;
                        break;
                        }
                  }
            if (!found)
                  break;
            }
      return name;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStaffType::accept()
      {
      QDialog::accept();
      }

//---------------------------------------------------------
//   savePresets
//---------------------------------------------------------

void EditStaffType::savePresets()
      {
      printf("savePresets\n");
      }

//---------------------------------------------------------
//   loadPresets
//---------------------------------------------------------

void EditStaffType::loadPresets()
      {
      printf("loadPresets\n");
      }

//---------------------------------------------------------
//   loadFromTemplate
//---------------------------------------------------------

void EditStaffType::loadFromTemplateClicked()
      {
      int idx = staffTypeList->currentItem()->data(Qt::UserRole).toInt();
      staffType = *StaffType::preset(idx);
      setValues();
      }

//---------------------------------------------------------
//   addToTemplates
//---------------------------------------------------------

void EditStaffType::addToTemplatesClicked()
      {
      printf("add to templates\n");
      }

}

