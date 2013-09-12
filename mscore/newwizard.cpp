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

namespace Ms {

extern Palette* newKeySigPalette();
extern void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   InstrumentWizard
//---------------------------------------------------------

InstrumentWizard::InstrumentWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      instrumentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);
      instrumentList->setHeaderLabels(QStringList(tr("Instrument List")));
      instrumentList->setHeaderHidden(true);

      QStringList header = (QStringList() << tr("Staves") << tr("Visib.") << tr("Clef") << tr("Link.") << tr("Staff type"));
      partiturList->setHeaderLabels(header);
      partiturList->setColumnHidden(1, true);  // hide "visible" flag
      partiturList->resizeColumnToContents(3);  // shrink "linked" column as much as possible

      buildTemplateList();

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      linkedButton->setEnabled(false);
      belowButton->setEnabled(false);
      connect( instrumentList, SIGNAL(clicked(const QModelIndex &)), SLOT(expandOrCollapse(const QModelIndex &)));
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentWizard::buildTemplateList()
      {
      // clear search if instrument list is updated
      search->clear();
      filterInstruments(instrumentList);

      populateInstrumentList(instrumentList);
      populateGenreCombo(instrumentGenreFilter);
      }

//---------------------------------------------------------
//   expandOrCollapse
//---------------------------------------------------------

void InstrumentWizard::expandOrCollapse(const QModelIndex &model)
      {
      if(instrumentList->isExpanded(model))
            instrumentList->collapse(model);
      else
            instrumentList->expand(model);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentWizard::init()
      {
      partiturList->clear();
      StaffListItem::populateStaffTypes(0);     // no score yet!
      instrumentList->clearSelection();
      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      linkedButton->setEnabled(false);
      belowButton->setEnabled(false);
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
      if (wi.isEmpty()) {
            removeButton->setEnabled(false);
            upButton->setEnabled(false);
            downButton->setEnabled(false);
            linkedButton->setEnabled(false);
            belowButton->setEnabled(false);
            return;
            }
      QTreeWidgetItem* item = wi.front();
      bool flag = item != 0;

      int count = 0; // item can be hidden
      QTreeWidgetItem* it = 0;
      QList<QTreeWidgetItem*> witems;
      if(item->type() == PART_LIST_ITEM) {
            for (int idx = 0; (it = partiturList->topLevelItem(idx)); ++idx) {
                  if (!it->isHidden()) {
                        count++;
                        witems.append(it);
                        }
                  }
            }
      else {
            for (int idx = 0; (it = item->parent()->child(idx)); ++idx) {
                  if (!it->isHidden()){
                        count++;
                        witems.append(it);
                        }
                  }
            }

      bool onlyOne = (count == 1);
      bool onlyOneStaff = ((item->type() == STAFF_LIST_ITEM) && onlyOne);
      bool first = (witems.first() == item);
      bool last = (witems.last() == item);

      removeButton->setEnabled(flag && !onlyOneStaff);
      upButton->setEnabled(flag && !onlyOne && !first);
      downButton->setEnabled(flag && !onlyOne && !last);
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
      foreach(QTreeWidgetItem* i, instrumentList->selectedItems()) {
            InstrumentTemplateListItem* item = static_cast<InstrumentTemplateListItem*>(i);
            const InstrumentTemplate* it     = item->instrumentTemplate();
            if (it == 0)
                  continue;
            PartListItem* pli = new PartListItem(it, partiturList);
            pli->setFirstColumnSpanned(true);
            pli->op = ITEM_ADD;

            int n = it->nstaves();
            for (int i = 0; i < n; ++i) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->op       = ITEM_ADD;
                  sli->staff    = 0;
                  sli->setPartIdx(i);
                  sli->staffIdx = -1;
                  if (i > MAX_STAVES)
                        sli->setClef(ClefTypeList(ClefType::G, ClefType::G));
                  else
                        sli->setClef(it->clefTypes[i]);
                  sli->setStaffType(it->staffTypePreset);
                  }
            partiturList->setItemExpanded(pli, true);
            partiturList->clearSelection();     // should not be necessary
            partiturList->setItemSelected(pli, true);
            }
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
                  item->setHidden(true);
                  }
            }
      else {
            if (((PartListItem*)item)->op == ITEM_ADD)
                  delete item;
            else {
                  ((PartListItem*)item)->op = ITEM_DELETE;
                  item->setHidden(true);
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
                  // Qt looses the QComboBox set into StaffListItem's when they are re-inserted into the tree:
                  // get the currently selected staff type of each combo and re-insert
                  int numOfStaffListItems = item->childCount();
                  int staffIdx[numOfStaffListItems];
                  int itemIdx;
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item->child(itemIdx)))->staffTypeIdx();
                  partiturList->insertTopLevelItem(idx-1, item);
                  // after-re-insertion, recreate each combo and set its index
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            if (idx) {
                  partiturList->selectionModel()->clear();
                  StaffListItem* item = static_cast<StaffListItem*>(parent->takeChild(idx));
                  // Qt looses the QComboBox set into StaffListItem when it is re-inserted into the tree:
                  // get currently selected staff type and re-insert
                  int staffTypeIdx = item->staffTypeIdx();
                  parent->insertChild(idx-1, item);
                  // after item has been inserted into the tree, create a new QComboBox and set its index
                  item->initStaffTypeCombo(true);
                  item->setStaffType(staffTypeIdx);
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
                  // Qt looses the QComboBox set into StaffListItem's when they are re-inserted into the tree:
                  // get the currently selected staff type of each combo and re-insert
                  int numOfStaffListItems = item->childCount();
                  int staffIdx[numOfStaffListItems];
                  int itemIdx;
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item->child(itemIdx)))->staffTypeIdx();
                  partiturList->insertTopLevelItem(idx+1, item);
                  // after-re-insertion, recreate each combo and set its index
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
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
                  StaffListItem* item = static_cast<StaffListItem*>(parent->takeChild(idx));
                  // Qt looses the QComboBox set into StaffListItem when it is re-inserted into the tree:
                  // get currently selected staff type and re-insert
                  int staffTypeIdx = item->staffTypeIdx();
                  parent->insertChild(idx+1, item);
                  // after item has been inserted into the tree, create a new QComboBox and set its index
                  item->initStaffTypeCombo(true);
                  item->setStaffType(staffTypeIdx);
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
      PartListItem* pli   = (PartListItem*)sli->QTreeWidgetItem::parent();
      pli->setVisible(true);
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      nsli->setLinked(true);
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli)+1, nsli);
      nsli->initStaffTypeCombo();               // StaffListItem needs to be inserted in the tree hierarchy
      nsli->setStaffType(sli->staffTypeIdx());  // before a widget can be set into it
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
      PartListItem* pli   = (PartListItem*)sli->QTreeWidgetItem::parent();
      StaffListItem* nsli = new StaffListItem();
      nsli->staff         = staff;
      nsli->setClef(sli->clef());
      if (staff)
            nsli->op = ITEM_ADD;
      pli->insertChild(pli->indexOfChild(sli)+1, nsli);
      nsli->initStaffTypeCombo();               // StaffListItem needs to be inserted in the tree hierarchy
      nsli->setStaffType(sli->staffTypeIdx());  // before a widget can be set into it
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
                  if(ci->isHidden())
                        continue;
                  StaffListItem* sli = (StaffListItem*)ci;
                  Staff* staff       = new Staff(cs, part, rstaff);
                  sli->staff         = staff;
                  staff->setRstaff(rstaff);
                  ++rstaff;

                  staff->init(t, sli->staffType(), cidx);
                  staff->setClef(0, sli->clef());

                  if (sli->linked() && !part->staves()->isEmpty()) {
                        Staff* linkedStaff = part->staves()->back();
                        linkedStaff->linkTo(staff);
                        }
                  part->staves()->push_back(staff);
                  cs->staves().insert(staffIdx + rstaff, staff);
                  }
            // if a staff was removed from instrument:
            if (part->staves()->at(0)->barLineSpan() > rstaff)
                  part->staves()->at(0)->setBarLineSpan(rstaff);

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

      //
      // check for bar lines
      //
      for (int staffIdx = 0; staffIdx < cs->nstaves();) {
            Staff* staff = cs->staff(staffIdx);
            int barLineSpan = staff->barLineSpan();
            if (barLineSpan == 0)
                  staff->setBarLineSpan(1);
            int nstaffIdx = staffIdx + barLineSpan;

            for (int idx = staffIdx+1; idx < nstaffIdx; ++idx) {
                  Staff* tStaff = cs->staff(idx);
                  if (tStaff)
                        tStaff->setBarLineSpan(0);
                  }

            staffIdx = nstaffIdx;
            }

      //
      // remove instrument names if only one part
      //
      if (cs->parts().size() == 1) {
            Part* part = cs->parts().front();
            if (part->instrList()->size() == 1) {
                  Instrument& instrument = part->instrList()->instrument(0);
                  instrument.setShortName(QTextDocumentFragment());
                  instrument.setLongName(QTextDocumentFragment());
                  }
            }

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
            return Fraction(timesigZ->value(), 1 << timesigN->currentIndex());
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
      *n = 1 << pickupTimesigN->currentIndex();
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
      if (templateFileDialog->selectedFiles().size() > 0) {
            path = templateFileDialog->selectedFiles()[0];
            emit completeChanged();
            }
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
      tempoGroup->setChecked(false);
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
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWizardStyle(QWizard::ClassicStyle);
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setPixmap(QWizard::WatermarkPixmap, QPixmap());
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
      resize(840, 560);
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

//---------------------------------------------------------
//   on_search_textChanged
//---------------------------------------------------------

void InstrumentWizard::on_search_textChanged(const QString &searchPhrase)
      {
      filterInstruments(instrumentList, searchPhrase);
      instrumentGenreFilter->blockSignals(true);
      instrumentGenreFilter->setCurrentIndex(0);
      instrumentGenreFilter->blockSignals(false);
      }

//---------------------------------------------------------
//   on_clearSearch_clicked
//---------------------------------------------------------

void InstrumentWizard::on_clearSearch_clicked()
      {
      search->clear();
      filterInstruments (instrumentList);
      }

//---------------------------------------------------------
//   on_instrumentGenreFilter_currentTextChanged
//---------------------------------------------------------

void InstrumentWizard::on_instrumentGenreFilter_currentIndexChanged(int index)
      {
      QString id = instrumentGenreFilter->itemData(index).toString();
      // Redisplay tree, only showing items from the selected genre
      filterInstrumentsByGenre(instrumentList, id);
      }


//---------------------------------------------------------
//   filterInstrumentsByGenre
//---------------------------------------------------------

void InstrumentWizard::filterInstrumentsByGenre(QTreeWidget *instrumentList, QString genre)
      {
      QTreeWidgetItemIterator iList(instrumentList);
      while (*iList) {
            (*iList)->setHidden(true);
            InstrumentTemplateListItem* itli = static_cast<InstrumentTemplateListItem*>(*iList);
            InstrumentTemplate *it=itli->instrumentTemplate();

            if(it) {
                  if (genre == "all" || it->genreMember(genre)) {
                        (*iList)->setHidden(false);

                        QTreeWidgetItem *iParent = (*iList)->parent();
                        while(iParent) {
                              if(!iParent->isHidden())
                                    break;

                              iParent->setHidden(false);
                              iParent = iParent->parent();
                              }
                        }
                  }
            ++iList;
            }
      }
}

