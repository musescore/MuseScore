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
      setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      setCheckState(Qt::Unchecked);
      _part = p;
      setText(p->partName());
      }

//---------------------------------------------------------
//   ExcerptsDialog
//---------------------------------------------------------

ExcerptsDialog::ExcerptsDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
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
      createExcerpt->setEnabled(false);

      connect(newButton, SIGNAL(clicked()), SLOT(newClicked()));
      connect(newAllButton, SIGNAL(clicked()), SLOT(newAllClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(excerptList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(excerptChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(excerptList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(createExcerptClicked(QListWidgetItem*)));
      connect(createAllExcerpts, SIGNAL(clicked()), SLOT(createAllExcerptsClicked()));
      connect(partList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
         SLOT(partDoubleClicked(QListWidgetItem*)));
      connect(partList, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(partClicked(QListWidgetItem*)));
      connect(createExcerpt, SIGNAL(clicked()), SLOT(createExcerptClicked()));
      connect(title, SIGNAL(textChanged(const QString&)), SLOT(titleChanged(const QString&)));

//      if (!sel->isEmpty())
//            excerptList->setCurrentRow(0);
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
      ed.exec();
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
      Excerpt* ex = static_cast<ExcerptItem*>(cur)->excerpt();

      if (ex->score()) {
            score->startCmd();
            score->undo(new RemoveExcerpt(ex->score()));
            score->endCmd();
            }
      int row = excerptList->row(cur);
      excerptList->takeItem(row);
      }

//---------------------------------------------------------
//   createName
//---------------------------------------------------------

QString ExcerptsDialog::createName(const QString& partName)
      {
      QString n = partName.simplified();
      QString name;
      for (int i = 0;; ++i) {
            name = i ? QString("%1-%2").arg(n).arg(i) : QString("%1").arg(n);
            Excerpt* ee = 0;
            int n = excerptList->count();
            for (int k = 0; k < n; ++k) {
                  ee = static_cast<ExcerptItem*>(excerptList->item(k))->excerpt();
                  if (ee->title() == name)
                        break;
                  }
            if ((ee == 0) || (ee->title() != name))
                  break;
            }
      return name;
      }

//---------------------------------------------------------
//   newClicked
//---------------------------------------------------------

void ExcerptsDialog::newClicked()
      {
      QString name = createName("Part");
      Excerpt* e   = new Excerpt(0);
      e->setTitle(name);
      ExcerptItem* ei = new ExcerptItem(e);
      excerptList->addItem(ei);
      excerptList->selectionModel()->clearSelection();
      excerptList->setCurrentItem(ei, QItemSelectionModel::SelectCurrent);
      }

//---------------------------------------------------------
//   newAllClicked
//---------------------------------------------------------

void ExcerptsDialog::newAllClicked()
      {
      int n = partList->count();
      ExcerptItem* ei = 0;
      for (int i = 0; i < n; ++i) {
	      Excerpt* e   = new Excerpt(0);
	      PartItem* pi = static_cast<PartItem*>(partList->item(i));
            e->parts().append(pi->part());
	      QString name = createName(pi->part()->partName());
	      e->setTitle(name);
            excerptList->addItem(new ExcerptItem(e));
            }
      if (ei) {
            excerptList->selectionModel()->clearSelection();
            excerptList->setCurrentItem(ei, QItemSelectionModel::SelectCurrent);
            }
      }

//---------------------------------------------------------
//   excerptChanged
//---------------------------------------------------------

void ExcerptsDialog::excerptChanged(QListWidgetItem* cur, QListWidgetItem*)
      {
      createExcerpt->setEnabled(true);
      bool b;
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
            b = e->score() == 0;
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
            foreach(Part* p, excerpt->parts()) {
                  if (p == pi->part())
                        return;
                  }
            excerpt->parts().append(pi->part());
           }
      else {
            excerpt->parts().removeOne(pi->part());
            }
      }

//---------------------------------------------------------
//   createExcerptClicked
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked()
      {
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            QListWidgetItem* item = excerptList->item(i);
            Excerpt* excerpt = static_cast<ExcerptItem*>(item)->excerpt();
            if (excerpt->score()) {
qDebug("  already there %d %d\n", i, n);
                  continue;
                  }
            createExcerptClicked(item);
            }
      }

//---------------------------------------------------------
//   createAllExcerptsClicked
//---------------------------------------------------------

void ExcerptsDialog::createAllExcerptsClicked()
      {
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            if (cur == 0)
                  continue;
            createExcerptClicked(cur);
            }
      }

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

void ExcerptsDialog::createExcerptClicked(QListWidgetItem* cur)
      {
      Excerpt* e = static_cast<ExcerptItem*>(cur)->excerpt();
      if (e->score())
            return;
      Score* nscore = ::createExcerpt(e->parts());
      if (nscore == 0)
            return;
      nscore->setParentScore(score);
      e->setScore(nscore);
      nscore->setName(e->title());
      nscore->rebuildMidiMapping();
      nscore->updateChannel();
      nscore->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
      nscore->setLayoutAll(true);
      score->startCmd();
      score->undo(new AddExcerpt(nscore));
      score->endCmd();
      nscore->style()->set(ST_createMultiMeasureRests, true);

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

