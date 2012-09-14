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

      // init tab presets
      //                                                                          clef   bars stemless time  durations                      size off genDur frets                            size off thru  minim style    onLin  rests  stmDn  stmThr upsDn  nums
      _tabPresets[TAB_PRESET_GUITAR]  = new StaffTypeTablature(QString(), 6, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Modern"),  10, 0, false, TAB_MINIM_NONE,true,  false, true,  true,  false, true);
      _tabPresets[TAB_PRESET_BASS]    = new StaffTypeTablature(QString(), 4, 1.5, true,  true, false, false, QString("MuseScore Tab Modern"), 15, 0, false, QString("MuseScore Tab Modern"),  10, 0, false, TAB_MINIM_NONE,true,  false, true,  true,  false, true);
      _tabPresets[TAB_PRESET_ITALIAN] = new StaffTypeTablature(QString(), 6, 1.5, false, true, true,  true,  QString("MuseScore Tab Italian"),15, 0, true,  QString("MuseScore Tab Renaiss"), 10, 0, true,  TAB_MINIM_NONE,true,  true,  false, false, true,  true);
      _tabPresets[TAB_PRESET_FRENCH]  = new StaffTypeTablature(QString(), 6, 1.5, false, true, true,  true,  QString("MuseScore Tab French"), 15, 0, true,  QString("MuseScore Tab Renaiss"), 10, 0, true,  TAB_MINIM_NONE,false, false, false, false, false, false);
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
      connect(presetTablatureCombo, SIGNAL(currentIndexChanged(int)), SLOT(presetTablatureChanged(int)));

      if (ci)
            staffTypeList->setCurrentItem(ci);

      connect(lineDistance,   SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(showBarlines,   SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(genClef,        SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(genTimesig,     SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(noteValues1,    SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(noteValues2,    SIGNAL(toggled(bool)),              SLOT(on_tabStemsToggled(bool)));
      connect(stemAboveRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(stemBelowRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(stemBesideRadio,SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(stemThroughRadio,SIGNAL(toggled(bool)),             SLOT(updateTabPreview()));
//    connect(minimNoneRadio, SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(minimShortRadio,SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(minimSlashedRadio,SIGNAL(toggled(bool)),            SLOT(updateTabPreview()));
      connect(showRests,      SIGNAL(toggled(bool)),              SLOT(updateTabPreview()));
      connect(durFontName,    SIGNAL(currentIndexChanged(int)),   SLOT(updateTabPreview()));
      connect(durFontSize,    SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(durY,           SIGNAL(valueChanged(double)),       SLOT(updateTabPreview()));
      connect(fretFontName,   SIGNAL(currentIndexChanged(int)),   SLOT(updateTabPreview()));
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
      if(_tabPresets[TAB_PRESET_GUITAR])
            delete _tabPresets[TAB_PRESET_GUITAR];
      if(_tabPresets[TAB_PRESET_BASS])
            delete _tabPresets[TAB_PRESET_BASS];
      if(_tabPresets[TAB_PRESET_ITALIAN])
            delete _tabPresets[TAB_PRESET_ITALIAN];
      if(_tabPresets[TAB_PRESET_FRENCH])
            delete _tabPresets[TAB_PRESET_FRENCH];
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
                     || ( noteValues0->isChecked() && (!stt->slashStyle() ||  stt->genDurations()) )
                     || ( noteValues1->isChecked() && (!stt->slashStyle() || !stt->genDurations()) )
                     // if stems, there are more values to take into account
                     || ( noteValues2->isChecked() && ( stt->slashStyle() ||  stt->genDurations()
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
                        setTabFromDlg(stt);
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
                  StaffTypeTablature* stt = static_cast<StaffTypeTablature*>(st);
                  setDlgFromTab(stt);
                  stack->setCurrentIndex(1);
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
//   setDlgFromTab
//
//    initializes dlg controls from a StaffTypeTablature
//---------------------------------------------------------

void EditStaffType::setDlgFromTab(const StaffTypeTablature * stt)
      {
      name->setText(stt->name());
      lines->setValue(stt->lines());
      lineDistance->setValue(stt->lineDistance().val());
      genClef->setChecked(stt->genClef());
      showBarlines->setChecked(stt->showBarlines());
      genTimesig->setChecked(stt->genTimesig());
      upsideDown->setChecked(stt->upsideDown());
      int idx = fretFontName->findText(stt->fretFontName(), Qt::MatchFixedString);
      if(idx == -1)     idx = 0;          // if name not found, use firstt name
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
            noteValues0->setChecked(false);
            noteValues1->setChecked(true);
            noteValues2->setChecked(false);
            }
      else {
            if(stt->slashStyle()) {
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
      showRests->setChecked(stt->showRests());
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
      if (noteValues1->isChecked())
            stt->setGenDurations(true);
      if (noteValues2->isChecked()) {
            stt->setSlashStyle(false);
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

void EditStaffType::presetTablatureChanged(int idx)
      {
      if(idx < TAB_PRESETS) {
            blockTabPreviewSignals(true);             // do not redraw preview for every value we change!
            setDlgFromTab(_tabPresets[idx]);
            blockTabPreviewSignals(false);
            }
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
//   Tabulature note stems toggled
//---------------------------------------------------------

void EditStaffType::on_tabStemsToggled(bool checked)
{
      stemAboveRadio->setEnabled(checked);
      stemBelowRadio->setEnabled(checked);
      stemBesideRadio->setEnabled(checked);
      stemThroughRadio->setEnabled(checked);
      updateTabPreview();
}

//---------------------------------------------------------
//   Block tabulature preview signals
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
      stemAboveRadio->blockSignals(block);
      stemBelowRadio->blockSignals(block);
      stemBesideRadio->blockSignals(block);
      stemThroughRadio->blockSignals(block);
      minimShortRadio->blockSignals(block);
      minimSlashedRadio->blockSignals(block);
      showRests->blockSignals(block);
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
      setTabFromDlg(stt);

      tabPreview->score()->doLayout();
#ifdef _USE_NAVIGATOR_PREVIEW_
      tabPreview->layoutChanged();
#endif
      tabPreview->updateAll();
      // set preset combo: check stt has the same structure as one of the presets
      // if none matches, set as custom
      int idx;
      for(idx=0; idx < TAB_PRESETS; idx++)
            if(stt->isSameStructure(*_tabPresets[idx]))
                  break;
      presetTablatureCombo->blockSignals(true);
      presetTablatureCombo->setCurrentIndex(idx);
      presetTablatureCombo->blockSignals(false);
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

