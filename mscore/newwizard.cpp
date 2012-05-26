//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: newwizard.cpp 5626 2012-05-13 18:33:52Z lasconic $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "newwizard.h"
#include "musescore.h"
#include "preferences.h"
#include "palette.h"
#include "instrdialog.h"

#include "libmscore/instrtemplate.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/clef.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/tablature.h"
#include "libmscore/stafftype.h"
#include "libmscore/timesig.h"
#include "libmscore/sym.h"

extern Palette* newKeySigPalette();

//---------------------------------------------------------
//   InstrumentWizard
//---------------------------------------------------------

InstrumentWizard::InstrumentWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      instrumentList->setSelectionMode(QAbstractItemView::SingleSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);

      instrumentList->setHeaderLabels(QStringList(tr("Instrument List")));

      QStringList header = (QStringList() << tr("Staves") << tr("Visible") << tr("Clef"));
      partiturList->setHeaderLabels(header);
      partiturList->setColumnHidden(1, true);  // hide "visible" flag

      buildTemplateList();

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      linkedButton->setEnabled(false);
      belowButton->setEnabled(false);
      connect(showMore, SIGNAL(clicked()), SLOT(buildTemplateList()));
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentWizard::buildTemplateList()
      {
      populateInstrumentList(instrumentList, showMore->isChecked());
      }


//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentWizard::init()
      {
      partiturList->clear();
      emit completeChanged(false);
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentWizard::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      addButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_partiturList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentWizard::on_partiturList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      bool flag = item != 0;
      removeButton->setEnabled(flag);
      upButton->setEnabled(flag);
      downButton->setEnabled(flag);
      linkedButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      belowButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void InstrumentWizard::on_instrumentList_itemActivated(QTreeWidgetItem*, int)
      {
      on_addButton_clicked();
      }

//---------------------------------------------------------
//   on_addButton_clicked
//    add instrument to partitur
//---------------------------------------------------------

void InstrumentWizard::on_addButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return;
      InstrumentTemplateListItem* item = (InstrumentTemplateListItem*)wi.front();
      const InstrumentTemplate* it     = item->instrumentTemplate();
      if (it == 0)
            return;
      PartListItem* pli = new PartListItem(it, partiturList);
      pli->op = ITEM_ADD;

      int n = it->staves;
      for (int i = 0; i < n; ++i) {
            StaffListItem* sli = new StaffListItem(pli);
            sli->op       = ITEM_ADD;
            sli->staff    = 0;
            sli->setPartIdx(i);
            sli->staffIdx = -1;
            if (i > MAX_STAVES)
                  sli->setClef(CLEF_G);
            else
                  sli->setClef(it->clefIdx[i]);
            }
      partiturList->setItemExpanded(pli, true);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(pli, true);
      emit completeChanged(true);
      }

//---------------------------------------------------------
//   on_removeButton_clicked
//    remove instrument from partitur
//---------------------------------------------------------

void InstrumentWizard::on_removeButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      QTreeWidgetItem* parent = item->parent();

      if (parent) {
            if (((StaffListItem*)item)->op == ITEM_ADD) {
                  if (parent->childCount() == 1) {
                        partiturList->takeTopLevelItem(partiturList->indexOfTopLevelItem(parent));
                        delete parent;
                        }
                  else {
                        parent->takeChild(parent->indexOfChild(item));
                        delete item;
                        }
                  }
            else {
                  ((StaffListItem*)item)->op = ITEM_DELETE;
                  partiturList->setItemHidden(item, true);
                  }
            }
      else {
            if (((PartListItem*)item)->op == ITEM_ADD)
                  delete item;
            else {
                  ((PartListItem*)item)->op = ITEM_DELETE;
                  partiturList->setItemHidden(item, true);
                  }
            }
      partiturList->clearSelection();
      emit completeChanged(partiturList->topLevelItemCount() > 0);
      }

//---------------------------------------------------------
//   on_upButton_clicked
//    move instrument up in partitur
//---------------------------------------------------------

void InstrumentWizard::on_upButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();

      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            if (idx) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                  partiturList->insertTopLevelItem(idx-1, item);
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            if (idx) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = parent->takeChild(idx);
                  parent->insertChild(idx-1, item);
                  partiturList->setItemSelected(item, true);
                  }
            }
      }

//---------------------------------------------------------
//   on_downButton_clicked
//    move instrument down in partitur
//---------------------------------------------------------

void InstrumentWizard::on_downButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            int n = partiturList->topLevelItemCount();
            if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                  partiturList->insertTopLevelItem(idx+1, item);
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            int n = parent->childCount();
            if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = parent->takeChild(idx);
                  parent->insertChild(idx+1, item);
                  partiturList->setItemSelected(item, true);
                  }
            }
      }

//---------------------------------------------------------
//   on_linkedButton_clicked
//---------------------------------------------------------

void InstrumentWizard::on_linkedButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return;

      StaffListItem* sli  = (StaffListItem*)item;
      Staff* staff        = sli->staff;
      PartListItem* pli   = (PartListItem*)sli->parent();
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      nsli->setLinked(true);
      nsli->setVisible(true);
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli)+1, nsli);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(nsli, true);
      }

//---------------------------------------------------------
//   on_belowButton_clicked
//---------------------------------------------------------

void InstrumentWizard::on_belowButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return;

      StaffListItem* sli  = (StaffListItem*)item;
      Staff* staff        = sli->staff;
      PartListItem* pli   = (PartListItem*)sli->parent();
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli)+1, nsli);
      partiturList->clearSelection();     // should not be necessary
      partiturList->setItemSelected(nsli, true);
      }

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void InstrumentWizard::createInstruments(Score* cs)
      {
      //
      // process modified partitur list
      //
      QTreeWidget* pl = partiturList;
      Part* part   = 0;
      int staffIdx = 0;

      QTreeWidgetItem* item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = (PartListItem*)item;
            if (pli->op != ITEM_ADD) {
                  qDebug("bad op\n");
                  continue;
                  }
            const InstrumentTemplate* t = ((PartListItem*)item)->it;
            part = new Part(cs);
            part->initFromInstrTemplate(t);

            pli->part = part;
            QTreeWidgetItem* ci = 0;
            int rstaff = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*)ci;
                  Staff* staff       = new Staff(cs, part, rstaff);
                  sli->staff         = staff;
                  staff->setRstaff(rstaff);
                  ++rstaff;

                  staff->init(t, cidx);

                  if (sli->linked() && !part->staves()->isEmpty()) {
                        Staff* linkedStaff = part->staves()->back();
                        linkedStaff->linkTo(staff);
                        }
                  part->staves()->push_back(staff);
                  cs->staves().insert(staffIdx + rstaff, staff);
                  }

            // insert part
            cs->insertPart(part, staffIdx);
            int sidx = cs->staffIdx(part);
            int eidx = sidx + part->nstaves();
            for (Measure* m = cs->firstMeasure(); m; m = m->nextMeasure())
                  m->cmdAddStaves(sidx, eidx, true);
            staffIdx += rstaff;
            }
      //
      //    sort staves
      //
      QList<Staff*> dst;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = (PartListItem*)item;
            if (pli->op == ITEM_DELETE)
                  continue;
            QTreeWidgetItem* ci = 0;
            for (int cidx = 0; (ci = item->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*) ci;
                  if (sli->op == ITEM_DELETE)
                        continue;
                  dst.push_back(sli->staff);
                  }
            }
#if 1
      //
      // check for bar lines
      //
      for (int staffIdx = 0; staffIdx < cs->nstaves();) {
            Staff* staff = cs->staff(staffIdx);
            int barLineSpan = staff->barLineSpan();
            if (barLineSpan == 0)
                  staff->setBarLineSpan(1);
            int nstaffIdx = staffIdx + barLineSpan;

            for (int idx = staffIdx+1; idx < nstaffIdx; ++idx)
                  cs->staff(idx)->setBarLineSpan(0);

            staffIdx = nstaffIdx;
            }
#endif
      cs->setLayoutAll(true);
      }

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

TimesigWizard::TimesigWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      QPixmap ct = sym2pixmap(&symbols[0][fourfourmeterSym], 3.0);
      tsCommonTime->setIcon(QIcon(ct));
      tsCommonTime->setText(QString());
      ct = sym2pixmap(&symbols[0][allabreveSym], 3.0);
      tsCutTime->setIcon(QIcon(ct));
      tsCutTime->setText(QString());
      connect(tsCommonTime, SIGNAL(toggled(bool)), SLOT(commonTimeToggled(bool)));
      connect(tsCutTime,    SIGNAL(toggled(bool)), SLOT(cutTimeToggled(bool)));
      connect(tsFraction,   SIGNAL(toggled(bool)), SLOT(fractionToggled(bool)));
      }

//---------------------------------------------------------
//   measures
//---------------------------------------------------------

int TimesigWizard::measures() const
      {
      return measureCount->value();
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

Fraction TimesigWizard::timesig() const
      {
      if (tsFraction->isChecked())
            return Fraction(timesigZ->value(), timesigN->value());
      else if (tsCommonTime->isChecked())
            return Fraction(4, 4);
      else
            return Fraction(2, 2);
      }

//---------------------------------------------------------
//   pickupMeasure
//---------------------------------------------------------

bool TimesigWizard::pickup(int* z, int* n) const
      {
      *z = pickupTimesigZ->value();
      *n = pickupTimesigN->value();
      return pickupMeasure->isChecked();
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

TimeSigType TimesigWizard::type() const
      {
      if (tsFraction->isChecked())
            return TSIG_NORMAL;
      if (tsCommonTime->isChecked())
            return TSIG_FOUR_FOUR;
      return TSIG_ALLA_BREVE;
      }

//---------------------------------------------------------
//   commonTimeToggled
//---------------------------------------------------------

void TimesigWizard::commonTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(4);
            // timesigN->setValue(4);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   cutTimeToggled
//---------------------------------------------------------

void TimesigWizard::cutTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(2);
            // timesigN->setValue(2);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   fractionToggled
//---------------------------------------------------------

void TimesigWizard::fractionToggled(bool val)
      {
      if (val) {
            timesigZ->setEnabled(true);
            timesigN->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

TitleWizard::TitleWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

NewWizardPage1::NewWizardPage1(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("This wizard creates a new score"));

      w = new TitleWizard;

      registerField("useTemplate", w->rb1, "checked");
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage1::initializePage()
      {
      w->title->setText("");
      w->subtitle->setText("");
      // w->composer->text();
      // w->poet->text();
      // w->copyright->text();
      }

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

NewWizardPage2::NewWizardPage2(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Define a set of instruments. Each instrument"
                     " is represented by one or more staves"));

      complete = false;
      w = new InstrumentWizard;
      QGridLayout* grid = new QGridLayout;
      grid->setSpacing(0);
      grid->setContentsMargins(0, 0, 0, 0);
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      connect(w, SIGNAL(completeChanged(bool)), this, SLOT(setComplete(bool)));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage2::initializePage()
      {
      w->init();
      }

//---------------------------------------------------------
//   setComplete
//---------------------------------------------------------

void NewWizardPage2::setComplete(bool val)
      {
      complete = val;
      emit completeChanged();
      }

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

NewWizardPage3::NewWizardPage3(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Create Time Signature"));
      w = new TimesigWizard;
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

NewWizardPage4::NewWizardPage4(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Select Template File:"));

      templateFileDialog = new QFileDialog;
      templateFileDialog->setParent(this);
      templateFileDialog->setModal(false);
      templateFileDialog->setSizeGripEnabled(false);
      templateFileDialog->setFileMode(QFileDialog::ExistingFile);
      templateFileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
      templateFileDialog->setWindowTitle(tr("MuseScore: Select Template"));
      QString filter = tr("MuseScore Template Files (*.mscz *.mscx)");
      templateFileDialog->setNameFilter(filter);
      templateFileDialog->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

      QFileInfo myTemplates(preferences.myTemplatesPath);
      if (myTemplates.isRelative())
            myTemplates.setFile(QDir::home(), preferences.myTemplatesPath);
      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare + "templates"));
      urls.append(QUrl::fromLocalFile(myTemplates.absoluteFilePath()));
      templateFileDialog->setSidebarUrls(urls);

      QSettings settings;
      templateFileDialog->restoreState(settings.value("templateFileDialog").toByteArray());
      templateFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
      templateFileDialog->setDirectory(mscoreGlobalShare + "templates");

      QLayout* layout = new QVBoxLayout;
      layout->addWidget(templateFileDialog);
      setLayout(layout);

      connect(templateFileDialog, SIGNAL(currentChanged(const QString&)), SLOT(templateChanged(const QString&)));
      connect(templateFileDialog, SIGNAL(accepted()), SLOT(fileAccepted()));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage4::initializePage()
      {
      // modify dialog
      // possibly this is not portable as we make some assumptions on the
      // implementation of QFileDialog

      templateFileDialog->show();
      QList<QPushButton*>widgets = templateFileDialog->findChildren<QPushButton*>();
      foreach(QPushButton* w, widgets) {
            w->setEnabled(false);
            w->setVisible(false);
            }
      path.clear();
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage4::isComplete() const
      {
      return !path.isEmpty();
      }

//---------------------------------------------------------
//   fileAccepted
//---------------------------------------------------------

void NewWizardPage4::fileAccepted()
      {
      templateFileDialog->show();
      wizard()->next();
      }

//---------------------------------------------------------
//   templateChanged
//---------------------------------------------------------

void NewWizardPage4::templateChanged(const QString& s)
      {
      path = s;
      emit completeChanged();
      }

//---------------------------------------------------------
//   templatePath
//---------------------------------------------------------

QString NewWizardPage4::templatePath() const
      {
      bool useTemplate = field("useTemplate").toBool();
      if (useTemplate)
            return path;
      return QString();
      }


//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

NewWizardPage5::NewWizardPage5(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Select Key Signature and Tempo:"));

      QGroupBox* b1 = new QGroupBox;
      b1->setTitle(tr("Key Signature"));
      sp = MuseScore::newKeySigPalette();
      sp->setSelectable(true);
      sp->setSelected(14);
      PaletteScrollArea* sa = new PaletteScrollArea(sp);
      QVBoxLayout* l1 = new QVBoxLayout;
      l1->addWidget(sa);
      b1->setLayout(l1);

      tempoGroup = new QGroupBox;
      tempoGroup->setCheckable(true);
      tempoGroup->setChecked(true);
      tempoGroup->setTitle(tr("Tempo"));
      QLabel* bpm = new QLabel;
      bpm->setText(tr("BPM:"));
      _tempo = new QDoubleSpinBox;
      _tempo->setRange(20.0, 400.0);
      _tempo->setValue(100.0);
      QHBoxLayout* l2 = new QHBoxLayout;
      l2->addWidget(bpm);
      l2->addWidget(_tempo);
      l2->addStretch(100);
      tempoGroup->setLayout(l2);

      QVBoxLayout* l3 = new QVBoxLayout;
      l3->addWidget(b1);
      l3->addWidget(tempoGroup);
      l3->addStretch(100);
      setLayout(l3);
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

KeySigEvent NewWizardPage5::keysig() const
      {
      int idx    = sp->getSelectedIdx();
      Element* e = sp->element(idx);
      return static_cast<KeySig*>(e)->keySigEvent();
      }

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

NewWizard::NewWizard(QWidget* parent)
   : QWizard(parent)
      {
      setWizardStyle(QWizard::ClassicStyle);
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setPixmap(QWizard::WatermarkPixmap, 0);
      setWindowTitle(tr("MuseScore: Create New Score"));
      setOption(QWizard::NoCancelButton, false);
      setOption(QWizard::CancelButtonOnLeft, true);
      setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
      setOption(QWizard::HaveNextButtonOnLastPage, true);


      p1 = new NewWizardPage1;
      p2 = new NewWizardPage2;
      p3 = new NewWizardPage3;
      p4 = new NewWizardPage4;
      p5 = new NewWizardPage5;

      setPage(Page_Type, p1);
      setPage(Page_Instruments, p2);
      setPage(Page_Template, p4);
      setPage(Page_Timesig, p3);
      setPage(Page_Keysig, p5);
      p2->setFinalPage(true);
      p3->setFinalPage(true);
      p4->setFinalPage(true);
      p5->setFinalPage(true);
      resize(700, 500);
      }

//---------------------------------------------------------
//   nextId
//---------------------------------------------------------

int NewWizard::nextId() const
      {
      switch(currentId()) {
            case Page_Type:
                  return useTemplate() ? Page_Template : Page_Instruments;
            case Page_Instruments:
                  return Page_Keysig;
            case Page_Keysig:
                  return Page_Timesig;
            case Page_Template:
                  return Page_Keysig;
            case Page_Timesig:
            default:
                  return -1;
            }
      }

//---------------------------------------------------------
//   useTemplate
//---------------------------------------------------------

bool NewWizard::useTemplate() const
      {
      return field("useTemplate").toBool();
      }

