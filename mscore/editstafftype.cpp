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

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

EditStaffType::EditStaffType(QWidget* parent, Staff* st)
   : QDialog(parent)
      {
      setupUi(this);
      staff = st;
      Score* score = staff->score();
      staffTypes   = score->staffTypes();
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

      // tab page configuration
      tabDetails->hide();                       // start tabulature page in simple mode
      // load a sample tabulature score in preview
      Score* sc = new Score(MScore::defaultStyle());
      tabPreview = 0;
      if(mscore->readScore(sc, QString(":/data/tab_sample.mscx"))) {
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
      connect(presetTablature,      SIGNAL(clicked()),            SLOT(presetTablatureClicked()));

      if (ci)
            staffTypeList->setCurrentItem(ci);

      connect(lineDistance,   SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(showBarlines,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(genClef,        SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(genTimesig,     SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(noteValues1,    SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(noteValues2,    SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(durFontName,    SIGNAL(editTextChanged(QString)),   SLOT(updateTabPreview()));
      connect(durFontSize,    SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(durY,           SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(fretFontName,   SIGNAL(editTextChanged(QString)),   SLOT(updateTabPreview()));
      connect(fretFontSize,   SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(fretY,          SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(linesThroughRadio, SIGNAL(toggled(bool)),           SLOT(updateTabPreview()));
      connect(onLinesRadio,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(upsideDown,     SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(numbersRadio,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));

      modified = false;
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
            case PITCHED_STAFF:
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

            case TAB_STAFF:
                  {
                  StaffTypeTablature*  stt = static_cast<StaffTypeTablature*>(st);
                  if (stt->durationFontName()    != durFontName->currentFont().family()
                     || stt->durationFontSize() != durFontSize->value()
                     || stt->durationFontUserY()!= durY->value()
                     || stt->fretFontName()     != fretFontName->currentFont().family()
                     || stt->fretFontSize()     != fretFontSize->value()
                     || stt->fretFontUserY()    != fretY->value()
                     || stt->linesThrough()     != linesThroughRadio->isChecked()
                     || stt->onLines()          != onLinesRadio->isChecked()
                     || stt->upsideDown()       != upsideDown->isChecked()
                     || stt->useNumbers()       != numbersRadio->isChecked()
                     || ( noteValues0->isChecked() && (!stt->slashStyle() ||  stt->genDurations()) )
                     || ( noteValues1->isChecked() && (!stt->slashStyle() || !stt->genDurations()) )
                     || ( noteValues2->isChecked() && ( stt->slashStyle() ||  stt->genDurations()) )
                     ) {
                        modif = true;
                        }
                  }
                  break;

            case PERCUSSION_STAFF:
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
            st->setName(o->text());
            st->setLines(lines->value());
            st->setLineDistance(Spatium(lineDistance->value()));
            st->setShowBarlines(showBarlines->isChecked());
            st->setGenClef(genClef->isChecked());
            st->setGenTimesig(genTimesig->isChecked());
            // save-group specific properties
            switch(st->group()) {
                  case PITCHED_STAFF:
                        {
                        StaffTypePitched* sp = static_cast<StaffTypePitched*>(st);
                        sp->setGenKeysig(genKeysigPitched->isChecked());
                        sp->setShowLedgerLines(showLedgerLinesPitched->isChecked());
                        st->setSlashStyle(stemlessPitched->isChecked());
                        }
                        break;
                  case TAB_STAFF:
                        {
                        StaffTypeTablature*  stt = static_cast<StaffTypeTablature*>(st);
                        stt->setSlashStyle(true);                 // assume no note values
                        stt->setGenDurations(false);
                        if (noteValues1->isChecked())
                              stt->setGenDurations(true);
                        if (noteValues2->isChecked())
                              stt->setSlashStyle(false);
                        stt->setDurationFontName(durFontName->currentText());
                        stt->setDurationFontSize(durFontSize->value());
                        stt->setDurationFontUserY(durY->value());
                        stt->setFretFontName(fretFontName->currentText());
                        stt->setFretFontSize(fretFontSize->value());
                        stt->setFretFontUserY(fretY->value());
                        stt->setLinesThrough(linesThroughRadio->isChecked());
                        stt->setOnLines(onLinesRadio->isChecked());
                        stt->setUpsideDown(upsideDown->isChecked());
                        stt->setUseNumbers(numbersRadio->isChecked());
                        }
                        break;
                  case PERCUSSION_STAFF:
                        {
                        StaffTypePercussion* sp = static_cast<StaffTypePercussion*>(st);
                        sp->setGenKeysig(genKeysigPercussion->isChecked());
                        sp->setShowLedgerLines(showLedgerLinesPercussion->isChecked());
                        st->setSlashStyle(stemlessPercussion->isChecked());
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

      // set properties common to all groups (some props appears in multiple group pages)
      blockTabPreviewSignals(true);
      name->setText(st->name());
      lines->setValue(st->lines());
      lineDistance->setValue(st->lineDistance().val());
      genClef->setChecked(st->genClef());
      showBarlines->setChecked(st->showBarlines());
      genTimesig->setChecked(st->genTimesig());

      // switch to stack page and set props specific to each staff group
      QFont f = QFont();

      switch(st->group()) {
            case PITCHED_STAFF:
                  {
                  StaffTypePitched* ps = static_cast<StaffTypePitched*>(st);
                  stack->setCurrentIndex(0);
                  genKeysigPitched->setChecked(ps->genKeysig());
                  showLedgerLinesPitched->setChecked(ps->showLedgerLines());
                  stemlessPitched->setChecked(st->slashStyle());
                  }
                  break;

            case TAB_STAFF:
                  {
                  StaffTypeTablature* tab = static_cast<StaffTypeTablature*>(st);
                  stack->setCurrentIndex(1);
                  upsideDown->setChecked(tab->upsideDown());
                  f.setFamily(tab->fretFontName());
                  f.setPointSizeF(tab->fretFontSize());
                  fretFontName->setCurrentFont(f);
                  fretFontSize->setValue(tab->fretFontSize());
                  fretY->setValue(tab->fretFontUserY());
                  numbersRadio->setChecked(tab->useNumbers());
                  lettersRadio->setChecked(!tab->useNumbers());
                  onLinesRadio->setChecked(tab->onLines());
                  aboveLinesRadio->setChecked(!tab->onLines());
                  linesThroughRadio->setChecked(tab->linesThrough());
                  linesBrokenRadio->setChecked(!tab->linesThrough());
                  f.setFamily(tab->durationFontName());
                  f.setPointSizeF(tab->durationFontSize());
                  durFontName->setCurrentFont(f);
                  durFontSize->setValue(tab->durationFontSize());
                  durY->setValue(tab->durationFontUserY());
                  // convert combined values of genDurations and slashStyle
                  // into noteValuesx radio buttons
                  if(tab->genDurations()) {
                        noteValues0->setChecked(false);
                        noteValues1->setChecked(true);
                        noteValues2->setChecked(false);
                        }
                  else {
                        if(tab->slashStyle()) {
                              noteValues0->setChecked(true);
                              noteValues1->setChecked(false);
                              noteValues2->setChecked(false);
                              }
                        else {
                              noteValues0->setChecked(false);
                              noteValues1->setChecked(false);
                              noteValues2->setChecked(true);
                              }
                        }
                  updateTabPreview();
                  }
                  break;

            case PERCUSSION_STAFF:
                  {
                  StaffTypePercussion* ps = static_cast<StaffTypePercussion*>(st);
                  stack->setCurrentIndex(2);
                  genKeysigPercussion->setChecked(ps->genKeysig());
                  showLedgerLinesPercussion->setChecked(ps->showLedgerLines());
                  stemlessPercussion->setChecked(st->slashStyle());
                  }
                  break;
            }
      blockTabPreviewSignals(false);
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

      //
      // create unique new name for StaffType
      //
      for (int i = 1;;++i) {
            QString name = QString("type-%1").arg(i);
            int n = staffTypes.size();
            int k;
            for (k = 0; k < n; ++k) {
                  if (staffTypes[k]->name() == name)
                        break;
                  }
            if (k == n) {
                  ns->setName(name);
                  break;
                  }
            }
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
//   Tabulature preset clicked
//---------------------------------------------------------

void EditStaffType::presetTablatureClicked()
      {
      QFont f = QFont();

      // retrieve item currently selected in preset combo
      int idx       = presetTablatureCombo->currentIndex();
      blockTabPreviewSignals(true);                   // do not redraw preview for every value we change!
      switch(idx)
      {
      case 0:                             // guitar
      case 1:                             // bass
            lines->setValue(idx == 0 ? 6 : 4);
            lineDistance->setValue(1.5);
            genClef->setChecked(true);
            showBarlines->setChecked(true);
            genTimesig->setChecked(false);
            upsideDown->setChecked(false);
            f.setFamily("MScoreTabulatureModern");
            f.setPointSizeF(10);
            fretFontName->setCurrentFont(f);
            fretFontSize->setValue(10);
            fretY->setValue(0);
            numbersRadio->setChecked(true);
            lettersRadio->setChecked(false);
            onLinesRadio->setChecked(true);
            aboveLinesRadio->setChecked(false);
            linesThroughRadio->setChecked(false);
            linesBrokenRadio->setChecked(true);
            f.setFamily("MScoreTabulatureModern");
            f.setPointSizeF(0);
            durFontName->setCurrentFont(f);
            durFontSize->setValue(15);
            durY->setValue(0);
            noteValues0->setChecked(false);
            noteValues1->setChecked(false);
            noteValues2->setChecked(true);
            break;
      case 2:                             // Italian
      case 3:                             // French
            lines->setValue(6);
            lineDistance->setValue(1.5);
            genClef->setChecked(false);
            showBarlines->setChecked(true);
            genTimesig->setChecked(true);
            upsideDown->setChecked(idx == 2 ? true : false);
            f.setFamily("MScoreTabulatureRenaiss");
            f.setPointSizeF(10);
            fretFontName->setCurrentFont(f);
            fretFontSize->setValue(10);
            fretY->setValue(0);
            numbersRadio->setChecked(idx == 2 ? true : false);
            lettersRadio->setChecked(idx == 2 ? false : true);
            onLinesRadio->setChecked(idx == 2 ? true : false);
            aboveLinesRadio->setChecked(idx == 2 ? false : true);
            linesThroughRadio->setChecked(true);
            linesBrokenRadio->setChecked(false);
            f.setFamily("MScoreTabulatureRenaiss");
            f.setPointSizeF(15);
            durFontName->setCurrentFont(f);
            durFontSize->setValue(15);
            durY->setValue(0);
            noteValues0->setChecked(false);
            noteValues1->setChecked(true);
            noteValues2->setChecked(false);
            break;
      }
      updateTabPreview();
      blockTabPreviewSignals(false);
      }

//---------------------------------------------------------
//   Tabulature FullCong/QuickConfig clicked
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
//   Block tabulature rpreview signals
//---------------------------------------------------------

void EditStaffType::blockTabPreviewSignals(bool block)
{
      lineDistance->blockSignals(block);
      showBarlines->blockSignals(block);
      genClef->blockSignals(block);
      genTimesig->blockSignals(block);
      noteValues1->blockSignals(block);
      noteValues2->blockSignals(block);
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
}

//---------------------------------------------------------
//   Update tabulature preview
//---------------------------------------------------------

void EditStaffType::updateTabPreview()
      {
      // update tabulature staff type in preview score
      if(!tabPreview)
            return;
      StaffTypeTablature*  stt = static_cast<StaffTypeTablature*>(tabPreview->score()->staffTypes()[1]);
//      stt->setName(o->text());
//      stt->setLines(lines->value());          // do not change num of lines: sample requires 6 lines
      stt->setLineDistance(Spatium(lineDistance->value()));
      stt->setShowBarlines(showBarlines->isChecked());
      stt->setGenClef(genClef->isChecked());
      stt->setGenTimesig(genTimesig->isChecked());
      stt->setSlashStyle(true);                 // assume no note values
      stt->setGenDurations(false);
      if (noteValues1->isChecked())
            stt->setGenDurations(true);
      if (noteValues2->isChecked())
            stt->setSlashStyle(false);
      stt->setDurationFontName(durFontName->currentText());
      stt->setDurationFontSize(durFontSize->value());
      stt->setDurationFontUserY(durY->value());
      stt->setFretFontName(fretFontName->currentText());
      stt->setFretFontSize(fretFontSize->value());
      stt->setFretFontUserY(fretY->value());
      stt->setLinesThrough(linesThroughRadio->isChecked());
      stt->setOnLines(onLinesRadio->isChecked());
      stt->setUpsideDown(upsideDown->isChecked());
      stt->setUseNumbers(numbersRadio->isChecked());

      tabPreview->score()->doLayout();
#ifdef _USE_NAVIGATOR_PREVIEW_
      tabPreview->layoutChanged();
#endif
      tabPreview->updateAll();
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

