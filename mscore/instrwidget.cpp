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

#include "icons.h"
#include "instrwidget.h"

#include "libmscore/clef.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/scoreOrder.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/style.h"
#include "libmscore/system.h"
#include "libmscore/stringdata.h"
#include "libmscore/undo.h"

namespace Ms {

int StaffListItem::customStandardIdx;
int StaffListItem::customPercussionIdx;
int StaffListItem::customTablatureIdx;

//---------------------------------------------------------
//   ScoreOrderListModel
//---------------------------------------------------------
ScoreOrderListModel::ScoreOrderListModel(ScoreOrderList* data, QObject* parent)
   : QAbstractListModel(parent)
      {
      _scoreOrders = data;
      }

//---------------------------------------------------------
//   headerData
//---------------------------------------------------------

QVariant ScoreOrderListModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
      {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return tr("Ordering");
      return QVariant();
      }

//---------------------------------------------------------
//   rowCount
//---------------------------------------------------------

int ScoreOrderListModel::rowCount(const QModelIndex& /*parent*/) const
      {
      return _scoreOrders->size();
      }

//---------------------------------------------------------
//   data
//---------------------------------------------------------

QVariant ScoreOrderListModel::data(const QModelIndex& index, int role) const
      {
      if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
      return (*_scoreOrders)[index.row()]->getFullName();
      }

//---------------------------------------------------------
//   rebuildData
//---------------------------------------------------------

void ScoreOrderListModel::rebuildData()
      {
      beginResetModel();
      endResetModel();
      }

//---------------------------------------------------------
//   ScoreOrderFilterProxyModel
//---------------------------------------------------------
ScoreOrderFilterProxyModel::ScoreOrderFilterProxyModel(ScoreOrderList* data, QObject* parent)
   : QSortFilterProxyModel(parent)
      {
      _scoreOrders = data;
      }

//---------------------------------------------------------
//   setCustomizedOrder
//---------------------------------------------------------

void ScoreOrderFilterProxyModel::setCustomizedOrder(ScoreOrder* order)
      {
      _customizedOrder = (order && order->isCustomized()) ? order : nullptr;
      invalidateFilter();
      }

//---------------------------------------------------------
//   filterAcceptsRow
//---------------------------------------------------------

bool ScoreOrderFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& /*sourceParent*/) const
      {
      ScoreOrder* so = (*_scoreOrders)[sourceRow];
      return !so->isCustomized() || (so == _customizedOrder);
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
      _staffTypeCombo->addItem(tr("Custom Standard"), CUSTOM_STAFF_TYPE_IDX);
      customPercussionIdx = _staffTypeCombo->count();
      _staffTypeCombo->addItem(tr("Custom Percussion"), CUSTOM_STAFF_TYPE_IDX);
      customTablatureIdx = _staffTypeCombo->count();
      _staffTypeCombo->addItem(tr("Custom Tablature"), CUSTOM_STAFF_TYPE_IDX);

      treeWidget()->setItemWidget(this, 4, _staffTypeCombo);
      connect(_staffTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(staffTypeChanged(int)) );
      }

//---------------------------------------------------------
//   setPartIdx
//---------------------------------------------------------

void StaffListItem::setPartIdx(int val)
      {
      _partIdx = val;
      setText(0, InstrumentsWidget::tr("Staff: %1").arg(_partIdx + 1));
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
      int typeIdx = staffTypeIdx();
      Q_ASSERT(typeIdx != CUSTOM_STAFF_TYPE_IDX);
      if (typeIdx == CUSTOM_STAFF_TYPE_IDX)
            typeIdx = 0;
      return StaffType::preset(StaffTypes(typeIdx));
      }

//---------------------------------------------------------
//   staffTypeIdx
//---------------------------------------------------------

int StaffListItem::staffTypeIdx(int idx) const
      {
      return _staffTypeCombo->itemData(idx).toInt();
      }

//---------------------------------------------------------
//   staffTypeIdx
//---------------------------------------------------------

int StaffListItem::staffTypeIdx() const
      {
      return staffTypeIdx(_staffTypeCombo->currentIndex());
      }

//---------------------------------------------------------
//   staffTypeChanged
//---------------------------------------------------------

void StaffListItem::staffTypeChanged(int idx)
      {
      // check current clef matches new staff type
      const int typeIdx = staffTypeIdx(idx);
      if (typeIdx == CUSTOM_STAFF_TYPE_IDX) // consider it not changed
            return;

      const StaffType* stfType = StaffType::preset(StaffTypes(typeIdx));

      PartListItem* pli = static_cast<PartListItem*>(QTreeWidgetItem::parent());
      pli->updateClefs();

      if (_staff && _staff->staffType(Fraction(0,1))->name() != stfType->name()) {
            if (_op != ListItemOp::I_DELETE && _op != ListItemOp::ADD)
                  _op = ListItemOp::UPDATE;
            }
      }

//---------------------------------------------------------
//   id
//---------------------------------------------------------

QString PartListItem::id() const
      {
      return it ? it->id : part->instrument()->getId();
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PartListItem::name() const
      {
      return it ? it->trackName : part->instrument()->trackName();
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
//   setSoloist
//---------------------------------------------------------

void PartListItem::setSoloist(bool val)
      {
      soloist = val;
      if (soloist)
            setText(0, QString(QObject::tr("Soloist: %1")).arg(_name));
      else
            setText(0, _name);
      }

//---------------------------------------------------------
//   isSoloist
//---------------------------------------------------------

bool PartListItem::isSoloist() const
      {
      return soloist;
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
      _name = QString(p->partName().isEmpty() ? p->instrument()->trackName() : p->partName());
      setSoloist(false); //TODO, must be taken from part.
      setFlags(flags() | Qt::ItemIsUserCheckable);
      }

PartListItem::PartListItem(const InstrumentTemplate* i)
   : QTreeWidgetItem(PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ListItemOp::ADD;
      _name = QString(it->trackName);
      setSoloist(false);
      }
PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ListItemOp::ADD;
      _name = QString(it->trackName);
      setSoloist(false);
      }
PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv, QTreeWidgetItem* prv)
   : QTreeWidgetItem(lv, prv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ListItemOp::ADD;
      _name = QString(it->trackName);
      setSoloist(false);
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
      _filter = new ScoreOrderFilterProxyModel(&scoreOrders, this);
      _model = new ScoreOrderListModel(&scoreOrders, this);
      _filter->setSourceModel(_model);
      scoreOrderComboBox->setModel(_filter);
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
      addStaffButton->setEnabled(false);
      addLinkedStaffButton->setEnabled(false);

      upButton->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      downButton->setIcon(*icons[int(Icons::arrowDown_ICON)]);

      instrumentSearch->setFilterableView(instrumentList);
      }

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void InstrumentsWidget::changeEvent(QEvent* event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

//---------------------------------------------------------
//   retranslate
//---------------------------------------------------------

void InstrumentsWidget::retranslate()
      {
      retranslateUi(this);
      }

//---------------------------------------------------------
//   populateGenreCombo
//---------------------------------------------------------

void populateGenreCombo(QComboBox* combo)
      {
      combo->blockSignals(true);
      combo->clear();
      combo->addItem(qApp->translate("InstrumentsDialog", "All instruments"), "all");
      int i = 1;
      int defaultIndex = 0;
      for (InstrumentGenre*& ig : instrumentGenres) {
            combo->addItem(ig->name, ig->id);
            if (ig->id == "common")
                  defaultIndex = i;
            ++i;
            }
      combo->blockSignals(false);
      combo->setCurrentIndex(defaultIndex);
      }

//---------------------------------------------------------
//   populateInstrumentList
//---------------------------------------------------------

void populateInstrumentList(QTreeWidget* instrumentList)
      {
      instrumentList->clear();
      // TODO: memory leak?
      for (InstrumentGroup*& g : instrumentGroups) {
            InstrumentTemplateListItem* group = new InstrumentTemplateListItem(g->name, instrumentList);
            // provide feedback to blind users that they have selected a group rather than an instrument
            group->setData(0, Qt::AccessibleTextRole, QVariant(QObject::tr("%1 category").arg(g->name))); // spoken by screen readers
            group->setFlags(Qt::ItemIsEnabled);
            for (InstrumentTemplate*& t : g->instrumentTemplates) {
                  InstrumentTemplateListItem* instrument = new InstrumentTemplateListItem(t, group);
                  instrument->setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren));
                  }
            }
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void InstrumentsWidget::buildTemplateList()
      {
      // clear search if instrument list is updated
      instrumentSearch->clear();

      populateInstrumentList(instrumentList);
      populateGenreCombo(instrumentGenreFilter);
      }

//---------------------------------------------------------
//   genPartList
//---------------------------------------------------------

void InstrumentsWidget::genPartList(Score* cs)
      {
      partiturList->clear();

      for (Part*& p : cs->parts()) {
            PartListItem* pli = new PartListItem(p, partiturList);
            pli->setVisible(p->show());
            pli->setSoloist(p->soloist());
            for (Staff*& s : *p->staves()) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->setStaff(s);
                  sli->setClefType(s->clefType(Fraction(0,1)));
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
                  sli->setStaffType(s->staffType(Fraction(0,1)));    // TODO
                  }
            pli->updateClefs();
            pli->setExpanded(true);
            }
      _filter->setCustomizedOrder(cs->scoreOrder());
      _model->rebuildData();
      partiturList->resizeColumnToContents(0);
      partiturList->resizeColumnToContents(2);  // adjust width of "Clef " and "Staff type" columns
      partiturList->resizeColumnToContents(4);
      }

//---------------------------------------------------------
//   updatePartIdx
//---------------------------------------------------------

void InstrumentsWidget::updatePartIdx()
      {
      for (int i = 0; i < partiturList->topLevelItemCount(); ++i) {
            PartListItem* tli = static_cast<PartListItem*>(partiturList->topLevelItem(i));
            int partIdx = -1;
            for (int j = 0; j < tli->childCount(); ++j) {
                  StaffListItem* sli = static_cast<StaffListItem*>(tli->child(j));
                  if (!sli->isHidden()) {
                        partIdx++;
                        sli->setPartIdx(partIdx);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   findPrvItem
//---------------------------------------------------------

int InstrumentsWidget::findPrvItem(PartListItem* pli, bool insert, int number)
      {
      ScoreOrder* order = getScoreOrder();
      const int orderNumber = order->instrumentIndex(pli->id(), pli->isSoloist());
      const int last = (number >= 0) ? number : partiturList->topLevelItemCount();

      QTreeWidgetItem* item = nullptr;
      for (int idx = 0; number-- && (item = partiturList->topLevelItem(idx)); ++idx) {
            PartListItem* p = static_cast<PartListItem*>(item);
            if (p->op == ListItemOp::I_DELETE)
                  continue;

            if (order->instrumentIndex(p->id(), p->isSoloist()) == orderNumber)
                  {
                  if (insert) {
                        const int selIdx = partiturList->currentIndex().row();
                        if ((selIdx >= 0) && (idx > selIdx))
                              return idx;
                        }
                  }
            else if (order->instrumentIndex(p->id(), p->isSoloist()) > orderNumber) {
                  return idx;
                  }
            }
      return last;
      }

//---------------------------------------------------------
//   movePartItem
//---------------------------------------------------------

QTreeWidgetItem* InstrumentsWidget::movePartItem(int oldPos, int newPos)
      {
      if (oldPos == newPos)
            return partiturList->topLevelItem(oldPos);

      QTreeWidgetItem* item = partiturList->takeTopLevelItem(oldPos);
      // Qt looses the QComboBox set into StaffListItem's when they are re-inserted into the tree:
      // get the currently selected staff type of each combo and re-insert
      int numOfStaffListItems = item->childCount();
      std::vector<int> staffIdx(numOfStaffListItems);
      for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
            staffIdx[itemIdx] = (static_cast<StaffListItem*>(item->child(itemIdx)))->staffTypeIdx();
      // do not consider hidden ones
      const int diff = oldPos - newPos;
      QTreeWidgetItem* prevParent = partiturList->topLevelItem(newPos);
      while (prevParent && prevParent->isHidden()) {
            newPos += diff;
            prevParent = partiturList->topLevelItem(newPos);
            }
      partiturList->insertTopLevelItem(newPos, item);
      // after-re-insertion, recreate each combo and set its index
      for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
            StaffListItem* staffItem = static_cast<StaffListItem*>(item->child(itemIdx));
            staffItem->initStaffTypeCombo(true);
            staffItem->setStaffType(staffIdx[itemIdx]);
            }
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
      item->setExpanded(true);
#else
      partiturList->setItemExpanded(item, true);
#endif
      return item;
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
            addLinkedStaffButton->setEnabled(false);
            addStaffButton->setEnabled(false);
            makeSoloistButton->setEnabled(false);
            return;
            }
      QTreeWidgetItem* item = wi.front();
      Q_ASSERT(partiturList->currentItem() == item);
      bool flag = item != nullptr;

      int count = 0; // item can be hidden
      QTreeWidgetItem* it = 0;
      QList<QTreeWidgetItem*> witems;
      if(item && item->type() == PART_LIST_ITEM) {
            for (int idx = 0; (it = partiturList->topLevelItem(idx)); ++idx) {
                  if (!it->isHidden()) {
                        count++;
                        witems.append(it);
                        }
                  }
            }
      else {
            for (int idx = 0; item && (it = item->parent()->child(idx)); ++idx) {
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
      addLinkedStaffButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      addStaffButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      makeSoloistButton->setEnabled(item && item->type() == PART_LIST_ITEM);
      setMakeSoloistButtonText();
      updateScoreOrder();
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void InstrumentsWidget::on_instrumentList_itemActivated(QTreeWidgetItem* item, int)
      {
      if (item->flags() & Qt::ItemIsSelectable)
            on_addButton_clicked();
      }

//---------------------------------------------------------
//   on_addButton_clicked
//    add instrument to partitur
//---------------------------------------------------------

void InstrumentsWidget::on_addButton_clicked()
      {
      for (QTreeWidgetItem*& i : instrumentList->selectedItems()) {
            InstrumentTemplateListItem* item = static_cast<InstrumentTemplateListItem*>(i);
            const InstrumentTemplate* it     = item->instrumentTemplate();
            if (it == 0)
                  continue;

            PartListItem* pli = new PartListItem(it);
            partiturList->insertTopLevelItem(findPrvItem(pli, true), pli);
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
            partiturList->resizeColumnToContents(0);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
            pli->setExpanded(true);
#else
            partiturList->setItemExpanded(pli, true);
#endif
            partiturList->clearSelection();     // should not be necessary
            partiturList->setCurrentItem(pli);
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
      if (wi.isEmpty()) {
            Q_ASSERT(false); // shouldn't get here (remove button disabled when no items selected)
            removeButton->setEnabled(false); // nevertheless, handle gracefully in release builds
            return;
            }
      QTreeWidgetItem* item   = wi.front();
      QTreeWidgetItem* parent = item->parent();

      if (parent) {
            if (parent->childCount() == 1) {
                  Q_ASSERT(false); // shouldn't get here (remove button disabled when one item left)
                  removeButton->setEnabled(false); // nevertheless, handle gracefully in release builds
                  return;
                  }
            if (((StaffListItem*)item)->op() == ListItemOp::ADD) {
                  if (parent->childCount() == 1) {
                        partiturList->takeTopLevelItem(partiturList->indexOfTopLevelItem(parent));
                        delete parent;
                        parent = nullptr;
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
            if (parent)
                  static_cast<PartListItem*>(parent)->updateClefs();
            partiturList->setCurrentItem(parent);
            updatePartIdx();
            }
      else {
            if (partiturList->topLevelItemCount() == 1) {
                  Q_ASSERT(false); // shouldn't get here as (remove button disabled when one item left)
                  removeButton->setEnabled(false); // nevertheless, handle gracefully in release builds
                  emit completeChanged(false);
                  return;
                  }
            int idx = partiturList->indexOfTopLevelItem(item);
            if (((PartListItem*)item)->op == ListItemOp::ADD) {
                  partiturList->blockSignals(true);
                  delete item; // fires selectionChanged too early (item is still in view)
                  partiturList->blockSignals(false);
                  /*emit*/ on_partiturList_itemSelectionChanged(); // fire manually (item gone by now)
                  }
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
            if(!nextParent) { // couldn't find one after, check before
                  plusIdx = 1;
                  nextParent = partiturList->topLevelItem(idx - plusIdx);
                  while (nextParent && nextParent->isHidden()) {
                        plusIdx++;
                        nextParent = partiturList->topLevelItem(idx - plusIdx);
                        }
                  }
            partiturList->setCurrentItem(nextParent);
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
            bool isExpanded = item->isExpanded();
            int idx = partiturList->indexOfTopLevelItem(item);
            // if part item not first, move one slot up
            if (idx) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item1 = movePartItem(idx, idx - 1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
                  item1->setExpanded(isExpanded);
#else
                  partiturList->setItemExpanded(item1, isExpanded);
#endif
                  partiturList->setCurrentItem(item1);
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
                  partiturList->setCurrentItem(item1);
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
                        partiturList->setCurrentItem(sli);
//                        PartListItem* pli = static_cast<PartListItem*>(prevParent);
//                        int idx = pli->part->nstaves();
//??                        cs->undo(new MoveStaff(sli->staff(), pli->part, idx));
                        //
                        // TODO : if staff was linked to a staff of the old parent part, unlink it!
                        //
                        }
                  }
            }
      updatePartIdx();
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
            bool isExpanded = item->isExpanded();
            int idx = partiturList->indexOfTopLevelItem(item);
            int n = partiturList->topLevelItemCount();
            // if part not last, move one slot down
            if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item1 = movePartItem(idx, idx + 1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
                  item1->setExpanded(isExpanded);
#else
                  partiturList->setItemExpanded(item1, isExpanded);
#endif
                  partiturList->setCurrentItem(item1);
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
                  partiturList->setCurrentItem(item1);
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
                        partiturList->setCurrentItem(sli);
//                        PartListItem* pli = static_cast<PartListItem*>(nextParent);
//                        cs->undo(new MoveStaff(sli->staff(), pli->part, 0));
                        //
                        // TODO : if staff was linked to a staff of the old parent part, unlink it!
                        //
                        }
                  }
            }
      updatePartIdx();
      }

//---------------------------------------------------------
//   on_addStaffButton_clicked
//---------------------------------------------------------

StaffListItem* InstrumentsWidget::on_addStaffButton_clicked()
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
      partiturList->setCurrentItem(nsli);
      pli->updateClefs();
      updatePartIdx();
      return nsli;
      }

//---------------------------------------------------------
//   on_addLinkedStaffButton_clicked
//---------------------------------------------------------

void InstrumentsWidget::on_addLinkedStaffButton_clicked()
      {
      StaffListItem* nsli = on_addStaffButton_clicked();
      if (nsli)
            nsli->setLinked(true);
      }

//---------------------------------------------------------
//   on_makeSoloistButton_clicked
//---------------------------------------------------------

void InstrumentsWidget::on_makeSoloistButton_clicked()
      {
      PartListItem* pli = static_cast<PartListItem*>(partiturList->currentItem());
      if (!pli || (pli->type() != PART_LIST_ITEM))
            return;

      pli->setSoloist(!pli->isSoloist());
      setMakeSoloistButtonText();
      sortInstruments();
      updateScoreOrder();
      }

//---------------------------------------------------------
//   on_scoreOrderComboBox_activated
//---------------------------------------------------------

void InstrumentsWidget::on_scoreOrderComboBox_activated(int index)
      {
      scoreOrderComboBox->setCurrentIndex(index);
      sortInstruments();
      }

//---------------------------------------------------------
//   on_instrumentSearch_textChanged
//---------------------------------------------------------

void InstrumentsWidget::on_instrumentSearch_textChanged(const QString&)
      {
      // searching is done in Ms::SearchBox so here we just reset the
      // genre dropdown to ensure that the search includes all genres
      const int idxAllGenres = 0;
      if (instrumentGenreFilter->currentIndex() != idxAllGenres) {
            instrumentGenreFilter->blockSignals(true);
            instrumentGenreFilter->setCurrentIndex(idxAllGenres);
            instrumentGenreFilter->blockSignals(false);
            }
      }

//---------------------------------------------------------
//   on_instrumentGenreFilter_currentTextChanged
//---------------------------------------------------------

void InstrumentsWidget::on_instrumentGenreFilter_currentIndexChanged(int index)
      {
      QSettings settings;
      settings.beginGroup("selectInstrument");  // hard coded, since this is also used in selinstrument
      settings.setValue("selectedGenre", instrumentGenreFilter->currentText());
      settings.endGroup();

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
//   updateScoreOrder
//---------------------------------------------------------

void InstrumentsWidget::updateScoreOrder()
      {
      ScoreOrder* order = getScoreOrder();
      if (order->isCustomized())
            {
            ScoreOrder* normal = scoreOrders.findByName(order->getName());
            if (isScoreOrder(normal))
                  setScoreOrder(normal);
            }
      else
            {
            if (!isScoreOrder(order))
                  {
                  ScoreOrder* custom = scoreOrders.findByName(order->getName(), true);

                  if (!custom) {
                        custom = order->clone();
                        scoreOrders.addScoreOrder(custom);
                        }
                  _filter->setCustomizedOrder(custom);
                  setScoreOrder(custom);
                  }
            }
      }

//---------------------------------------------------------
//   sortInstruments
//---------------------------------------------------------

void InstrumentsWidget::sortInstruments()
      {
      if (getScoreOrder()->isCustom() || getScoreOrder()->isCustomized())
            return;

      PartListItem* pli = nullptr;
      for (int idx = 1; (pli = static_cast<PartListItem*>(partiturList->topLevelItem(idx))); ++idx)
            movePartItem(idx, findPrvItem(pli, false, idx));
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
            part->setSoloist(pli->isSoloist());

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
      numberInstrumentNames(cs);
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
      setBracketsAndBarlines(cs);
      cs->setLayoutAll();
      }

//---------------------------------------------------------
//   numberInstrumentNames
//---------------------------------------------------------

void InstrumentsWidget::numberInstrumentNames(Score* cs)
      {
      class PartNamer {
            Part* part;
            bool  soloist;
            int   number;

            void setNamesN(Part *p) {
                  ++number;
                  QString tmpl = soloist ? QString(qApp->translate("InstrumentsDialog", "%1 solo %2")) : QString("%1 %2");
                  p->setLongName(tmpl.arg(p->longName()).arg(number));
                  p->setPartName(tmpl.arg(p->partName()).arg(number));
                  p->setShortName(tmpl.arg(p->shortName()).arg(number));
                  }
         public:
            PartNamer(Part* p=nullptr) : part(p), soloist(p && p->soloist()), number(0) {}
            ~PartNamer() {
                  if (part && !number) {
                        QString tmpl = soloist ? QString(qApp->translate("InstrumentsDialog", "%1 solo")) : QString("%1");
                        part->setLongName(tmpl.arg(part->longName()));
                        part->setPartName(tmpl.arg(part->partName()));
                        part->setShortName(tmpl.arg(part->shortName()));
                        }
                  }

            void update(Part* p) {
                  if (!number)
                        setNamesN(part);
                  setNamesN(p);
                  }

      };

      QMap<QString, PartNamer*> namers;
      for (Part*& p : cs->parts()) {
            const QString key = QString("%1/%2").arg(p->partName()).arg(p->soloist());
            if (namers.contains(key))
                  namers[key]->update(p);
            else
                  namers.insert(key, new PartNamer(p));
            }

      const QList<PartNamer*> namerVals;
      for (auto& namer : namerVals)
            delete namer;
      }

//---------------------------------------------------------
//   setBracketsAndBarlines
//---------------------------------------------------------

void InstrumentsWidget::setBracketsAndBarlines(Score* cs)
      {
      getScoreOrder()->setBracketsAndBarlines(cs);
      }

//---------------------------------------------------------
//   isScoreOrder
//---------------------------------------------------------

bool InstrumentsWidget::isScoreOrder(const ScoreOrder* order) const
      {
      QList<int> indices;
      QTreeWidgetItem* item = nullptr;
      for (int idx = 0; (item = partiturList->topLevelItem(idx)); ++idx) {
            PartListItem* pli = (PartListItem*)item;
            if (pli->op == ListItemOp::I_DELETE)
                  continue;
            indices << order->instrumentIndex(pli->id(), pli->isSoloist());
            }
      return order->isScoreOrder(indices);
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
      addLinkedStaffButton->setEnabled(false);
      addStaffButton->setEnabled(false);

      int curIndex = scoreOrderComboBox->currentIndex();
      _filter->setCustomizedOrder(nullptr);
      _model->rebuildData();
      scoreOrderComboBox->setCurrentIndex(curIndex);

      // get last saved, user-selected instrument genre and set filter to it
      QSettings settings;
      settings.beginGroup("selectInstrument");
      if (!settings.value("selectedGenre").isNull()){
            QString selectedGenre = settings.value("selectedGenre").value<QString>();
            instrumentGenreFilter->setCurrentText(selectedGenre);
            }
      settings.endGroup();

      emit completeChanged(false);
      }

//---------------------------------------------------------
//   setMakeSoloistButtonText
//---------------------------------------------------------

void InstrumentsWidget::setMakeSoloistButtonText()
      {
      PartListItem* pli = static_cast<PartListItem*>(partiturList->currentItem());
      if (!pli || (pli->type() != PART_LIST_ITEM))
            return;

      if (pli->isSoloist())
            makeSoloistButton->setText(InstrumentsWidget::tr("Undo soloist"));
      else
            makeSoloistButton->setText(InstrumentsWidget::tr("Make soloist"));
      partiturList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   setScoreOrder
//---------------------------------------------------------

void InstrumentsWidget::setScoreOrder(ScoreOrder* order)
      {
      const QModelIndex idx1 = _model->index(scoreOrders.getScoreOrderIndex(order), 0);
      const QModelIndex idx2 = _filter->mapFromSource(idx1);
      scoreOrderComboBox->setCurrentIndex(idx2.row());
      sortInstruments();
      }

//---------------------------------------------------------
//   getScoreOrder
//---------------------------------------------------------

ScoreOrder* InstrumentsWidget::getScoreOrder() const
      {
      const QModelIndex idx1 = _filter->index(scoreOrderComboBox->currentIndex(), 0);
      const QModelIndex idx2 = _filter->mapToSource(idx1);
      return scoreOrders[idx2.row()];
      }

//---------------------------------------------------------
//   getPartiturList
//---------------------------------------------------------

QTreeWidget* InstrumentsWidget::getPartiturList()
      {
      return partiturList;
      }
}
