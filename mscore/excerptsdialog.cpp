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
#include "libmscore/excerpt.h"
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

PartItem::PartItem(Part* p, QListWidget* parent)
   : QListWidgetItem(parent)
      {
      setFlags(Qt::ItemFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable));
      setCheckState(Qt::Unchecked);
      _part = p;
      setText(p->partName().replace("/", "_"));
      }

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

ExcerptsDialog::ExcerptsDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("PartEditor");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      score = s;
      if (score->parentScore())
            score = score->parentScore();

      foreach(Excerpt* e, score->excerpts()) {
            ExcerptItem* ei = new ExcerptItem(e);
            excerptList->addItem(ei);
            }
      foreach(Part* p, score->parts()) {
            PartItem* item = new PartItem(p);
            partList->addItem(item);
            }

      moveUpButton->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      moveDownButton->setIcon(*icons[int(Icons::arrowDown_ICON)]);

      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));
      connect(newAllButton, SIGNAL(clicked()), SLOT(newAllClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(moveUpButton, SIGNAL(clicked()), SLOT(moveUpClicked()));
      connect(moveDownButton, SIGNAL(clicked()), SLOT(moveDownClicked()));
      connect(excerptList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(excerptChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(partList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(partDoubleClicked(QListWidgetItem*)));
      connect(partList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(partClicked(QListWidgetItem*)));
      connect(title, SIGNAL(textChanged(const QString&)), SLOT(titleChanged(const QString&)));

      int n = score->excerpts().size();
      if (n > 0)
            excerptList->setCurrentRow(0);
      moveDownButton->setEnabled(n > 1);
      moveUpButton->setEnabled(false);
      bool flag = excerptList->currentItem() != 0;
      editGroup->setEnabled(flag);
      deleteButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   startExcerptsDialog
//---------------------------------------------------------

void MuseScore::startExcerptsDialog()
      {
      if (cs == 0)
            return;
      ExcerptsDialog ed(cs, 0);
      MuseScore::restoreGeometry(&ed);
      ed.exec();
      MuseScore::saveGeometry(&ed);
      cs->setLayoutAll(true);
      cs->end();
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
      //excerptList->takeItem(row);
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
            ExcerptItem* e = (ExcerptItem*)excerptList->item(i);
            if (e->excerpt()->title() != e->text()) {
                  // if except score not created yet, change the UI title
                  // if already created, change back(see createName) the excerpt title
                  if (!e->excerpt()->partScore())
                        e->setText(e->excerpt()->title());
                  else
                        e->excerpt()->setTitle(e->text());
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
//   excerptChanged
//---------------------------------------------------------

void ExcerptsDialog::excerptChanged(QListWidgetItem* cur, QListWidgetItem*)
      {
      bool b = true;
      if (cur) {
            Excerpt* e = ((ExcerptItem*)cur)->excerpt();
            title->setText(e->title());

            // set selection:
            QList<Part*>& pl = e->parts();
            int n = partList->count();
            for (int i = 0; i < n; ++i) {
                  PartItem* pi = (PartItem*)partList->item(i);
                  int idx = pl.indexOf(pi->part());
                  pi->setCheckState(idx != -1 ? Qt::Checked : Qt::Unchecked);
                  }
            b = e->partScore() == 0;
            }
      else {
            title->setText("");
            int n = partList->count();
            for (int i = 0; i < n; ++i) {
                  PartItem* pi = (PartItem*)partList->item(i);
                  pi->setCheckState(Qt::Unchecked);
                  }
            b = false;
            }
      partList->setEnabled(b);
      title->setEnabled(b);

      bool flag = excerptList->currentItem() != 0;
      int n = excerptList->count();
      int idx = excerptList->currentIndex().row();
      moveUpButton->setEnabled(idx > 0);
      moveDownButton->setEnabled(idx < (n-1));
      editGroup->setEnabled(flag);
      deleteButton->setEnabled(flag);
      }

//---------------------------------------------------------
//   partDoubleClicked
//---------------------------------------------------------

void ExcerptsDialog::partDoubleClicked(QListWidgetItem* item)
      {
      PartItem* pi = (PartItem*)item;
      title->setText(pi->part()->partName());
      }

//---------------------------------------------------------
//   partClicked
//---------------------------------------------------------

void ExcerptsDialog::partClicked(QListWidgetItem* item)
      {
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;
      Excerpt* excerpt = static_cast<ExcerptItem*>(cur)->excerpt();

      PartItem* pi = static_cast<PartItem*>(item);
      if (item->checkState() == Qt::Checked) {
            excerpt->parts().clear();
            for (int i = 0; i < partList->count(); i++) {
                  PartItem* pii = static_cast<PartItem*>(partList->item(i));
                  if (pii->checkState() == Qt::Checked)
                        excerpt->parts().append(pii->part());
                  }
           }
      else {
            excerpt->parts().removeOne(pi->part());
            }
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked(QListWidgetItem* cur)
      {
      Excerpt* e = static_cast<ExcerptItem*>(cur)->excerpt();
      if (e->partScore())
            return;
      if (e->parts().isEmpty())
            return;

      Score* nscore = new Score(e->oscore());
      e->setPartScore(nscore);

      nscore->setName(e->title()); // needed before AddExcerpt
      nscore->style()->set(StyleIdx::createMultiMeasureRests, true);

      qDebug() << " + Add part : " << e->title();
      score->undo(new AddExcerpt(nscore));
      createExcerpt(e);

      // a new excerpt is created in AddExcerpt, make sure the parts are filed
      for (Excerpt* ee : e->oscore()->excerpts()) {
            if (ee->partScore() == nscore) {
                  ee->parts().clear();
                  ee->parts().append(e->parts());
                  }
            }

      partList->setEnabled(false);
      title->setEnabled(false);
      }

//---------------------------------------------------------
//   titleChanged
//---------------------------------------------------------

void ExcerptsDialog::titleChanged(const QString& s)
      {
      QListWidgetItem* cur = excerptList->currentItem();
      if (cur == 0)
            return;
      Excerpt* excerpt = ((ExcerptItem*)cur)->excerpt();
      excerpt->setTitle(s);
      cur->setText(s);
      }

//---------------------------------------------------------
//   titleChanged
//---------------------------------------------------------

bool ExcerptsDialog::isInPartsList(Excerpt* e)
      {
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            if (cur == 0)
                  continue;
            if (((ExcerptItem*)cur)->excerpt() == ExcerptItem(e).excerpt())
                  return true;
                  //qDebug() << "#" << i << "¸-> " << (((ExcerptItem*)cur)->excerpt())->title();
            }
      return false;
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ExcerptsDialog::accept()
      {
      score->startCmd();

      // first pass : see if actual parts needs to be deleted
      qDebug() << "\nFirst pass : delete unwanted parts";

      int pos = 0;
      foreach(Excerpt* e, score->excerpts()) {
            if (!isInPartsList(e)) {
                  // Delete it because not in the list anymore
                  if (e->partScore()) {
                        Score* partScore = e->partScore();
                        qDebug() << " - Deleting parts : " << ExcerptItem(e).excerpt()->title();

                        // Swap Excerpts to the end before deleting, so if undoing, the part will be reordered
                        int lastPos = score->excerpts().size()-1;
                        if ((lastPos > 0) && (pos != lastPos))
                              score->undo(new SwapExcerpt(score, pos, lastPos));

                        deleteExcerpt(e);
                        // remove the excerpt
                        score->undo(new RemoveExcerpt(partScore));
                        }
                  }
            else
                  pos++;
            }

      // Second pass : Create new parts
      qDebug() << "\nSecond pass : create new parts";
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
      qDebug ()  << "\nFourth pass : Reordering parts";
      qDebug ()  << "   Nb parts in score->excerpts().size() = " << score->excerpts().size();
      qDebug ()  << "   Nb parts in the parts dialog : excerptList->count() = " << excerptList->count();

      // The reference is the excerpt list. So we iterate following it and swap parts in the score accordingly
      for (int i = 0; i < excerptList->count(); ++i) {
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
            if ((found) && (position != i)) {
                  qDebug() << "   i=" << i << " <-> position=" << position;
                  score->undo(new SwapExcerpt(score, i, position));
                  }
            }

      score->endCmd();
      score->setExcerptsChanged(true);

      QDialog::accept();
      }
}

