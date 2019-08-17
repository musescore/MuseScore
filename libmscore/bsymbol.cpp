//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "image.h"
#include "xml.h"
#include "staff.h"
#include "segment.h"
#include "page.h"
#include "system.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(Score* s, ElementFlags f)
   : Element(s, f)
      {
      _align = Align::LEFT | Align::BASELINE;
      }

BSymbol::BSymbol(const BSymbol& s)
   : Element(s)
      {
      _align = s._align;
      for (Element* e : s._leafs) {
            Element* ee = e->clone();
            ee->setParent(this);
            _leafs.append(ee);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void BSymbol::writeProperties(XmlWriter& xml) const
      {
      for (const Element* e : leafs())
            e->write(xml);
      Element::writeProperties(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool BSymbol::readProperties(XmlReader& e)
      {
      const QStringRef& tag = e.name();

      if (Element::readProperties(e))
            return true;
      else if (tag == "systemFlag")
            setSystemFlag(e.readInt());
      else if (tag == "Symbol" || tag == "FSymbol") {
            Element* element = name2Element(tag, score());
            element->read(e);
            add(element);
            }
      else if ( tag == "Image") {
            if (MScore::noImages)
                  e.skipCurrentElement();
            else {
                  Element* element = name2Element(tag, score());
                  element->read(e);
                  add(element);
                  }
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BSymbol::add(Element* e)
      {
      if (e->isSymbol() || e->isImage()) {
            e->setParent(this);
            _leafs.append(e);
            toBSymbol(e)->setZ(z() - 1);    // draw on top of parent
            }
      else
            qDebug("BSymbol::add: unsupported type %s", e->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BSymbol::remove(Element* e)
      {
      if (e->isSymbol() || e->isImage()) {
            if (!_leafs.removeOne(e))
                  qDebug("BSymbol::remove: element <%s> not found", e->name());
            }
      else
            qDebug("BSymbol::remove: unsupported type %s", e->name());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BSymbol::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
      foreach (Element* e, _leafs)
            e->scanElements(data, func, all);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BSymbol::acceptDrop(EditData& data) const
      {
      return data.dropElement->isSymbol() || data.dropElement->isImage();
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BSymbol::drop(EditData& data)
      {
      Element* el = data.dropElement;
      if (el->isSymbol() || el->isImage()) {
            el->setParent(this);
            QPointF p = data.pos - pagePos() - data.dragOffset;
            el->setOffset(p);
            score()->undoAddElement(el);
            return el;
            }
      else
            delete el;
      return 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BSymbol::layout()
      {
      if (staff())
            setMag(staff()->mag(tick()));
      if (!parent()) {
            setOffset(.0, .0);
            setPos(.0, .0);
            }
      for (Element* e : _leafs)
            e->layout();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF BSymbol::drag(EditData& ed)
      {
      QRectF r(canvasBoundingRect());
      foreach(const Element* e, _leafs)
            r |= e->canvasBoundingRect();

      qreal x = ed.delta.x();
      qreal y = ed.delta.y();

      qreal _spatium = spatium();
      if (ed.hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (ed.vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }

      setOffset(QPointF(x, y));

      r |= canvasBoundingRect();
      foreach(const Element* e, _leafs)
            r |= e->canvasBoundingRect();
      return r;
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF BSymbol::dragAnchor() const
      {
      if (parent() && parent()->type() == ElementType::SEGMENT) {
            System* system = segment()->measure()->system();
            qreal y        = system->staffCanvasYpage(staffIdx());
            QPointF anchor(segment()->canvasPos().x(), y);
            return QLineF(canvasPos(), anchor);
            }
      else {
            return QLineF(canvasPos(), parent()->canvasPos());
            }
      }

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

QPointF BSymbol::pagePos() const
      {
      if (parent() && (parent()->type() == ElementType::SEGMENT)) {
            QPointF p(pos());
            System* system = segment()->measure()->system();
            if (system) {
                  p.ry() += system->staff(staffIdx())->y() + system->y();
                  }
            p.rx() = pageX();
            return p;
            }
      else
            return Element::pagePos();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF BSymbol::canvasPos() const
      {
      if (parent() && (parent()->type() == ElementType::SEGMENT)) {
            QPointF p(pos());
            Segment* s = toSegment(parent());

            System* system = s->measure()->system();
            if (system) {
                  int si = staffIdx();
                  p.ry() += system->staff(si)->y() + system->y();
                  Page* page = system->page();
                  if (page)
                        p.ry() += page->y();
                  }
            p.rx() = canvasX();
            return p;
            }
      else
            return Element::canvasPos();
      }


}

