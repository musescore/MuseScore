//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: instrdialog.cpp 5580 2012-04-27 15:36:57Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "config.h"
#include "editinstrument.h"
#include "icons.h"
#include "instrdialog.h"
#include "musescore.h"
#include "scoreview.h"
#include "seq.h"
#include "libmscore/beam.h"
#include "libmscore/clef.h"
#include "libmscore/drumset.h"
#include "libmscore/excerpt.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/line.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/style.h"
#include "libmscore/system.h"
#include "libmscore/tablature.h"
#include "libmscore/undo.h"

namespace Ms {

void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

StaffListItem::StaffListItem(PartListItem* li)
   : QTreeWidgetItem(li, STAFF_LIST_ITEM)
      {
      op       = ITEM_KEEP;
      staff    = 0;
      setPartIdx(0);
      staffIdx = 0;
      setLinked(false);
      setClef(ClefTypeList(CLEF_G, CLEF_G));
      _staffTypeCombo = 0;
      initStaffTypeCombo();
      }

StaffListItem::StaffListItem()
   : QTreeWidgetItem(STAFF_LIST_ITEM)
      {
      op       = ITEM_KEEP;
      staff    = 0;
      setPartIdx(0);
      staffIdx = 0;
      setClef(ClefTypeList(CLEF_G, CLEF_G));
      setLinked(false);
      _staffTypeCombo = 0;
      }

//---------------------------------------------------------
//   initStaffTypeCombo
//---------------------------------------------------------

void StaffListItem::initStaffTypeCombo(bool forceRecreate)
      {
      if (_staffTypeCombo && !forceRecreate)    // do not init more than once
            return;

      // NOTE: DO NOT DELETE the old _staffTypeCombo if already created:
      // a bug in Qt looses track of (and presumably deletes) the combo set into the item
      // if the item is repositioned in the tree; in this case, the pointer to the combo
      // is no longer valid and cannot be used to delete it
      // Call initStaffTypeCombo(true) ONLY if the item has been repositioned
      // or a memory leak may result

      bool canUseTabs = false;                   // assume only normal staves are applicable
      bool canUsePerc = false;
      PartListItem* part = static_cast<PartListItem*>(QTreeWidgetItem::parent());
      // PartListItem has different members filled out if used in New Score Wizard or in Instruments Wizard
      if (part) {
            Tablature* tab = part->it ? part->it->tablature :
                        ( (part->part && part->part->instr(0)) ? part->part->instr(0)->tablature() : 0);
            canUseTabs = tab && tab->strings() > 0;
            canUsePerc = part->it ? part->it->useDrumset :
                        ( (part->part && part->part->instr(0)) ? part->part->instr(0)->useDrumset() : false);
            }
      _staffTypeCombo = new QComboBox();
      _staffTypeCombo->setAutoFillBackground(true);
      foreach (STAFF_LIST_STAFF_TYPE staffTypeData, staffTypeList)
            if ( (canUseTabs && staffTypeData.staffType->group() == TAB_STAFF)
                        || ( canUsePerc && staffTypeData.staffType->group() == PERCUSSION_STAFF)
                        || (!canUsePerc && staffTypeData.staffType->group() == PITCHED_STAFF) )
                  _staffTypeCombo->addItem(staffTypeData.displayName, staffTypeData.idx);
      treeWidget()->setItemWidget(this, 4, _staffTypeCombo);
      connect(_staffTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(staffTypeChanged(int)) );
      }

//---------------------------------------------------------
//   setPartIdx
//---------------------------------------------------------

void StaffListItem::setPartIdx(int val)
      {
      _partIdx = val;
      setText(0, InstrumentsDialog::tr("Staff %1").arg(_partIdx + 1));
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void StaffListItem::setClef(const ClefTypeList& val)
      {
      _clef = val;
      setText(2, qApp->translate("clefTable", clefTable[_clef._transposingClef].name));
      }

//---------------------------------------------------------
//   setLinked
//---------------------------------------------------------

void StaffListItem::setLinked(bool val)
      {
      _linked = val;
//      setText(3, _linked ? InstrumentsDialog::tr("linked") : "");
      setIcon(3, _linked ? *icons[checkmark_ICON] : QIcon() );
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void StaffListItem::setStaffType(int staffTypeIdx)
      {
      int itemIdx = _staffTypeCombo->findData(staffTypeIdx);
      _staffTypeCombo->setCurrentIndex(itemIdx >= 0 ? itemIdx : 0);
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* StaffListItem::staffType() const
      {
      int staffTypeIdx = _staffTypeCombo->itemData(_staffTypeCombo->currentIndex()).toInt();
      const StaffType* stfType = getListedStaffType(staffTypeIdx);
      return (stfType ? stfType : StaffType::preset(0));
      }

//---------------------------------------------------------
//   staffTypeIdx
//---------------------------------------------------------

int StaffListItem::staffTypeIdx() const
      {
      return _staffTypeCombo->itemData(_staffTypeCombo->currentIndex()).toInt();
      }

//---------------------------------------------------------
//   staffTypeChanged
//---------------------------------------------------------

void StaffListItem::staffTypeChanged(int idx)
      {
      // check current clef matches new staff type
      int staffTypeIdx = _staffTypeCombo->itemData(idx).toInt();
      const StaffType* stfType = getListedStaffType(staffTypeIdx);
      if (stfType->group() != clefTable[_clef._transposingClef].staffGroup) {
            ClefType clefType;
            switch (stfType->group()) {
                  case PITCHED_STAFF:
                        clefType = CLEF_G2;
                        break;
                  case TAB_STAFF:
                        clefType = CLEF_TAB2;
                        break;
                  case PERCUSSION_STAFF:
                        clefType = CLEF_PERC;
                        break;
                  }
            setClef(ClefTypeList(clefType, clefType));
            }
      if (_score && staff && staff->staffType() != stfType)
            if (op != ITEM_DELETE && op != ITEM_ADD)
                  op = ITEM_UPDATE;
      }

//---------------------------------------------------------
//   static members
//---------------------------------------------------------

std::vector<StaffListItem::STAFF_LIST_STAFF_TYPE> StaffListItem::staffTypeList;
Score * StaffListItem::_score = 0;

void StaffListItem::populateStaffTypes(Score *score)
      {
      STAFF_LIST_STAFF_TYPE staffTypeData;
      int idx;

      staffTypeList.clear();
      _score = score;
      if (score) {
            idx = -1;
            foreach(StaffType** staffType, score->staffTypes() ) {
                  staffTypeData.idx             = idx;
                  staffTypeData.displayName     = (*staffType)->name();
                  staffTypeData.staffType       = *staffType;
                  staffTypeList.push_back(staffTypeData);
                  idx--;
                  }
            }
      int numOfPresets = StaffType::numOfPresets();
      for (idx = 0; idx < numOfPresets; idx++) {
            staffTypeData.idx             = idx;
            staffTypeData.staffType       = StaffType::preset(idx);
            staffTypeData.displayName     = staffTypeData.staffType->name();
            staffTypeList.push_back(staffTypeData);
            }
      }

const StaffType* StaffListItem::getListedStaffType(int idx)
      {
      foreach (STAFF_LIST_STAFF_TYPE staffTypeData, staffTypeList) {
            if (staffTypeData.idx == idx)
                  return staffTypeData.staffType;
            }
      return 0;
      }

//---------------------------------------------------------
//   setVisible
//---------------------------------------------------------

void PartListItem::setVisible(bool val)
      {
      setCheckState(1, val ? Qt::Checked : Qt::Unchecked);
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool PartListItem::visible() const
      {
      return checkState(1) == Qt::Checked;
      }

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

PartListItem::PartListItem(Part* p, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = p;
      it   = 0;
      op   = ITEM_KEEP;
      setText(0, p->partName());
      setFlags(flags() | Qt::ItemIsUserCheckable);
      }

PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ITEM_ADD;
      setText(0, it->trackName);
      }

//---------------------------------------------------------
//   InstrumentTemplateListItem
//---------------------------------------------------------

InstrumentTemplateListItem::InstrumentTemplateListItem(QString group, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = 0;
      _group = group;
      setText(0, group);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* item)
   : QTreeWidgetItem(item) {
      _instrumentTemplate = i;
      setText(0, i->trackName);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = i;
      setText(0, i->trackName);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString InstrumentTemplateListItem::text(int col) const
      {
      switch (col) {
            case 0:
                  return _instrumentTemplate ?
                     _instrumentTemplate->trackName : _group;
            default:
                  return QString("");
            }
      }

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

InstrumentsDialog::InstrumentsDialog(QWidget* parent)
   : QDialog(parent)
      {
      editInstrument = 0;
      setupUi(this);
      splitter->setStretchFactor(0, 10);
      splitter->setStretchFactor(1, 0);
      splitter->setStretchFactor(2, 15);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      cs = 0;

      QAction* a = getAction("instruments");
      connect(a, SIGNAL(triggered()), SLOT(reject()));
      addAction(a);

      instrumentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);
      QStringList header = (QStringList() << tr("Staves") << tr("Visib.") << tr("Clef") << tr("Link.") << tr("Staff type"));
      partiturList->setHeaderLabels(header);
      partiturList->resizeColumnToContents(1);  // shrink "visible "and "linked" columns as much as possible
      partiturList->resizeColumnToContents(3);

      buildTemplateList();

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      belowButton->setEnabled(false);
      linkedButton->setEnabled(false);
      connect(showMore, SIGNAL(clicked()), SLOT(buildTemplateList()));
      connect(instrumentList, SIGNAL(clicked(const QModelIndex &)), SLOT(expandOrCollapse(const QModelIndex &)));
      }

//---------------------------------------------------------
//   populateInstrumentList
//---------------------------------------------------------

void populateInstrumentList(QTreeWidget* instrumentList, bool extended)
      {
      instrumentList->clear();
      // TODO: memory leak
      foreach(InstrumentGroup* g, instrumentGroups) {
            if (!extended && g->extended)
                  continue;
            InstrumentTemplateListItem* group = new InstrumentTemplateListItem(g->name, instrumentList);
            group->setFlags(Qt::ItemIsEnabled);
            foreach(InstrumentTemplate* t, g->instrumentTemplates) {
                  if (!extended && t->extended)
                        continue;
                  new InstrumentTemplateListItem(t, group);
                  }
            }
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentsDialog::buildTemplateList()
      {
      // clear search if instrument list is updated
      search->clear();
      filterInstruments(instrumentList, search->text());

      populateInstrumentList(instrumentList, showMore->isChecked());
      }

//---------------------------------------------------------
//   expandOrCollapse
//---------------------------------------------------------

void InstrumentsDialog::expandOrCollapse(const QModelIndex &model)
      {
      if(instrumentList->isExpanded(model))
            instrumentList->collapse(model);
      else
            instrumentList->expand(model);
      }


//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsDialog::genPartList()
      {
      partiturList->clear();
      StaffListItem::populateStaffTypes(cs);

      foreach(Part* p, cs->parts()) {
            PartListItem* pli = new PartListItem(p, partiturList);
            pli->setVisible(p->show());
            foreach(Staff* s, *p->staves()) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->staff    = s;
                  sli->setPartIdx(s->rstaff());
                  sli->staffIdx = s->idx();
                  if (s->isTabStaff()) {
                        ClefType ct(ClefType(cs->styleI(ST_tabClef)));
                        sli->setClef(ClefTypeList(ct, ct));
                        }
                  else
                        sli->setClef(s->clefTypeList(0));
                  const LinkedStaves* ls = s->linkedStaves();
                  sli->setLinked(ls && !ls->isEmpty());
                  int staffTypeIdx = cs->staffTypeIdx(s->staffType());
                  sli->setStaffType(staffTypeIdx != -1 ? -staffTypeIdx - 1 : -1);
                  }
            partiturList->setItemExpanded(pli, true);
            }
      partiturList->resizeColumnToContents(2);  // adjust width of "Clef " and "Staff type" columns
      partiturList->resizeColumnToContents(4);
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsDialog::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      addButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_partiturList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsDialog::on_partiturList_itemSelectionChanged()
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
      bool first = (witems.first() == item);
      bool last = (witems.last() == item);

      removeButton->setEnabled(flag && !onlyOne);
      upButton->setEnabled(flag && !onlyOne && !first);
      downButton->setEnabled(flag && !onlyOne && !last);
      linkedButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      belowButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void InstrumentsDialog::on_instrumentList_itemDoubleClicked(QTreeWidgetItem*, int)
      {
      on_addButton_clicked();
      }

//---------------------------------------------------------
//   on_addButton_clicked
//    add instrument to partitur
//---------------------------------------------------------

void InstrumentsDialog::on_addButton_clicked()
      {
      foreach(QTreeWidgetItem* i, instrumentList->selectedItems()) {
            InstrumentTemplateListItem* item = static_cast<InstrumentTemplateListItem*>(i);
            const InstrumentTemplate* it = item->instrumentTemplate();
            if (it == 0)
                  return;
            PartListItem* pli = new PartListItem(it, partiturList);
            pli->op = ITEM_ADD;

            int n = it->nstaves();
            for (int i = 0; i < n; ++i) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->op       = ITEM_ADD;
                  sli->staff    = 0;
                  sli->setPartIdx(i);
                  sli->staffIdx = -1;
                  sli->setClef(it->clefTypes[i]);
                  sli->setStaffType(it->staffTypePreset);
                  }
            partiturList->setItemExpanded(pli, true);
            partiturList->clearSelection();     // should not be necessary
            partiturList->setItemSelected(pli, true);
            }
      }

//---------------------------------------------------------
//   on_removeButton_clicked
//    remove instrument from partitur
//---------------------------------------------------------

void InstrumentsDialog::on_removeButton_clicked()
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
      }

//---------------------------------------------------------
//   on_upButton_clicked
//    move instrument up in partitur
//---------------------------------------------------------

void InstrumentsDialog::on_upButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();

      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            // if part item not first, move one slot up
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
            // if staff item not first of its part, move one slot up
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
            else {
                  // if staff item first of its part...
                  int parentIdx = partiturList->indexOfTopLevelItem(parent);
                  // ...and there is a previous part, move staff item to previous part
                  if (parentIdx) {
                        partiturList->selectionModel()->clear();
                        QTreeWidgetItem* prevParent = partiturList->topLevelItem(parentIdx - 1);
                        StaffListItem* sli = static_cast<StaffListItem*>(parent->takeChild(idx));
                        int staffTypeIdx = sli->staffTypeIdx();
                        prevParent->addChild(sli);
                        sli->initStaffTypeCombo(true);
                        sli->setStaffType(staffTypeIdx);
                        partiturList->setItemSelected(sli, true);
                        PartListItem* pli = static_cast<PartListItem*>(prevParent);
                        int idx = pli->part->nstaves();
                        cs->undo(new MoveStaff(sli->staff, pli->part, idx));
                        //
                        // TODO : if staff was linked to a staff of the old parent part, unlink it!
                        //
                        }
                  }
            }
      }

//---------------------------------------------------------
//   on_downButton_clicked
//    move instrument down in partitur
//---------------------------------------------------------

void InstrumentsDialog::on_downButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            int n = partiturList->topLevelItemCount();
            // if part not last, move one slot down
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
            // if staff item is not last of its part, move one slot down in part
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
            else {
                  // if staff item is last of its part...
                  int parentIdx = partiturList->indexOfTopLevelItem(parent);
                  int n = partiturList->topLevelItemCount();
                  //..and there is a next part, move to next part
                  if (parentIdx < (n-1)) {
                        partiturList->selectionModel()->clear();
                        StaffListItem* sli = static_cast<StaffListItem*>(parent->takeChild(idx));
                        QTreeWidgetItem* nextParent = partiturList->topLevelItem(parentIdx - 1);
                        int staffTypeIdx = sli->staffTypeIdx();
                        nextParent->addChild(sli);
                        sli->initStaffTypeCombo(true);
                        sli->setStaffType(staffTypeIdx);
                        partiturList->setItemSelected(sli, true);
                        PartListItem* pli = static_cast<PartListItem*>(nextParent);
                        cs->undo(new MoveStaff(sli->staff, pli->part, 0));
                        //
                        // TODO : if staff was linked to a staff of the old parent part, unlink it!
                        //
                        }
                  }
            }
      }

//---------------------------------------------------------
//   on_editButton_clicked
//    start instrument editor for selected instrument
//---------------------------------------------------------

#if 0
void InstrumentsDialog::on_editButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();

      InstrumentTemplateListItem* ti = (InstrumentTemplateListItem*) item;
      InstrumentTemplate* tp         = ti->instrumentTemplate();
      if (tp == 0)
            return;

      if (editInstrument == 0)
            editInstrument = new EditInstrument(this);

      editInstrument->setInstrument(tp);
      editInstrument->show();
      }
#endif

//---------------------------------------------------------
//   on_belowButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_belowButton_clicked()
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
//   on_linkedButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_linkedButton_clicked()
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
//   accept
//---------------------------------------------------------

void InstrumentsDialog::accept()
      {
      done(1);
      }

//---------------------------------------------------------
//   editInstrList
//---------------------------------------------------------

void MuseScore::editInstrList()
      {
      if (cs == 0)
            return;
      if (!instrList)
            instrList = new InstrumentsDialog(this);
      else if (instrList->isVisible()) {
            instrList->done(0);
            return;
            }

      instrList->setScore(cs);
      instrList->genPartList();
      cs->startCmd();
  	cs->deselectAll();
      int rv = instrList->exec();

      if (rv == 0) {
            cs->endCmd();
            return;
            }
  	cs->inputState().setTrack(-1);
      //
      // process modified partitur list
      //

      QTreeWidget* pl = instrList->partiturList;
      Part* part   = 0;
      int staffIdx = 0;
      int rstaff   = 0;

      QTreeWidgetItem* item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = static_cast<PartListItem*>(item);
            // check if the part contains any remaining staves
            // mark to remove part if not
            QTreeWidgetItem* ci = 0;
            int staves = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = static_cast<StaffListItem*>(ci);
                  if (sli->op != ITEM_DELETE)
                        ++staves;
                  }
            if (staves == 0)
                  pli->op = ITEM_DELETE;
            }

      item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            rstaff = 0;
            PartListItem* pli = static_cast<PartListItem*>(item);
            if (pli->op == ITEM_DELETE)
                  cs->cmdRemovePart(pli->part);
            else if (pli->op == ITEM_ADD) {
                  const InstrumentTemplate* t = ((PartListItem*)item)->it;
                  part = new Part(cs);
                  part->initFromInstrTemplate(t);

                  pli->part = part;
                  QTreeWidgetItem* ci = 0;
                  rstaff = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = static_cast<StaffListItem*>(ci);
                        Staff* staff       = new Staff(cs, part, rstaff);
                        sli->staff         = staff;
                        staff->setRstaff(rstaff);

                        staff->init(t, sli->staffType(), cidx);
                        staff->setInitialClef(sli->clef());

                        cs->undoInsertStaff(staff, staffIdx + rstaff);
                        if (sli->linked()) {
                              // TODO: link staff
                              qDebug("TODO: link staff\n");
                              }

                        ++rstaff;
                        }
                  part->staves()->front()->setBarLineSpan(part->nstaves());
                  cs->cmdInsertPart(part, staffIdx);
                  staffIdx += rstaff;
                  }
            else {
                  part = pli->part;
                  if (part->show() != pli->visible()) {
                        part->score()->undo()->push(new ChangePartProperty(part, Part::SHOW, pli->visible()));
                        }
                  QTreeWidgetItem* ci = 0;
                  for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                        StaffListItem* sli = (StaffListItem*)ci;
                        if (sli->op == ITEM_DELETE) {
                              cs->systems()->clear();
                              Staff* staff = sli->staff;
                              int sidx = staff->idx();
                              int eidx = sidx + 1;
                              for (MeasureBase* mb = cs->measures()->first(); mb; mb = mb->next()) {
                                    if (mb->type() != Element::MEASURE)
                                          continue;
                                    Measure* m = (Measure*)mb;
                                    m->cmdRemoveStaves(sidx, eidx);
                                    }
/*                              foreach(Beam* e, cs->beams()) {
                                    int staffIdx = e->staffIdx();
                                    if (staffIdx >= sidx && staffIdx < eidx)
                                          cs->undoRemoveElement(e);
                                    }
 */
                              cs->cmdRemoveStaff(sidx);
                              }
                        else if (sli->op == ITEM_ADD) {
                              Staff* staff = new Staff(cs, part, rstaff);
                              sli->staff   = staff;
                              staff->setRstaff(rstaff);

                              cs->undoInsertStaff(staff, staffIdx);

                              for (Measure* m = cs->firstMeasure(); m; m = m->nextMeasure()) {
                                    // do not create whole measure rests for linked staves
                                    m->cmdAddStaves(staffIdx, staffIdx+1, !sli->linked());
                                    }

                              cs->adjustBracketsIns(staffIdx, staffIdx+1);
                              staff->initFromStaffType(sli->staffType());
                              staff->setInitialClef(sli->clef());
                              KeySigEvent nKey = part->staff(0)->key(0);
                              staff->setKey(0, nKey);

                              if (sli->linked() && rstaff > 0) {
                                    // link to top staff of same part
                                    Staff* linkedStaff = part->staves()->front();
                                    linkedStaff->linkTo(staff);
                                    cloneStaff(linkedStaff, staff);
                                    }
                              ++staffIdx;
                              ++rstaff;
                              }
                        else if (sli->op == ITEM_UPDATE) {
                              // check changes in staff type
                              Staff* staff = sli->staff;
                              const StaffType* stfType = sli->staffType();
                              // before changing staff type, check if notes need to be updated
                              // (true if changing into or away from TAB)
                              StaffGroup ng = stfType->group();         // new staff group
                              StaffGroup og = staff->staffGroup();      // old staff group
                              bool updateNeeded = (ng == TAB_STAFF) != (og == TAB_STAFF);

                              // look for a staff type with same structure among staff types already defined in the score
                              StaffType* st;
                              bool found = false;
                              foreach (StaffType** scoreStaffType, cs->staffTypes()) {
                                    if ( (*scoreStaffType)->isSameStructure(*stfType) ) {
                                          st = *scoreStaffType;         // staff type found in score: use for instrument staff
                                          found = true;
                                          break;
                                          }
                                    }
                              // if staff type not found in score, use from preset (for staff and for adding to score staff types)
                              if (!found) {
                                    st = stfType->clone();
                                    cs->addStaffType(st);
                                    }

                              // use selected staff type
                              if (st != staff->staffType())
                                    cs->undo(new ChangeStaff(staff, staff->small(), staff->invisible(), staff->userDist(), st));
                              if (updateNeeded)
                                    cs->cmdUpdateNotes();
                              }
                        else {
                              ++staffIdx;
                              ++rstaff;
                              }
                        }
                  }
            }

      //
      //    sort staves
      //
      QList<Staff*> dst;
      for (int idx = 0; idx < pl->topLevelItemCount(); ++idx) {
            PartListItem* pli = (PartListItem*)pl->topLevelItem(idx);
            if (pli->op == ITEM_DELETE)
                  continue;
            QTreeWidgetItem* ci = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  StaffListItem* sli = (StaffListItem*) ci;
                  if (sli->op == ITEM_DELETE)
                        continue;
                  dst.push_back(sli->staff);
                  }
            }

      QList<int> dl;
      foreach(Staff* staff, dst) {
            int idx = cs->staves().indexOf(staff);
            if (idx == -1)
                  qDebug("staff in dialog(%p) not found in score\n", staff);
            else
                  dl.push_back(idx);
            }

//      bool sort = false;
//      for (int i = 0; i < dl.size(); ++i) {
//            if (dl[i] != i) {
//                  sort = true;
//                  break;
//                  }
//            }

//      if (sort)
            cs->undo(new SortStaves(cs, dl));

      //
      // check for valid barLineSpan and bracketSpan
      // in all staves
      //

      int n = cs->nstaves();
      for (int i = 0; i < n; ++i) {
            Staff* staff = cs->staff(i);
            if (staff->barLineSpan() > (n - i))
                  cs->undoChangeBarLineSpan(staff, n - i, 0, cs->staff(n-1)->lines()-1);
            QList<BracketItem> brackets = staff->brackets();
            int nn = brackets.size();
            for (int ii = 0; ii < nn; ++ii) {
                  if ((brackets[ii]._bracket != -1) && (brackets[ii]._bracketSpan > (n - i)))
                        cs->undoChangeBracketSpan(staff, ii, n - i);
                  }
            }
      //
      // there should be at least one measure
      //
      if (cs->measures()->size() == 0)
            cs->insertMeasure(Element::MEASURE, 0, false);

      cs->setLayoutAll(true);
      cs->endCmd();
      cs->rebuildMidiMapping();
      seq->initInstruments();
      }

//---------------------------------------------------------
//   on_saveButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_saveButton_clicked()
      {
      QString name = QFileDialog::getSaveFileName(
         this,
         tr("MuseScore: Save Instrument List"),
         ".",
         tr("MuseScore Instruments (*.xml);;")
         );
      if (name.isEmpty())
            return;
      QString ext(".xml");
      QFileInfo info(name);

      if (info.suffix().isEmpty())
            info.setFile(info.filePath() + ext);
      QFile f(info.filePath());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = tr("Open Instruments File\n") + f.fileName() + tr("\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, tr("MuseScore: Open Instruments file"), s);
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      foreach(InstrumentGroup* g, instrumentGroups) {
            xml.stag(QString("InstrumentGroup name=\"%1\" extended=\"%2\"").arg(g->name).arg(g->extended));
            foreach(InstrumentTemplate* t, g->instrumentTemplates)
                  t->write(xml);
            xml.etag();
            }
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = tr("Write Style failed: ") + f.errorString();
            QMessageBox::critical(this, tr("MuseScore: Write Style"), s);
            }
      }

//---------------------------------------------------------
//   on_loadButton_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_loadButton_clicked()
      {
      QString fn = QFileDialog::getOpenFileName(
         this, tr("MuseScore: Load Instrument List"),
          mscoreGlobalShare + "/templates",
         tr("MuseScore Instruments (*.xml);;"
            "All files (*)"
            )
         );
      if (fn.isEmpty())
            return;
      QFile f(fn);
      if (!loadInstrumentTemplates(fn)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: load Style failed:"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      buildTemplateList();
      }

//---------------------------------------------------------
//   filterInstruments
//---------------------------------------------------------

void filterInstruments(QTreeWidget* instrumentList, const QString &searchPhrase)
      {
      QTreeWidgetItem* item = 0;

      for (int idx = 0; (item = instrumentList->topLevelItem(idx)); ++idx) {
            int numMatchedChildren = 0;
            QTreeWidgetItem* ci = 0;

            for (int cidx = 0; (ci = item->child(cidx)); ++cidx) {
                  // replace the unicode b (accidential) so a search phrase of "bb" would give Bb Trumpet...
                  QString text = ci->text(0).replace(QChar(0x266d), QChar('b'));
                  bool isMatch = text.contains(searchPhrase, Qt::CaseInsensitive);
                  ci->setHidden(!isMatch);

                  if (isMatch)
                        numMatchedChildren++;
                  }

            item->setHidden(numMatchedChildren == 0);
            item->setExpanded(numMatchedChildren > 0 && !searchPhrase.isEmpty());
            }
      }

//---------------------------------------------------------
//   on_search_textChanged
//---------------------------------------------------------

void InstrumentsDialog::on_search_textChanged(const QString &searchPhrase)
      {
      filterInstruments(instrumentList, searchPhrase);
      }

//---------------------------------------------------------
//   on_clearSearch_clicked
//---------------------------------------------------------

void InstrumentsDialog::on_clearSearch_clicked()
      {
      search->clear();
      filterInstruments (instrumentList);
      }
}

