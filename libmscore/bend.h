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

   public:
      Bend(Score* s);
      virtual Bend* clone() const        { return new Bend(*this); }
      virtual Element::Type type() const { return Element::Type::BEND; }
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void write(Xml&) const;
      virtual void read(XmlReader& e);
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      };


}     // namespace Ms
#endif

