//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BEND_H__
#define __BEND_H__

#include "element.h"
#include "pitchvalue.h"
#include "property.h"
#include "style.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Bend
//---------------------------------------------------------

class Bend final : public Element {
      M_PROPERTY(QString,   fontFace,  setFontFace)
      M_PROPERTY(qreal,     fontSize,  setFontSize)
      M_PROPERTY(FontStyle, fontStyle, setFontStyle)
      M_PROPERTY(Spatium,   lineWidth, setLineWidth)

      bool _playBend     { true };
      QList<PitchValue> _points;

      QPointF notePos;
      qreal noteWidth;

      QFont font(qreal) const;

   public:
      Bend(Score* s);
      virtual Bend* clone() const override        { return new Bend(*this); }
      virtual ElementType type() const override   { return ElementType::BEND; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader& e) override;
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      bool playBend() const                      { return _playBend; }
      void setPlayBend(bool v)                   { _playBend = v;    }

      // property methods
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };


}     // namespace Ms
#endif

