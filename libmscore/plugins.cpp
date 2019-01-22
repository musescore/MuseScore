//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2009-2012 Werner Schweer and others
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

#include "config.h"
#include "plugins.h"

#ifdef SCRIPT_INTERFACE
#include "libmscore/score.h"
#include "libmscore/undo.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/utils.h"
#include "libmscore/mscore.h"
#include "libmscore/measurebase.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/rest.h"
#include "libmscore/stafftext.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/timesig.h"
#include "libmscore/keysig.h"
#include "libmscore/harmony.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/notedot.h"
#include "libmscore/figuredbass.h"
#include "libmscore/accidental.h"
#include "libmscore/lyrics.h"
#include "libmscore/layoutbreak.h"

namespace Ms {

//---------------------------------------------------------
//   MsScoreView
//---------------------------------------------------------

MsScoreView::MsScoreView(QQuickItem* parent)
   : QQuickPaintedItem(parent)
      {
      setAcceptedMouseButtons(Qt::LeftButton);
      score = 0;
      }

//---------------------------------------------------------
//   FileIO
//---------------------------------------------------------

FileIO::FileIO(QObject *parent) :
    QObject(parent)
      {
      }

QString FileIO::read()
      {
      if (mSource.isEmpty()) {
            emit error("source is empty");
            return QString();
            }
      QUrl url(mSource);
      QString source(mSource);
      if(url.isValid() && url.isLocalFile()) {
            source = url.toLocalFile();
            }
      QFile file(source);
      QString fileContent;
      if ( file.open(QIODevice::ReadOnly) ) {
            QString line;
            QTextStream t( &file );
            do {
                line = t.readLine();
                fileContent += line + "\n";
                } while (!line.isNull());
            file.close();
            }
      else {
          emit error("Unable to open the file");
          return QString();
          }
      return fileContent;
      }

bool FileIO::write(const QString& data)
      {
      if (mSource.isEmpty())
            return false;

      QFile file(mSource);
      if (!file.open(QFile::WriteOnly | QFile::Truncate))
            return false;

      QTextStream out(&file);
      out << data;
      file.close();
      return true;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

bool FileIO::remove()
      {
      if (mSource.isEmpty())
            return false;

      QFile file(mSource);
      return file.remove();
      }

bool FileIO::exists()
      {
      QFile file(mSource);
      return file.exists();
      }

int FileIO::modifiedTime()
      {
      if (mSource.isEmpty()) {
            emit error("source is empty");
            return 0;
            }
      QUrl url(mSource);
      QString source(mSource);
      if(url.isValid() && url.isLocalFile()) {
            source = url.toLocalFile();
            }
      QFileInfo fileInfo(source);
      return fileInfo.lastModified().toTime_t();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void MsScoreView::setScore(Score* s)
      {
      _currentPage = 0;
      score = s;

      if (score) {
            score->doLayout();

            Page* page = score->pages()[_currentPage];
            QRectF pr(page->abbox());
            qreal m1 = width()  / pr.width();
            qreal m2 = height() / pr.height();
            mag = qMax(m1, m2);

            _boundingRect = QRectF(0.0, 0.0, pr.width() * mag, pr.height() * mag);

            setWidth(pr.width() * mag);
            setHeight(pr.height() * mag);
            }
      update();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void MsScoreView::paint(QPainter* p)
      {
      p->setRenderHint(QPainter::Antialiasing, true);
      p->setRenderHint(QPainter::TextAntialiasing, true);
      p->fillRect(QRect(0, 0, width(), height()), _color);
      if (!score)
            return;
      p->scale(mag, mag);

      Page* page = score->pages()[_currentPage];
      QList<const Element*> el;
      for (System* s : page->systems()) {
            for (MeasureBase* m : s->measures())
                  m->scanElements(&el, collectElements, false);
            }
      page->scanElements(&el, collectElements, false);

      foreach(const Element* e, el) {
            QPointF pos(e->pagePos());
            p->translate(pos);
            e->draw(p);
            p->translate(-pos);
            }
      }

//---------------------------------------------------------
//   setCurrentPage
//---------------------------------------------------------

void MsScoreView::setCurrentPage(int n)
      {
      if (score == 0)
            return;
      if (n < 0)
            n = 0;
      int nn = score->pages().size();
      if (nn == 0)
            return;
      if (n >= nn)
            n = nn - 1;
      _currentPage = n;
      update();
      }

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void MsScoreView::nextPage()
      {
      setCurrentPage(_currentPage + 1);
      }

//---------------------------------------------------------
//   prevPage
//---------------------------------------------------------

void MsScoreView::prevPage()
      {
      setCurrentPage(_currentPage - 1);
      }

const QTransform& MsScoreView::matrix() const
      {
      static const QTransform t;
      return t; // _matrix;
      }

}
#endif
