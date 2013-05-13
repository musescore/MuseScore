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

#include "symbol.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "page.h"
#include "score.h"
#include "image.h"

namespace Ms {

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
      BSymbol::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(XmlReader& e)
      {
      QPointF pos;
      SymId s = noSym;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  QString val(e.readElementText());
                  if (val == "acc dot")               // compatibility hack
                        val = "accordion.accDot";
                  else if (val == "acc old ee")
                        val = "accordion.accOldEE";
                  s = Sym::name2id(val);
                  if (s == noSym) {
                        // if symbol name not found, fall back to mnames
                        s = Sym::userName2id(val);
                        if (s == noSym) {
                              qDebug("unknown symbol <%s> (%d symbols), falling back to default symbol",
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
                  Image* image = new Image(score());
                  QString path;
                  image->read(e);
                  add(image);
                  }
            else if (tag == "small" || tag == "subtype")
                  ;
            else if (!BSymbol::readProperties(e))
                  e.unknown();
            }
      if (s == noSym)
            qDebug("unknown symbol");
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
  : BSymbol(s)
      {
      _code = 0;
      _font.setStyleStrategy(QFont::NoFontMerging);
      }

FSymbol::FSymbol(const FSymbol& s)
  : BSymbol(s)
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
      BSymbol::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FSymbol::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "font")
                  _font.setFamily(e.readElementText());
            else if (tag == "fontsize")
                  _font.setPixelSize(e.readInt());
            else if (tag == "code")
                  _code = e.readInt();
            else if (!BSymbol::readProperties(e))
                  e.unknown();
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

}

