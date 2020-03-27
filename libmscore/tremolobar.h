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

#ifndef __TREMOLOBAR_H__
#define __TREMOLOBAR_H__

#include "element.h"
#include "pitchvalue.h"

namespace Ms {

//---------------------------------------------------------
//   @@ TremoloBar
//
//   @P userMag    qreal
//   @P lineWidth  qreal
//   @P play       bool         play tremolo bar
//---------------------------------------------------------

class TremoloBar final : public Element {
      Spatium _lw;
      qreal _userMag     { 1.0   };       // allowed 0.1 - 10.0
      bool  _play        { true  };

      QList<PitchValue> _points;

      QPolygonF polygon;                  // computed by layout

   public:
      TremoloBar(Score* s);

      TremoloBar* clone() const override  { return new TremoloBar(*this); }
      ElementType type() const override   { return ElementType::TREMOLOBAR; }

      void layout() override;
      void draw(QPainter*) const override;

      void write(XmlWriter&) const override;
      void read(XmlReader& e) override;

      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      qreal userMag() const               { return _userMag;   }
      void setUserMag(qreal m)            { _userMag = m;      }

      void setLineWidth(Spatium v)        { _lw = v;        }
      Spatium lineWidth() const           { return _lw;     }

      bool play() const                   { return _play;    }
      void setPlay(bool val)              { _play = val;     }
      };


}     // namespace Ms
#endif

