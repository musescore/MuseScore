//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "exampleview.h"
#include "preferences.h"
#include "libmscore/score.h"
#include "libmscore/element.h"
#include "libmscore/page.h"
#include "libmscore/icon.h"
#include "libmscore/chord.h"
#include "libmscore/xml.h"

namespace Ms {

//---------------------------------------------------------
//   ExampleView
//---------------------------------------------------------

ExampleView::ExampleView(QWidget* parent)
   : QFrame(parent)
      {
      _score = 0;
      setAcceptDrops(true);
      setFocusPolicy(Qt::StrongFocus);
      double mag = 0.9 * guiScaling * (DPI_DISPLAY / DPI);  // 90% of nominal
      qreal _spatium = SPATIUM20 * mag;
      // example would normally be 10sp from top of page; this leaves 3sp margin above
//      _matrix  = QTransform(mag, 0.0, 0.0, mag, _spatium, -_spatium * 7.0);
      _matrix  = QTransform(mag, 0.0, 0.0, mag, _spatium, -_spatium * 9.0);
      imatrix  = _matrix.inverted();
      }

void ExampleView::layoutChanged()
      {
      }

void ExampleView::dataChanged(const QRectF&)
      {
      }

void ExampleView::updateAll()
      {
      update();
      }

void ExampleView::adjustCanvasPosition(const Element* /*el*/, bool /*playBack*/)
      {
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ExampleView::setScore(Score* s)
      {
      delete _score;
      _score = s;
      _score->addViewer(this);
      _score->setLayoutMode(LayoutMode::LINE);
      _score->doLayout();
      update();
      }

void ExampleView::removeScore()
      {
      }

void ExampleView::changeEditElement(Element*)
      {
      }

QCursor ExampleView::cursor() const
      {
      return QCursor();
      }

void ExampleView::setCursor(const QCursor&)
      {
      }

int ExampleView::gripCount() const
      {
      return 0;
      }

const QRectF& ExampleView::getGrip(Grip) const
      {
      static QRectF r;
      return r;
      }

void ExampleView::setDropRectangle(const QRectF&)
      {
      }

void ExampleView::cmdAddSlur(Note* /*firstNote*/, Note* /*lastNote*/)
      {
      }

void ExampleView::startEdit()
      {
      }

void ExampleView::startEdit(Element*, Grip /*startGrip*/)
      {
      }

Element* ExampleView::elementNear(QPointF)
      {
      return 0;
      }

void ExampleView::drawBackground(QPainter*, const QRectF&) const
      {
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void ExampleView::drawElements(QPainter& painter, const QList<Element*>& el)
      {
      for (Element* e : el) {
            e->itemDiscovered = 0;
            QPointF pos(e->pagePos());
            painter.translate(pos);
            e->draw(&painter);
            painter.translate(-pos);
            }
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ExampleView::paintEvent(QPaintEvent* ev)
      {
      if (_score) {
            QPainter p(this);
            p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
            p.setRenderHint(QPainter::TextAntialiasing, true);
            QRect r(ev->rect());

            p.setTransform(_matrix);
            QRectF fr = imatrix.mapRect(QRectF(r));

            QRegion r1(r);
            Page* page = _score->pages().front();
            QList<Element*> ell = page->items(fr);
            qStableSort(ell.begin(), ell.end(), elementLessThan);
            drawElements(p, ell);
            }
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void ExampleView::dragEnterEvent(QDragEnterEvent* event)
      {
      const QMimeData* data = event->mimeData();
      if (data->hasFormat(mimeSymbolFormat)) {
            event->acceptProposedAction();

            QByteArray a = data->data(mimeSymbolFormat);

// qDebug("ExampleView::dragEnterEvent Symbol: <%s>", a.data());

            XmlReader e(a);
            QPointF dragOffset;
            Fraction duration;  // dummy
            Element::Type type = Element::readType(e, &dragOffset, &duration);

            dragElement = Element::create(type, _score);
            if (dragElement) {
                  dragElement->setParent(0);
                  dragElement->read(e);
                  dragElement->layout();
                  }
            return;
            }
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ExampleView::dragLeaveEvent(QDragLeaveEvent*)
      {
      if (dragElement) {
            delete dragElement;
            dragElement = 0;
            }
      setDropTarget(0);
      }

//---------------------------------------------------------
//   moveElement
//---------------------------------------------------------

static void moveElement(void* data, Element* e)
      {
      QPointF* pos = (QPointF*)data;
      e->score()->addRefresh(e->canvasBoundingRect());
      e->setPos(*pos);
      e->score()->addRefresh(e->canvasBoundingRect());
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void ExampleView::dragMoveEvent(QDragMoveEvent* event)
      {
      event->acceptProposedAction();

      if (!dragElement || dragElement->type() != Element::Type::ICON)
            return;

      QPointF pos(imatrix.map(QPointF(event->pos())));
      QList<Element*> el = elementsAt(pos);
      bool found = false;
      foreach(const Element* e, el) {
            if (e->type() == Element::Type::NOTE) {
                  setDropTarget(const_cast<Element*>(e));
                  found = true;
                  break;
                  }
            }
      if (!found)
            setDropTarget(0);
      dragElement->scanElements(&pos, moveElement, false);
      _score->update();
      return;
      }

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ExampleView::setDropTarget(const Element* el)
      {
      if (dropTarget != el) {
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  }
            }
      if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            dropAnchor = QLineF();
            }
      if (dropRectangle.isValid()) {
            dropRectangle = QRectF();
            }
      update();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void ExampleView::dropEvent(QDropEvent* event)
      {
      QPointF pos(imatrix.map(QPointF(event->pos())));

      if (!dragElement)
           return;
      if (dragElement->type() != Element::Type::ICON) {
            delete dragElement;
            dragElement = 0;
            return;
            }
      foreach (Element* e, elementsAt(pos)) {
            if (e->type() == Element::Type::NOTE) {
                  Icon* icon = static_cast<Icon*>(dragElement);
                  Chord* chord = static_cast<Note*>(e)->chord();
                  switch (icon->iconType()) {
                        case IconType::SBEAM:
                              chord->setBeamMode(Beam::Mode::BEGIN);
                              break;
                        case IconType::MBEAM:
                              chord->setBeamMode(Beam::Mode::AUTO);
                              break;
                        case IconType::BEAM32:
                              chord->setBeamMode(Beam::Mode::BEGIN32);
                              break;
                        case IconType::BEAM64:
                              chord->setBeamMode(Beam::Mode::BEGIN64);
                              break;
                        default:
                              break;
                        }
                  score()->doLayout();
                  break;
                  }
            }
      event->acceptProposedAction();
      delete dragElement;
      dragElement = 0;
      setDropTarget(0);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ExampleView::mousePressEvent(QMouseEvent* event)
      {
      QPointF pos(imatrix.map(QPointF(event->pos())));
      foreach (Element* e, elementsAt(pos)) {
            if (e->type() == Element::Type::NOTE) {
                  emit noteClicked(static_cast<Note*>(e));
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize ExampleView::sizeHint() const
      {
      qreal mag = 0.9 * guiScaling * (DPI_DISPLAY / DPI);
      qreal _spatium = SPATIUM20 * mag;
      // staff is 4sp tall with 3sp margin above; this leaves 3sp margin below
      return QSize(1000 * mag, _spatium * 10.0);
      }


}

