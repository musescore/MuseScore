//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "icons.h"
#include "instrwidget.h"
#include "stringutils.h"

#include "libmscore/clef.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/line.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/style.h"
#include "libmscore/system.h"
#include "libmscore/stringdata.h"
#include "libmscore/undo.h"
#include "libmscore/keysig.h"

namespace Ms {

int StaffListItem::customStandardIdx;
int StaffListItem::customPercussionIdx;
int StaffListItem::customTablatureIdx;

void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString());

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
                  // replace the unicode b (accidental) so a search phrase of "bb" would give Bb Trumpet...
                  QString text = ci->text(0).replace(QChar(0x266d), QChar('b'));

                  // remove ligatures and diacritics
                  QString removedSpecialChar = stringutils::removeLigatures(text);
                  removedSpecialChar = stringutils::removeDiacritics(removedSpecialChar);

                  bool isMatch = text.contains(searchPhrase, Qt::CaseInsensitive) || removedSpecialChar.contains(searchPhrase, Qt::CaseInsensitive);
                  ci->setHidden(!isMatch);

                  if (isMatch)
                        numMatchedChildren++;
                  }

            item->setHidden(numMatchedChildren == 0);
            item->setExpanded(numMatchedChildren > 0 && !searchPhrase.isEmpty());
            }
      }

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

StaffListItem::StaffListItem(PartListItem* li)
   : QTreeWidgetItem(li, STAFF_LIST_ITEM)
      {
      setPartIdx(0);
      setLinked(false);
      setClefType(ClefTypeList(ClefType::G));
      initStaffTypeCombo();
      }

StaffListItem::StaffListItem()
   : QTreeWidgetItem(STAFF_LIST_ITEM)
      {
      setPartIdx(0);
      setLinked(false);
      setClefType(ClefTypeList(ClefType::G));
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

      bool canUseTabs = false; // assume only normal staves are applicable
      int numFrettedStrings = 0;
      bool canUsePerc = false;
      PartListItem* part = static_cast<PartListItem*>(QTreeWidgetItem::parent());

      // PartListItem has different members filled out if used in New Score Wizard
      // or in Instruments Wizard
      if (part) {
            const StringData* stringData = part->it ? &(part->it->stringData) :
                        ( (part->part && part->part->instrument()) ? part->part->instrument()->stringData() : 0);
            canUseTabs = stringData && stringData->frettedStrings() > 0;
            if (canUseTabs)
                  numFrettedStrings = stringData->frettedStrings();
            canUsePerc = part->it ? part->it->useDrumset :
                        ( (part->part && part->part->instrument()) ? part->part->instrument()->useDrumset() : false);
            }
      _staffTypeCombo = new QComboBox();
      _staffTypeCombo->setAutoFillBackground(true);
      int idx = 0;
      for (const StaffType& st : StaffType::presets()) {
            if ( (st.group() == StaffGroup::STANDARD && (!canUsePerc))    // percussion excludes standard
                        || (st.group() == StaffGroup::PERCUSSION && canUsePerc)
                        || (st.group() == StaffGroup::TAB && canUseTabs && st.lines() <= numFrettedStrings)) {
                  _staffTypeCombo->addItem(st.name(), idx);
                  }
            ++idx;
            }
      customStandardIdx = _staffTypeCombo->count();
      _staffTypeCombo->addItem(tr("Custom Standard"), 0);
      customPercussionIdx = _staffTypeCombo->count();
      _staffTypeCombo->addItem(tr("Custom Percussion"), 0);
      customTablatureIdx = _staffTypeCombo->count();
      _staffTypeCombo->addItem(tr("Custom Tablature"), 0);

      treeWidget()->setItemWidget(this, 4, _staffTypeCombo);
      connect(_staffTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(staffTypeChanged(int)) );
      }

//---------------------------------------------------------
//   setPartIdx
//---------------------------------------------------------

void StaffListItem::setPartIdx(int val)
      {
      _partIdx = val;
      setText(0, InstrumentsWidget::tr("Staff %1").arg(_partIdx + 1));
      }

//---------------------------------------------------------
//   setClefType
//---------------------------------------------------------

void StaffListItem::setClefType(const ClefTypeList& val)
      {
      _clefType = val;
      setText(2, qApp->translate("clefTable", ClefInfo::name(_clefType._transposingClef)));
      }

//---------------------------------------------------------
//   setLinked
//---------------------------------------------------------

void StaffListItem::setLinked(bool val)
      {
      _linked = val;
      setIcon(3, _linked ? *icons[int(Icons::checkmark_ICON)] : QIcon() );
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void StaffListItem::setStaffType(const StaffType* st)
      {
      if (!st)                                        // if no staff type given, dault to stadard
            _staffTypeCombo->setCurrentIndex(0);      // staff type (at combo box index 0)
      else {
            // if staff type given, look into combo box item data for a preset equal to staff type
            for (int i = 0; i < _staffTypeCombo->count(); ++i) {
                  const StaffType* _st = StaffType::preset(StaffTypes(_staffTypeCombo->itemData(i).toInt()));
                  if (*_st == *st) {
                        _staffTypeCombo->setCurrentIndex(i);
                        return;
                        }
                  }
            // try harder
            for (int i = 0; i < _staffTypeCombo->count(); ++i) {
                  const StaffType* _st = StaffType::preset(StaffTypes(_staffTypeCombo->itemData(i).toInt()));
                  if (_st->isSameStructure(*st)) {
                        _staffTypeCombo->setCurrentIndex(i);
                        return;
                        }
                  }
            int idx = 0;
            switch (st->group()) {
                  case StaffGroup::STANDARD:
                        idx = customStandardIdx;
                        break;
                  case StaffGroup::PERCUSSION:
                        idx = customPercussionIdx;
                        break;
                  case StaffGroup::TAB:
                        idx = customTablatureIdx;
                        break;
                  }
            _staffTypeCombo->setCurrentIndex(idx);
            }
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void StaffListItem::setStaffType(int idx)
      {
      int i = _staffTypeCombo->findData(idx);
      if (i != -1)
            _staffTypeCombo->setCurrentIndex(i);
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* StaffListItem::staffType() const
      {
      return StaffType::preset(StaffTypes((staffTypeIdx())));
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
      const StaffType* stfType = StaffType::preset(StaffTypes(staffTypeIdx));

      PartListItem* pli = static_cast<PartListItem*>(QTreeWidgetItem::parent());
      pli->updateClefs();

      if (_staff && _staff->staffType(0)->name() != stfType->name()) {
            if (_op != ListItemOp::I_DELETE && _op != ListItemOp::ADD)
                  _op = ListItemOp::UPDATE;
            }
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
//   updateClefs
//---------------------------------------------------------

void PartListItem::updateClefs()
      {
      for (int i = 0; i < childCount(); ++i) {
            StaffListItem* sli = static_cast<StaffListItem*>(child(i));
            const StaffType* stfType = StaffType::preset(StaffTypes(sli->staffTypeIdx()));

            ClefTypeList clefType;
            switch (stfType->group()) {
                  case StaffGroup::STANDARD:
                        clefType = sli->defaultClefType();
                        break;
                  case StaffGroup::TAB:
                        clefType = ClefTypeList(ClefType::TAB);
                        break;
                  case StaffGroup::PERCUSSION:
                        clefType = ClefTypeList(ClefType::PERC);
                        break;
                  }
            sli->setClefType(clefType);
            }
      }

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

PartListItem::PartListItem(Part* p, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = p;
      it   = 0;
      op   = ListItemOp::KEEP;
      setText(0, p->partName());
      setFlags(flags() | Qt::ItemIsUserCheckable);
      }

PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ListItemOp::ADD;
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
//   InstrumentsWidget
//---------------------------------------------------------

InstrumentsWidget::InstrumentsWidget(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      splitter->setStretchFactor(0, 10);
      splitter->setStretchFactor(1, 0);
      splitter->setStretchFactor(2, 15);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      instrumentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);
      QStringList header = (QStringList() << tr("Staves") << tr("Visible") << tr("Clef") << tr("Linked") << tr("Staff type"));
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

      connect(instrumentList, SIGNAL(clicked(const QModelIndex &)), SLOT(expandOrCollapse(const QModelIndex &)));
      }

//---------------------------------------------------------
//   populateGenreCombo
//---------------------------------------------------------

void populateGenreCombo(QComboBox* combo)
      {
      combo->clear();
      combo->addItem(qApp->translate("InstrumentsDialog", "All instruments"), "all");
      int i = 1;
      int defaultIndex = 0;
      foreach (InstrumentGenre *ig, instrumentGenres) {
            combo->addItem(ig->name, ig->id);
            if (ig->id == "common")
                  defaultIndex = i;
            ++i;
            }
      combo->setCurrentIndex(defaultIndex);
      }

//---------------------------------------------------------
 //   populateInstrumentList
//---------------------------------------------------------

void populateInstrumentList(QTreeWidget* instrumentList)
      {
      instrumentList->clear();
      // TODO: memory leak?
      foreach(InstrumentGroup* g, instrumentGroups) {
            InstrumentTemplateListItem* group = new InstrumentTemplateListItem(g->name, instrumentList);
            group->setFlags(Qt::ItemIsEnabled);
            for (InstrumentTemplate* t : g->instrumentTemplates) {
                  new InstrumentTemplateListItem(t, group);
                  }
            }
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentsWidget::buildTemplateList()
      {
      // clear search if instrument list is updated
      search->clear();

      populateInstrumentList(instrumentList);
      populateGenreCombo(instrumentGenreFilter);
      }

//---------------------------------------------------------
//   expandOrCollapse
//---------------------------------------------------------

void InstrumentsWidget::expandOrCollapse(const QModelIndex &model)
      {
      if(instrumentList->isExpanded(model))
            instrumentList->collapse(model);
      else
            instrumentList->expand(model);
      }

//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsWidget::genPartList(Score* cs)
      {
      partiturList->clear();

      foreach (Part* p, cs->parts()) {
            PartListItem* pli = new PartListItem(p, partiturList);
            pli->setVisible(p->show());
            for (Staff* s : *p->staves()) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->setStaff(s);
                  sli->setClefType(s->clefType(0));
                  sli->setDefaultClefType(s->defaultClefType());
                  sli->setPartIdx(s->rstaff());
                  const LinkedElements* ls = s->links();
                  bool bLinked = false;
                  if (ls && !ls->empty()) {
                        for (auto le : *ls) {
                              Staff* ps = toStaff(le);
                              if (ps != s && ps->score() == s->score()) {
                                    bLinked = true;
                                    break;
                                    }
                              }
                        }
                  sli->setLinked(bLinked);
                  sli->setStaffType(s->staffType(0));    // TODO
                  }
            pli->updateClefs();
            partiturList->setItemExpanded(pli, true);
            }
      partiturList->resizeColumnToContents(2);  // adjust width of "Clef " and "Staff type" columns
      partiturList->resizeColumnToContents(4);
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsWidget::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      addButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_partiturList_itemSelectionChanged
//---------------------------------------------------------

void InstrumentsWidget::on_partiturList_itemSelectionChanged()
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

void InstrumentsWidget::on_instrumentList_itemDoubleClicked(QTreeWidgetItem*, int)
      {
      on_addButton_clicked();
      }

//---------------------------------------------------------
//   on_addButton_clicked
//    add instrument to partitur
//---------------------------------------------------------

void InstrumentsWidget::on_addButton_clicked()
      {
      foreach(QTreeWidgetItem* i, instrumentList->selectedItems()) {
            InstrumentTemplateListItem* item = static_cast<InstrumentTemplateListItem*>(i);
            const InstrumentTemplate* it     = item->instrumentTemplate();
            if (it == 0)
                  continue;
            PartListItem* pli = new PartListItem(it, partiturList);
            pli->setFirstColumnSpanned(true);
            pli->op = ListItemOp::ADD;

            int n = it->nstaves();
            for (int i1 = 0; i1 < n; ++i1) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->setOp(ListItemOp::ADD);
                  sli->setStaff(0);
                  sli->setPartIdx(i1);
                  sli->setDefaultClefType(it->clefType(i1));
                  sli->setStaffType(it->staffTypePreset);
                  }
            pli->updateClefs();
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

void InstrumentsWidget::on_removeButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item   = wi.front();
      QTreeWidgetItem* parent = item->parent();

      if (parent) {
            if (((StaffListItem*)item)->op() == ListItemOp::ADD) {
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
                  ((StaffListItem*)item)->setOp(ListItemOp::I_DELETE);
                  item->setHidden(true);

                  // check if a staff is linked to this staff

                  int idx = parent->indexOfChild(item);
                  StaffListItem* sli = static_cast<StaffListItem*>(parent->child(idx+1));
                  if (sli) {
                        StaffListItem* sli2 = static_cast<StaffListItem*>(parent->child(idx+2));
                        if (sli->linked() && !(sli2 && sli2->linked())) {
                              sli->setLinked(false);
                              partiturList->update();
                              }
                        }
                  }
            static_cast<PartListItem*>(parent)->updateClefs();
            partiturList->setItemSelected(parent, true);
            }
      else {
            int idx = partiturList->indexOfTopLevelItem(item);
            if (((PartListItem*)item)->op == ListItemOp::ADD)
                  delete item;
            else {
                  ((PartListItem*)item)->op = ListItemOp::I_DELETE;
                  item->setHidden(true);
                  }
            // select an item, do not consider hidden ones
            int plusIdx = 0;
            QTreeWidgetItem* nextParent = partiturList->topLevelItem(idx + plusIdx);
            while (nextParent && nextParent->isHidden()) {
                  plusIdx++;
                  nextParent = partiturList->topLevelItem(idx + plusIdx);
                  }
            if(!nextParent) { // could find after, check before
                  plusIdx = 1;
                  nextParent = partiturList->topLevelItem(idx - plusIdx);
                  while (nextParent && nextParent->isHidden()) {
                       plusIdx++;
                       nextParent = partiturList->topLevelItem(idx - plusIdx);
                  }
                  }
            partiturList->setItemSelected(nextParent, true);
            }
      }

//---------------------------------------------------------
//   on_upButton_clicked
//    move instrument up in partitur
//---------------------------------------------------------

void InstrumentsWidget::on_upButton_clicked()
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
                  QTreeWidgetItem* item1 = partiturList->takeTopLevelItem(idx);
                  // Qt looses the QComboBox set into StaffListItem's when they are re-inserted into the tree:
                  // get the currently selected staff type of each combo and re-insert
                  int numOfStaffListItems = item1->childCount();
#if (!defined (_MSCVER) && !defined (_MSC_VER))
                  int staffIdx[numOfStaffListItems];
#else
                  // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
                  //    heap allocation is slow, an optimization might be used.
                  std::vector<int> staffIdx(numOfStaffListItems);
#endif
                  for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item1->child(itemIdx)))->staffTypeIdx();
                  // do not consider hidden ones
                  int minusIdx = 1;
                  QTreeWidgetItem* prevParent = partiturList->topLevelItem(idx - minusIdx);
                  while (prevParent && prevParent->isHidden()) {
                       minusIdx++;
                       prevParent = partiturList->topLevelItem(idx - minusIdx);
                  }
                  partiturList->insertTopLevelItem(idx - minusIdx, item1);
                  // after-re-insertion, recreate each combo and set its index
                  for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item1->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
                  partiturList->setItemExpanded(item1, isExpanded);
                  partiturList->setItemSelected(item1, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            // if staff item not first of its part, move one slot up
            if (idx) {
                  partiturList->selectionModel()->clear();
                  StaffListItem* item1 = static_cast<StaffListItem*>(parent->takeChild(idx));
                  // Qt looses the QComboBox set into StaffListItem when it is re-inserted into the tree:
                  // get currently selected staff type and re-insert
                  int staffTypeIdx = item1->staffTypeIdx();
                  parent->insertChild(idx - 1, item1);
                  // after item has been inserted into the tree, create a new QComboBox and set its index
                  item1->initStaffTypeCombo(true);
                  item1->setStaffType(staffTypeIdx);
                  partiturList->setItemSelected(item1, true);
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
//                        PartListItem* pli = static_cast<PartListItem*>(prevParent);
//                        int idx = pli->part->nstaves();
//??                        cs->undo(new MoveStaff(sli->staff(), pli->part, idx));
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

void InstrumentsWidget::on_downButton_clicked()
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
                  QTreeWidgetItem* item1 = partiturList->takeTopLevelItem(idx);
                  // Qt looses the QComboBox set into StaffListItem's when they are re-inserted into the tree:
                  // get the currently selected staff type of each combo and re-insert
                  int numOfStaffListItems = item1->childCount();
#if (!defined (_MSCVER) && !defined (_MSC_VER))
                  int staffIdx[numOfStaffListItems];
#else
                  // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
                  //    heap allocation is slow, an optimization might be used.
                  std::vector<int> staffIdx(numOfStaffListItems);
#endif
                  int itemIdx;
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item1->child(itemIdx)))->staffTypeIdx();
                  // do not consider hidden ones
                  int plusIdx = 1;
                  QTreeWidgetItem* nextParent = partiturList->topLevelItem(idx + plusIdx);
                  while (nextParent && nextParent->isHidden()) {
                       plusIdx++;
                       nextParent = partiturList->topLevelItem(idx + plusIdx);
                  }
                  partiturList->insertTopLevelItem(idx + plusIdx, item1);
                  // after-re-insertion, recreate each combo and set its index
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item1->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
                  partiturList->setItemExpanded(item1, isExpanded);
                  partiturList->setItemSelected(item1, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            int n = parent->childCount();
            // if staff item is not last of its part, move one slot down in part
            if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  StaffListItem* item1 = static_cast<StaffListItem*>(parent->takeChild(idx));
                  // Qt looses the QComboBox set into StaffListItem when it is re-inserted into the tree:
                  // get currently selected staff type and re-insert
                  int staffTypeIdx = item1->staffTypeIdx();
                  parent->insertChild(idx+1, item1);
                  // after item has been inserted into the tree, create a new QComboBox and set its index
                  item1->initStaffTypeCombo(true);
                  item1->setStaffType(staffTypeIdx);
                  partiturList->setItemSelected(item1, true);
                  }
            else {
                  // if staff item is last of its part...
                  int parentIdx = partiturList->indexOfTopLevelItem(parent);
                  int n1 = partiturList->topLevelItemCount();
                  //..and there is a next part, move to next part
                  if (parentIdx < (n1-1)) {
                        partiturList->selectionModel()->clear();
                        StaffListItem* sli = static_cast<StaffListItem*>(parent->takeChild(idx));
                        QTreeWidgetItem* nextParent = partiturList->topLevelItem(parentIdx - 1);
                        int staffTypeIdx = sli->staffTypeIdx();
                        nextParent->addChild(sli);
                        sli->initStaffTypeCombo(true);
                        sli->setStaffType(staffTypeIdx);
                        partiturList->setItemSelected(sli, true);
//                        PartListItem* pli = static_cast<PartListItem*>(nextParent);
//                        cs->undo(new MoveStaff(sli->staff(), pli->part, 0));
                        //
                        // TODO : if staff was linked to a staff of the old parent part, unlink it!
                        //
                        }
                  }
            }
      }

//---------------------------------------------------------
//   on_belowButton_clicked
//    (actually "Add Staff" button)
//---------------------------------------------------------

StaffListItem* InstrumentsWidget::on_belowButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return 0;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return 0;

      StaffListItem* sli  = static_cast<StaffListItem*>(item);
//      Staff* staff        = sli->staff();
      PartListItem* pli   = static_cast<PartListItem*>(sli->QTreeWidgetItem::parent());
      StaffListItem* nsli = new StaffListItem();
//      nsli->setStaff(staff);
      nsli->setStaff(0);
//      if (staff)
            nsli->setOp(ListItemOp::ADD);
      int ridx = pli->indexOfChild(sli) + 1;
      pli->insertChild(ridx, nsli);
      nsli->initStaffTypeCombo();               // StaffListItem needs to be inserted in the tree hierarchy
      nsli->setStaffType(sli->staffType());     // before a widget can be set into it

      ClefTypeList clefType;
      if (pli->it)
            clefType = pli->it->clefType(ridx);
      else
            clefType = pli->part->instrument()->clefType(ridx);
      nsli->setDefaultClefType(clefType);
      pli->updateClefs();

      partiturList->clearSelection();           // should not be necessary
      partiturList->setItemSelected(nsli, true);
      pli->updateClefs();
      return nsli;
      }

//---------------------------------------------------------
//   on_linkedButton_clicked
//---------------------------------------------------------

void InstrumentsWidget::on_linkedButton_clicked()
      {
      StaffListItem* nsli = on_belowButton_clicked();
      if (nsli)
            nsli->setLinked(true);
      }

//---------------------------------------------------------
//   on_search_textChanged
//---------------------------------------------------------

void InstrumentsWidget::on_search_textChanged(const QString &searchPhrase)
      {
      instrumentGenreFilter->blockSignals(true);
      instrumentGenreFilter->setCurrentIndex(0);
      instrumentGenreFilter->blockSignals(false);
      filterInstruments(instrumentList, searchPhrase);
      }


//---------------------------------------------------------
//   on_instrumentGenreFilter_currentTextChanged
//---------------------------------------------------------

void InstrumentsWidget::on_instrumentGenreFilter_currentIndexChanged(int index)
      {
      QString id = instrumentGenreFilter->itemData(index).toString();
      // Redisplay tree, only showing items from the selected genre
      filterInstrumentsByGenre(instrumentList, id);
      }

//---------------------------------------------------------
//   filterInstrumentsByGenre
//---------------------------------------------------------

void InstrumentsWidget::filterInstrumentsByGenre(QTreeWidget *instrList, QString genre)
      {
      QTreeWidgetItemIterator iList(instrList);
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

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void InstrumentsWidget::createInstruments(Score* cs)
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
            if (pli->op != ListItemOp::ADD) {
                  qDebug("bad op");
                  continue;
                  }
            const InstrumentTemplate* t = ((PartListItem*)item)->it;
            part = new Part(cs);
            part->initFromInstrTemplate(t);

            pli->part = part;
            QTreeWidgetItem* ci = 0;
            int rstaff = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  if (ci->isHidden())
                        continue;
                  StaffListItem* sli = (StaffListItem*)ci;
                  Staff* staff       = new Staff(cs);
                  staff->setPart(part);
                  sli->setStaff(staff);
                  ++rstaff;

                  staff->init(t, sli->staffType(), cidx);
                  staff->setDefaultClefType(sli->defaultClefType());

                  if (sli->linked() && !part->staves()->isEmpty()) {
                        Staff* linkedStaff = part->staves()->back();
                        staff->linkTo(linkedStaff);
                        }
                  part->staves()->push_back(staff);
                  cs->staves().insert(staffIdx + rstaff, staff);
                  }
            // if a staff was removed from instrument:
            if (part->staff(0)->barLineSpan() > rstaff) {
//TODO                  part->staff(0)->setBarLineSpan(rstaff);
                  part->staff(0)->setBracketType(0, BracketType::NO_BRACKET);
                  }

            // insert part
            cs->insertPart(part, staffIdx);
            int sidx = cs->staffIdx(part);
            int eidx = sidx + part->nstaves();
            for (Measure* m = cs->firstMeasure(); m; m = m->nextMeasure())
                  m->cmdAddStaves(sidx, eidx, true);
            staffIdx += rstaff;
            }
#if 0 // TODO
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
#endif
      cs->setLayoutAll();
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentsWidget::init()
      {
      partiturList->clear();
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
//   getPartiturList
//---------------------------------------------------------

QTreeWidget* InstrumentsWidget::getPartiturList()
      {
      return partiturList;
      }
}
