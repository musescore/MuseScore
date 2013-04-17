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

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "bsymbol.h"
#include "sym.h"

class Segment;
class QPainter;

//---------------------------------------------------------
//   @@ Symbol
///   Symbol constructed from builtin symbol.
//---------------------------------------------------------

class Symbol : public BSymbol {
      Q_OBJECT

   protected:
      SymId _sym;

   public:
      Symbol(Score* s);
      Symbol(Score* s, SymId sy);
      Symbol(const Symbol&);

      Symbol &operator=(const Symbol&);

      virtual Symbol* clone() const     { return new Symbol(*this); }
      virtual ElementType type() const  { return SYMBOL; }

      void setSym(SymId s) { _sym  = s;    }
      SymId sym() const    { return _sym;  }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual void layout();
      void setAbove(bool);

      virtual qreal baseLine() const { return 0.0; }
      Segment* segment() const       { return (Segment*)parent(); }
      };

//---------------------------------------------------------
//   @@ FSymbol
///   Symbol constructed from a font glyph.
//---------------------------------------------------------

class FSymbol : public BSymbol {
      Q_OBJECT

      QFont _font;
      int _code;

   public:
      FSymbol(Score* s);
      FSymbol(const FSymbol&);

      virtual FSymbol* clone() const    { return new FSymbol(*this); }
      virtual ElementType type() const  { return FSYMBOL; }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual void layout();

      virtual qreal baseLine() const { return 0.0; }
      Segment* segment() const       { return (Segment*)parent(); }
      QFont font() const             { return _font; }
      int code() const               { return _code; }
      void setFont(const QFont& f);
      void setCode(int val)          { _code = val; }
      };

#endif

