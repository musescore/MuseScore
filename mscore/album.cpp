//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: mscore.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "album.h"
#include "globals.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "musescore.h"
#include "preferences.h"
#include "icons.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"

namespace Ms {

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

Album::Album()
      {
      _dirty = false;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void Album::print()
      {
      if (_scores.isEmpty())
            return;
      loadScores();
      QPrinter printer(QPrinter::HighResolution);

      Score* score = _scores[0]->score;
      printer.setCreator("MuseScore Version: " MSC_VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setDoubleSidedPrinting(score->pageFormat()->twosided());
      printer.setOutputFormat(QPrinter::NativeFormat);

      QPrintDialog pd(&printer, 0);
      if (!pd.exec())
            return;

      QPainter painter(&printer);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = printer.logicalDpiX() / MScore::DPI;
      painter.scale(mag, mag);

      int fromPage = printer.fromPage() - 1;
      int toPage   = printer.toPage()   - 1;
      if (fromPage < 0)
            fromPage = 0;
      if (toPage < 0)
            toPage = 100000;

      //
      // start pageOffset with configured offset of
      // first score
      //
      int pageOffset = 0;
      if (_scores[0]->score)
            pageOffset = _scores[0]->score->pageNumberOffset();

      foreach(AlbumItem* item, _scores) {
            Score* score = item->score;
            if (score == 0)
                  continue;
            score->setPrinting(true);
            //
            // here we ignore the configured page offset
            //
            int oldPageOffset = score->pageNumberOffset();
            score->setPageNumberOffset(pageOffset);
            score->doLayout();
            const QList<Page*> pl = score->pages();
            int pages    = pl.size();
            for (int n = 0; n < pages; ++n) {
                  if (n)
                        printer.newPage();
                  Page* page = pl.at(n);

                  QRectF fr = page->abbox();
                  QList<Element*> ell = page->items(fr);
                  qStableSort(ell.begin(), ell.end(), elementLessThan);
                  foreach(const Element* e, ell) {
                        e->itemDiscovered = 0;
                        if (!e->visible())
                              continue;
                        QPointF pos(e->pagePos());
                        painter.translate(pos);
                        e->draw(&painter);
                        painter.translate(-pos);
                        }
                  }
            pageOffset += pages;
            if(item != _scores.last())
                  printer.newPage();
            score->setPrinting(false);
            score->setPageNumberOffset(oldPageOffset);
            }
      painter.end();
      }

//---------------------------------------------------------
//   createScore
//---------------------------------------------------------

void Album::createScore()
      {
      if (_scores.isEmpty())
            return;
      QString filter = QWidget::tr("MuseScore File (*.mscz)");
      QSettings settings;
      if (mscore->lastSaveDirectory.isEmpty())
            mscore->lastSaveDirectory = settings.value("lastSaveDirectory", preferences.myScoresPath).toString();
      QString saveDirectory = mscore->lastSaveDirectory;
      
      if (saveDirectory.isEmpty())
          saveDirectory = preferences.myScoresPath;
      QString fname   = QString("%1/%2.mscz").arg(saveDirectory).arg(_name);
      QString fn     = mscore->getSaveScoreName(
         QWidget::tr("MuseScore: Save Album into Score"),
         fname,
         filter
         );
      if (fn.isEmpty())
            return;

      loadScores();

      Score* firstScore = _scores[0]->score;
      if (!firstScore)
            return;
      firstScore->doLayout();
      Score* score = firstScore->clone();
      foreach (AlbumItem* item, _scores) {
            if (item->score == 0 || item->score == firstScore)
                  continue;
            item->score->doLayout();
            if (!score->appendScore(item->score)) {
                  qDebug("cannot append score");
                  delete score;
                  return;
                  }
            }
      score->fileInfo()->setFile(fn);
      qDebug("Album::createScore: save file");
      try {
            score->saveCompressedFile(*score->fileInfo(), false);
            mscore->lastSaveDirectory = score->fileInfo()->absolutePath();
            }
      catch (QString s) {
            QMessageBox::critical(mscore, QWidget::tr("MuseScore: Save File"), s);
            }
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Album::read(const QString& p)
      {
      _path = p;
      QFile f(_path);
      if (!f.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Open Album failed"),
               QString(strerror(errno)),
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return false;
            }

      XmlReader e(&f);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl = version.split('.');
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "Album")
                              load(e);
                        else if (tag == "programVersion")
                              e.skipCurrentElement();
                        else
                              e.unknown();
                        }
                  }
            }
      _dirty = false;
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Album::load(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "Score") {
                  AlbumItem* i = new AlbumItem;
                  i->score = 0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "name")
                              i->name = e.readElementText();
                        else if (tag == "path")
                              i->path = e.readElementText();
                        else
                              e.unknown();
                        }
                  append(i);
                  }
            else if (tag == "name")
                  _name = e.readElementText();
            else
                  e.unknown();
            }
      _dirty = false;
      }

//---------------------------------------------------------
//   loadScores
//---------------------------------------------------------

void Album::loadScores()
      {
      foreach(AlbumItem* item, _scores) {
            if (item->path.isEmpty())
                  continue;
            QString ip = item->path;
            if (ip[0] != '/') {
                  // score path it relative to album path:
                  QFileInfo f(_path);
                  ip = f.path() + "/" + item->path;
                  }
            item->score = mscore->readScore(item->path);
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Album::save(Xml& xml)
      {
      xml.stag("Album");
      xml.tag("name", _name);
      foreach(AlbumItem* item, _scores) {
            xml.stag("Score");
            xml.tag("name", item->name);
            xml.tag("path", item->path);
            xml.etag();
            }
      xml.etag();
      _dirty = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Album::write()
      {
      if (_path.isEmpty()) {
            QString home = preferences.myScoresPath;
            QString fn = mscore->getSaveScoreName(
               QWidget::tr("MuseScore: Save Album"),
               _name,
               QWidget::tr("MuseScore Files (*.album)")
               );
            if (fn.isEmpty()) {
                  _dirty = false;
                  return;
                  }
            _path = fn;
            }
      if (QFileInfo(_path).suffix().isEmpty())
            _path += ".album";
      QFile f(_path);
      if (!f.open(QIODevice::WriteOnly)) {
            QString s = QWidget::tr("Open Album File\n%1\nfailed: ")
               + QString(strerror(errno));
            QMessageBox::critical(mscore, QWidget::tr("MuseScore: Open Album File"), s.arg(_path));
            return;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      save(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QWidget::tr("Write Album failed: ") + f.errorString();
            QMessageBox::critical(0, QWidget::tr("MuseScore: Write Album"), s);
            }
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void Album::append(AlbumItem* item)
      {
      _scores.append(item);
      _dirty = true;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Album::remove(int idx)
      {
      _scores.removeAt(idx);
      _dirty = true;
      }

//---------------------------------------------------------
//   swap
//---------------------------------------------------------

void Album::swap(int a, int b)
      {
      _scores.swap(a, b);
      _dirty = true;
      }

//---------------------------------------------------------
//   setName
//---------------------------------------------------------

void Album::setName(const QString& s)
      {
      if (_name != s) {
            _name = s;
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void Album::setPath(const QString& s)
      {
      if (_path != s) {
            _path = s;
            _dirty = true;
            }
      }

//---------------------------------------------------------
//   AlbumManager
//---------------------------------------------------------

AlbumManager::AlbumManager(QWidget* parent)
   : QDialog(parent)
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
      QString home = preferences.myScoresPath;
      QStringList files = mscore->getOpenScoreNames(
         home,
         tr("MuseScore Files (*.mscz *.mscx)"),
         tr("MuseScore: Add Score")
         );
      if (files.isEmpty())
            return;
      foreach(QString fn, files) {
            if (fn.isEmpty())
                  continue;
            if(fn.endsWith (".mscz") || fn.endsWith (".mscx")) {
                  AlbumItem* item = new AlbumItem;
                  item->path = fn;
                  album->append(item);
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
      QString home = preferences.myScoresPath;
      QStringList files = mscore->getOpenScoreNames(
         home,
         tr("MuseScore Album Files (*.album)"),
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
      if (album)
            album->createScore();
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
      if (album && album->dirty())
            album->write();
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
            album->write();
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
                  album->write();
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

