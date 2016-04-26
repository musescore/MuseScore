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

class QPainter;

namespace Ms {

class Chord;

enum class ArpeggioType : char {
      NORMAL, UP, DOWN, BRACKET, UP_STRAIGHT, DOWN_STRAIGHT
      };

//---------------------------------------------------------
//   @@ Arpeggio
//---------------------------------------------------------

class Arpeggio : public Element {
      Q_OBJECT

      ArpeggioType _arpeggioType;
      qreal _userLen1;
      qreal _userLen2;
      qreal _height;
      int _span;              // spanning staves
      std::vector<SymId> symbols;
      bool _playArpeggio;

      void symbolLine(SymId start, SymId fill);
      void symbolLine2(SymId end, SymId fill);

      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;
      virtual QLineF dragAnchor() const override;
      virtual QPointF gripAnchor(Grip) const override;
      virtual void startEdit(MuseScoreView*, const QPointF&) override;

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const override      { return new Arpeggio(*this); }
      virtual Element::Type type() const override   { return Element::Type::ARPEGGIO; }

      ArpeggioType arpeggioType() const    { return _arpeggioType; }
      void setArpeggioType(ArpeggioType v) { _arpeggioType = v;    }

      Chord* chord() const                 { return (Chord*)parent(); }

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual bool isEditable() const override { return true; }
      virtual void editDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }
      virtual bool edit(MuseScoreView*, Grip, int key, Qt::KeyboardModifiers, const QString&) override;

      virtual void read(XmlReader& e) override;
      virtual void write(Xml& xml) const override;

      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }
      void setHeight(qreal);

      qreal userLen1() const    { return _userLen1; }
      qreal userLen2() const    { return _userLen2; }
      void setUserLen1(qreal v) { _userLen1 = v; }
      void setUserLen2(qreal v) { _userLen2 = v; }

      bool playArpeggio()       { return _playArpeggio; }
      void setPlayArpeggio(bool p) { _playArpeggio = p; }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;
      };


}     // namespace Ms
#endif

