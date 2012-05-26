//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigator.cpp 5630 2012-05-15 15:07:54Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "navigator.h"
#include "musescore.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"

//---------------------------------------------------------
//   showNavigator
//---------------------------------------------------------

void MuseScore::showNavigator(bool visible)
      {
      Navigator* n = static_cast<Navigator*>(_navigator->widget());
      if (n == 0 && visible) {
            n = new Navigator(_navigator, this);
            n->setScoreView(cv);
            n->updateViewRect();
            }
      _navigator->setShown(visible);
      getAction("toggle-navigator")->setChecked(visible);
      }

//---------------------------------------------------------
//   NScrollArea
//---------------------------------------------------------

NScrollArea::NScrollArea(QWidget* w)
   : QScrollArea(w)
      {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setMinimumHeight(40);
      setLineWidth(0);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void NScrollArea::resizeEvent(QResizeEvent* ev)
      {
      if (widget() && (ev->size().height() != ev->oldSize().height())) {
            widget()->resize(widget()->width(), ev->size().height());
            }
      QScrollArea::resizeEvent(ev);
      }

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

Navigator::Navigator(NScrollArea* sa, QWidget* parent)
  : QWidget(parent)
      {
      setAttribute(Qt::WA_NoBackground);
      _score         = 0;
      scrollArea     = sa;
      _cv            = 0;
      recreatePixmap = false;
      viewRect       = QRect();
      cachedWidth    = -1;
      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
      sa->setWidget(this);
      sa->setWidgetResizable(false);
      connect(&watcher, SIGNAL(finished()), SLOT(pmFinished()));
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Navigator::resizeEvent(QResizeEvent* ev)
      {
      if (ev->size().height() == ev->oldSize().height())
            return;
      if (_score) {
            rescale();
            Page* lp = _score->pages().back();
            int w    = int ((lp->x() + lp->width()) * matrix.m11());

            if (w != cachedWidth) {
                  cachedWidth = w;
                  updateViewRect();
                  layoutChanged();
                  }

            }
      else
            recreatePixmap = true;
      }

//---------------------------------------------------------
//   setScoreView
//---------------------------------------------------------

void Navigator::setScoreView(ScoreView* v)
      {
      if (_cv) {
            disconnect(this, SIGNAL(viewRectMoved(const QRectF&)), _cv, SLOT(setViewRect(const QRectF&)));
            disconnect(_cv, SIGNAL(viewRectChanged()), this, SLOT(updateViewRect()));
            }
      _cv = QPointer<ScoreView>(v);
      if (v) {
            _score  = v->score();
            rescale();
            connect(this, SIGNAL(viewRectMoved(const QRectF&)), v, SLOT(setViewRect(const QRectF&)));
            connect(_cv,  SIGNAL(viewRectChanged()), this, SLOT(updateViewRect()));
            updateViewRect();
            layoutChanged();
            rescale();
            }
      else {
            _score = 0;
            pcl.clear();
            update();
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Navigator::setScore(Score* v)
      {
      _cv = 0;
      if (v) {
            _score  = v;
            rescale();
            setViewRect(QRect());
            }
      else {
            _score = 0;
            pcl.clear();
            update();
            }
      }

//---------------------------------------------------------
//   rescale
//---------------------------------------------------------

void Navigator::rescale()
      {
      if (_score->pages().isEmpty())
            return;
      int h       = height();
      Page* lp    = _score->pages().back();
      qreal scoreWidth  = lp->x() + lp->width();
      qreal scoreHeight = lp->height();

      qreal m;
      qreal m1    = h / scoreHeight;
      int w1      = int (scoreWidth * m1);
      
      setFixedWidth(w1);
      m = m1;
      
      matrix.setMatrix(m, matrix.m12(), matrix.m13(), matrix.m21(), m,
         matrix.m23(), matrix.m31(), matrix.m32(), matrix.m33());
      int n = pcl.size();
      for (int i = 0; i < n; ++i)
            pcl[i].matrix = matrix;
      }

//---------------------------------------------------------
//   updateViewRect
//---------------------------------------------------------

void Navigator::updateViewRect()
      {
      if (_score == 0) {
            setViewRect(QRect());
            return;
            }
      if (_cv) {
            QRectF r(0.0, 0.0, _cv->width(), _cv->height());
            setViewRect(_cv->toLogical(r));
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Navigator::mousePressEvent(QMouseEvent* ev)
      {
      if (_cv == 0)
            return;
      startMove = ev->pos();
      if (!viewRect.contains(startMove)) {
            QPointF p = matrix.inverted().map(QPointF(ev->pos()));
            QRectF r(_cv->toLogical(QRectF(0.0, 0.0, _cv->width(), _cv->height())));
            double dx = p.x() - (r.x() + (r.width() * .5));
            double dy = p.y() - (r.y() + (r.height() * .5));
            r.translate(dx, dy);
            setViewRect(r);
            emit viewRectMoved(matrix.inverted().mapRect(viewRect));
            update();
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Navigator::mouseMoveEvent(QMouseEvent* ev)
      {
      QPoint delta = ev->pos() - startMove;
      viewRect.translate(delta);
      startMove = ev->pos();

/*      if (viewRect.x() <= 0 && viewRect.width() < width())
            viewRect.moveLeft(0);
      else if (viewRect.right() > width() && viewRect.width() < width())
            viewRect.moveRight(width());
  */

      if (viewRect.width() == width())
            viewRect.moveLeft(0);
      else if (viewRect.width() < width()) {
            if (viewRect.x() < 0)
                  viewRect.moveLeft(0);
            else if (viewRect.right() > width())
                  viewRect.moveRight(width());
            }
      else {
            if (viewRect.right() < width())
                  viewRect.moveRight(width());
            else if (viewRect.left() > 0)
                  viewRect.moveLeft(0);
            }

      if (viewRect.height() == height())
            viewRect.moveTop(0);
      else if (viewRect.height() < height()) {
            if (viewRect.y() < 0)
                  viewRect.moveTop(0);
            else if (viewRect.bottom() > height())
                  viewRect.moveBottom(height());
            }
      else {
            if (viewRect.bottom() < height())
                  viewRect.moveBottom(height());
            else if (viewRect.top() > 0)
                  viewRect.moveTop(0);
            }

      emit viewRectMoved(matrix.inverted().mapRect(viewRect));
      int x = delta.x() > 0 ? viewRect.x() + viewRect.width() : viewRect.x();
      scrollArea->ensureVisible(x, height()/2, 0, 0);
      update();
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Navigator::setViewRect(const QRectF& _viewRect)
      {
      viewRect = matrix.mapRect(_viewRect).toRect();
      scrollArea->ensureVisible(viewRect.x(), 0);
      update();
      }

//---------------------------------------------------------
//   paintElement
//---------------------------------------------------------

static void paintElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      p->save();
//      p->setPen(QPen(e->curColor()));
      p->translate(e->pagePos());
      e->draw(p);
      p->restore();
      }

//---------------------------------------------------------
//   createPixmap
//---------------------------------------------------------

static void createPixmap(PageCache* pc)
      {
      QReadLocker locker (pc->page->score()->layoutLock());
      pc->valid = false;
      QRect pageRect = pc->matrix.mapRect(pc->page->bbox()).toRect();
      if (pageRect.width() == 0 || pageRect.height() == 0) {
            return;
            }

      pc->pm = QImage(pageRect.size(), QImage::Format_ARGB32_Premultiplied);
      QPainter p(&pc->pm);

      QColor _fgColor(Qt::white);
      QColor _bgColor(Qt::darkGray);

      p.setRenderHint(QPainter::Antialiasing, false);
      p.setTransform(pc->matrix);
      p.fillRect(pc->page->bbox(), _fgColor);
      foreach(System* s, *pc->page->systems()) {
            foreach(MeasureBase* m, s->measures())
                  m->scanElements(&p, paintElement, false);
            }
      pc->page->scanElements(&p, paintElement, false);

      p.setFont(QFont("FreeSans", 400));  // !!
      p.setPen(QColor(0, 0, 255, 50));
      p.drawText(pc->page->bbox(), Qt::AlignCenter, QString("%1").arg(pc->page->no()+1));
      pc->navigator->update(pageRect);
      pc->valid = true;
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void Navigator::layoutChanged()
      {
      if (watcher.isRunning()) {
            updatePixmap.cancel();
            recreatePixmap = true;
            return;
            }
      if (_score == 0 || _score->pages().isEmpty()) {
            recreatePixmap = true;
            update();
            return;
            }
      int n = _score->pages().size();
//      if (n != pcl.size())
            rescale();
      pcl.clear();
      for (int i = 0; i < n; ++i) {
            PageCache pc;
            pc.page      = _score->pages()[i];
            pc.matrix    = matrix;
            pc.valid     = false;
            pc.navigator = this;
            pcl.append(pc);
            }
      update();
      }

//---------------------------------------------------------
//   pmFinished
//---------------------------------------------------------

void Navigator::pmFinished()
      {
      if (recreatePixmap) {
            recreatePixmap = false;
            layoutChanged();
            }
      else
            update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Navigator::paintEvent(QPaintEvent* ev)
      {
      if (watcher.isRunning())
            return;
      QPainter p(this);
      QRect r(ev->rect());

      // p.fillRect(r, Qt::gray);
      QRegion region(r);

      npcl.clear();
      if (matrix.m11() != .0) {
            for (int i = 0; i < pcl.size(); ++i) {
                  const PageCache& pc = pcl[i];
                  QRect rr = matrix.mapRect(pc.page->canvasBoundingRect()).toRect();
                  if (rr.intersects(r)) {
                        if (pc.valid) {
                              QPixmap pm = QPixmap::fromImage(pc.pm);
                              p.drawPixmap(rr.topLeft(), pm);
                              }
                        else {
                              npcl.append(&pcl[i]);
                              }
                        region -= rr;
                        }
                  }
            }
      foreach(QRect r, region.rects())
            p.fillRect(r, Qt::gray);

      if (_score && !recreatePixmap) {
            QPen pen(Qt::blue, 2.0);
            p.setPen(pen);
            p.setBrush(QColor(0, 0, 255, 40));
            p.drawRect(viewRect);
            }
      if (!npcl.isEmpty()) {
            updatePixmap = QtConcurrent::map(npcl, createPixmap);
            watcher.setFuture(updatePixmap);
            }
      }


