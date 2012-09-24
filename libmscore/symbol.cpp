//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: symbol.cpp 5313 2012-02-13 08:39:50Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "symbol.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "page.h"
#include "score.h"
#include "image.h"
#include "segment.h"
#include "mscore.h"

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(const BSymbol& s)
   : Element(s), ElementLayout(s)
      {
      _z = s._z;
      foreach(Element* e, s._leafs) {
            Element* ee = e->clone();
            ee->setParent(this);
            _leafs.append(ee);
            }
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

QRectF BSymbol::drag(const EditData& data)
      {
      QRectF r(canvasBoundingRect());
      foreach(const Element* e, _leafs)
            r |= e->canvasBoundingRect();

      qreal x = data.pos.x();
      qreal y = data.pos.y();

      qreal _spatium = spatium();
      if (data.hRaster) {
            qreal hRaster = _spatium / MScore::hRaster();
            int n = lrint(x / hRaster);
            x = hRaster * n;
            }
      if (data.vRaster) {
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

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(Score* s)
   : BSymbol(s)
      {
      _sym = sharpSym;        // arbitrary valid default
      setZ(SYMBOL * 100);
      }

Symbol::Symbol(Score* s, SymId sy)
   : BSymbol(s)
      {
      _sym = sy;
      setZ(SYMBOL * 100);
      }

Symbol::Symbol(const Symbol& s)
   : BSymbol(s)
      {
      _sym   = s._sym;
      setZ(SYMBOL * 100);
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Symbol::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Symbol::layout()
      {
//      qreal m = parent() ? parent()->mag() : 1.0;
//      if (_small)
//            m *= score()->styleD(ST_smallNoteMag);
//      setMag(m);
      foreach(Element* e, leafs())
            e->layout();
      ElementLayout::layout(this);
      BSymbol::layout();
      setbbox(symbols[score()->symIdx()][_sym].bbox(magS()));
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(QPainter* p) const
      {
      if (type() != NOTEDOT || !staff()->isTabStaff()) {
            p->setPen(curColor());
            symbols[score()->symIdx()][_sym].draw(p, magS());
            }
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("name", Sym::id2name(_sym));
      Element::writeProperties(xml);
      foreach(const Element* e, leafs())
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(const QDomElement& de)
      {
      QPointF pos;
      SymId s = noSym;

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "name") {
                  s = Sym::name2id(val);
                  if (s == noSym) {
                        // if symbol name not found, fall back to mnames
                        s = Sym::userName2id(val);
                        if (s == noSym) {
                              qDebug("unknown symbol <%s> (%d symbols), falling back to default symbol\n",
                                 qPrintable(val), symbols[0].size());
                              // set a default symbol, or layout() will crash
                              s = s1miHeadSym;
                              }
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  s->adjustReadPos();
                  add(s);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else
                        if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".gif")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        qDebug("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "small" || tag == "subtype")
                  ;
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (s == noSym)
            qDebug("unknown symbol\n");
      setPos(pos);
      setSym(s);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF BSymbol::dragAnchor() const
      {
      if (parent() && parent()->type() == SEGMENT) {
            System* system = segment()->measure()->system();
            qreal y        = system->staff(staffIdx())->y() + system->y();
            QPointF anchor(segment()->pageX(), y);
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
      if (parent() && (parent()->type() == SEGMENT)) {
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
      if (parent() && (parent()->type() == SEGMENT)) {
            QPointF p(pos());
            Segment* s = static_cast<Segment*>(parent());

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

//---------------------------------------------------------
//   FSymbol
//---------------------------------------------------------

FSymbol::FSymbol(Score* s)
  : Element(s)
      {
      _code = 0;
      _font.setStyleStrategy(QFont::NoFontMerging);
      }

FSymbol::FSymbol(const FSymbol& s)
  : Element(s)
      {
      _font = s._font;
      _code = s._code;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FSymbol::draw(QPainter* painter) const
      {
      QString s;
      painter->setFont(_font);
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      painter->drawText(QPointF(0, 0), s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FSymbol::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("font",     _font.family());
      xml.tag("fontsize", _font.pixelSize());
      xml.tag("code",     _code);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FSymbol::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "font")
                  _font.setFamily(val);
            else if (tag == "fontsize")
                  _font.setPixelSize(val.toInt());
            else if (tag == "code")
                  _code = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setPos(QPointF());
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FSymbol::layout()
      {
      QString s;
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      QFontMetricsF fm(_font);
      setbbox(fm.boundingRect(s));
      adjustReadPos();
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void FSymbol::setFont(const QFont& f)
      {
      _font = f;
      _font.setStyleStrategy(QFont::NoFontMerging);
      }

