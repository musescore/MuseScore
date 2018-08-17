//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: excerptsdialog.cpp 5497 2012-03-26 10:59:16Z lasconic $
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
//   ExcerptItem
//---------------------------------------------------------

ExcerptItem::ExcerptItem(Excerpt* e, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      _excerpt = e;
      setText(e->title());
      }

//---------------------------------------------------------
//   PartItem
//---------------------------------------------------------

PartItem::PartItem(Part* p, QTreeWidget* parent)
   : QTreeWidgetItem(parent)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable));
      _part   = p;
      setText(0, p->partName().replace("/", "_"));
      }

//---------------------------------------------------------
//   InstrumentItem
//---------------------------------------------------------

InstrumentItem::InstrumentItem(PartItem* p, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable));
      _partItem = p;
      setText(p->part()->partName().replace("/", "_"));
      }

//---------------------------------------------------------
//   StaffItem
//---------------------------------------------------------

StaffItem::StaffItem(PartItem* li)
   : QTreeWidgetItem(li)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable));
      for (int i = 1; i <= VOICES; i++) {
            setCheckState(i, Qt::Checked);
            }
      }

void StaffItem::setData(int column, int role, const QVariant& value)
      {
      const bool isCheckChange = column > 0
         && role == Qt::CheckStateRole
         && data(column, role).isValid() // Don't "change" during initialization
         && checkState(column) != value;
      QTreeWidgetItem::setData(column, role, value);
      if (isCheckChange) {
            int unchecked = 0;
            for (int i = 1; i <= VOICES; i++) {
                  if (checkState(i) == Qt::Unchecked)
                        unchecked += 1;
                  }
            if (unchecked == VOICES)
                  setCheckState(column, Qt::Checked);
            }
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

      for (Excerpt* e : score->excerpts()) {
            ExcerptItem* ei = new ExcerptItem(e);
            excerptList->addItem(ei);
            }
      QMultiMap<int, int> t;
      for (Part* p : score->parts()) {
            PartItem* pI = new PartItem(p);
            InstrumentItem* item = new InstrumentItem(pI);
            instrumentList->addItem(item);
            }

      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));
      connect(newAllButton, SIGNAL(clicked()), SLOT(newAllClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(moveUpButton, SIGNAL(clicked()), SLOT(moveUpClicked()));
      connect(moveDownButton, SIGNAL(clicked()), SLOT(moveDownClicked()));
      connect(addButton, SIGNAL(clicked()), SLOT(addButtonClicked()));
      connect(removeButton, SIGNAL(clicked()), SLOT(removeButtonClicked()));

      connect(excerptList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(excerptChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(partList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
         SLOT(partDoubleClicked(QTreeWidgetItem*, int)));
      connect(partList, SIGNAL(itemClicked(QTreeWidgetItem*,int)), SLOT(partClicked(QTreeWidgetItem*,int)));
      connect(instrumentList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
              SLOT(addButtonClicked()));
      connect(title, SIGNAL(textChanged(const QString&)), SLOT(titleChanged(const QString&)));

      for (int i = 1; i <= VOICES; i++) {
            //partList->model()->setHeaderData(i, Qt::Horizontal, MScore::selectColor[i-1], Qt::BackgroundRole);
            partList->header()->resizeSection(i, 30);
            }

      int n = score->excerpts().size();
      if (n > 0)
            excerptList->setCurrentRow(0);
      moveDownButton->setEnabled(n > 1);
      moveUpButton->setEnabled(false);
      bool flag = excerptList->currentItem() != 0;
      deleteButton->setEnabled(flag);
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
//   newClicked
//---------------------------------------------------------

void ExcerptsDialog::newClicked()
      {
      QString name = createName("Part");
      Excerpt* e   = new Excerpt(score);
      e->setTitle(name);
      ExcerptItem* ei = new ExcerptItem(e);
      excerptList->addItem(ei);
      excerptList->selectionModel()->clearSelection();
      excerptList->setCurrentItem(ei, QItemSelectionModel::SelectCurrent);
      for (int i = 0; i < excerptList->count(); ++i) {
            ExcerptItem* eii = (ExcerptItem*)excerptList->item(i);
            if (eii->excerpt()->title() != eii->text()) {
                  // if except score not created yet, change the UI title
                  // if already created, change back(see createName) the excerpt title
                  if (!eii->excerpt()->partScore())
                        eii->setText(eii->excerpt()->title());
                  else
                        eii->excerpt()->setTitle(eii->text());
                  }
            }
      }

//---------------------------------------------------------
//   newAllClicked
//---------------------------------------------------------

void ExcerptsDialog::newAllClicked()
      {
      QList<Excerpt*> excerpts = Excerpt::createAllExcerpt(score);
      ExcerptItem* ei = 0;
      for (Excerpt* e : excerpts) {
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
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;

      delete cur;
      }

//---------------------------------------------------------
//   moveUpClicked
//---------------------------------------------------------

void ExcerptsDialog::moveUpClicked()
      {
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;
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
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;
      int currentRow = excerptList->currentRow();
      int nbRows = excerptList->count();
      if (currentRow >= nbRows - 1)
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
      if (!excerptList->currentItem() || !partList->isEnabled())
            return;

      Excerpt* cur = ((ExcerptItem*)(excerptList->currentItem()))->excerpt();

      foreach(QListWidgetItem* i, instrumentList->selectedItems()) {
            InstrumentItem* item = static_cast<InstrumentItem*>(i);
            const PartItem* it   = item->partItem();
            if (it == 0)
                  continue;
            PartItem* pi = new PartItem(it->part(), 0);
            pi->setText(0, pi->part()->name());
            cur->parts().append(pi->part());
            partList->addTopLevelItem(pi);
            for (Staff* s : *pi->part()->staves()) {
                  StaffItem* sli = new StaffItem(pi);
                  sli->setStaff(s);
                  for (int j = 0; j < VOICES; j++)
                        sli->setCheckState(j + 1, Qt::Checked);
                  }
            pi->setText(0, pi->part()->partName());
            }

      cur->setTracks(mapTracks());
      partList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   removeButtonClicked
//    remove instrument from score
//---------------------------------------------------------

void ExcerptsDialog::removeButtonClicked()
      {
      QList<QTreeWidgetItem*> wi = partList->selectedItems();
      if (wi.isEmpty())
            return;

      Excerpt* cur = ((ExcerptItem*)(excerptList->currentItem()))->excerpt();
      QTreeWidgetItem* item = wi.first();

      cur->parts().removeAt(partList->indexOfTopLevelItem(item));
      delete item;

      cur->setTracks(mapTracks());
      partList->resizeColumnToContents(0);
      }

//---------------------------------------------------------
//   excerptChanged
//---------------------------------------------------------

void ExcerptsDialog::excerptChanged(QListWidgetItem* cur, QListWidgetItem*)
      {
      bool b = true;
      if (cur) {
            ExcerptItem* curItem = static_cast<ExcerptItem*>(cur);
            Excerpt* e = curItem->excerpt();
            title->setText(curItem->text());
            b = e->partScore() == 0;

            // set selection:
            QList<Part*>& pl = e->parts();
            QMultiMap<int, int> tracks = e->tracks();
            partList->clear();
            for (Part* p: pl) {
                  PartItem* pi = new PartItem(p, partList);
                  partList->addTopLevelItem(pi);
                  for (Staff* s : *p->staves()) {
                        StaffItem* sli = new StaffItem(pi);
                        sli->setStaff(s);
                        sli->setDisabled(!b);
                        }
                  pi->setText(0, p->partName());
                  partList->setItemExpanded(pi, false);
                  }
            assignTracks(tracks);
            }
      else {
            title->setText("");
            partList->clear();
            b = false;
            }
      instrumentList->setEnabled(b);
      title->setEnabled(true);
      addButton->setEnabled(b);
      removeButton->setEnabled(b);

      bool flag = excerptList->currentItem() != 0;
      int n = excerptList->count();
      int idx = excerptList->currentIndex().row();
      moveUpButton->setEnabled(idx > 0);
      moveDownButton->setEnabled(idx < (n-1));
      deleteButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   partDoubleClicked
//---------------------------------------------------------

void ExcerptsDialog::partDoubleClicked(QTreeWidgetItem* item, int)
      {
      if (!item->parent()) { // top level items are PartItem
            PartItem* pi = (PartItem*)item;
            QString s = pi->part()->partName();
            title->setText(s);
            titleChanged(s);
            }
      }

//---------------------------------------------------------
//   partClicked
//---------------------------------------------------------

void ExcerptsDialog::partClicked(QTreeWidgetItem*, int)
      {
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;

      Excerpt* excerpt = static_cast<ExcerptItem*>(cur)->excerpt();
      excerpt->setTracks(mapTracks());
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
      ExcerptItem* cur = static_cast<ExcerptItem*>(excerptList->currentItem());
      if (cur == 0)
            return;
      cur->setText(s);
      }

//---------------------------------------------------------
//   createName
//---------------------------------------------------------

QString ExcerptsDialog::createName(const QString& partName)
      {
      int count = excerptList->count();
      QList<Excerpt*> excerpts;
      for (int i = 0; i < count; ++i) {
            Excerpt* ee = static_cast<ExcerptItem*>(excerptList->item(i))->excerpt();
            excerpts.append(ee);
            }
      return Excerpt::createName(partName, excerpts);
      }

//---------------------------------------------------------
//   mapTracks
//---------------------------------------------------------

QMultiMap<int, int> ExcerptsDialog::mapTracks()
      {
      QMultiMap<int, int> tracks;
      int track = 0;

      for (QTreeWidgetItem* pwi = partList->itemAt(0,0); pwi; pwi = partList->itemBelow(pwi)) {
            PartItem* pi = (PartItem*)pwi;
            Part* p = pi->part();
            for (int j = 0; j < pwi->childCount(); j++) {
                  for (int k = 0; k < VOICES; k++) {
                        if (pwi->child(j)->checkState(k+1) == Qt::Checked) {
                              int voiceOff = 0;
                              int srcTrack = p->startTrack() + j * VOICES + k;
                              for (int i = srcTrack & ~3; i < srcTrack; i++) {
                                    QList<int> t = tracks.values(i);
                                    for (int ti : t) {
                                          if (ti >= (track & ~3))
                                                voiceOff++;
                                          }
                                    }
                              tracks.insert(srcTrack, (track & ~3) + voiceOff);
                              }
                        track++;
                        }
                  }
            }
      return tracks;
      }

//---------------------------------------------------------
//   assignTracks
//---------------------------------------------------------

void ExcerptsDialog::assignTracks(QMultiMap<int, int> tracks)
      {
      int track = 0;
      for (QTreeWidgetItem* pwi = partList->itemAt(0,0); pwi; pwi = partList->itemBelow(pwi)) {
            for (int j = 0; j < pwi->childCount(); j++) {
                  for (int k = 0; k < VOICES; k++) {
                        int checkTrack = tracks.key(track, -1);
                        if (checkTrack != -1)
                              pwi->child(j)->setCheckState((checkTrack % VOICES) + 1, Qt::Checked);
                        else
                              pwi->child(j)->setCheckState((track % VOICES) + 1, Qt::Unchecked);
                        track++;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   isInPartList
//---------------------------------------------------------

ExcerptItem* ExcerptsDialog::isInPartsList(Excerpt* e)
      {
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            excerptList->setCurrentRow(i);
            // ExcerptItem* cur = static_cast<ExcerptItem*>(excerptList->currentItem());
            ExcerptItem* cur = static_cast<ExcerptItem*>(excerptList->item(i));
            if (cur == 0)
                  continue;
//            if (((ExcerptItem*)cur)->excerpt() == ExcerptItem(e).excerpt())
            if (cur->excerpt() == e)
                  return cur;
            }
      return 0;
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked(QListWidgetItem* cur)
      {
      Excerpt* e = static_cast<ExcerptItem*>(cur)->excerpt();
      e->setTitle(title->text());
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
      score->startCmd();

      // first pass : see if actual parts needs to be deleted
      foreach (Excerpt* e, score->excerpts()) {
            Score* partScore  = e->partScore();
            ExcerptItem* item = isInPartsList(e);
            if (!isInPartsList(e) && partScore)      // Delete it because not in the list anymore
                  score->deleteExcerpt(e);
            else {
                  if (item->text() != e->title())
                        score->undo(new ChangeExcerptTitle(e, item->text()));
                  }
            }
      // Second pass : Create new parts
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            if (cur == 0)
                  continue;
            createExcerptClicked(cur);
            }

      // Third pass : Remove empty parts.
      int i = 0;
      while (i < excerptList->count()) {
            // This new part is empty, so we don't create an excerpt but remove it from the list.
            // Necessary to order the parts later on.
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            Excerpt* e = static_cast<ExcerptItem*>(cur)->excerpt();
            if (e->parts().isEmpty() && !e->partScore()) {
                  qDebug() << " - Deleting empty parts : " << cur->text();
                  delete cur;
                  }
            else
                  i++;
            }

      // Update the score parts order following excerpList widget
      // The reference is the excerpt list. So we iterate following it and swap parts in the score accordingly

      for (int j = 0; j < excerptList->count(); ++j) {
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            if (cur == 0)
                  continue;

            int position = 0;  // Actual order position in score
            bool found = false;

            // Looks for the excerpt and its position.
            foreach(Excerpt* e, score->excerpts()) {
                  if (((ExcerptItem*)cur)->excerpt() == ExcerptItem(e).excerpt()) {
                        found = true;
                        break;
                        }
                  position++;
                  }
            if ((found) && (position != j))
                  score->undo(new SwapExcerpt(score, j, position));
            }
      score->endCmd();
      QDialog::accept();
      }
}
