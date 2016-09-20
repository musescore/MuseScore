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

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   @@ TremoloBar
//
//   @P userMag    qreal
//   @P lineWidth  qreal
//   @P play       bool         play tremolo bar
//---------------------------------------------------------

class TremoloBar : public Element {
      Q_OBJECT

      Q_PROPERTY(qreal   userMag   READ userMag    WRITE undoSetUserMag)
      Q_PROPERTY(Spatium lineWidth READ lineWidth  WRITE undoSetLineWidth)
      Q_PROPERTY(bool    play      READ play       WRITE undoSetPlay)

      Spatium _lw;
      PropertyStyle lineWidthStyle;
      qreal _userMag     { 1.0   };       // allowed 0.1 - 10.0
      bool  _play        { true  };

      QList<PitchValue> _points;

      QPolygonF polygon;                  // computed by layout

   public:
      TremoloBar(Score* s);
      virtual TremoloBar* clone() const override  { return new TremoloBar(*this); }
      virtual Element::Type type() const override { return Element::Type::TREMOLOBAR; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual void write(Xml&) const override;
      virtual void read(XmlReader& e) override;

      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID id) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      virtual void reset() override;
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;

      qreal userMag() const               { return _userMag;   }
      void setUserMag(qreal m)            { _userMag = m;      }
      void undoSetUserMag(qreal val);

      void setLineWidth(Spatium v)        { _lw = v;        }
      Spatium lineWidth() const           { return _lw;     }
      void undoSetLineWidth(Spatium);

      bool play() const                   { return _play;    }
      void setPlay(bool val)              { _play = val;     }
      void undoSetPlay(bool);
      };


}     // namespace Ms
#endif

