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

Symbol::Symbol(Score* s, ElementFlags f)
   : BSymbol(s, f)
      {
      _sym = SymId::accidentalSharp;        // arbitrary valid default
      }

Symbol::Symbol(const Symbol& s)
   : BSymbol(s)
      {
      _sym       = s._sym;
      _scoreFont = s._scoreFont;
      }

//---------------------------------------------------------
//   symName
//---------------------------------------------------------

QString Symbol::symName() const
      {
      return Sym::id2name(_sym);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Symbol::layout()
      {
      // foreach(Element* e, leafs())     done in BSymbol::layout() ?
      //      e->layout();
      setbbox(_scoreFont ? _scoreFont->bbox(_sym, magS()) : symBbox(_sym));
      QPointF o(offset());
      qreal w = width();
      QPointF p;
      if (align() & Align::BOTTOM)
            p.setY(- height());
      else if (align() & Align::VCENTER)
            p.setY((- height()) * .5);
      else if (align() & Align::BASELINE)
            p.setY(-baseLine());
      if (align() & Align::RIGHT)
            p.setX(-w);
      else if (align() & Align::HCENTER)
            p.setX(-(w * .5));
      setPos(p + o);
      BSymbol::layout();
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(QPainter* p) const
      {
      if (type() != ElementType::NOTEDOT || !staff()->isTabStaff(tick())) {
            p->setPen(curColor());
            if (_scoreFont)
                  _scoreFont->draw(_sym, p, magS(), QPointF());
            else
                  drawSymbol(_sym, p);
            }
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(XmlWriter& xml) const
      {
      xml.stag(this);
      xml.tag("name", Sym::id2name(_sym));
      if (_scoreFont)
            xml.tag("font", _scoreFont->name());
      BSymbol::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(XmlReader& e)
      {
      QPointF pos;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  QString val(e.readElementText());
                  SymId symId = Sym::name2id(val);
                  if (val != "noSym") {
                        if (symId == SymId::noSym) {
                              // if symbol name not found, fall back to user names
                              // TODO : does it make sense? user names are probably localized
                              symId = Sym::userName2id(val);
                              if (symId == SymId::noSym) {
                                    qDebug("unknown symbol <%s>, falling back to no symbol", qPrintable(val));
                                    // set a default symbol, or layout() will crash
                                    symId = SymId::noSym;
                                    }
                              }
                        }
                  setSym(symId);
                  }
            else if (tag == "font")
                  _scoreFont = ScoreFont::fontFactory(e.readElementText());
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  if (MScore::noImages)
                        e.skipCurrentElement();
                  else {
                        Image* image = new Image(score());
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "small" || tag == "subtype")    // obsolete
                  e.skipCurrentElement();
            else if (!BSymbol::readProperties(e))
                  e.unknown();
            }
      setPos(pos);
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
      QFont f(_font);
      f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
      painter->setFont(f);
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      painter->setPen(curColor());
      painter->drawText(QPointF(0, 0), s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FSymbol::write(XmlWriter& xml) const
      {
      xml.stag(this);
      xml.tag("font",     _font.family());
      xml.tag("fontsize", _font.pointSizeF());
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
                  _font.setPointSizeF(e.readDouble());
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
      QFontMetricsF fm(_font, MScore::paintDevice());
      setbbox(fm.boundingRect(s));
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

