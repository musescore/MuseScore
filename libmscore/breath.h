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

#ifndef __BREATH_H__
#define __BREATH_H__

#include "element.h"

class QPainter;

namespace Ms {

enum class SymId;

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath : public Element {
      Q_OBJECT

      int _breathType;
      static const int breathSymbols = 4;
      static SymId symList[breathSymbols];

   public:
      Breath(Score* s);
      virtual Element::Type type() const override { return Element::Type::BREATH; }
      virtual Breath* clone() const override      { return new Breath(*this); }

      int breathType() const           { return _breathType; }
      void setBreathType(int v)        { _breathType = v; }

      Segment* segment() const         { return (Segment*)parent(); }
      virtual Space space() const override;

      virtual void draw(QPainter*) const override;
      virtual void layout() override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;
      virtual QPointF pagePos() const override;      ///< position in page coordinates

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual QString accessibleInfo() override;
      };


}     // namespace Ms
#endif

