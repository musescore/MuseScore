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
#include "album.h"
#include "globals.h"
#include "musescore.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"

namespace Ms {

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

AlbumManager::AlbumManager(QWidget* parent)
   : AbstractDialog(parent)
      {
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      album = 0;
      connect(add,         SIGNAL(clicked()), SLOT(addClicked()));
      connect(load,        SIGNAL(clicked()), SLOT(loadClicked()));
      connect(print,       SIGNAL(clicked()), SLOT(printClicked()));
      connect(createScore, SIGNAL(clicked()), SLOT(createScoreClicked()));
      connect(up,          SIGNAL(clicked()), SLOT(upClicked()));
      connect(down,        SIGNAL(clicked()), SLOT(downClicked()));
      connect(remove,      SIGNAL(clicked()), SLOT(removeClicked()));
      connect(createNew,   SIGNAL(clicked()), SLOT(createNewClicked()));
      connect(albumName,   SIGNAL(textChanged(const QString&)), SLOT(albumNameChanged(const QString&)));
      connect(scoreList,   SIGNAL(currentRowChanged(int)), SLOT(currentScoreChanged(int)));
      connect(scoreList,   SIGNAL(itemChanged(QListWidgetItem*)), SLOT(itemChanged(QListWidgetItem*)));
      connect(buttonBox,   SIGNAL(clicked(QAbstractButton*)), SLOT(buttonBoxClicked(QAbstractButton*)));
      currentScoreChanged(-1);
      add->setEnabled(false);
      print->setEnabled(false);
      albumName->setEnabled(false);
      createScore->setEnabled(false);
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void AlbumManager::addClicked()
      {
      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Files") + " (*.mscz *.mscx)",
         tr("MuseScore: Add Score")
         );
      if (files.isEmpty())
            return;
      foreach(QString fn, files) {
            if (fn.isEmpty())
                  continue;
            if(fn.endsWith (".mscz") || fn.endsWith (".mscx")) {
                  album->append(new AlbumItem(fn));
                  QFileInfo fi(fn);

                  QListWidgetItem* li = new QListWidgetItem(fi.completeBaseName(), scoreList);
                  li->setToolTip(fn);
                  li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
                  }
            }
      }

//---------------------------------------------------------
//   loadClicked
//---------------------------------------------------------

void AlbumManager::loadClicked()
      {
      QStringList files = mscore->getOpenScoreNames(
         tr("MuseScore Album Files") + " (*.album)",
         tr("MuseScore: Load Album")
         );
      if (files.isEmpty())
            return;
      QString fn = files.front();
      if (fn.isEmpty())
            return;
      Album* a = new Album;
      if (a->read(fn))
            setAlbum(a);
      }

//---------------------------------------------------------
//   printClicked
//---------------------------------------------------------

void AlbumManager::printClicked()
      {
      album->print();
      }

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

void AlbumManager::createScoreClicked()
      {
      if (album) {
            if (album->scores().isEmpty())
                   return;
            QString filter = QWidget::tr("MuseScore File") + " (*.mscz)";
            QSettings settings;
            if (mscore->lastSaveDirectory.isEmpty())
                  mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
            QString saveDirectory = mscore->lastSaveDirectory;

            if (saveDirectory.isEmpty())
                  saveDirectory = preferences.myScoresPath;
            QString fname   = QString("%1/%2.mscz").arg(saveDirectory).arg(album->name());
            QString fn     = mscore->getSaveScoreName(
            QWidget::tr("MuseScore: Save Album into Score"),
                  fname,
                  filter
            );
            if (fn.isEmpty())
                  return;
            if (!album->createScore(fn, checkBoxAddPageBreak->isChecked(), checkBoxAddSectionBreak->isChecked()))
                  QMessageBox::critical(mscore, QWidget::tr("MuseScore: Save File"), tr("Error while creating score from album."));
            }
      }

//---------------------------------------------------------
//   upClicked
//---------------------------------------------------------

void AlbumManager::upClicked()
      {
      int idx = scoreList->currentRow();
      if (idx == -1 || idx == 0)
            return;
      QListWidgetItem* item = scoreList->takeItem(idx);
      scoreList->insertItem(idx-1, item);
      album->swap(idx, idx - 1);
      scoreList->setCurrentRow(idx-1);
      }

//---------------------------------------------------------
//   downClicked
//---------------------------------------------------------

void AlbumManager::downClicked()
      {
      int idx = scoreList->currentRow();
      int n = scoreList->count();
      if (idx == -1 || idx >= (n-1))
            return;
      QListWidgetItem* item = scoreList->takeItem(idx+1);
      scoreList->insertItem(idx, item);
      album->swap(idx, idx+1);
      scoreList->setCurrentRow(idx+1);
      }

//---------------------------------------------------------
//   removeClicked
//---------------------------------------------------------

void AlbumManager::removeClicked()
      {
      int n = scoreList->currentRow();
      if (n == -1)
            return;
      delete scoreList->takeItem(n);
      album->remove(n);
      }

//---------------------------------------------------------
//   setAlbum
//---------------------------------------------------------

void AlbumManager::setAlbum(Album* a)
      {
      if (album && album->dirty()) {
            writeAlbum();
            }
      delete album;
      album = a;
      scoreList->clear();
      albumName->setText(album->name().isEmpty() ? QWidget::tr("Untitled") : album->name());
      foreach(AlbumItem* a, album->scores()) {
            QListWidgetItem* li = new QListWidgetItem(a->name, scoreList);
            li->setToolTip(a->path);
            li->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled));
            }
      albumName->setEnabled(true);
      add->setEnabled(true);
      print->setEnabled(true);
      createScore->setEnabled(true);
      }

//---------------------------------------------------------
//   createNewClicked
//---------------------------------------------------------

void AlbumManager::createNewClicked()
      {
      setAlbum(new Album);
      }

//---------------------------------------------------------
//   albumNameChanged
//---------------------------------------------------------

void AlbumManager::albumNameChanged(const QString& s)
      {
      album->setName(s);
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
      AlbumItem* ai = album->item(row);
      if (ai->name != item->text()) {
            ai->name = item->text();
            album->setDirty(true);
            }
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void AlbumManager::closeEvent(QCloseEvent* event)
      {
      if (album && album->dirty())
            writeAlbum();
      QDialog::closeEvent(event);
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void AlbumManager::buttonBoxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::StandardButton sb = buttonBox->standardButton(button);
      if (sb == QDialogButtonBox::Close) {
            if (album && album->dirty())
                  writeAlbum();
            }
      }

//---------------------------------------------------------
//   buttonBoxClicked
//---------------------------------------------------------

void AlbumManager::writeAlbum()
      {
      if (!album)
            return;
      if (album->path().isEmpty()) {
            QString home = preferences.myScoresPath;
            QString albumName = album->name();
            QString fn = mscore->getSaveScoreName(
               QWidget::tr("MuseScore: Save Album"),
               albumName,
               QWidget::tr("MuseScore Files") + " (*.album)"
               );
            if (fn.isEmpty()) {
                  album->setDirty(false);
                  return;
                  }
            album->setPath(fn);
            }
      if (QFileInfo(album->path()).suffix().isEmpty())
            album->setPath(album->path() + ".album");
      QFile f(album->path());
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = QWidget::tr("Open Album File\n%1\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, QWidget::tr("MuseScore: Open Album File"), s.arg(album->path()));
            return;
            }
      Xml xml(&f);
      album->write(xml);
      if (f.error() != QFile::NoError) {
            QString s = QWidget::tr("Write Album failed: ") + f.errorString();
            QMessageBox::critical(0, QWidget::tr("MuseScore: Write Album"), s);
            }
      }

//---------------------------------------------------------
//   showAlbumManager
//---------------------------------------------------------

void MuseScore::showAlbumManager()
      {
      if (albumManager == 0)
            albumManager = new AlbumManager(this);
      albumManager->show();
      }
}

