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

#ifndef __TREMOLO_H__
#define __TREMOLO_H__

#include "symbol.h"

class QPainter;

namespace Ms {

class Chord;

// Tremolo subtypes:
enum class TremoloType : char {
      OLD_R8 = 0,
      OLD_R16,
      OLD_R32,
      OLD_C8,
      OLD_C16,
      OLD_C32,

      R8=6, R16, R32, R64,  // one note tremolo (repeat)
      C8, C16, C32, C64     // two note tremolo (change)
      };

//---------------------------------------------------------
//   @@ Tremolo
//---------------------------------------------------------

class Tremolo : public Element {
      Q_OBJECT

      TremoloType _tremoloType;
      Chord* _chord1;
      Chord* _chord2;
      QPainterPath path;

      int _lines;       // derived from _subtype

   public:
      Tremolo(Score*);
      Tremolo(const Tremolo&);
      Tremolo &operator=(const Tremolo&) = delete;
      virtual Tremolo* clone() const     { return new Tremolo(*this); }
      virtual Element::Type type() const { return Element::Type::TREMOLO; }
      virtual int subtype() const         { return (int) _tremoloType; }
      virtual QString subtypeName() const;

      QString tremoloTypeName() const;
      void setTremoloType(const QString& s);

      Chord* chord() const { return (Chord*)parent(); }

      void setTremoloType(TremoloType t);
      TremoloType tremoloType() const      { return _tremoloType; }

      virtual qreal mag() const;
      virtual void draw(QPainter*) const;
      virtual void layout();
      void layout2();
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

      Chord* chord1() const { return _chord1; }
      Chord* chord2() const { return _chord2; }

      void setChords(Chord* c1, Chord* c2) {
            _chord1 = c1;
            _chord2 = c2;
            }
      Fraction tremoloLen() const;
      bool twoNotes() const { return tremoloType() > TremoloType::R64; } // is it a two note tremolo?
      int lines() const { return _lines; }

      virtual QString accessibleInfo() const override;
      };


}     // namespace Ms
#endif

