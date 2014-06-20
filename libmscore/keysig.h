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

#ifndef __KEYSIG_H__
#define __KEYSIG_H__

#include "key.h"
#include "element.h"

class QPainter;

namespace Ms {

class Sym;
class Segment;
enum class SymId : short;

//---------------------------------------------------------
//   KeySym
//    position of one symbol in KeySig
//---------------------------------------------------------

struct KeySym {
      SymId sym;
      QPointF spos;
      QPointF pos;
      };

//---------------------------------------------------------------------------------------
//   @@ KeySig
///    The KeySig class represents a Key Signature on a staff
//
//   @P showCourtesy  bool  show courtesy key signature for this sig if appropriate
//---------------------------------------------------------------------------------------

class KeySig : public Element {
      Q_OBJECT
      Q_PROPERTY(bool showCourtesy READ showCourtesy   WRITE undoSetShowCourtesy)

      bool _showCourtesy;
      QList<KeySym*> keySymbols;
      KeySigEvent _sig;
      void addLayout(SymId sym, qreal x, int y);

   public:
      KeySig(Score* = 0);
      KeySig(const KeySig&);
      virtual KeySig* clone() const { return new KeySig(*this); }
      virtual void draw(QPainter*) const;
      virtual ElementType type() const { return ElementType::KEYSIG; }
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual qreal mag() const;

      Q_INVOKABLE void setKey(Key);

      Segment* segment() const            { return (Segment*)parent(); }
      Measure* measure() const            { return parent() ? (Measure*)parent()->parent() : nullptr; }
      Space space() const;
      void setCustom(const QList<KeySym*>& symbols);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      Q_INVOKABLE Key key() const         { return _sig.key(); }    //@ -7 (flats) -- +7 (sharps)
      int customType() const              { return _sig.customType(); }
      bool isCustom() const               { return _sig.custom(); }
      KeySigEvent keySigEvent() const     { return _sig; }
      bool operator==(const KeySig&) const;
      void changeKeySigEvent(const KeySigEvent&);
      void setKeySigEvent(const KeySigEvent& e)      { _sig = e; }
      int tick() const;

      bool showCourtesy() const           { return _showCourtesy; }
      void setShowCourtesy(bool v)        { _showCourtesy = v;    }
      void undoSetShowCourtesy(bool v);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

extern const char* keyNames[];


}     // namespace Ms
#endif

