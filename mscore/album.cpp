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
#include "libmscore/excerpt.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"

namespace Ms {

//---------------------------------------------------------
//   ~AlbumItem
//---------------------------------------------------------

AlbumItem::~AlbumItem()
      {
      delete score;
      }

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

Album::Album()
      {
      _dirty = false;
      }

Album::~Album()
      {
      qDeleteAll(_scores);
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

      printer.setCreator("MuseScore Version: " MSC_VERSION);
      printer.setFullPage(true);
      printer.setColorMode(QPrinter::Color);
      printer.setOutputFormat(QPrinter::NativeFormat);

      QPrintDialog pd(&printer, 0);
      if (!pd.exec())
            return;

      QPainter painter(&printer);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      double mag = printer.logicalDpiX() / DPI;
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

      for (AlbumItem* item : _scores) {
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
                  for (const Element* e : ell) {
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

bool Album::createScore(const QString& fn, bool addPageBreak, bool addSectionBreak)
      {
      loadScores();

      MasterScore* firstScore = _scores[0]->score->masterScore();
      if (!firstScore) {
            qDebug("First score is NULL. Will not attempt to join scores.");
            return false;
            }

      // do layout for first score's root and excerpt scores
      firstScore->doLayout();
      for (int i = 0; i < firstScore->excerpts().count(); i++) {
            if (firstScore->excerpts().at(i)->partScore()) {
                  firstScore->excerpts().at(i)->partScore()->doLayout();
                  }
            else {
                  qDebug("First score has excerpts, but excerpt %d is NULL.  Will not attempt to join scores.", i);
                  return false;
                  }
            }

      MasterScore* score = firstScore->clone();

      int excerptCount = firstScore->excerpts().count();
      bool joinExcerpt = true;
	for (AlbumItem* item : _scores) {
            if (item->score == 0 || item->score == firstScore)
                  continue;
            if (item->score->excerpts().count() != excerptCount) {
                  joinExcerpt = false;
                  qDebug("Will not join parts. Album item \"%s\".  Mismatch between number of excerpts with first album item \"%s\"", qPrintable(item->name), qPrintable(_scores[0]->name));
                  break;
                  }
            }
      if (!joinExcerpt) {
            for (Excerpt* ex : score->excerpts())
                  score->removeExcerpt(ex);
            }

      for (AlbumItem* item : _scores) {

            if (item->score == 0 || item->score == firstScore)
                  continue;

            // try to append root score
            item->score->doLayout();
            if (!score->appendScore(item->score, addPageBreak, addSectionBreak)) {
                  qDebug("Cannot append root score of album item \"%s\".", qPrintable(item->name));
                  delete score;
                  return false;
                  }

            // try to append each excerpt
            if (joinExcerpt) {
                  for (int i = 0; i < score->excerpts().count(); i++) {
                        Score* currentScoreExcerpt = item->score->excerpts().at(i)->partScore();
                        if (currentScoreExcerpt) {
                              currentScoreExcerpt->doLayout();
                              if (!score->excerpts().at(i)->partScore()->appendScore(currentScoreExcerpt, addPageBreak, addSectionBreak)) {
                                    qDebug("Cannot append excerpt %d of album item \"%s\".", i, qPrintable(item->name));
                                    delete score;
                                    return false;
                                    }
                              }
                        else {
                              qDebug("First score has excerpts, but excerpt %d of album item \"%s\" is NULL.  Will not attempt to join scores.",
                                          i, qPrintable(item->name));
                              delete score;
                              return false;
                              }
                        }
                  }
            }

      score->masterScore()->fileInfo()->setFile(fn);
      qDebug("Album::createScore: save file");
      try {
            QString suffix  = score->masterScore()->fileInfo()->suffix().toLower();
            if (suffix == "mscz")
                  score->saveCompressedFile(*score->masterScore()->fileInfo(), false);
            else if (suffix == "mscx")
                  score->Score::saveFile(*score->masterScore()->fileInfo());
            }
      catch (QString s) {
            delete score;
            return false;
            }
      delete score;
      return true;
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
      for (AlbumItem* item : _scores) {
            if (item->score || item->path.isEmpty())
                  continue;
            QString ip = item->path;
            if (ip[0] != '/') {
                  // score path is relative to album path:
                  QFileInfo f(_path);
                  ip = f.path() + "/" + item->path;
                  }
            MasterScore* score = new MasterScore(MScore::baseStyle());  // start with built-in style
            score->loadMsc(item->path, false);
            item->score = score;
            }
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Album::save(Xml& xml)
      {
      xml.stag("Album");
      xml.tag("name", _name);
      for (AlbumItem* item : _scores) {
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

void Album::write(Xml& xml)
      {
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");
      save(xml);
      xml.etag();
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
      delete _scores.takeAt(idx);
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
}

