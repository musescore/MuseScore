//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: symbol.h 5253 2012-01-25 20:40:25Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "bsymbol.h"

class Segment;
class QPainter;

//---------------------------------------------------------
//   Symbol
//    score symbol
//---------------------------------------------------------

class Symbol : public BSymbol {

   protected:
      int _sym;

   public:
      Symbol(Score* s);
      Symbol(Score* s, int sy);
      Symbol(const Symbol&);

      Symbol &operator=(const Symbol&);

      virtual Symbol* clone() const     { return new Symbol(*this); }
      virtual ElementType type() const  { return SYMBOL; }

      void setSym(int s) { _sym  = s;    }
      int sym() const    { return _sym;  }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();
      void setAbove(bool);

      virtual qreal baseLine() const { return 0.0; }
      Segment* segment() const       { return (Segment*)parent(); }
      };

//---------------------------------------------------------
//   FSymbol
//    score symbol
//---------------------------------------------------------

class FSymbol : public Element {
      QFont _font;
      int _code;

   public:
      FSymbol(Score* s);
      FSymbol(const FSymbol&);

      virtual FSymbol* clone() const    { return new FSymbol(*this); }
      virtual ElementType type() const  { return FSYMBOL; }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      virtual void layout();

      virtual qreal baseLine() const { return 0.0; }
      Segment* segment() const       { return (Segment*)parent(); }
      QFont font() const           { return _font; }
      int code() const             { return _code; }
      void setFont(const QFont& f);
      void setCode(int val)        { _code = val; }
      };

#endif

