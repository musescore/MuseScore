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

class QPainter;

namespace Ms {

class Segment;
class ScoreFont;
enum class SymId;

//---------------------------------------------------------
//   @@ Symbol
///    Symbol constructed from builtin symbol.
//---------------------------------------------------------

class Symbol : public BSymbol {
      Q_OBJECT

   protected:
      SymId _sym;
      const ScoreFont* _scoreFont = nullptr;

   public:
      Symbol(Score* s);
      Symbol(const Symbol&);

      Symbol &operator=(const Symbol&) = delete;

      virtual Symbol* clone() const      { return new Symbol(*this); }
      virtual Element::Type type() const { return Element::Type::SYMBOL; }

      void setSym(SymId s, const ScoreFont* sf = nullptr) { _sym  = s; _scoreFont = sf;    }
      SymId sym() const                  { return _sym;  }

      virtual void draw(QPainter*) const override;
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      void setAbove(bool);

      virtual qreal baseLine() const     { return 0.0; }
      Segment* segment() const           { return (Segment*)parent(); }
      };

//---------------------------------------------------------
//   @@ FSymbol
///    Symbol constructed from a font glyph.
//---------------------------------------------------------

class FSymbol : public BSymbol {
      Q_OBJECT

      QFont _font;
      int _code;

   public:
      FSymbol(Score* s);
      FSymbol(const FSymbol&);

      virtual FSymbol* clone() const    { return new FSymbol(*this); }
      virtual Element::Type type() const  { return Element::Type::FSYMBOL; }

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

}     // namespace Ms
#endif

