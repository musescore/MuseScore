//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2017 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
      up->setIcon(*icons[int(Icons::arrowUp_ICON)]);
      down->setIcon(*icons[int(Icons::arrowDown_ICON)]);

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
      //createNewClicked();
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void AlbumManager::buttonBoxClicked(QAbstractButton*)
      {
      printf("buttonBox clicked\n");
      }

//---------------------------------------------------------
//   getScoreTitle
//---------------------------------------------------------

static QString getScoreTitle(Score* score)
      {
      QString name = score->metaTag("movementTitle");
      if (name.isEmpty()) {
            Text* t = score->getText(Tid::TITLE);
            if (t)
                  name = QTextDocumentFragment::fromHtml(t->xmlText()).toPlainText().replace("&amp;","&").replace("&gt;",">").replace("&lt;","<").replace("&quot;", "\"");
            name = name.simplified();
            }
      if (name.isEmpty())
            name = score->title();
      return name;
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void AlbumManager::addClicked()
      {
      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Files") + " (*.mscz *.mscx);;", tr("Load Score")
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
            QString name = getScoreTitle(score);
            QListWidgetItem* li = new QListWidgetItem(name, scoreList);
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
      MasterScore* score = mscore->getNewFile();
      if (!score)
            return;

      delete score->movements();
      MasterScore* topScore = album->front();

      scoreList->blockSignals(true);
      topScore->addMovement(score);
      QString name = getScoreTitle(score);
      QListWidgetItem* li = new QListWidgetItem(name, scoreList);
      li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
      scoreList->blockSignals(false);

      topScore->setLayoutAll();
      topScore->update();
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
      scoreList->clear();
      if (!a)
            return;

      album = a;

      scoreList->blockSignals(true);
      for (MasterScore* score : *album) {
            QString name = getScoreTitle(score);
            QListWidgetItem* li = new QListWidgetItem(name, scoreList);
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

      albumManager->setAlbum(currentScoreView() ? currentScoreView()->score()->masterScore()->movements() : 0);
      albumManager->show();

      // focus on album name on opening the Album Manager
      //albumManager->albumName->setFocus();
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

