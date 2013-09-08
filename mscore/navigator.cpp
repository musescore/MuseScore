//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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

namespace Ms {

//---------------------------------------------------------
//   showNavigator
//---------------------------------------------------------

void MuseScore::showNavigator(bool visible)
      {
      Navigator* n = static_cast<Navigator*>(_navigator->widget());
      if (n == 0 && visible) {
            n = new Navigator(_navigator, this);
            n->setScoreView(cv);
            }
      _navigator->setVisible(visible);
      getAction("toggle-navigator")->setChecked(visible);
      }

//---------------------------------------------------------
//   NScrollArea
//---------------------------------------------------------

NScrollArea::NScrollArea(QWidget* w)
   : QScrollArea(w)
      {
      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setMinimumHeight(40);
      setMaximumHeight(140);
      setLineWidth(0);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void NScrollArea::resizeEvent(QResizeEvent* ev)
      {
      if (widget() && (ev->size().height() != ev->oldSize().height()))
            widget()->resize(widget()->width(), ev->size().height());
      QScrollArea::resizeEvent(ev);
      }

//---------------------------------------------------------
//   ViewRect
//---------------------------------------------------------

ViewRect::ViewRect(QWidget* w)
   : QWidget(w)
      {
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ViewRect::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      QColor c(MScore::selectColor[0]);
      QPen pen(c, 2.0);
      p.setPen(pen);
      p.setBrush(QColor(c.red(), c.green(), c.blue(), 40));
      p.drawRect(ev->rect());
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
      scrollArea->setWidgetResizable(true);
      _cv            = 0;
      viewRect       = new ViewRect(this);
      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
      sa->setWidget(this);
      sa->setWidgetResizable(false);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Navigator::resizeEvent(QResizeEvent* /*ev*/)
      {
      if (_score) {
            rescale();
            updateViewRect();
            }
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
            rescale();
            updateViewRect();
            }
      else {
            _score = 0;
            update();
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Navigator::setScore(Score* v)
      {
      _cv    = 0;
      _score = v;
      rescale();
      updateViewRect();
      update();
      }

//---------------------------------------------------------
//   rescale
//    recompute scale of score view
//---------------------------------------------------------

void Navigator::rescale()
      {
      if (!_score || _score->pages().isEmpty()) {
            setFixedWidth(0);
            return;
            }
      Page* lp          = _score->pages().back();
      qreal scoreWidth  = lp->x() + lp->width();
      qreal scoreHeight = lp->height();

      qreal m  = height() / scoreHeight;

      setFixedWidth(int(scoreWidth * m));
      matrix = QTransform(m, 0, 0, m, 0, 0);
      }

//---------------------------------------------------------
//   updateViewRect
//---------------------------------------------------------

void Navigator::updateViewRect()
      {
      QRect r;
      if (_cv)
            r = _cv->toLogical(QRect(0.0, 0.0, _cv->width(), _cv->height())).toRect();
      setViewRect(r);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Navigator::mousePressEvent(QMouseEvent* ev)
      {
      if (_cv == 0)
            return;
      startMove = ev->pos();
      if (!viewRect->geometry().contains(startMove)) {
            QPointF p = matrix.inverted().map(QPointF(ev->pos()));
            QRectF r(_cv->toLogical(QRectF(0.0, 0.0, _cv->width(), _cv->height())));
            double dx = p.x() - (r.x() + (r.width() * .5));
            double dy = p.y() - (r.y() + (r.height() * .5));
            r.translate(dx, dy);
            setViewRect(r);
            emit viewRectMoved(matrix.inverted().mapRect(viewRect->geometry()));
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Navigator::mouseMoveEvent(QMouseEvent* ev)
      {
      QPoint delta = ev->pos() - startMove;
      QRect r(viewRect->geometry().translated(delta));
      startMove = ev->pos();

      if (r.width() == width())
            r.moveLeft(0);
      else if (r.width() < width()) {
            if (r.x() < 0)
                  r.moveLeft(0);
            else if (r.right() > width())
                  r.moveRight(width());
            }
      else {
            if (r.right() < width())
                  r.moveRight(width());
            else if (r.left() > 0)
                  r.moveLeft(0);
            }

      if (r.height() == height())
            r.moveTop(0);
      else if (r.height() < height()) {
            if (r.y() < 0)
                  r.moveTop(0);
            else if (r.bottom() > height())
                  r.moveBottom(height());
            }
      else {
            if (r.bottom() < height())
                  r.moveBottom(height());
            else if (r.top() > 0)
                  r.moveTop(0);
            }

      viewRect->setGeometry(r);

      emit viewRectMoved(matrix.inverted().mapRect(r));
      int x = delta.x() > 0 ? r.x() + r.width() : r.x();
      scrollArea->ensureVisible(x, height()/2, 0, 0);
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Navigator::setViewRect(const QRectF& _viewRect)
      {
      viewRect->setGeometry(matrix.mapRect(_viewRect).toRect());
      scrollArea->ensureVisible(viewRect->x(), 0);
      }

//---------------------------------------------------------
//   paintElement
//---------------------------------------------------------

static void paintElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      QPointF pos(e->pagePos());
      p->translate(pos);
      e->draw(p);
      p->translate(-pos);
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void Navigator::layoutChanged()
      {
      if (_score && !_score->pages().isEmpty())
            rescale();
      update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Navigator::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      QRect r(ev->rect());
      p.fillRect(r, Qt::gray);

//      printf("navigator paint %d %d\n", r.width(), r.height());

      if (!_score)
            return;

      p.setTransform(matrix);
      QRectF fr = matrix.inverted().mapRect(QRectF(r));

      foreach (Page* page, _score->pages()) {
            QPointF pos(page->pos());
            QRectF pr(page->abbox().translated(pos));
            if (pr.right() < fr.left())
                  continue;
            if (pr.left() > fr.right())
                  break;

            p.fillRect(pr, Qt::white);
            p.translate(pos);
            foreach(System* s, *page->systems()) {
                  foreach(MeasureBase* m, s->measures())
                        m->scanElements(&p, paintElement, false);
                  }
            page->scanElements(&p, paintElement, false);
            if (page->score()->layoutMode() == LayoutPage) {
                  p.setFont(QFont("FreeSans", 400));  // !!
                  p.setPen(MScore::layoutBreakColor);
                  p.drawText(page->bbox(), Qt::AlignCenter, QString("%1").arg(page->no()+1));
                  }
            p.translate(-pos);
            }
      }
}

