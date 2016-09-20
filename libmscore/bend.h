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

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   @@ Bend
//---------------------------------------------------------

class Bend : public Element {
      Q_OBJECT

      QList<PitchValue> _points;
      qreal _lw;
      QPointF notePos;
      qreal noteWidth;
      bool _playBend;

   public:
      Bend(Score* s);
      virtual Bend* clone() const override        { return new Bend(*this); }
      virtual Element::Type type() const override { return Element::Type::BEND; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader& e) override;
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      bool playBend() const                      { return _playBend; }
      void setPlayBend(bool v)                   { _playBend = v;    }

      // property methods
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms
#endif

