//=============================================================================
//  MusE Reader
//  Music Score Reader
//
//  Copyright (C) 2010 Werner Schweer
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

#include "mscore/globals.h"
#include "omrview.h"
#include "omr.h"
#include "libmscore/page.h"
#include "omrpage.h"
#include "libmscore/score.h"
#include "mscore/scoreview.h"
#include "libmscore/sym.h"
#include "libmscore/mscore.h"

namespace Ms {

//---------------------------------------------------------
//   OmrView
//---------------------------------------------------------

OmrView::OmrView(ScoreView* sv, QWidget* parent)
   : QWidget(parent)
      {
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setMouseTracking(true);

      _omr       = 0;
      _scoreView = sv;
      double m   = .25;
      _fotoMode  = false;
      _matrix    = QTransform(m, 0.0, 0.0, m, 0.0, 0.0);
      xoff = yoff = 0;
      _showLines      = false;
      _showBarlines   = true;
      _showSlices     = true;
      _showStaves     = true;
      }

//---------------------------------------------------------
//   setOmr
//---------------------------------------------------------

void OmrView::setOmr(Omr* s)
      {
      delete _omr;
      _omr    = s;
      if (s == 0 || s->numPages() == 0) {
            maxTiles = 0;
            return;
            }
      int n                = s->numPages();
      OmrPage* page        = _omr->page(0);
      const QImage& i      = page->image();
      Score* score         = _scoreView->score();
      double mag           = _omr->spatium() / score->spatium();
      pageWidth            = lrint(score->styleD(Sid::pageWidth) * mag * DPI);

      int htiles = ((pageWidth + TILE_W - 1) / TILE_W);
      pageWidth  = htiles * TILE_W;
      int vtiles = (i.height() + TILE_H - 1) / TILE_H;
      maxTiles   = n * htiles *  vtiles;
      }

//---------------------------------------------------------
//   initTile
//---------------------------------------------------------

void OmrView::initTile(Tile* t)
      {
      int page1  = t->r.x() / pageWidth;
      if (page1 < 0)
            page1 = 0;
      int n = _omr->numPages();
      if (page1 >= n)
            return;

      t->page     = _omr->page(page1);
      t->pageNo   = page1;

      const QImage& i = t->page->image();
      int xoffset     = 0; // (pageWidth - i.width()) / 2;
      int x           = t->r.x() - (t->pageNo * pageWidth) - xoffset;
      t->pm           = QPixmap::fromImage(i.copy(x, t->r.y(), TILE_W, TILE_H));
      }

//---------------------------------------------------------
//   Tile
//---------------------------------------------------------

Tile::Tile()
   : no(0), r(0, 0, TILE_W, TILE_H), pm(TILE_W, TILE_H)
      {
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void OmrView::paintEvent(QPaintEvent* event)
      {
      if (_omr == 0)
            return;

      QPainter p(this);
      p.setTransform(_matrix);

      QRect r(event->rect());

      //
      // remove unused tiles
      //
      QRect rr = _matrix.inverted().mapRect(QRectF(r)).toRect();
      rr.adjust(-1, -1, 2, 2);

      QList<Tile*> nl;
      for(Tile* t : usedTiles) {
            if (t->r.intersects(rr))
                  nl.append(t);
            else
                  freeTiles.append(t);
            }
#if QT_VERSION >= 0x040800
      usedTiles.swap(nl);
#else
      QList<Tile*> tmp = nl;
      nl = usedTiles;
      usedTiles = tmp;
#endif

      //
      // add visible tiles
      //
      Score* score    = _scoreView->score();

      double sSpatium = score->spatium();
      double spatium  = _omr->spatium();
      double mag      = spatium / sSpatium;

      int w = pageWidth;
      int h = lrint(score->styleD(Sid::pageHeight) * mag * DPI);
      int n = _omr->numPages();

      int nx = (w * n) / TILE_W;
      int ny = (h + TILE_H - 1) / TILE_H;

      int y1 = rr.y() / TILE_H;
      int y2 = (rr.y() + rr.height() + TILE_H - 1) / TILE_H;
      int x1 = rr.x() / TILE_W;
      int x2 = (rr.x() + rr.width() + TILE_W -1) / TILE_W;

      if (x1 < 0)
            x1 = 0;
      if (y1 < 0)
            y1 = 0;
      if (x2 > nx)
            x2 = nx;
      if (y2 > ny)
            y2 = ny;

      for (int y = y1; y < y2; ++y) {
            for (int x = x1; x < x2; ++x) {
                  int no = nx * y + x;
                  if (no < 0 || no >= maxTiles)
                        continue;
                  int i;
                  for (i = 0; i < usedTiles.size(); ++i) {
                        if (usedTiles[i]->no == no)
                              break;
                        }
                  if (i == usedTiles.size()) {
                        // create new tile
                        Tile* t = freeTiles.isEmpty() ? new Tile : freeTiles.pop();
                        t->no = no;
                        t->r  = QRect(x * TILE_W, y * TILE_H, TILE_W, TILE_H);
                        initTile(t);
                        usedTiles.append(t);
                        }
                  }
            }

      int minPage = 9000;
      int maxPage = 0;
      for(const Tile* t : usedTiles) {
            p.drawPixmap(t->r, t->pm);
            if (t->pageNo < minPage)
                  minPage = t->pageNo;
            if (t->pageNo > maxPage)
                  maxPage = t->pageNo;
            }
      for (int pageNo = minPage; pageNo <= maxPage; ++pageNo) {
            OmrPage* page = _omr->page(pageNo);
            p.save();
            p.translate(w * pageNo, 0);
            if (_showLines) {
                  p.setPen(QPen(QColor(255, 0, 0, 80), 1.0));
                  for(QLine l : page->sl())
                        p.drawLine(QLineF(l.x1()+.5, l.y1()+.5, l.x2()+.5, l.y2()+.5));
                  }
            if (_showSlices) {
                  for(const QRect r1 : page->slices())
                        p.fillRect(r1, QBrush(QColor(0, 100, 100, 50)));
                  }

            if (_showStaves) {
                  for(const OmrSystem& s : page->systems()) {       // staves
                        for(const OmrStaff& r1 : s.staves())
                              p.fillRect(r1, QBrush(QColor(0, 0, 100, 50)));
                        }
                  }

            for (const OmrSystem& system : page->systems()) {
                  if (_showBarlines) {
                        p.setPen(QPen(Qt::blue, 3.0));
                        for(const QLineF& l : system.barLines)
                            for(int w1 = 0; w1 < 10; w1++)
                              p.drawLine(l.x1()+w1, l.y1(), l.x2()+w1, l.y2() ); //add width to barline
                        }

                  for (const OmrStaff& staff : system.staves()) {
                        for (const OmrNote* n1 : staff.notes()) {
                            if (n1->sym == SymId::noteheadBlack)
                                    p.setPen(QPen(QColor(255, 0, 0), 2.0));
                              else
                                    p.setPen(QPen(QColor(0, 0, 255), 2.0));
                              p.drawRect(*n1);
                              }
                        }
                  }
            p.restore();
            }

      if (fotoMode()) {
            // TODO
            p.setBrush(QColor(0, 0, 50, 50));
            QPen pen(QColor(0, 0, 255));
            // always 2 pixel width
            qreal w1 = 2.0 / p.matrix().m11();
            pen.setWidthF(w1);
            p.setPen(pen);
            p.drawRect(_foto);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void OmrView::mousePressEvent(QMouseEvent* e)
      {
      startDrag = e->pos();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void OmrView::mouseMoveEvent(QMouseEvent* e)
      {
      if (QApplication::mouseButtons()) {
            QPoint delta = e->pos() - startDrag;
            int dx       = delta.x();
            int dy       = delta.y();
            xoff += dx;
            yoff += dy;
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
               _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());

            scroll(dx, dy, QRect(0, 0, width(), height()));
            startDrag = e->pos();
            }
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void OmrView::setMag(double nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m13(), _matrix.m21(),
         nmag, _matrix.m23(), _matrix.dx()*deltamag, _matrix.dy()*deltamag, _matrix.m33());
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void OmrView::zoom(int step, const QPoint& pos)
      {
      QTransform imatrix(_matrix.inverted());
      QPointF p1 = imatrix.map(QPointF(pos));
      double _scale = mag();
      if (step > 0) {
            for (int i = 0; i < step; ++i)
                   _scale *= 1.1;
            }
      else {
            for (int i = 0; i < -step; ++i)
                  _scale /= 1.1;
            }
      if (_scale > 16.0)
            _scale = 16.0;
      else if (_scale < 0.05)
            _scale = 0.05;
      setMag(_scale);

      QPointF p2 = imatrix.map(QPointF(pos));
      QPointF p3 = p2 - p1;
      int dx     = lrint(p3.x() * _scale);
      int dy     = lrint(p3.y() * _scale);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void OmrView::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            QApplication::sendPostedEvents(this, 0);
            zoom(event->delta() / 120, event->pos());
            return;
            }
      int dx = 0;
      int dy = 0;
      if (event->modifiers() & Qt::ShiftModifier || event->orientation() == Qt::Horizontal) {
            //
            //    scroll horizontal
            //
            int n = width() / 10;
            if (n < 2)
                  n = 2;
            dx = event->delta() * n / 120;
            }
      else {
            //
            //    scroll vertical
            //
            int n = height() / 10;
            if (n < 2)
                  n = 2;
            dy = event->delta() * n / 120;
            }

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      scroll(dx, dy, QRect(0, 0, width(), height()));
      }

//---------------------------------------------------------
//   setScale
//---------------------------------------------------------

void OmrView::setScale(double v)
      {
      double spatium = _omr->spatium();
      setMag(v/spatium);
      update();
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void OmrView::setOffset(double x, double y)
      {
      Score* score    = _omr->score();
      double sSpatium = score->spatium() * _scoreView->matrix().m11();
      double spatium  = _omr->spatium() * _matrix.m11();

      double nx = x / sSpatium * spatium + xoff;
      double ny = y / sSpatium * spatium + yoff;

      double ox = _matrix.dx();
      double oy = _matrix.dy();

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), nx, ny, _matrix.m33());

      scroll(ox-nx, oy-ny, QRect(0, 0, width(), height()));
      update();
      }

//---------------------------------------------------------
//   contextMenuEvent
//---------------------------------------------------------

void OmrView::contextMenuEvent(QContextMenuEvent*)
      {
//printf("context menu\n");
      }

//---------------------------------------------------------
//   setShowBarlines
//---------------------------------------------------------

void OmrView::setShowBarlines(bool val)
      {
      _showBarlines = val;
      update();
      }

//---------------------------------------------------------
//   setShowSlices
//---------------------------------------------------------

void OmrView::setShowSlices(bool val)
      {
      _showSlices = val;
      update();
      }

//---------------------------------------------------------
//   setShowStaves
//---------------------------------------------------------

void OmrView::setShowStaves(bool val)
      {
      _showStaves = val;
      update();
      }

}


