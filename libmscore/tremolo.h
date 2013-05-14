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
enum TremoloType {
      OLD_TREMOLO_R8 = 0,
      OLD_TREMOLO_R16,
      OLD_TREMOLO_R32,
      OLD_TREMOLO_C8,
      OLD_TREMOLO_C16,
      OLD_TREMOLO_C32,

      TREMOLO_R8=6, TREMOLO_R16, TREMOLO_R32, TREMOLO_R64,    // one note tremolo (repeat)
      TREMOLO_C8, TREMOLO_C16, TREMOLO_C32, TREMOLO_C64     // two note tremolo (change)
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
      Tremolo &operator=(const Tremolo&);
      virtual Tremolo* clone() const   { return new Tremolo(*this); }
      virtual ElementType type() const { return TREMOLO; }

      QString tremoloTypeName() const;
      void setTremoloType(const QString& s);

      void setTremoloType(TremoloType t);
      TremoloType tremoloType() const      { return _tremoloType; }

      virtual void draw(QPainter*) const;
      virtual void layout();
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

      Chord* chord1() const { return _chord1; }
      Chord* chord2() const { return _chord2; }

      void setChords(Chord* c1, Chord* c2) {
            _chord1 = c1;
            _chord2 = c2;
            }
      Fraction tremoloLen() const;
      bool twoNotes() const { return tremoloType() > TREMOLO_R64; } // is it a two note tremolo?
      int lines() const { return _lines; }
      };


}     // namespace Ms
#endif

