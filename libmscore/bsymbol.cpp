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

namespace Ms {

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(Score* s)
   : Element(s)
      {
      _z = SYMBOL * 100;
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      _systemFlag = false;
      }

BSymbol::BSymbol(const BSymbol& s)
   : Element(s), ElementLayout(s)
      {
      _z          = s._z;
      _systemFlag = s._systemFlag;
      foreach(Element* e, s._leafs) {
            Element* ee = e->clone();
            ee->setParent(this);
            _leafs.append(ee);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void BSymbol::writeProperties(Xml& xml) const
      {
      if (_systemFlag)
            xml.tag("systemFlag", _systemFlag);
      foreach(const Element* e, leafs())
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
            _systemFlag = e.readInt();
      else if (tag == "Symbol" || tag == "Image" || tag == "FSymbol") {
            Element* element = name2Element(tag, score());
            element->read(e);
            add(element);
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
      if (e->type() == SYMBOL || e->type() == IMAGE) {
            e->setParent(this);
            _leafs.append(e);
            static_cast<BSymbol*>(e)->setZ(z() - 1);    // draw on top of parent
            }
      else
            qDebug("BSymbol::add: unsupported type %s\n", e->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BSymbol::remove(Element* e)
      {
      if (e->type() == SYMBOL || e->type() == IMAGE) {
            if (!_leafs.removeOne(e))
                  qDebug("BSymbol::remove: element <%s> not found\n", e->name());
            }
      else
            qDebug("BSymbol::remove: unsupported type %s\n", e->name());
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

bool BSymbol::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      return type == SYMBOL || type == IMAGE;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BSymbol::drop(const DropData& data)
      {
      Element* el = data.element;
      if (el->type() == SYMBOL || el->type() == IMAGE) {
            el->setParent(this);
            QPointF p = data.pos - pagePos() - data.dragOffset;
            el->setUserOff(p);
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
      foreach(Element* e, _leafs)
            e->layout();
      adjustReadPos();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF BSymbol::drag(EditData* data)
      {
      QRectF r(canvasBoundingRect());
      foreach(const Element* e, _leafs)
            r |= e->canvasBoundingRect();

      qreal x = data->delta.x();
      qreal y = data->delta.y();

      qreal _spatium = spatium();
      if (data->hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (data->vRaster) {
            qreal vRaster = _spatium / MScore::vRaster();
            int n = lrint(y / vRaster);
            y = vRaster * n;
            }

      setUserOff(QPointF(x, y));

      r |= canvasBoundingRect();
      foreach(const Element* e, _leafs)
            r |= e->canvasBoundingRect();
      return r;
      }


}

