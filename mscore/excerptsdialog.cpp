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

namespace Ms {

extern bool useFactorySettings;

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

      if (score->excerpts().size() > 0)
            excerptList->setCurrentRow(0);
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
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup("PartEditor");
            ed.resize(settings.value("size", QSize(484, 184)).toSize());
            ed.move(settings.value("pos", QPoint(10, 10)).toPoint());
            settings.endGroup();
            }
      ed.exec();
      QSettings settings;
      settings.beginGroup("PartEditor");
      settings.setValue("size", ed.size());
      settings.setValue("pos", ed.pos());
      settings.endGroup();
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

      if (ex->partScore()) {
            score->startCmd();
            score->undo(new RemoveExcerpt(ex->partScore()));
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

      score->excerpts().swap(currentRow, currentRow-1);
      score->setExcerptsChanged(true);
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

      score->excerpts().swap(currentRow, currentRow+1);
      score->setExcerptsChanged(true);
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

      score->startCmd();
      score->undo(new AddExcerpt(nscore));
      createExcerpt(e);
      score->endCmd();

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
//   accept
//---------------------------------------------------------

void ExcerptsDialog::accept()
      {
      int n = excerptList->count();
      for (int i = 0; i < n; ++i) {
            excerptList->setCurrentRow(i);
            QListWidgetItem* cur = excerptList->currentItem();
            if (cur == 0)
                  continue;
            createExcerptClicked(cur);
            }
      QDialog::accept();
      }
}

