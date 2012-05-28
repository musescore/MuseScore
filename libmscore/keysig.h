//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: keysig.h 5149 2011-12-29 08:38:43Z wschweer $
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

class Sym;
class Segment;
class QPainter;


//---------------------------------------------------------
//   KeySym
//    position of one symbol in KeySig
//---------------------------------------------------------

struct KeySym {
      int sym;
      QPointF spos;
      QPointF pos;
      };

//---------------------------------------------------------
//   KeySig
///   The KeySig class represents a Key Signature on a staff
//---------------------------------------------------------

class KeySig : public Element {
      Q_OBJECT

	bool	_showCourtesySig;
	bool	_showNaturals;
      QList<KeySym*> keySymbols;
      KeySigEvent _sig;
      void addLayout(int sym, qreal x, int y);

   public:
      KeySig(Score*);
      KeySig(const KeySig&);
      virtual KeySig* clone() const { return new KeySig(*this); }
      virtual void draw(QPainter*) const;
      virtual ElementType type() const { return KEYSIG; }
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();

      void setSig(int oldSig, int newSig);
      void setOldSig(int oldSig);

      Segment* segment() const            { return (Segment*)parent(); }
      Measure* measure() const            { return (Measure*)parent()->parent(); }
      Space space() const;
      void setCustom(const QList<KeySym*>& symbols);
      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);
      int keySignature() const            { return _sig.accidentalType(); }    // -7 - +7
      int customType() const              { return _sig.customType(); }
      bool isCustom() const               { return _sig.custom(); }
      KeySigEvent keySigEvent() const     { return _sig; }
      bool operator==(const KeySig&) const;
      void changeKeySigEvent(const KeySigEvent&);
      void setKeySigEvent(const KeySigEvent& e)      { _sig = e; }
      int tick() const;

      bool showCourtesySig() const        { return _showCourtesySig; };
      bool showNaturals() const           { return _showNaturals;    };
      void setShowCourtesySig(bool v)     { _showCourtesySig = v;    };
	void setShowNaturals(bool v)        { _showNaturals = v;       };
	};

extern const char* keyNames[15];

#endif

