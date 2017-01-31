//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: albummanager.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011-2016 Werner Schweer and others
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

#include "albummanager.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"

namespace Ms {

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

AlbumManager::AlbumManager(QWidget* parent)
   : AbstractDialog(parent)
      {
      setObjectName("AlbumManager");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      album = 0;
      connect(add,         SIGNAL(clicked()), SLOT(addClicked()));
      connect(addNew,      SIGNAL(clicked()), SLOT(addNewClicked()));
      connect(up,          SIGNAL(clicked()), SLOT(upClicked()));
      connect(down,        SIGNAL(clicked()), SLOT(downClicked()));
      connect(remove,      SIGNAL(clicked()), SLOT(removeClicked()));
      connect(scoreList,   SIGNAL(currentRowChanged(int)), SLOT(currentScoreChanged(int)));
      connect(scoreList,   SIGNAL(itemChanged(QListWidgetItem*)), SLOT(itemChanged(QListWidgetItem*)));
      connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      currentScoreChanged(-1);
      add->setEnabled(false);

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void AlbumManager::buttonBoxClicked(QAbstractButton*)
      {
      printf("buttonBox clicked\n");
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void AlbumManager::addClicked()
      {
      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Files") + " (*.mscz *.mscx);;", tr("MuseScore: Load Score")
         );
      QList<MasterScore*> scores;
      for (const QString& fn : files) {
            MasterScore* score = mscore->readScore(fn);
            Movements* m = score->movements();
            for (MasterScore* ms : *m) {
                  scores.push_back(ms);
                  ms->setMovements(0);
                  }
            delete m;
            }
      if (scores.empty())
            return;
      MasterScore* topScore = album->front();
      scoreList->blockSignals(true);
      for (MasterScore* score : scores) {
            topScore->addMovement(score);
            QListWidgetItem* li = new QListWidgetItem(score->metaTag("movementTitle"), scoreList);
            li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
            }
      scoreList->blockSignals(false);
      topScore->setLayoutAll();
      topScore->update();
      }

//---------------------------------------------------------
//   addNewClicked
//---------------------------------------------------------

void AlbumManager::addNewClicked()
      {
#if 0
      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Files") + " (*.mscz *.mscx)",
         tr("MuseScore: Add Score")
         );
      if (files.isEmpty())
            return;
      for (QString fn : files) {
            if (fn.isEmpty())
                  continue;
            if (fn.endsWith (".mscz") || fn.endsWith (".mscx")) {
                  album->append(new AlbumItem(fn));
                  QFileInfo fi(fn);

                  QListWidgetItem* li = new QListWidgetItem(fi.completeBaseName(), scoreList);
                  li->setToolTip(fn);
                  li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
                  }
            }
#endif
      }

//---------------------------------------------------------
//   upClicked
//---------------------------------------------------------

void AlbumManager::upClicked()
      {
#if 0
      int idx = scoreList->currentRow();
      if (idx == -1 || idx == 0)
            return;
      QListWidgetItem* item = scoreList->takeItem(idx);
      scoreList->insertItem(idx-1, item);
      album->swap(idx, idx - 1);
      scoreList->setCurrentRow(idx-1);
#endif
      }

//---------------------------------------------------------
//   downClicked
//---------------------------------------------------------

void AlbumManager::downClicked()
      {
#if 0
      int idx = scoreList->currentRow();
      int n = scoreList->count();
      if (idx == -1 || idx >= (n-1))
            return;
      QListWidgetItem* item = scoreList->takeItem(idx+1);
      scoreList->insertItem(idx, item);
      album->swap(idx, idx+1);
      scoreList->setCurrentRow(idx+1);
#endif
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked()
      {
#if 0
      int n = scoreList->currentRow();
      if (n == -1)
            return;
      delete scoreList->takeItem(n);
      album->remove(n);
#endif
      }

//---------------------------------------------------------
//   setAlbum
//---------------------------------------------------------

void AlbumManager::setAlbum(Movements* a)
      {
      album = a;
      scoreList->clear();

      scoreList->blockSignals(true);
      for (MasterScore* score : *album) {
            QListWidgetItem* li = new QListWidgetItem(score->metaTag("movementTitle"), scoreList);
            li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
            }
      scoreList->blockSignals(false);
      add->setEnabled(true);
      }

//---------------------------------------------------------
//   currentScoreChanged
//---------------------------------------------------------

void AlbumManager::currentScoreChanged(int idx)
      {
      int n = scoreList->count();
      if (idx == -1) {
            up->setEnabled(false);
            down->setEnabled(false);
            remove->setEnabled(false);
            return;
            }
      down->setEnabled(idx < (n-1));
      up->setEnabled(idx > 0);
      remove->setEnabled(true);
      }

//---------------------------------------------------------
//   itemChanged
//---------------------------------------------------------

void AlbumManager::itemChanged(QListWidgetItem* item)
      {
      int row = scoreList->row(item);
      MasterScore* ms = *(album->begin() + row);
      ms->undo(new ChangeMetaText(ms, "movementTitle", item->text()));
      }

//---------------------------------------------------------
//   showAlbumManager
//---------------------------------------------------------

void MuseScore::showAlbumManager()
      {
      if (albumManager == 0)
            albumManager = new AlbumManager(this);
      albumManager->setAlbum(currentScoreView()->score()->masterScore()->movements());
      albumManager->show();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void AlbumManager::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }
}

