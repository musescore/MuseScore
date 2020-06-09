//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include "excerptsdialog.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/part.h"
#include "libmscore/undo.h"
#include "icons.h"

namespace Ms {

//---------------------------------------------------------
//   StaffItem
//---------------------------------------------------------

StaffItem::StaffItem(int firstTrack, int idx1, int idx2, const QMultiMap<int, int> tracks, bool disabled, PartItem* parent)
   : QTreeWidgetItem(parent)
      {
      _firstTrack = -1; // Indicates initialising.

      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable));

      for (int i = 0; i < VOICES; ++i) {
            bool inRange { true };
            if (idx1 >= 0) {
                  int checkTrack = tracks.value(firstTrack + i, -1);
                  inRange = ((idx1 * VOICES) <= checkTrack) && ((checkTrack < (idx2 + 1) * VOICES));
                  }
            setCheckState(i + 1, inRange ? Qt::Checked : Qt::Unchecked);
            setDisabled(disabled);
            }

       // By setting _firstTrack the check in setData is no longer bypassed.
       _firstTrack = firstTrack;
      }

//---------------------------------------------------------
//   setLinkMaster
//---------------------------------------------------------

void StaffItem::setLink(StaffItem* si)
      {
      _linked.append(si);
      si->setDisabled(true);
      }

//---------------------------------------------------------
//   setData
//---------------------------------------------------------

void StaffItem::setData(int column, int role, const QVariant& value)
      {
      QTreeWidgetItem::setData(column, role, value);

      if (_firstTrack < 0)
            // Is initially set to -1 in the constructor to bypass
            // the checked for all unchecked items during initialisation.
            return;

      // Make sure there is always one voice selected.
      // Is bypassed during initialisation.

      int unchecked = 0;
      for (int i = 1; i <= VOICES; i++) {
            if (checkState(i) == Qt::Unchecked)
                  unchecked += 1;
            }
      if (unchecked == VOICES)
            setCheckState(column, Qt::Checked);

      for (auto si : _linked)
            si->setData(column, role, value);
      }

//---------------------------------------------------------
//   tracks
//---------------------------------------------------------

QMultiMap<int, int> StaffItem::tracks(int& staff) const
      {
      QMultiMap<int, int> tracks;
      int voice = 0;
      for (int i = 0; i < VOICES; ++i) {
            if (checkState(i + 1) == Qt::Checked)
                  tracks.insert(_firstTrack + i, staff * VOICES + voice++);
            }
      staff++;
      return tracks;
      }

//---------------------------------------------------------
//   PartItem
//---------------------------------------------------------

PartItem::PartItem(Part* part, const QMultiMap<int, int> tracks, int& staffIdx, bool disabled, QTreeWidget* parent)
   : QTreeWidgetItem(parent)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable));
      _part   = part;
      setText(0, _part->partName());

      Staff* master { nullptr };
      StaffItem* msli { nullptr };
      int firstTrack = _part->startTrack();
      for (int i = 0; i < _part->nstaves(); ++i) {
            StaffItem* sli = new StaffItem(firstTrack, staffIdx, staffIdx + _part->nstaves() - 1, tracks, disabled, this);
            _staffItems.append(sli);

            // If the part contains linked staves they all must have the same
            // selection. Make only the first editable.
            Staff* s = _part->staves()->at(i);
            if (!master) {
                  if  (s->staffList().length() > 1) {
                        master = s;
                        msli = sli;
                        }
                  }
            else if (master && !s->staffList().length()) {
                  master = nullptr;
                  msli = nullptr;
                  }
            else {
                  if (master->staffList().contains(s))
                        msli->setLink(sli);
                  }
            firstTrack += VOICES;
            }

      if (staffIdx >= 0)
            staffIdx += _part->nstaves();
      }

//---------------------------------------------------------
//   ~PartItem
//---------------------------------------------------------

PartItem::~PartItem()
      {
      while (!_staffItems.isEmpty())
            delete _staffItems.takeFirst();
      _staffItems.clear();
      }

//---------------------------------------------------------
//   findStaffItem
//---------------------------------------------------------

StaffItem* PartItem::findStaffItem(QTreeWidgetItem* widget) const
      {
      for (StaffItem* si : _staffItems) {
            if (si == widget)
                  return si;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   tracks
//---------------------------------------------------------

QMultiMap<int, int> PartItem::tracks(int& staff) const
      {
      QMultiMap<int, int> tracks;
      for (StaffItem* si : _staffItems)
            tracks += si->tracks(staff);
      return tracks;
      }

//---------------------------------------------------------
//   ExcerptItem
//---------------------------------------------------------

ExcerptItem::ExcerptItem(Excerpt* e, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      _excerpt = e;
      _tracks = _excerpt->tracks();
      setText(_excerpt->title());

      int staffIdx { 0 };
      for (Part* part : _excerpt->parts())
            _partItems.append(new PartItem(part, _excerpt->tracks(), staffIdx, _excerpt->partScore(), 0));
      }

//---------------------------------------------------------
//   ExcerptItem
//---------------------------------------------------------

ExcerptItem::~ExcerptItem()
      {
      while (!_partItems.isEmpty())
            delete _partItems.takeFirst();
      _partItems.clear();
      }

//---------------------------------------------------------
//   addPartItem
//---------------------------------------------------------

void ExcerptItem::addPartItem(PartItem* pi)
      {
      _excerpt->parts().append(pi->part());
      _partItems.append(pi);
      }

//---------------------------------------------------------
//   removePartItem
//---------------------------------------------------------

void ExcerptItem::removePartItem(PartItem* pi)
      {
      if (_partItems.contains(pi))
            _partItems.removeAll(pi);
      }

//---------------------------------------------------------
//   findPartItem
//---------------------------------------------------------

PartItem* ExcerptItem::findPartItem(QTreeWidgetItem* widget) const
      {
      for (PartItem* pi : _partItems) {
            if ((pi == widget) || pi->findStaffItem(widget))
                  return pi;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   tracks
//---------------------------------------------------------

QMultiMap<int, int> ExcerptItem::tracks() const
      {
      QMultiMap<int, int> tracks;
      int staff = 0;

      for (PartItem* pi : _partItems)
            tracks += pi->tracks(staff);
      return tracks;
      }

//---------------------------------------------------------
//   setTitle
//---------------------------------------------------------

void ExcerptItem::setTitle(const QString name)
      {
      setText(name);
      if (!isPartScore())
            _excerpt->setTitle(name);
      }

//---------------------------------------------------------
//   InstrumentItem
//---------------------------------------------------------

InstrumentItem::InstrumentItem(Part* part, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      _part = part;
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
      setText(_part->partName().replace("/", "_"));
      }

//---------------------------------------------------------
//   newPartItem
//---------------------------------------------------------

PartItem* InstrumentItem::newPartItem(int count) const
      {
      QMultiMap<int, int> tracks;
      for (int track = _part->startTrack(); track < _part->endTrack(); ++track)
            tracks.insert(track, count * VOICES + track - _part->startTrack());

      int staffIdx { -1 };
      return new PartItem(_part, tracks, staffIdx, false, 0);
      }

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

ExcerptsDialog::ExcerptsDialog(MasterScore* s, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PartEditor");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      score = s->masterScore();

      for (Part* p : score->parts())
            instrumentList->addItem(new InstrumentItem(p));

      for (Excerpt* e : score->excerpts())
            excerptList->addItem(new ExcerptItem(e));


      connect(singlePartButton, SIGNAL(clicked()), SLOT(singlePartClicked()));
      connect(allPartsButton, SIGNAL(clicked()), SLOT(allPartsClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(moveUpButton, SIGNAL(clicked()), SLOT(moveUpClicked()));
      connect(moveDownButton, SIGNAL(clicked()), SLOT(moveDownClicked()));
      connect(addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
      connect(removeButton, SIGNAL(clicked()), SLOT(removeButtonClicked()));

      connect(excerptList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(excerptChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(partList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
         SLOT(partDoubleClicked(QTreeWidgetItem*, int)));
      connect(partList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
         SLOT(partChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
      connect(instrumentList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
              SLOT(addButtonClicked()));
      connect(instrumentList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(instrumentChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(title, SIGNAL(textChanged(const QString&)), SLOT(titleChanged(const QString&)));

      moveUpButton->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      moveDownButton->setIcon(*icons[int(Icons::arrowDown_ICON)]);

      for (int i = 1; i <= VOICES; i++)
            partList->header()->resizeSection(i, 30);

      if (excerptList->count())
            excerptList->setCurrentRow(0);
      }

//---------------------------------------------------------
//   ~ExcerptsDialog
//---------------------------------------------------------

ExcerptsDialog::~ExcerptsDialog()
      {
      // A delete of a QTreeWidget includes a delete of all item. But PartItem's in the
      // partList are owned by ExcerptItem. So deleting excerptList will delete ExcerptItem's
      // in the list **and** all its children (in ExcerptItem), leaving invalid pointer.
      // So, just remove the item from partList and leave the delete to the owner.
      clearPartList();
      }

//---------------------------------------------------------
//   startExcerptsDialog
//---------------------------------------------------------

void MuseScore::startExcerptsDialog()
      {
      if (cs == 0)
            return;
      ExcerptsDialog ed(cs->masterScore(), 0);
      MuseScore::restoreGeometry(&ed);
      ed.exec();
      MuseScore::saveGeometry(&ed);
      cs->setLayoutAll();
      cs->update();
      }

//---------------------------------------------------------
//   singlePartClicked
//---------------------------------------------------------

void ExcerptsDialog::singlePartClicked()
      {
      QString name = createName("Part");
      Excerpt* e   = new Excerpt(score);
      e->setTitle(name);
      ExcerptItem* ei = new ExcerptItem(e);
      excerptList->addItem(ei);
      excerptList->selectionModel()->clearSelection();
      excerptList->setCurrentItem(ei, QItemSelectionModel::SelectCurrent);

      for (int i = 0; i < excerptList->count(); ++i) {
            // if except score not created yet, change the UI title
            // if already created, change back(see createName) the excerpt title
            ExcerptItem* eii = getExcerptItemAt(i);
            eii->setText(eii->excerpt()->title());
            }
      }

//---------------------------------------------------------
//   allPartsClicked
//---------------------------------------------------------

void ExcerptsDialog::allPartsClicked()
      {
      ExcerptItem* ei = 0;
      for (Excerpt* e : Excerpt::createAllExcerpt(score)) {
            ei = new ExcerptItem(e);
            excerptList->addItem(ei);
            }
      if (ei) {
            excerptList->selectionModel()->clearSelection();
            excerptList->setCurrentItem(ei, QItemSelectionModel::SelectCurrent);
            }
      }

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void ExcerptsDialog::deleteClicked()
      {
      ExcerptItem* ei = getCurrentExcerptItem();
      if (ei == 0)
            return;

      delete ei;
      setWidgetState();
      }

//---------------------------------------------------------
//   moveUpClicked
//---------------------------------------------------------

void ExcerptsDialog::moveUpClicked()
      {
      int currentRow = excerptList->currentRow();
      if (currentRow <= 0)
            return;
      QListWidgetItem* currentItem = excerptList->takeItem(currentRow);
      excerptList->insertItem(currentRow - 1, currentItem);
      excerptList->setCurrentRow(currentRow - 1);
      }

//---------------------------------------------------------
//   moveDownClicked
//---------------------------------------------------------

void ExcerptsDialog::moveDownClicked()
      {
      int currentRow = excerptList->currentRow();
      if (currentRow >= excerptList->count() - 1)
            return;
      QListWidgetItem* currentItem = excerptList->takeItem(currentRow);
      excerptList->insertItem(currentRow + 1, currentItem);
      excerptList->setCurrentRow(currentRow + 1);
      }

//---------------------------------------------------------
//  addButtonClicked
//    add instrument to excerpt
//---------------------------------------------------------

void ExcerptsDialog::addButtonClicked()
      {
      InstrumentItem* ii = getCurrentInstrumentItem();
      ExcerptItem* ei = getCurrentExcerptItem();

      if (!ii || !ei || !partList->isEnabled())
            return;

      PartItem* pi = ii->newPartItem(partList->topLevelItemCount());
      ei->addPartItem(pi);
      partList->addTopLevelItem(pi);
      partList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   removeButtonClicked
//    remove instrument from score
//---------------------------------------------------------

void ExcerptsDialog::removeButtonClicked()
      {
      ExcerptItem* ei = getCurrentExcerptItem();
      PartItem* pi = getCurrentPartItem();
      if (!ei || !pi)
            return;

      ei->removePartItem(pi);
      partList->takeTopLevelItem(partList->indexOfTopLevelItem(pi));

      partList->resizeColumnToContents(0);
//       removeButton->setEnabled(partList->topLevelItemCount() > 0);
      }

//---------------------------------------------------------
//   excerptChanged
//---------------------------------------------------------

void ExcerptsDialog::excerptChanged(QListWidgetItem*, QListWidgetItem*)
      {
      clearPartList();
      ExcerptItem* ei = getCurrentExcerptItem();
      if (ei) {
            title->setText(ei->text());
            for (PartItem* pi : ei->partItems())
                  partList->addTopLevelItem(pi);
            }
      else {
            title->setText("");
            }

      setWidgetState();
      }

//---------------------------------------------------------
//   partChanged
//---------------------------------------------------------

void ExcerptsDialog::partChanged(QTreeWidgetItem*, QTreeWidgetItem*)
      {
      removeButton->setEnabled(getCurrentPartItem());
      }

//---------------------------------------------------------
//   instrumentChanged
//---------------------------------------------------------

void ExcerptsDialog::instrumentChanged(QListWidgetItem*, QListWidgetItem*)
      {
      addButton->setEnabled(getCurrentInstrumentItem());
      }

//---------------------------------------------------------
//   partDoubleClicked
//---------------------------------------------------------

void ExcerptsDialog::partDoubleClicked(QTreeWidgetItem* item, int)
      {
      if (!item->parent()) { // top level items are PartItem
            PartItem* pi = static_cast<PartItem*>(item); // TODO
            QString s = pi->part()->partName();
            title->setText(s);
            titleChanged(s);
            }
      }

//---------------------------------------------------------
//   doubleClickedInstrument
//---------------------------------------------------------

void ExcerptsDialog::doubleClickedInstrument(QTreeWidgetItem*)
      {
      addButtonClicked();
      }

//---------------------------------------------------------
//   titleChanged
//---------------------------------------------------------

void ExcerptsDialog::titleChanged(const QString& s)
      {
      ExcerptItem* ei = getCurrentExcerptItem();
      if (ei)
            ei->setTitle(s);

      score->masterScore()->setExcerptsChanged(true);
      }

//---------------------------------------------------------
//   createName
//---------------------------------------------------------

QString ExcerptsDialog::createName(const QString& partName)
      {
      int count = excerptList->count();
      QList<Excerpt*> excerpts;
      for (int i = 0; i < count; ++i)
            excerpts.append(getExcerptItemAt(i)->excerpt());

      return Excerpt::createName(partName, excerpts);
      }

//---------------------------------------------------------
//   getCurrentPartItem
//---------------------------------------------------------

PartItem* ExcerptsDialog::getCurrentPartItem() const
      {
      return static_cast<PartItem*>(partList->currentItem());
      }

//---------------------------------------------------------
//   getCurrentExcerptItem
//---------------------------------------------------------

ExcerptItem* ExcerptsDialog::getCurrentExcerptItem() const
      {
      return static_cast<ExcerptItem*>(excerptList->currentItem());
      }

//---------------------------------------------------------
//   getExcerptItemAt
//---------------------------------------------------------

ExcerptItem* ExcerptsDialog::getExcerptItemAt(int index) const
      {
      return static_cast<ExcerptItem*>(excerptList->item(index));
      }

//---------------------------------------------------------
//   getInstrumentItemAt
//---------------------------------------------------------

InstrumentItem* ExcerptsDialog::getCurrentInstrumentItem() const
      {
      return static_cast<InstrumentItem*>(instrumentList->currentItem());
      }

//---------------------------------------------------------
//   clearPartList
//---------------------------------------------------------

void ExcerptsDialog::clearPartList()
      {
      // The clear method of QTreeWidget cannot be used because it will delete
      // all items, leaving behind invalid pointer.
      // takeTopLevelItem on the other hand will remove the item from the tree
      // but will not delete it.
      while (partList->takeTopLevelItem(0)) {}
      }


//---------------------------------------------------------
//   setWidgetState
//---------------------------------------------------------

void ExcerptsDialog::setWidgetState()
      {
      ExcerptItem* ei = getCurrentExcerptItem();

      const bool enable = ei && !ei->isPartScore();

      instrumentList->setEnabled(enable);
      title->setEnabled(ei);
      deleteButton->setEnabled(ei);

      addButton->setEnabled(ei && getCurrentInstrumentItem());
      removeButton->setEnabled(ei && getCurrentPartItem());

      int idx = excerptList->currentIndex().row();
      moveUpButton->setEnabled(ei && (idx > 0));
      moveDownButton->setEnabled(ei && (idx < (excerptList->count() - 1)));
      }

//---------------------------------------------------------
//   getExcerptItem
//---------------------------------------------------------

ExcerptItem* ExcerptsDialog::getExcerptItem(Excerpt* e)
      {
      for (int i = 0; i < excerptList->count(); ++i) {
            ExcerptItem* ei = getExcerptItemAt(i);
            if (ei && (ei->excerpt() == e))
                  return ei;
            }
      return 0;
      }

//---------------------------------------------------------
//   createNewExcerpt
//---------------------------------------------------------

void ExcerptsDialog::createNewExcerpt(ExcerptItem* ei)
      {
      Excerpt* e = ei->excerpt();
      e->setTitle(ei->text());
      if (e->partScore())
            return;
      if (e->parts().isEmpty()) {
            qDebug("no parts");
            return;
            }

      Score* nscore = new Score(e->oscore());
      e->setPartScore(nscore);

      qDebug() << " + Add part : " << e->title();
      score->undo(new AddExcerpt(e));
      e->setTracks(ei->tracks());
      Excerpt::createExcerpt(e);

      // a new excerpt is created in AddExcerpt, make sure the parts are filed
      for (Excerpt* ee : e->oscore()->excerpts()) {
            if (ee->partScore() == nscore && ee != e) {
                  ee->parts().clear();
                  ee->parts().append(e->parts());
                  }
            }

      partList->setEnabled(false);
      title->setEnabled(false);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ExcerptsDialog::accept()
      {
      for (int i = 0; i < excerptList->count(); ++i)
            excerptList->setCurrentRow(i);

      score->startCmd();

      // first pass : see if actual parts needs to be deleted or renamed
      foreach (Excerpt* e, score->excerpts()) {
            Score* partScore  = e->partScore();
            ExcerptItem* ei = getExcerptItem(e);
            if (!getExcerptItem(e) && partScore)      // Delete it because not in the list anymore
                  score->deleteExcerpt(e);
            else if (ei->text() != e->title())
                  score->undo(new ChangeExcerptTitle(e, ei->text()));
            }

      // Second pass : Create new parts
      for (int i = 0; i < excerptList->count(); ++i) {
            ExcerptItem* ei = getExcerptItemAt(i);
            if (ei && !ei->isEmpty())
                  createNewExcerpt(ei);
            }

      // Third pass : Remove empty parts.
      int j = 0;
      while (j < excerptList->count()) {
            // This new part is empty, so we don't create an excerpt but remove it from the list.
            // Necessary to order the parts later on.
            ExcerptItem* ei = getExcerptItemAt(j);
            if (ei->excerpt()->parts().isEmpty() && !ei->excerpt()->partScore()) {
                  qDebug() << " - Deleting empty parts : " << ei->text();
                  delete ei;
                  }
            else
                  j++;
            }

      // Update the score parts order following excerptList widget
      // The reference is the excerpt list. So we iterate following it and swap parts in the score accordingly
      for (int i = 0; i < excerptList->count(); ++i) {
            ExcerptItem* ei = getExcerptItemAt(i);

            if (!ei || ei->isEmpty())
                  continue;

            int position = 0;  // Actual order position in score
            bool found = false;

            // Looks for the excerpt and its position.
            foreach(Excerpt* e, score->excerpts()) {
                  if (ei->excerpt() == e) {
                        found = true;
                        break;
                        }
                  position++;
                  }
            if (found && (position != i))
                  score->undo(new SwapExcerpt(score, i, position));
            }
      score->endCmd();
      QDialog::accept();
      }
}
