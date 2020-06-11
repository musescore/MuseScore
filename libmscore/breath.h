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
#include "sym.h"

namespace Ms {

//---------------------------------------------------------
//   BreathType
//---------------------------------------------------------

struct BreathType {
      SymId id;
      bool isCaesura;
      qreal pause;
      };

//---------------------------------------------------------
//   @@ Breath
//!    breathType() is index in symList
//---------------------------------------------------------

class Breath final : public Element {
      qreal _pause;
      SymId _symId;

   public:
      Breath(Score* s);

      ElementType type() const override { return ElementType::BREATH; }
      Breath* clone() const override    { return new Breath(*this); }

      qreal mag() const override;

      void setSymId(SymId id)          { _symId = id; }
      SymId symId() const              { return _symId; }
      qreal pause() const              { return _pause; }
      void setPause(qreal v)           { _pause = v; }

      Segment* segment() const         { return (Segment*)parent(); }

      void draw(QPainter*) const override;
      void layout() override;
      void write(XmlWriter&) const override;
      void read(XmlReader&) override;
      QPointF pagePos() const override;      ///< position in page coordinates

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      Element* nextSegmentElement() override;
      Element* prevSegmentElement() override;
      QString accessibleInfo() const override;

      bool isCaesura() const;

      static const std::vector<BreathType> breathList;
      };


}     // namespace Ms
#endif

