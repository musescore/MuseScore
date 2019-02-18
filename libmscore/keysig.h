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

namespace Ms {

class Sym;
class Segment;

//---------------------------------------------------------------------------------------
//   @@ KeySig
///    The KeySig class represents a Key Signature on a staff
//
//   @P showCourtesy  bool  show courtesy key signature for this sig if appropriate
//---------------------------------------------------------------------------------------

class KeySig final : public Element {
      bool _showCourtesy;
      bool _hideNaturals;     // used in layout to override score style (needed for the Continuous panel)
      KeySigEvent _sig;
      void addLayout(SymId sym, qreal x, int y);

   public:
      KeySig(Score* = 0);
      KeySig(const KeySig&);
      virtual KeySig* clone() const override       { return new KeySig(*this); }
      virtual void draw(QPainter*) const override;
      virtual ElementType type() const override    { return ElementType::KEYSIG; }
      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;
      virtual Shape shape() const override;
      virtual qreal mag() const override;

      //@ sets the key of the key signature
      Q_INVOKABLE void setKey(Key);

      Segment* segment() const            { return (Segment*)parent(); }
      Measure* measure() const            { return parent() ? (Measure*)parent()->parent() : nullptr; }
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;
      //@ returns the key of the key signature (from -7 (flats) to +7 (sharps) )
      Q_INVOKABLE Key key() const         { return _sig.key(); }
      bool isCustom() const               { return _sig.custom(); }
      bool isAtonal() const               { return _sig.isAtonal(); }
      KeySigEvent keySigEvent() const     { return _sig; }
      bool operator==(const KeySig&) const;
      void changeKeySigEvent(const KeySigEvent&);
      void setKeySigEvent(const KeySigEvent& e)      { _sig = e; }

      bool showCourtesy() const           { return _showCourtesy; }
      void setShowCourtesy(bool v)        { _showCourtesy = v;    }
      void undoSetShowCourtesy(bool v);

      void setHideNaturals(bool hide)     { _hideNaturals = hide; }

      QVariant getProperty(Pid propertyId) const;
      bool setProperty(Pid propertyId, const QVariant&);
      QVariant propertyDefault(Pid id) const;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      virtual QString accessibleInfo() const override;

      SymId convertFromOldId(int val) const;
      };

extern const char* keyNames[];

}     // namespace Ms
#endif

