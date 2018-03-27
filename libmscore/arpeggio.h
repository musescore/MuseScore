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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "element.h"

namespace Ms {

class Chord;

enum class ArpeggioType : char {
      NORMAL, UP, DOWN, BRACKET, UP_STRAIGHT, DOWN_STRAIGHT
      };

//---------------------------------------------------------
//   @@ Arpeggio
//---------------------------------------------------------

class Arpeggio final : public Element {
      ArpeggioType _arpeggioType;
      qreal _userLen1;
      qreal _userLen2;
      qreal _height;
      int _span;              // spanning staves
      std::vector<SymId> symbols;
      bool _playArpeggio;

      bool _hidden = false; // set in layout, will skip draw if true

      void symbolLine(SymId start, SymId fill);
      void symbolLine2(SymId end, SymId fill);

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;
      virtual QLineF dragAnchor() const override;
      virtual QPointF gripAnchor(Grip) const override;
      virtual void startEdit(EditData&) override;

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const override    { return new Arpeggio(*this); }
      virtual ElementType type() const override   { return ElementType::ARPEGGIO; }

      ArpeggioType arpeggioType() const    { return _arpeggioType; }
      void setArpeggioType(ArpeggioType v) { _arpeggioType = v;    }

      Chord* chord() const                 { return (Chord*)parent(); }

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override { return true; }
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;
      virtual bool edit(EditData&) override;

      virtual void read(XmlReader& e) override;
      virtual void write(XmlWriter& xml) const override;

      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }
      void setHeight(qreal);

      qreal userLen1() const    { return _userLen1; }
      qreal userLen2() const    { return _userLen2; }
      void setUserLen1(qreal v) { _userLen1 = v; }
      void setUserLen2(qreal v) { _userLen2 = v; }

      bool playArpeggio()       { return _playArpeggio; }
      void setPlayArpeggio(bool p) { _playArpeggio = p; }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid propertyId) const override;
      };


}     // namespace Ms
#endif

