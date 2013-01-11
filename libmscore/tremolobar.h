//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TREMOLOBAR_H__
#define __TREMOLOBAR_H__

#include "element.h"
#include "pitchvalue.h"

class QPainter;

//---------------------------------------------------------
//   @@ TremoloBar
//---------------------------------------------------------

class TremoloBar : public Element {
      Q_OBJECT

      QList<PitchValue> _points;
      qreal _lw;
      QPointF notePos;
      qreal noteWidth;

   public:
      TremoloBar(Score* s);
      virtual TremoloBar* clone() const { return new TremoloBar(*this); }
      virtual ElementType type() const { return TREMOLOBAR; }
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void write(Xml&) const;
      virtual void read(XmlReader& e);
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      };

#endif

